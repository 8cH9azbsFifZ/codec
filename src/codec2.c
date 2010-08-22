/*---------------------------------------------------------------------------*\

  FILE........: codec2.c
  AUTHOR......: David Rowe
  DATE CREATED: 21/8/2010

  Codec2 fully quantised encoder and decoder functions.  If you want use 
  codec2, these are the functions you need to call.

\*---------------------------------------------------------------------------*/

/*
  Copyright (C) 2010 David Rowe

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

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "defines.h"
#include "sine.h"
#include "nlp.h"
#include "dump.h"
#include "lpc.h"
#include "quantise.h"
#include "phase.h"
#include "postfilter.h"
#include "interp.h"

typedef struct {
    float  Sn[M];        /* input speech                              */
    float  w[M];	 /* time domain hamming window                */
    COMP   W[FFT_ENC];	 /* DFT of w[]                                */
    float  Pn[2*N];	 /* trapezoidal synthesis window              */
    float  Sn_[2*N];	 /* synthesised speech                        */
    float  prev_Wo;      /* previous frame's pitch estimate           */
    float  ex_phase;     /* excitation model phase track              */
    float  bg_est;       /* background noise estimate for post filter */
    MODEL *prev_model;   /* model parameters from 20ms ago            */
} CODEC2;

/*---------------------------------------------------------------------------*\
                                                       
  FUNCTION....: codec2_create	     
  AUTHOR......: David Rowe			      
  DATE CREATED: 21/8/2010 

  Create and initialise an instance of the codec.

\*---------------------------------------------------------------------------*/

void *codec2_create()
{
    CODEC2 *c2;
    int     i,l;

    c2 = (CODEC2*)malloc(sizeof(CODEC2));

    for(i=0; i<M; i++)
	c2->Sn[i] = 1.0;
    for(i=0; i<2*N; i++)
	c2->Sn_[i] = 0;
    make_analysis_window(c2->w,c2->W);
    make_synthesis_window(c2->Pn);
    quantise_init();
    c2->prev_Wo = 0.0;
    c2->bg_est = 0.0;
    c2->ex_phase = 0.0;

    for(l=1; l<=MAX_AMP; l++)
	c2->prev_model.A[l] = 0.0;
    c2->prev_model.Wo = = TWO_PI/P_MAX;

    return (void*)c2;
}

/*---------------------------------------------------------------------------*\
                                                       
  FUNCTION....: codec2_create	     
  AUTHOR......: David Rowe			      
  DATE CREATED: 21/8/2010 

  Destroy an instance of the codec.

\*---------------------------------------------------------------------------*/

void codec2_destroy(void *codec2_state)
{
    assert(codec2_state != NULL);
    free(codec2_state);
}

/*---------------------------------------------------------------------------*\
                                                       
  FUNCTION....: codec2_encode	     
  AUTHOR......: David Rowe			      
  DATE CREATED: 21/8/2010 

  Encodes 160 speech samples (20ms of speech) into 51 bits.  

  The bits[] array is not packed, each bit is stored in the LSB of
  each byte in the bits[] array.

  The codec2 algorithm actually operates internally on 10ms (80
  sample) frames, so we run the encoding algorithm twice.  On the
  first frame we just send the voicing bit.  One the second frame we
  send all model parameters.

  The bit allocation is:

    Parameter                      bits/frame
    --------------------------------------
    Harmonic magnitudes (LSPs)     36
    Low frequency LPC correction    1
    Energy                          5
    Pitch (fundamental frequnecy)   7
    Voicing (10ms update)           2
    TOTAL                          51
 
\*---------------------------------------------------------------------------*/

