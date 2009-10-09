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

void synthesise_mixed(
  float   Pn[],		/* time domain Parzen window */
  MODEL *model,		/* ptr to model parameters for this frame */
  float  Sn_[],		/* time domain synthesised signal */
  int    shift          /* if non-zero update memories */
)
{
  int i,l,j,b;	        /* loop variables */
  COMP Sw_[FFT_DEC];	/* DFT of synthesised signal */

  if (shift) {
      /* Update memories */

      for(i=0; i<N-1; i++) {
	  Sn_[i] = Sn_[i+N];
      }
      Sn_[N-1] = 0.0;
  }

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

  if (shift)
      for(i=N-1,j=0; i<2*N; i++,j++)
	  Sn_[i] = Sw_[j].real*Pn[i];
  else
      for(i=N-1,j=0; i<2*N; i++,j++)
	  Sn_[i] += Sw_[j].real*Pn[i];

}

/*---------------------------------------------------------------------------*\

  synthesise_continuous_phase()
                                                                             
  This version adjusts the frequency of each harmonic slightly to
  ensure a continuous phase track from the previous frame.  Used with
  the zero phase model, when original phases are not available.

  At sample n=0 of this frame, we assume the phase of harmonic m is
  set to phi_prev[m].  We want the final phase at sample N to be
  phi[m].  This means the phase track must start at phi_prev[m],
  rotate several times based on mWo, then end up at phase phi[m].

  To ensure that the phase track arrives at phi[m] by sample N we add
  a small frequency offset by slightly shifting the frequency of each
  harmonic.

  The continuous phase track model is only useful for voiced speech.
  In fact, for unvoiced speech we desire a rough, discontinuous phase
  track. So in unvoiced frames or in cases where the fundamental
  frequency varies by more that 20%, we don't add the small frequency
  offset.

  Result: when tested was no difference in output speech quality.  The
  partial unvoiced sound when using zero phase model was found to be
  due mis-alignment of the LPC analysis window and accidental addition
  of a random phase component.  So we are sticking with synthesise_mixed()
  above for now.

\*---------------------------------------------------------------------------*/

void synthesise_continuous_phase(
  float  Pn[],		/* time domain Parzen window */
  MODEL *model,		/* ptr to model parameters for this frame */
  float  Sn_[],		/* time domain synthesised signal */
  int    voiced,        /* non-zero if frame is voiced */  
  float *Wo_prev,       /* previous frames Wo */
  float  phi_prev[]     /* previous frames phases */
)
{
  int   i,l,j;	        /* loop variables            */
  COMP  Sw_[FFT_DEC];	/* DFT of synthesised signal */
  int   b[MAX_AMP];     /* DFT bin of each harmonic  */
  float delta_w;        /* frequency offset required */

  /* Update memories */

  for(i=0; i<N-1; i++) {
    Sn_[i] = Sn_[i+N];
  }
  Sn_[N-1] = 0.0;

  for(i=0; i<FFT_DEC; i++) {
    Sw_[i].real = 0.0;
    Sw_[i].imag = 0.0;
  }

  if (!voiced || (fabs(*Wo_prev - model->Wo) > 0.2*model->Wo)) {
      //printf("disc voiced = %d\n", voiced);
      //printf("%f %f\n", fabs(*Wo_prev - model->Wo), 0.2*model->Wo);
      /* discontinous phase tracks: no phase adjustment of frequency
	 as we want discontinuous phase tracks */

      for(l=1; l<=model->L; l++)
	  b[l] = floor(l*model->Wo*FFT_DEC/TWO_PI + 0.5);
  }
  else {
      /* continous phase tracks: determine frequency of each harmonic
	 to ensure smooth phase track at the centre of next synthesis
	 frame */

      for(l=1; l<=model->L; l++) {
	  //printf("Wo_prev = %f  Wo = %f\n", *Wo_prev, model->Wo);
	  delta_w = (model->phi[l] - l*N*(*Wo_prev + model->Wo)/2.0 - phi_prev[l]);
	  delta_w -= TWO_PI*floor(delta_w/TWO_PI + 0.5);
          delta_w /= N;
	  b[l] = floor((l*model->Wo+delta_w)*FFT_DEC/TWO_PI + 0.5);
	  //printf("delta_w = %f b[%d] = %d\n", delta_w,l,b[l]);
      }
  }

  /* update memories for phase tracking */

  *Wo_prev = model->Wo;
  for(l=1; l<=model->L; l++)
      phi_prev[l] = model->phi[l];

  /* Now set up frequency domain synthesised speech */

  for(l=1; l<=model->L; l++) {
    Sw_[b[l]].real = model->A[l]*cos(model->phi[l]);
    Sw_[b[l]].imag = model->A[l]*sin(model->phi[l]);
    Sw_[FFT_DEC-b[l]].real = Sw_[b[l]].real;
    Sw_[FFT_DEC-b[l]].imag = -Sw_[b[l]].imag;
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

