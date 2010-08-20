/*---------------------------------------------------------------------------*\
                                                                  
  FILE........: window.c
  AUTHOR......: David Rowe                               
  DATE CREATED: 11/5/94            
                                        
  Generates the time domain analysis window and it's DFT.
                                                          
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

#include <math.h>
#include "defines.h"
#include "window.h"

float make_window(
    float w[],   /* time domain analysis window      */
    COMP  W[]    /* w[] in frequency domain          */
)
{
  float m;
  COMP  temp;
  int   i,j;

  /* 
     Generate Hamming window centered on M-sample pitch analysis window
  
  0            M/2           M-1
  |-------------|-------------|
        |-------|-------|
            NW samples

     All our analysis/synthsis is centred on the M/2 sample.               
  */

  m = 0.0;
  for(i=0; i<M/2-NW/2; i++)
    w[i] = 0.0;
  for(i=M/2-NW/2,j=0; i<M/2+NW/2; i++,j++) {
    w[i] = 0.5 - 0.5*cos(TWO_PI*j/(NW-1));
    m += w[i]*w[i];
  }
  for(i=M/2+NW/2; i<M; i++)
    w[i] = 0.0;
 
  /* Normalise - makes freq domain amplitude estimation straight
     forward */

  m = 1.0/sqrt(m*FFT_ENC);
  for(i=0; i<M; i++) {
    w[i] *= m;
  }

  /* 
     Generate DFT of analysis window, used for later processing.  Note
     we modulo FFT_ENC shift the time domain window w[], this makes the
     imaginary part of the DFT W[] equal to zero as the shifted w[] is
     even about the n=0 time axis if NW is odd.  Having the imag part
     of the DFT W[] makes computation easier.

     0                      FFT_ENC-1
     |-------------------------|

      ----\               /----
           \             / 
            \           /          <- shifted version of window w[n]
             \         /
              \       /
               -------

     |---------|     |---------|      
       NW/2              NW/2
  */

  for(i=0; i<FFT_ENC; i++) {
    W[i].real = 0.0;
    W[i].imag = 0.0;
  }
  for(i=0; i<NW/2; i++)
    W[i].real = w[i+M/2];
  for(i=FFT_ENC-NW/2,j=M/2-NW/2; i<FFT_ENC; i++,j++)
    W[i].real = w[j];

  four1(&W[-1].imag,FFT_ENC,-1);         /* "Numerical Recipes in C" FFT */

  /* 
      Re-arrange W[] to be symmetrical about FFT_ENC/2.  Makes later 
      analysis convenient.

   Before:


     0                 FFT_ENC-1
     |----------|---------|
     __                   _       
       \                 /          
        \_______________/      

   After:

     0                 FFT_ENC-1
     |----------|---------|
               ___                        
              /   \                
     ________/     \_______     

  */
       
      
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

