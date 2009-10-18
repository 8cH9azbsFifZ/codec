/*---------------------------------------------------------------------------*\

  FILE........: sinedec.c
  AUTHOR......: David Rowe
  DATE CREATED: 20/2/95

  Decoder program for sinudoidal codec.

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

#include <assert.h>
#include <string.h>
#include "sine.h"
#include "quantise.h"
#include "dump.h"
#include "phase.h"
#include "lpc.h"
#include "synth.h"
#include "postfilter.h"
#include "interp.h"

/*---------------------------------------------------------------------------*\
                                                                             
 switch_present()                                                            
                                                                             
 Searches the command line arguments for a "switch".  If the switch is       
 found, returns the command line argument where it ws found, else returns    
 NULL.                                                                       
                                                                             
\*---------------------------------------------------------------------------*/

int switch_present(sw,argc,argv)
register char sw[];     /* switch in string form */
register int argc;      /* number of command line arguments */
register char *argv[];  /* array of command line arguments in string form */
{
  register int i;       /* loop variable */

  for(i=1; i<argc; i++)
    if (!strcmp(sw,argv[i]))
      return(i);

  return 0;
}

/*---------------------------------------------------------------------------*\
                                                                          
				MAIN                                        
                                                                         
\*---------------------------------------------------------------------------*/

