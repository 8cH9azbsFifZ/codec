/*---------------------------------------------------------------------------*\
                                                                             
  FILE........: sine.h
  AUTHOR......: David Rowe                                                          
  DATE CREATED: 1/11/94
                                                                             
  Header file for Sinusoidal coder.
                                                                             
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

/*---------------------------------------------------------------------------*\
                                                                             
				INCLUDES                                      
                                                                             
\*---------------------------------------------------------------------------*/

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "defines.h"	/* defines for sinusoidal coder */
#include "globals.h"	/* external globals */

/*---------------------------------------------------------------------------*\
                                                                             
				FUNCTIONS                                     
                                                                             
\*---------------------------------------------------------------------------*/

/* functions in refine.c */

void dft_speech();
void two_stage_pitch_refinement();
void hs_pitch_refinement(float pmin, float pmax, float pstep);

/* functions in spec.c */

void estimate_amplitudes();
void estimate_voicing();
void estimate_voicing_av();
float voicing(int lower, int upper);

/* functions in four1.c */

void four1();

/* functions in synth.c */

void synthesise_mixed();

/* functions in initenc.c and initdec.c */

void init_encoder(void);
float make_window();
void init_decoder();

/* functions in gasdev.c */

float gasdev();

