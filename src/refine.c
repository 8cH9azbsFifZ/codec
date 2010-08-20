/*---------------------------------------------------------------------------*\

  FILE........: refine.c
  AUTHOR......: David Rowe                  
  DATE CREATED: 27/5/94          
                                               
  Functions for refining the pitch estimate using the harmonic sum method.

\*---------------------------------------------------------------------------*/

/*
  Copyright (C) 2009 David Rowe

  All rights reserved.

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License version 2, as
  published by the Free Software Foundation.  This program is
  distributed in the hope that it will be useful, but WITHOUT ANY
  WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
  License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#include "sine.h"

/*---------------------------------------------------------------------------*\
                                                       
  FUNCTION....: dft_speech	     
  AUTHOR......: David Rowe			      
  DATE CREATED: 27/5/94 

  Finds the DFT of the current speech input speech frame.

\*---------------------------------------------------------------------------*/

void dft_speech(float Sn[], COMP Sw[])
{
  int i;
  
  for(i=0; i<FFT_ENC; i++) {
    Sw[i].real = 0.0;
    Sw[i].imag = 0.0;
  }

  /* Centre analysis window on time axis, we need to arrange input
     to FFT this way to make FFT phases correct */
  
  /* move 2nd half to start of FFT input vector */

  for(i=0; i<NW/2; i++)
    Sw[i].real = Sn[i+M/2]*w[i+M/2];

  /* move 1st half to end of FFT input vector */

  for(i=0; i<NW/2; i++)
    Sw[FFT_ENC-NW/2+i].real = Sn[i+M/2-NW/2]*w[i+M/2-NW/2];

  four1(&Sw[-1].imag,FFT_ENC,-1);
}

/*---------------------------------------------------------------------------*\
                                                                     
  FUNCTION....: two_stage_pitch_refinement			
  AUTHOR......: David Rowe
  DATE CREATED: 27/5/94				

  Refines the current pitch estimate using the harmonic sum pitch
  estimation technique.

\*---------------------------------------------------------------------------*/

void two_stage_pitch_refinement()
{
  float pmin,pmax,pstep;	/* pitch refinment minimum, maximum and step */ 

  /* Coarse refinement */

  pmax = TWO_PI/model.Wo + 5;
  pmin = TWO_PI/model.Wo - 5;
  pstep = 1.0;
  hs_pitch_refinement(pmin,pmax,pstep);
  
  /* Fine refinement */
  
  pmax = TWO_PI/model.Wo + 1;
  pmin = TWO_PI/model.Wo - 1;
  pstep = 0.25;
  hs_pitch_refinement(pmin,pmax,pstep);
  
  /* Limit range */
  
  if (model.Wo < TWO_PI/P_MAX)
    model.Wo = TWO_PI/P_MAX;
  if (model.Wo > TWO_PI/P_MIN)
    model.Wo = TWO_PI/P_MIN;

  model.L = floor(PI/model.Wo);
}

/*---------------------------------------------------------------------------*\
                                                                
 FUNCTION....: hs_pitch_refinement				
 AUTHOR......: David Rowe			
 DATE CREATED: 27/5/94							     
									  
 Harmonic sum pitch refinement function.			   
									    
 pmin   pitch search range minimum	    
 pmax	pitch search range maximum	    
 step   pitch search step size		    
 model	current pitch estimate in model.Wo  
									    
 model 	refined pitch estimate in model.Wo  
									     
\*---------------------------------------------------------------------------*/

void hs_pitch_refinement(float pmin, float pmax, float pstep)
{
  int m;		/* loop variable */
  int b;		/* bin for current harmonic centre */
  float E;		/* energy for current pitch*/
  float Wo;		/* current "test" fundamental freq. */
  float Wom;		/* Wo that maximises E */
  float Em;		/* mamimum energy */
  float r;		/* number of rads/bin */
  float p;		/* current pitch */
  
  /* Initialisation */
  
  model.L = PI/model.Wo;	/* use initial pitch est. for L */
  Em = 0.0;
  r = TWO_PI/FFT_ENC;
  
  /* Determine harmonic sum for a range of Wo values */

  for(p=pmin; p<=pmax; p+=pstep) {
    E = 0.0;
    Wo = TWO_PI/p;

    /* Sum harmonic magnitudes */

    for(m=1; m<=model.L; m++) {
      b = floor(m*Wo/r + 0.5);
      E += Sw[b].real*Sw[b].real + Sw[b].imag*Sw[b].imag;
    }  

    /* Compare to see if this is a maximum */
    
    if (E > Em) {
      Em = E;
      Wom = Wo;
    }
  }

  model.Wo = Wom;
}

