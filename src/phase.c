/*---------------------------------------------------------------------------*\
                                                                             
  FILE........: phase.c                                           
  AUTHOR......: David Rowe                                             
  DATE CREATED: 1/2/09                                                 
                                                                             
  Functions for modelling and synthesising phase.
                                                                             
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
#include "phase.h"
#include "lpc.h"
#include <assert.h>
#include <string.h>

#define VTHRESH1 2.0
#define VTHRESH2 2.0

/*---------------------------------------------------------------------------*\

  aks_to_H()

  Samples the complex LPC synthesis filter spectrum at the harmonic
  frequencies.

\*---------------------------------------------------------------------------*/

void aks_to_H(model,aks,G,H, order)
MODEL *model;	/* model parameters */
float  aks[];	/* LPC's */
float  G;	/* energy term */
COMP   H[];	/* complex LPC spectral samples */
int    order;
{
  COMP  Pw[FFT_DEC];	/* power spectrum */
  int   i,m;		/* loop variables */
  int   am,bm;		/* limits of current band */
  float r;		/* no. rads/bin */
  float Em;		/* energy in band */
  float Am;		/* spectral amplitude sample */
  int   b;		/* centre bin of harmonic */
  float phi_;		/* phase of LPC spectra */

  r = TWO_PI/(FFT_DEC);

  /* Determine DFT of A(exp(jw)) ------------------------------------------*/

  for(i=0; i<FFT_DEC; i++) {
    Pw[i].real = 0.0;
    Pw[i].imag = 0.0;
  }

  for(i=0; i<=order; i++)
    Pw[i].real = aks[i];

  four1(&Pw[-1].imag,FFT_DEC,-1);

  /* Sample magnitude and phase at harmonics */

  for(m=1; m<=model->L; m++) {
    am = floor((m - 0.5)*model->Wo/r + 0.5);
    bm = floor((m + 0.5)*model->Wo/r + 0.5);
    b = floor(m*model->Wo/r + 0.5);

    Em = 0.0;
    for(i=am; i<bm; i++)
      Em += G/(Pw[i].real*Pw[i].real + Pw[i].imag*Pw[i].imag);
    Am = sqrt(fabs(Em/(bm-am)));

    phi_ = -atan2(Pw[b].imag,Pw[b].real);
    H[m].real = Am*cos(phi_);
    H[m].imag = Am*sin(phi_);
  }
}

/*---------------------------------------------------------------------------*\

   phase_model_first_order()

   Models the current frames phase samples {phi[m]} as a LPC synthesis
   filter excited by an impulse at time i_min with a complex gain min_Am.
   The phase of the complex gain min_Am is a constant phase term, and the
   impulse position i_min creates a phase term that varies linearly
   with frequency.  We are therefore modelling the excitation phase as
   a 1st order polynomial function of mWo:

     Ex[m] = -m*Wo*i_min + arg(min_Am);

   and the harmonic phase is given by:
  
     phi[m] = -m*Wo*i_min + arg(min_Am) + arg[H[m]];

   where H[m] is the LPC spectra sampled at the mWo harmonic frequencies.

   This actually works out to be a pretty good model for voiced
   speech.  The fit of the model (snr) is therefore used as a voicing
   measure.  If the snr is less than a threshold, the frame is
   declared unvoiced all all the phases are randomised.

   Reference:  
     [1] http://www.itr.unisa.edu.au/~steven/thesis/dgr.pdf Chapter 6

   NOTE: min_Am is a dumb name for the complex gain as {A} or A[m] is
   commonly used for the spectral magnitudes.  TODO: change name to match
   thesis term G(mWo) (which of course clashes with LPC gain! AAAAAAHHH!).

\*---------------------------------------------------------------------------*/

