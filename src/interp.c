/*---------------------------------------------------------------------------*\

  FILE........: interp.c
  AUTHOR......: David Rowe
  DATE CREATED: 9/10/09

  Interpolation of 20ms frames to 10ms frames.

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
#include <string.h>

#include "defines.h"
#include "interp.h"

/*---------------------------------------------------------------------------*\

  interp()
        
  Given two frames decribed by model parameters 20ms apart, determines the
  model parameters of the 10ms frame between them.

\*---------------------------------------------------------------------------*/

void interp(
  MODEL *prev,      /* previous frames model params                  */
  MODEL *next,      /* next frames model params                      */
  MODEL *synth,     /* interp model params for cont frame            */
  MODEL *a,         /* prev frame extended into this frame           */
  MODEL *b,         /* next frame extended into this frame           */
  int   *transition /* non-zero if this is a transition frame, this
		       information is used for synthesis             */
)
{
    int m;
    
    if (fabs(next->Wo - prev->Wo) < 0.1*next->Wo) {

	/* If the Wo of adjacent frames is within 10% we synthesise a 
	   continuous track through this frame by linear interpolation
	   of the amplitudes and Wo.  This is typical of a strongly 
	   voiced frame.
	*/

	*transition = 0;

	synth->Wo = (next->Wo + prev->Wo)/2.0;
	if (next->L > prev->L)
	    synth->L = prev->L;
	else
	    synth->L = next->L;
	for(m=1; m<=synth->L; m++) {
	    synth->A[m] = (prev->A[m] + next->A[m])/2.0;
	}
    }
    else {
	/* 
	   transition frame, adjacent frames have different Wo and L
	   so set up two sets of model parameters based on prev and
	   next.  We then synthesise both of them and add them
	   together in the time domain.

	   The transition case is typical of unvoiced speech or
	   background noise or a voiced/unvoiced transition.
	*/

	*transition = 1;

	/* a is prev extended forward into this frame, b is next
	   extended backward into this frame.  Note the adjustments to
	   phase to time-shift the model forward or backward N
	   samples. */

	memcpy(a, prev, sizeof(MODEL));
	memcpy(b, next, sizeof(MODEL));
	for(m=1; m<=a->L; m++) {
	    a->A[m] /= 2.0;
	    a->phi[m] += a->Wo*m*N;
	}
	for(m=1; m<=b->L; m++) {
	    b->A[m] /= 2.0;
	    b->phi[m] -= b->Wo*m*N;
	}
    }

}

