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
  int i,j;

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

  /* Normalise - this might be useful later on */

  m = 1.0/sqrt(m*FFT_ENC);
  for(i=0; i<AW_ENC; i++) {
    w[i] *= m;
  }

  return(m);
}

