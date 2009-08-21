/*---------------------------------------------------------------------------*\
                                                        
  FILE........: initdec.c                                     
  AUTHOR......: David Rowe                                 
  DATE CREATED: 11/5/94                                
                                                       
  Initialises sinusoidal speech decoder globals.          
                                                             
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

void init_decoder() {
  int i;
  float win;

  /* Generate Parzen window in time domain */

  win = 0.0;
  for(i=0; i<N/2-TW; i++)
    Pn[i] = 0.0;
  win = 0.0;
  for(i=N/2-TW; i<N/2+TW; win+=1.0/(2*TW), i++ )
    Pn[i] = win;
  for(i=N/2+TW; i<3*N/2-TW; i++)
    Pn[i] = 1.0;
  win = 1.0;
  for(i=3*N/2-TW; i<3*N/2+TW; win-=1.0/(2*TW), i++)
    Pn[i] = win;
  for(i=3*N/2+TW; i<2*N; i++)
    Pn[i] = 0.0;

  /* Init output buffer */

  for(i=0; i<AW_DEC; i++)
    Sn_[i] = 0.0;

}
