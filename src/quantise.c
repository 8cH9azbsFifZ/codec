/*---------------------------------------------------------------------------*\
                                                                             
  FILE........: quantise.c
  AUTHOR......: David Rowe                                                          
  DATE CREATED: 31/5/92                                                       
                                                                             
  Quantisation functions for the sinusoidal coder.  
                                                                             
\*---------------------------------------------------------------------------*/

/*
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
#include "quantise.h"
#include "lpc.h"
#include "dump.h"

#define MAX_ORDER 20

/*---------------------------------------------------------------------------*\
									      
  lpc_model_amplitudes

  Derive a LPC model for amplitude samples then estimate amplitude samples
  from this model with optional LSP quantisation.

  Returns the spectral distortion for this frame.

\*---------------------------------------------------------------------------*/

float lpc_model_amplitudes(
  float  Sn[],			/* Input frame of speech samples */
  MODEL *model,			/* sinusoidal model parameters */
  int    order,                 /* LPC model order */
  int    lsp                    /* optional LSP quantisation if non-zero */
)
{
  float Wn[AW_ENC];
  float R[MAX_ORDER+1];
  float ak[MAX_ORDER+1];
  float E;
  int   i;
  float sd;			/* spectral distortion for this frame */

  for(i=0; i<AW_ENC; i++)
      Wn[i] = Sn[i]*w[i];
  autocorrelate(Wn,R,AW_ENC,order);
  levinson_durbin(R,ak,order);
  E = 0.0;
  for(i=0; i<=order; i++)
      E += ak[i]*R[i];
  
  aks_to_M2(ak,order,model,E,&sd);   /* {ak} -> {Am} LPC decode */

  return sd;
}

/*---------------------------------------------------------------------------*\
                                                                         
   aks_to_M2()                                                             
                                                                         
   Transforms the linear prediction coefficients to spectral amplitude    
   samples.  This function determines A(m) from the average energy per    
   band using an FFT.                                                     
                                                                        
\*---------------------------------------------------------------------------*/

void aks_to_M2(
  float ak[],	/* LPC's */
  int   order,
  MODEL *model,	/* sinusoidal model parameters for this frame */
  float E,	/* energy term */
  float *sd	/* spectral distortion for this frame in dB */
)
{
  COMP Pw[FFT_DEC];	/* power spectrum */
  int i,m;		/* loop variables */
  int am,bm;		/* limits of current band */
  float r;		/* no. rads/bin */
  float Em;		/* energy in band */
  float Am;		/* spectral amplitude sample */
  float noise;

  r = TWO_PI/(FFT_DEC);

  /* Determine DFT of A(exp(jw)) --------------------------------------------*/

  for(i=0; i<FFT_DEC; i++) {
    Pw[i].real = 0.0;
    Pw[i].imag = 0.0;
  }

  for(i=0; i<=order; i++)
    Pw[i].real = ak[i];
  four1(&Pw[-1].imag,FFT_DEC,1);

  /* Determine power spectrum P(w) = E/(A(exp(jw))^2 ------------------------*/

  for(i=0; i<FFT_DEC/2; i++)
    Pw[i].real = E/(Pw[i].real*Pw[i].real + Pw[i].imag*Pw[i].imag);
  dump_Pw(Pw);

  /* Determine magnitudes by linear interpolation of P(w) -------------------*/

  noise = 0.0;
  for(m=1; m<=model->L; m++) {
    am = floor((m - 0.5)*model->Wo/r + 0.5);
    bm = floor((m + 0.5)*model->Wo/r + 0.5);
    Em = 0.0;

    for(i=am; i<bm; i++)
      Em += Pw[i].real;
    Am = sqrt(Em);

    noise += pow(log10(Am/model->A[m]),2.0);
    model->A[m] = Am;
  }
  *sd = 20.0*sqrt(noise/model->L);

}
