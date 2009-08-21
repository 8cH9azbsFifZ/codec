/*---------------------------------------------------------------------------*\
                                        
  FILE........: globals.c
  AUTHOR......: David Rowe                                                          
  DATE CREATED: 11/5/94                          
                                                                             
  Globals for sinusoidal speech coder.		      
                                                                             
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

#include "sine.h"	/* global defines for coder */

/* Globals used in encoder and decoder */

int frames;		/* number of frames processed so far */
float Sn[M+AW_ENC/2];	/* float input speech samples */
MODEL model;		/* model parameters for the current frame */
int Nw;			/* number of samples in analysis window */
float sig;		/* energy of current frame */

/* Globals used in encoder */

float w[AW_ENC];	/* time domain hamming window */
COMP Sw[FFT_ENC];	/* DFT of current frame */

/* Globals used in decoder */

COMP Sw_[FFT_ENC];	/* DFT of all voiced synthesised signal */
float Sn_[AW_DEC];	/* synthesised speech */
float Pn[AW_DEC];	/* time domain Parzen (trapezoidal) window */

