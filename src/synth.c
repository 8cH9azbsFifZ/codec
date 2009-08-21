/*---------------------------------------------------------------------------*\
                                                                             
  FILE........: synth.c                                           
  AUTHOR......: David Rowe                                             
  DATE CREATED: 20/2/95                                                 
                                                                             
  Function for synthesising a speech signal in the frequency domain from      
  the sinusodal model parameters.                                             
                                                                             
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

void synthesise_mixed(Pn,model,Sn_)
float Pn[];		/* time domain Parzen window */
MODEL *model;		/* ptr to model parameters for this frame */
float Sn_[];		/* time domain synthesised signal */
{
  int i,l,j,b;	        /* loop variables */
  COMP Nw[FFT_DEC];	/* DFT of noise signal */
  COMP Sw_[FFT_DEC];	/* DFT of synthesised signal */

  /* Update memories */

  for(i=0; i<N-1; i++) {
    Sn_[i] = Sn_[i+N];
  }
  Sn_[N-1] = 0.0;

  for(i=0; i<FFT_DEC; i++) {
    Sw_[i].real = 0.0;
    Sw_[i].imag = 0.0;
  }

  /* Now set up frequency domain synthesised speech */

  for(l=1; l<=model->L; l++) {
    b = floor(l*model->Wo*FFT_DEC/TWO_PI + 0.5);
    Sw_[b].real = model->A[l]*cos(model->phi[l]);
    Sw_[b].imag = model->A[l]*sin(model->phi[l]);
    Sw_[FFT_DEC-b].real = Sw_[b].real;
    Sw_[FFT_DEC-b].imag = -Sw_[b].imag;
  }

  /* Perform inverse DFT */

  four1(&Sw_[-1].imag,FFT_DEC,1);

  /* Overlap add to previous samples */

  for(i=0; i<N-1; i++) {
    Sn_[i] += Sw_[FFT_DEC-N+1+i].real*Pn[i];
  }
  for(i=N-1,j=0; i<2*N; i++,j++)
    Sn_[i] = Sw_[j].real*Pn[i];
}