float phase_model_first_order(
  float  aks[],                  /* LPC coeffs for this frame      */
  COMP   H[],		         /* LPC filter freq doamin samples */
  float *n_min,                  /* pulse position for min error   */ 
  COMP  *minAm                   /* complex gain for min error     */
) 
{
  float G;			/* LPC gain */
  int   m;

  float E,Emin;			/* current and minimum error so far */
  int P;			/* current pitch period */
  COMP A[MAX_AMP];		/* harmonic samples */
  COMP Ex[MAX_AMP];		/* excitation samples */
  COMP A_[MAX_AMP];		/* synthesised harmonic samples */
  COMP Am;    			/* complex gain */
  COMP Em;  			/* error for m-th band */
  float den;     		/* energy of synthesised */
  float snr;		        /* snr of each excitation source */
  int   Lmax;
  float n;

  Lmax = model.L;

  /* Construct target vector */

  sig = 0.0;
  for(m=1; m<=Lmax; m++) {
    A[m].real = model.A[m]*cos(model.phi[m]);
    A[m].imag = model.A[m]*sin(model.phi[m]);
    sig += model.A[m]*model.A[m];
  }

  /* Sample LPC model at harmonics */

  //#define NO_LPC_PHASE
  #ifdef NO_LPC_PHASE
  /* useful for testing with Sn[] an impulse train */
  for(m=1; m<=PHASE_LPC_ORD; m++)
     aks[m] = 0;
  #endif
  G = 1.0;
  aks_to_H(&model,aks,G,H,PHASE_LPC_ORD);

  /* Now attempt to fit impulse, by trying positions 0..P-1 */

  Emin = 1E32;
  P = floor(TWO_PI/model.Wo + 0.5);
  for(n=0; n<P; n+=0.25) {

    /* determine complex gain */

    Am.real = 0.0;
    Am.imag = 0.0;
    den = 0.0;
    for(m=1; m<=Lmax; m++) {
      Ex[m].real = cos(model.Wo*m*n);
      Ex[m].imag = sin(-model.Wo*m*n);
      A_[m].real = H[m].real*Ex[m].real - H[m].imag*Ex[m].imag;
      A_[m].imag = H[m].imag*Ex[m].real + H[m].real*Ex[m].imag;
      Am.real += A[m].real*A_[m].real + A[m].imag*A_[m].imag;
      Am.imag += A[m].imag*A_[m].real - A[m].real*A_[m].imag;
      den += A_[m].real*A_[m].real + A_[m].imag*A_[m].imag;
    }

    Am.real /= den;
    Am.imag /= den;

    /* determine error */

    E = 0.0;
    for(m=1; m<=Lmax; m++) {
      float new_phi;

      Em.real = A_[m].real*Am.real - A_[m].imag*Am.imag;
      Em.imag = A_[m].imag*Am.real + A_[m].real*Am.imag;
	  
      new_phi = atan2(Em.imag, Em.real+1E-12);
      E += pow(model.A[m]*(cos(model.phi[m])-cos(new_phi)),2.0);
      E += pow(model.A[m]*(sin(model.phi[m])-sin(new_phi)),2.0);
    }

    if (E < Emin) {
      Emin = E;
      *n_min = n;
      minAm->real = Am.real;
      minAm->imag = Am.imag;
    }

  }

  snr = 10.0*log10(sig/Emin);

  return snr;
}