int main(int argc, char *argv[])
{
  FILE *fmodel;		/* file of model parameters from encoder */
  FILE *fout;		/* output speech file */
  FILE *fin;		/* input speech file */
  short buf[N];		/* input/output buffer */
  int i;		/* loop variable */
  int length;		/* number of frames so far */

  char  out_file[MAX_STR];
  int   arg;
  float snr;
  float sum_snr;

  int lpc_model, order;
  int lsp, lsp_quantiser;
  float ak[LPC_MAX_ORDER+1];
  
  int dump;
  
  int phase, phase_model;
  float ex_phase[1];
  int voiced, voiced_1, voiced_2, voiced_synth;

  int   postfilt;
  float bg_est;

  int   hand_snr;
  FILE *fsnr;

  MODEL model_1, model_2, model_3, model_synth, model_a, model_b;
  int transition;

  int vf=0;

  voiced_1 = voiced_2 = 0;
  model_1.Wo = TWO_PI/P_MIN;
  model_1.L = floor(PI/model_1.Wo);
  for(i=1; i<=model_1.L; i++) {
      model_1.A[i] = 0.0;
      model_1.phi[i] = 0.0;
  }
  model_synth = model_3 = model_2 = model_1;

  if (argc < 3) {
    printf("usage: sinedec InputFile ModelFile [-o OutputFile] [-o lpc Order]\n");
    printf("       [--dump DumpFilePrefix]\n");
    exit(0);
  }

  /* Interpret command line arguments -------------------------------------*/

  /* Input file */

  if ((fin = fopen(argv[1],"rb")) == NULL) {
    printf("Error opening input speech file: %s\n",argv[1]);
    exit(1);
  }

  /* Model parameter file */

  if ((fmodel = fopen(argv[2],"rb")) == NULL) {
    printf("Error opening model file: %s\n",argv[2]);
    exit(1);
  }

  /* Output file */

  if ((arg = switch_present("-o",argc,argv))) {
    if ((fout = fopen(argv[arg+1],"wb")) == NULL) {
      printf("Error opening output speech file: %s\n",argv[arg+1]);
      exit(1);
    }
    strcpy(out_file,argv[arg+1]);
  }
  else
    fout = NULL;

  /* Length (no. of frames) */

  if ((length = switch_present("-l",argc,argv))) {
    length = atoi(argv[length+1]);
    if (length < 0) {
      printf("Error in length: %d\n",length);
      exit(1);
    }
  }
  else
    length = 32000;

  lpc_model = 0;
  if ((arg = switch_present("--lpc",argc,argv))) {
      lpc_model = 1;
      order = atoi(argv[arg+1]);
      if ((order < 4) || (order > 20)) {
        printf("Error in lpc order: %d\n", order);
        exit(1);
      }	  
  }

  dump = switch_present("--dump",argc,argv);
  if (dump) 
      dump_on(argv[dump+1]);

  lsp = switch_present("--lsp",argc,argv);
  lsp_quantiser = 0;

  /* phase_model 0: zero phase
     phase_model 1: 1st order polynomial */
  phase = switch_present("--phase",argc,argv);
  if (phase) {
      phase_model = atoi(argv[phase+1]);
      assert((phase_model == 0) || (phase_model == 1));
      ex_phase[0] = 0;
  }

  hand_snr = switch_present("--hand_snr",argc,argv);
  if (hand_snr) {
      fsnr = fopen(argv[hand_snr+1],"rt");
      assert(fsnr != NULL);
  }

  bg_est = 0.0;
  postfilt = switch_present("--postfilter",argc,argv);

  transition = 0;

  /* Initialise ------------------------------------------------------------*/

  init_decoder();
  init_encoder();
  Nw = 220;
  make_window(Nw);
  quantise_init();

  /* Main loop ------------------------------------------------------------*/

  frames = 0;
  sum_snr = 0;
  while(fread(&model,sizeof(model),1,fmodel)) {
    frames++;

    /* Read input speech */

    fread(buf,sizeof(short),N,fin);
    for(i=0; i<M-N; i++)
      Sn[i] = Sn[i+N];
    for(i=0; i<N; i++)
      Sn[i+M-N] = buf[i];
    dump_Sn(Sn);
    dft_speech(); dump_Sw(Sw);   

    dump_model(&model);

    /* optional phase modelling - make sure this happens before LPC
       modelling of {Am} as first order model fit doesn't work well
       with LPC Modelled {Am} (not sure why - investigate later) */

    if (phase) {
	float Wn[M];		        /* windowed speech samples */
	float Rk[PHASE_LPC_ORD+1];	/* autocorrelation coeffs  */
        float ak_phase[PHASE_LPC_ORD+1];/* LPCs                    */
        COMP  H[MAX_AMP];               /* LPC freq domain samples */
	float n_min;
	COMP  min_Am;
  	
	dump_phase(&model.phi[0]);

	/* Determine LPCs for phase modelling.  Note that we may also
	   find the LPCs as part of the {Am} modelling, this can
	   probably be combined in the final codec.  However during
	   development some subtle bugs were found when combining LPC
	   and phase models so for the purpose of development it's
	   easier to find LPCs indepenently for phase modelling
	   here. */

	for(i=0; i<M; i++)
	    Wn[i] = Sn[i]*w[i];
	autocorrelate(Wn,Rk,M,PHASE_LPC_ORD);
	levinson_durbin(Rk,ak_phase,PHASE_LPC_ORD);

	if (lpc_model)
	    assert(order == PHASE_LPC_ORD);

	dump_ak(ak_phase, PHASE_LPC_ORD);
	snr = phase_model_first_order(ak_phase, H, &n_min, &min_Am, &voiced);

	dump_snr(snr);
	if (phase_model == 0) {
	    /* just to make sure we are not cheating - kill all phases */
	    for(i=0; i<MAX_AMP; i++)
	    	model.phi[i] = 0;
	    if (hand_snr) {
		fscanf(fsnr,"%f\n",&snr);
		voiced = snr > 2.0;
	    }
	    phase_synth_zero_order(voiced, H, ex_phase, voiced);
	}

	if (phase_model == 1) {
	    phase_synth_first_order(voiced, H, n_min, min_Am, voiced);
        }

        if (postfilt)
	    postfilter(&model, voiced, &bg_est);

        //dump_phase_(&model.phi[0]);
    }

    /* optional LPC model amplitudes */

    if (lpc_model) {
	snr = lpc_model_amplitudes(Sn, &model, order, lsp, ak);
	sum_snr += snr;
        dump_quantised_model(&model);
    }

    //#define MAKE_CLICKY
#ifdef MAKE_CLICKY
    {
	float maxA = 0.0;
	float dB;
	int   max_m;

	for(i=1; i<=model.L; i++) {
	    if (model.A[i] > maxA) {
		maxA = model.A[i];
		max_m = i;
	    }
	}
	for(i=1; i<=model.L; i++) {
	    if (model.A[i] > 0.1*maxA) {
		model.A[i] = 0.0;
	    }
	}

    }
#endif
	

    //#define REDUCE_CLICKY
#ifdef REDUCE_CLICKY
    {
	float maxA = 0.0;
	float dB;
	int   max_m;

	for(i=1; i<=model.L; i++) {
	    if (model.A[i] > maxA) {
		maxA = model.A[i];
		max_m = i;
	    }
	}
	for(i=1; i<=model.L; i++) {
	    if (model.A[i] < 0.1*maxA) {
		model.phi[i] += 0.2*TWO_PI*(float)rand()/RAND_MAX;
		dB = 3.0 - 6.0*(float)rand()/RAND_MAX;
		model.A[i] *= pow(10.0, dB/20.0);
	    }
	}

    }
#endif


    //#define DEC
 #ifdef DEC
   /* Decimate to 20ms frame rate.  In the code we only send
      off frames to the receiver.  To simulate this on odd
      frames the model parameters pass straight thru.  On even
      frames we interpolate from adjacent odd frames.  A one
      frame delay is required for the odd frames.
   */

    /* 
       frames  Transmitted to Rx  Decimator output
 
         0     n                  0.5*model(-3) + 0.5*model(-1)
         1     y                  model(-1)
         2     n                  0.5*model(-1) + 0.5*model(1)
         3     y                  model(1)
 	 4     n                  0.5*model(1) + 0.5*model(3)
	 5     y                  model(3)
    */

    /* 
       TODO: 
       [ ] Voicing decision
       [ ] unvoiced
           [ ] amplitudes
           [ ] phases
           [ ] Wo
       [ ] unvoiced
           [ ] amplitudes
           [ ] phases
               + OK to run zero phase model on 10ms rate, using info
                 from adjacent 20 ms frames
           [ ] Wo
    */

    dump_model(&model_2);

    if (frames%2) {

	/* odd frames use the original model parameters */

	model_synth = model_2;
	transition = 0;

    }
    else {
	/* even frame so we need to synthesise the model parameters by
	   interpolating between adjacent frames */

	if (fabs(model_1.Wo - model_3.Wo) < 0.1*model_1.Wo) {
	    /* If the Wo of adjacent frames is within 10% we synthesise a 
	       continuous track through this frame by linear interpolation
	       of the amplitudes and Wo.  This is typical of a strongly 
	       voiced frame.
	    */

	    transition = 0;

	    model_synth.Wo = (model_1.Wo + model_3.Wo)/2.0;
	    if (model_1.L > model_3.L)
		model_synth.L = model_3.L;
	    else
		model_synth.L = model_1.L;
	    for(i=1; i<=model_synth.L; i++) {
		model_synth.A[i] = (model_3.A[i] + model_1.A[i])/2.0;
		/* cheat on phases for now, these were constructed using
		   LPC model from actual speech for this frame - fix later */
		model_synth.phi[i] = model_2.phi[i];
	    }
	}
	else {
	    /* 
	       transition frame, adjacent frames have different Wo and
	       L so set up two sets of model parameters based on
	       previous and next frame.  We then synthesise both of
	       them and add them together in the time domain.  

	       This case is typical of unvoiced speech or background
	       noise or a voiced/unvoiced transition.
	    */

	    transition = 1;

	    /* model_a is the previous frames extended forward into
	       this frame, model_b is the next frame extended backward
	       into this frame.  Note the adjustments to phase to
	       time-shift the model forward or backward N samples. */

	    memcpy(&model_a, &model_3, sizeof(model));
	    memcpy(&model_b, &model_1, sizeof(model));
	    for(i=1; i<=model_a.L; i++) {
		model_a.A[i] /= 2.0;
		model_a.phi[i] += model_a.Wo*i*N;
	    }
	    for(i=1; i<=model_b.L; i++) {
		model_b.A[i] /= 2.0;
		model_b.phi[i] -= model_b.Wo*i*N;
	    }
	}
    }

    voiced_2 = voiced_1;
    voiced_1 = voiced;

    model_3 = model_2;
    model_2 = model_1;
    model_1 = model;
    model = model_synth;
#endif
    //dump_quantised_model(&model);

#define INTERP
#ifdef INTERP
    if (frames%2) {

	/* odd frames use the original model parameters */

	model_synth = model_2;
	transition = 0;

    }
    else {
	interp(&model_3, &model_1, &model_synth, &model_a, &model_b, &transition);
	for(i=1; i<=model_synth.L; i++)
	    model_synth.phi[i] = model_2.phi[i];
    }

    model_3 = model_2;
    model_2 = model_1;
    model_1 = model;
    model = model_synth;
	
#endif

    /* Synthesise speech */

    if (fout != NULL) {

	if (transition) {
	    synthesise_mixed(Pn,&model_a,Sn_,1);
	    synthesise_mixed(Pn,&model_b,Sn_,0);
	}
	else {
	    synthesise_mixed(Pn,&model,Sn_,1);
	}

	/* Save output speech to disk */

	for(i=0; i<N; i++) {
	    if (Sn_[i] > 32767.0)
		buf[i] = 32767;
	    else if (Sn_[i] < -32767.0)
		buf[i] = -32767;
	    else
		buf[i] = Sn_[i];
	}
	fwrite(buf,sizeof(short),N,fout);
    }    
  }

  if (fout != NULL)
    fclose(fout);

  if (lpc_model)
      printf("SNR av = %5.2f dB\n", sum_snr/frames);

  if (dump)
      dump_off();

  if (hand_snr)
    fclose(fsnr);

  return 0;
}