void codec2_encode(void *codec2_state, char bits[], short speech[])
{
    CODEC2 *c2;
    COMP    Sw[FFT_ENC];
    COMP    Sw_[FFT_ENC];
    MODEL   model;
    float   pitch;
    int     voiced1, voiced2;
    int     i, nbits;

    assert(codec2_state != NULL);
    c2 = (CODEC2*)codec2_state;

    /* First Frame - just send voicing ----------------------------------*/

    /* Read input speech */

    for(i=0; i<M-N; i++)
      c2->Sn[i] = c2->Sn[i+N];
    for(i=0; i<N; i++)
      c2->Sn[i+M-N] = speech[i];
    dft_speech(Sw, c2->Sn, c2->w);

    /* Estimate pitch */

    nlp(c2->Sn,N,M,P_MIN,P_MAX,&pitch,Sw,&c2->prev_Wo);
    c2->prev_Wo = TWO_PI/pitch;
    model.Wo = TWO_PI/pitch;
    model.L = PI/model.Wo;

    /* estimate model parameters */

    dft_speech(Sw, c2->Sn, c2->w); 
    two_stage_pitch_refinement(&model, Sw);
    estimate_amplitudes(&model, Sw, c2->W);
    est_voicing_mbe(&model, Sw, c2->W, (FS/TWO_PI)*model.Wo, Sw_);
    voiced1 = model.voiced;

    /* Second Frame - send all parameters --------------------------------*/

    /* Read input speech */

    for(i=0; i<M-N; i++)
      c2->Sn[i] = c2->Sn[i+N];
    for(i=0; i<N; i++)
      c2->Sn[i+M-N] = speech[i+N];
    dft_speech(Sw, c2->Sn, c2->w);

    /* Estimate pitch */

    nlp(c2->Sn,N,M,P_MIN,P_MAX,&pitch,Sw,&c2->prev_Wo);
    c2->prev_Wo = TWO_PI/pitch;
    model.Wo = TWO_PI/pitch;
    model.L = PI/model.Wo;

    /* estimate model parameters */

    dft_speech(Sw, c2->Sn, c2->w); 
    two_stage_pitch_refinement(&model, Sw);
    estimate_amplitudes(&model, Sw, c2->W);
    est_voicing_mbe(&model, Sw, c2->W, (FS/TWO_PI)*model.Wo, Sw_);
    voiced2 = model.voiced;

    /* quantise */
    
    nbits = 0;
    encode_Wo(bits, &nbits, model.Wo);
    encode_voicing(bits, &nbits, voiced1, voiced2);
    encode_amplitudes(bits, &nbits, c2->Sn, c2->w);
    assert(nbits == CODEC2_BITS_PER_FRAME);
}

/*---------------------------------------------------------------------------*\
                                                       
  FUNCTION....: codec2_decode	     
  AUTHOR......: David Rowe			      
  DATE CREATED: 21/8/2010 

  Decodes frames of 51 bits into 160 samples (20ms) of speech.

\*---------------------------------------------------------------------------*/

void codec2_decode(void *codec2_state, short speech[], char bits[])
{
    CODEC2 *c2;
    MODEL   model;
    float   ak[LPC_ORD+1];
    int     voiced1, voiced2;
    int     i, nbits;
    MODEL   model_interp;

    assert(codec2_state != NULL);
    c2 = (CODEC2*)codec2_state;

    nbits = 0;
    model.Wo = decode_Wo(bits, &nbits);
    model.L = PI/model.Wo;
    decode_voicing(&voiced1, &voiced2, bits, &nbits);
    decode_amplitudes(&model, ak, bits, &nbits);
    assert(nbits == CODEC2_BITS_PER_FRAME);

    /* First synthesis frame - interpolated from adjacent frames */

    model_interp.voiced = voiced1;
    interp(&model_interp, &c2->prev_model, &model);
    phase_synth_zero_order(&model_interp, ak, voiced1, &c2->ex_phase);
    postfilter(&model_interp, voiced1, &c2->bg_est);
    synthesise(c2->Sn_, &model_interp, c2->Pn, 1);

    for(i=0; i<N; i++) {
	if (Sn_[i] > 32767.0)
	    speech[i] = 32767;
	else if (Sn_[i] < -32767.0)
	    speech[i] = -32767;
	else
	    speech[i] = Sn_[i];
    }

    /* Second synthesis frame */

    model.voiced = voiced2;
    phase_synth_zero_order(&model, ak, voiced2, &c2->ex_phase);
    postfilter(&model, voiced2, &c2->bg_est);
    synthesise(c2->Sn_, &model, c2->Pn, 1);

    for(i=0; i<N; i++) {
	if (Sn_[i] > 32767.0)
	    speech[i+N] = 32767;
	else if (Sn_[i] < -32767.0)
	    speech[i+N] = -32767;
	else
	    speech[i+N] = Sn_[i];
    }

    memcpy(&c2->prev_model, model, sizeof(MODEL);
}