/*---------------------------------------------------------------------------*\

   phase_synth_zero_order()

   Synthesises phases based on SNR and a rule based approach.  No phase 
   parameters are required apart from the SNR (which can be reduec to a
   1 bit V/UV decision per frame).

   The phase of each harmonic is modelled as the phase of a LPC
   synthesis filter excited by an impulse.  Unlike the first order
   model the position of the impulse is not transmitted, so we create
   an excitation pulse train using a rule based approach.  

   Consider a pulse train with a pulse starting time n=0, with pulses
   repeated at a rate of Wo, the fundamental frequency.  A pulse train
   in the time domain is equivalent to a pulse train in the frequency
   domain.  We can make an excitation pulse train using a sum of
   sinsusoids:

     for(m=1; m<=L; m++)
       ex[n] = cos(m*Wo*n)

   Note: the Octave script ../octave/phase.m is an example of this if you would
   like to try making a pulse train.

   The phase of each excitation harmonic is:

     arg(E[m]) = mWo

   where E[m] are the complex excitation (freq domain) samples,
   arg(x), just returns the phase of a complex sample x.

   As we don't transmit the pulse position for this model, we need to
   synthesise it.  Now the excitation pulses occur at a rate of Wo.
   This means the phase of the first harmonic advances by N samples
   over a synthesis frame of N samples.  For example if Wo is pi/20
   (200 Hz), then over a 10ms frame (N=80 samples), the phase of the
   first harmonic would advance (pi/20)*80 = 4*pi or two complete
   cycles.

   We track the excitation phase of the fundamental (first harmonic):

     arg[E[1]] = Wo*N;

   We then relate the phase of the m-th excitation harmonic to the
   phase of the fundamental as:

     arg(E[m]) = marg(E[1])

   This E[m] then gets passed through the LPC synthesis filter to
   determine the final harmonic phase.
     
   NOTES:

     1/ This synthsis model is effectvely the same as simple LPC-10
     vocoders, and yet sounds much better.  Why?

     2/ I am pretty sure the Lincoln Lab sinusoidal coding guys (like xMBE
     also from MIT) first described this zero phase model, I need to look
     up the paper.

     3/ Note that this approach could cause some discontinuities in
     the phase at the edge of synthesis frames, as no attempt is made
     to make sure that the phase tracks are continuous (the excitation
     phases are continuous, but not teh final phases after filtering
     by the LPC spectra).  Technically this is a bad thing.  However
     this may actually be a good thing, disturbing the phase tracks a
     bit.  More research needed, e.g. test a synthsis model that adds
     a small delta-W to make phase tracks line up for voiced
     harmonics.

\*---------------------------------------------------------------------------*/

void phase_synth_zero_order(
  float  snr,     /* SNR from first order model                */
  COMP   H[],     /* LPC spectra samples                       */
  float *ex_phase /* excitation phase of fundamental           */
)
{
  int   Lrand;
  int   m;
  float new_phi;
  COMP  Ex[MAX_AMP];		/* excitation samples */
  COMP  A_[MAX_AMP];		/* synthesised harmonic samples */

  /* 
     Bunch of mixed voicing thresholds tried but in the end a simple
     voiced/unvoiced model worked best.  With mixed voicing some
     unvoiced speech had a "clicky" sound due to occasional high SNR
     causing the first few harmonics to be modelled as voiced. I don't
     really understand why simple one bit V/UV sounds so good -
     everyone else seems to think mixed voicing models are required
     for good quality speech.

     Note code below supports mixed voicing but with VTHRESH1 == VTHRESH2
     we get a simple V/UV model.
  */

  Lrand = model.L;
  if (snr < VTHRESH2) {
    Lrand = floor(model.L*(snr-VTHRESH1)/(VTHRESH2-VTHRESH1));
    if (Lrand < 1) Lrand = 0;
    if (Lrand > model.L) Lrand = model.L;
  }

  /* 
     Update excitation fundamental phase track, this sets the position
     of each pitch pulse during voiced speech.  After much experiment
     I found that using just this frame Wo improved quality for UV
     sounds compared to interpolating two frames Wo like this:
     
     ex_phase[0] += (*prev_Wo+model.Wo)*N/2;
  */

  ex_phase[0] += (model.Wo)*N;
  ex_phase[0] -= TWO_PI*floor(ex_phase[0]/TWO_PI + 0.5);

  /* now modify this frames phase using zero phase model */

  for(m=1; m<=model.L; m++) {

    /* generate excitation */

    if (m <= Lrand) {
	Ex[m].real = cos(ex_phase[0]*m);
        Ex[m].imag = sin(ex_phase[0]*m);

	/* following is an experiment in dispersing pulse energy over
	   time, didn't really change sound at all, e.g. mmt1 still
	   sounded "clicky.  I think this is because this provides
	   just a small phase shift between adjacent harmonics.
	   However for voiced speech it is the high energy harmonics
	   that form pitch pulses, so we need a relatively high phase
	   shift between them to disperse pulse energy */

        //Ex[m].real = cos(ex_phase[0]*m + model.Wo*m*m*0.3);
	//Ex[m].imag = sin(ex_phase[0]*m + model.Wo*m*m*0.3);

	/* following is an experiment to use the phase of a glottal pulse
	   (see octave/glottal.m) in an attempt io make 9mmt1 and hts1 a little
	   less "clicky", i.e. disperse the pusle energy away from the point
	   of onset.  Result was no difference in speech quality, in fact
	   no difference at all. Could be an implementation error I guess. 
	   One again - this model doesnt change phases much between adjacent
	   harmonics, so not much dispersion. */
	//b = floor(m*model->Wo*FFT_DEC/TWO_PI + 0.5);
        //Ex[m].real = cos(ex_phase[0]*m + glottal[b]);
	//Ex[m].imag = sin(ex_phase[0]*m + glottal[b]);

    }
    else {
	/* When a few samples were tested I found that LPC filter
	   phase is not needed in the unvoiced case, but no harm in keeping it.
        */
	float phi = TWO_PI*(float)rand()/RAND_MAX;
        Ex[m].real = cos(phi);
	Ex[m].imag = sin(phi);
    }

    /* filter using LPC filter */

    A_[m].real = H[m].real*Ex[m].real - H[m].imag*Ex[m].imag;
    A_[m].imag = H[m].imag*Ex[m].real + H[m].real*Ex[m].imag;

    /* modify sinusoidal phase */

    new_phi = atan2(A_[m].imag, A_[m].real+1E-12);
    model.phi[m] = new_phi;
  }
}

