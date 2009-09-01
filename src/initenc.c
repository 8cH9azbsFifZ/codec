/*---------------------------------------------------------------------------*\
                                                                  
  FILE: initenc.c
  AUTHOR: David Rowe                               
  DATE CREATED: 11/5/94            
                                        
  Initialises sinusoidal speech encoder.     
                                                          
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

#include "sine.h"	/* sinusoidal header file */

void init_encoder()
{
  int i;

  frames = 0;

  /* Initialise sample buffer memories */

  for(i=0; i<M+AW_ENC/2; i++)
    Sn[i] = 1.0;

}

float make_window(int Nw)
{
  float m;
  COMP  temp;
  int   i,j;

  /* Generate Hamming window centered on analysis window */

  m = 0.0;
  for(i=0; i<AW_ENC/2-Nw/2; i++)
    w[i] = 0.0;
  for(i=AW_ENC/2-Nw/2,j=0; i<AW_ENC/2+Nw/2; i++,j++) {
    w[i] = 0.5 - 0.5*cos(TWO_PI*j/(Nw-1));
    m += w[i]*w[i];
  }
  for(i=AW_ENC/2+Nw/2; i<AW_ENC; i++)
    w[i] = 0.0;

  /* Normalise - make amplitude estimation straight forward */

  m = 1.0/sqrt(m*FFT_ENC);
  for(i=0; i<AW_ENC; i++) {
    w[i] *= m;
  }

  /* Generate DFT of analysis window, used for voicing estimation */

  for(i=0; i<FFT_ENC; i++) {
    W[i].real = 0.0;
    W[i].imag = 0.0;
  }
  for(i=0; i<AW_ENC/2; i++)
    W[i].real = w[i+AW_ENC/2];
  for(i=FFT_ENC-AW_ENC/2; i<FFT_ENC; i++)
    W[i].real = w[i-FFT_ENC+AW_ENC/2];

  four1(&W[-1].imag,FFT_ENC,-1);         /* "Numerical Recipes in C" FFT */

  /* re-arrange so that W is symmetrical about FFT_ENC/2 */

  for(i=0; i<FFT_ENC/2; i++) {
    temp.real = W[i].real;
    temp.imag = W[i].imag;
    W[i].real = W[i+FFT_ENC/2].real;
    W[i].imag = W[i+FFT_ENC/2].imag;
    W[i+FFT_ENC/2].real = temp.real;
    W[i+FFT_ENC/2].imag = temp.imag;
  }

  return(m);
}

