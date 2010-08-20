/*---------------------------------------------------------------------------*\
                                                          
  FILE........: spec.c                                             
  AUTHOR......: David Rowe                                          
  DATE CREATED: 27/5/94                                         
                                                               
  Functions for estimating the sinusoidal model parameters.     
                                                                   
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

#include "defines.h"
#include "spec.h"

/*---------------------------------------------------------------------------*\
                                                                             
  FUNCTION....: estimate_amplitudes 			      
  AUTHOR......: David Rowe		
  DATE CREATED: 27/5/94			       
									      
  Estimates the complex amplitudes of the harmonics.    
									      
\*---------------------------------------------------------------------------*/

void estimate_amplitudes(MODEL *model, float Sw[])
{
  int   i,m;		/* loop variables */
  int   am,bm;		/* bounds of current harmonic */
  int   b;		/* DFT bin of centre of current harmonic */
  float den;		/* denominator of amplitude expression */
  float r;		/* number of rads/bin */
  float E;
  int   offset;
  COMP  Am;

  r = TWO_PI/FFT_ENC;
  for(i=0; i<FFT_ENC; i++) {
      Sw_[i].real = 1.0;
      Sw_[i].imag = 0.0;
  }

  for(m=1; m<=model.L; m++) {
    den = 0.0;
    am = floor((m - 0.5)*model.Wo/r + 0.5);
    bm = floor((m + 0.5)*model.Wo/r + 0.5);
    b = floor(m*model.Wo/r + 0.5);

    /* Estimate ampltude of harmonic */

    den = 0.0;
    Am.real = Am.imag = 0.0;
    for(i=am; i<bm; i++) {
      den += Sw[i].real*Sw[i].real + Sw[i].imag*Sw[i].imag;
      offset = i + FFT_ENC/2 - floor(m*model.Wo/r + 0.5);
      Am.real += Sw[i].real*W[offset].real;
      Am.imag += Sw[i].imag*W[offset].real;
    }

    model.A[m] = sqrt(den);

    /* Estimate phase of harmonic */

    model.phi[m] = atan2(Sw[b].imag,Sw[b].real);
}