/*---------------------------------------------------------------------------*\

   phase_synth_first_order()

   Synthesises phases based on SNR and the first oreder phase model
   parameters.

\*---------------------------------------------------------------------------*/

void phase_synth_first_order(
  float snr,     /* SNR from first order model */
  COMP  H[],     /* LPC spectra samples        */
  float n_min,   /* best pulse position        */
  COMP  minAm    /* best complex gain          */
)
{
  int   Lrand;
  int   m;
  float new_phi;
  COMP  Ex[MAX_AMP];		/* excitation samples */
  COMP  A_[MAX_AMP];		/* synthesised harmonic samples */
  COMP  Tm;  			

  /* see notes in zero phase function above to V/UV model */

  Lrand = model.L;
  if (snr < VTHRESH2) {
    Lrand = floor(model.L*(snr-VTHRESH1)/(VTHRESH2-VTHRESH1));
    if (Lrand < 1) Lrand = 1;
    if (Lrand > model.L) Lrand = model.L;
  }

  /* now modify sinusoidal model phase using phase model */

  for(m=1; m<=model.L; m++) {

    /* generate excitation */

    if (m <= Lrand) {
	Ex[m].real = cos(model.Wo*m*n_min);
	Ex[m].imag = sin(-model.Wo*m*n_min);
    }
    else {
      float phi = TWO_PI*(float)rand()/RAND_MAX;
      Ex[m].real = cos(phi);
      Ex[m].imag = sin(phi);
    }

    /* filter using LPC filter */

    A_[m].real = H[m].real*Ex[m].real - H[m].imag*Ex[m].imag;
    A_[m].imag = H[m].imag*Ex[m].real + H[m].real*Ex[m].imag;

    /* scale by complex gain (could have done this earlier at Ex[]
       stage) */

    Tm.real = A_[m].real*minAm.real - A_[m].imag*minAm.imag;
    Tm.imag = A_[m].imag*minAm.real + A_[m].real*minAm.imag;

    /* modify sinusoidal phase */

    new_phi = atan2(Tm.imag, Tm.real+1E-12);
    model.phi[m] = new_phi;
  }
}

