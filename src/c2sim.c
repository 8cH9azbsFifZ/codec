/*---------------------------------------------------------------------------*\

  FILE........: c2sim.c
  AUTHOR......: David Rowe
  DATE CREATED: 20/8/2010

  Codec2 simulation.  Combines encoder and decoder and allows switching in 
  out various algorithms and quantisation steps. 

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
  FILE *fout;		/* output speech file                    */
  FILE *fin;		/* input speech file                     */
  short buf[N];		/* input/output buffer                   */
  float Sn[M];	        /* float input speech samples            */
  COMP  Sw[FFT_ENC];	/* DFT of Sn[]                           */
  float w[M];	        /* time domain hamming window            */
  COMP  W[FFT_ENC];	/* DFT of w[]                            */
  MODEL model;
  float Pn[2*N];	/* trapezoidal synthesis window          */
  float Sn_[2*N];	/* synthesised speech */
  int   i;		/* loop variable                         */
  int   frames;
  float prev_Wo;
  float pitch;

  char  out_file[MAX_STR];
  int   arg;
  float snr;
  float sum_snr;

  int lpc_model, order;
  int lsp, lsp_quantiser;
  float ak[LPC_MAX+1];
  
  int dump;
  
  int phase0;
  float ex_phase[MAX_AMP+1];
  int voiced, voiced_1, voiced_2;

  int   postfilt;
  float bg_est;

  int   hand_snr;
  FILE *fsnr;

  MODEL model_1, model_2, model_3, model_synth, model_a, model_b;
  int transition, decimate;

  for(i=0; i<M; i++)
      Sn[i] = 1.0;
  for(i=0; i<2*N; i++)
      Sn_[i] = 0;

  prev_Wo = TWO_PI/P_MAX;

  voiced_1 = voiced_2 = 0;
  model_1.Wo = TWO_PI/P_MIN;
  model_1.L = floor(PI/model_1.Wo);
  for(i=1; i<=model_1.L; i++) {
      model_1.A[i] = 0.0;
      model_1.phi[i] = 0.0;
  }
  for(i=1; i<=MAX_AMP; i++) {
      ex_phase[i] = 0.0;
  }
  model_synth = model_3 = model_2 = model_1;

  if (argc < 2) {
      printf("\nCodec2 - 2400 bit/s speech codec - Simulation Program\n");
      printf("            http://rowetel.com/codec2.html\n\n");
      printf("usage: %s InputFile [-o OutputFile]\n", argv[0]);
      printf("        [-o lpc Order]\n");
      printf("        [--lsp]\n");
      printf("        [--phase0]\n");
      printf("        [--postfilter]\n");
      printf("        [--hand_snr]\n");
      printf("        [--dec]\n");
      printf("        [--dump DumpFilePrefix]\n");
    exit(0);
  }

  /* Interpret command line arguments -------------------------------------*/

  /* Input file */

  if ((fin = fopen(argv[1],"rb")) == NULL) {
    printf("Error opening input speech file: %s\n",argv[1]);
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

  phase0 = switch_present("--phase0",argc,argv);
  if (phase0) {
      ex_phase[0] = 0;
  }

  hand_snr = switch_present("--hand_snr",argc,argv);
  if (hand_snr) {
      fsnr = fopen(argv[hand_snr+1],"rt");
      assert(fsnr != NULL);
  }

  bg_est = 0.0;
  postfilt = switch_present("--postfilter",argc,argv);

  decimate = switch_present("--dec",argc,argv);
  transition = 0;

  /* Initialise ------------------------------------------------------------*/

  make_analysis_window(w,W);
  make_synthesis_window(Pn);
  quantise_init();

  /* Main loop ------------------------------------------------------------*/

  frames = 0;
  sum_snr = 0;
  while(fread(buf,sizeof(short),N,fin)) {
    frames++;
    
    /* Read input speech */

    for(i=0; i<M-N; i++)
      Sn[i] = Sn[i+N];
    for(i=0; i<N; i++)
      Sn[i+M-N] = buf[i];
    dump_Sn(Sn);
 
    /* Estimate pitch */

    nlp(Sn,N,M,P_MIN,P_MAX,&pitch,Sw,&prev_Wo);
    prev_Wo = TWO_PI/pitch;
    model.Wo = TWO_PI/pitch;

    /* estimate model parameters */

    dft_speech(Sw, Sn, w); dump_Sw(Sw);   
    two_stage_pitch_refinement(&model, Sw);
    estimate_amplitudes(&model, Sw, W);
    dump_Sn(Sn); dump_Sw(Sw); dump_model(&model);

    /* optional zero-phase modelling */

    if (phase0) {
	float Wn[M];		        /* windowed speech samples */
	float ak_phase[PHASE_LPC+1];	/* autocorrelation coeffs  */
	float Rk[PHASE_LPC+1];	        /* autocorrelation coeffs  */
	COMP  Sw_[FFT_ENC];
  	
	dump_phase(&model.phi[0], model.L);

	/* Determine LPCs for phase modelling.  Note that we may also
	   find the LPCs as part of the {Am} modelling, this can
	   probably be combined in the final codec.  However during
	   development some subtle bugs were found when combining LPC
	   and phase models so for the purpose of development it's
	   easier to find LPCs indepenently for phase modelling
	   here. */

	for(i=0; i<M; i++)
	    Wn[i] = Sn[i]*w[i];
	autocorrelate(Wn,Rk,M,LPC_ORD);
	levinson_durbin(Rk,ak_phase,LPC_ORD);

	if (lpc_model)
	    assert(order == LPC_ORD);

	dump_ak(ak_phase, LPC_ORD);
	
	/* determine voicing */

	snr = est_voicing_mbe(&model, Sw, W, (FS/TWO_PI)*model.Wo, Sw_, &voiced);
	dump_Sw_(Sw_);
	dump_snr(snr);

	/* just to make sure we are not cheating - kill all phases */

	for(i=0; i<MAX_AMP; i++)
	    model.phi[i] = 0;
	if (hand_snr) {
	    fscanf(fsnr,"%f\n",&snr);
	    voiced = snr > 2.0;
	}
	phase_synth_zero_order(&model, ak_phase, voiced, ex_phase);
	
       if (postfilt)
	    postfilter(&model, voiced, &bg_est);
    }
 
    /* optional LPC model amplitudes */

    if (lpc_model) {
	snr = lpc_model_amplitudes(Sn, w, &model, order, lsp, ak);
	sum_snr += snr;
        dump_quantised_model(&model);
    }

    /* option decimation to 20ms rate, which enables interpolation
       routine to synthesise in between frame */

    if (decimate) {
	if (frames%2) {

	    /* odd frames use the original model parameters */

	    model_synth = model_2;
	    transition = 0;
	}
	else {
	    interp(&model_3, &model_1, &model_synth, &model_a, &model_b, 
		   &transition);

	    /* phase need to be supplied outside of this routine, e.g. via
	       a phase model */
	       
	    for(i=1; i<=model_synth.L; i++)
		model_synth.phi[i] = model_2.phi[i];
	}

	model_3 = model_2;
	model_2 = model_1;
	model_1 = model;
	model = model_synth;
    }

    /* 
       Simulate Wo quantisation noise
       model.Wo += 2.0*(PI/8000)*(1.0 - 2.0*(float)rand()/RAND_MAX);
       if (model.Wo > TWO_PI/20.0) model.Wo = TWO_PI/20.0;
       if (model.Wo < TWO_PI/160.0) model.Wo = TWO_PI/160.0;
       model.L = floor(PI/model.Wo);   
    */

    /* Synthesise speech */

    if (fout != NULL) {

	if (transition) {
	    synthesise(Sn_,&model_a,Pn,1);
	    synthesise(Sn_,&model_b,Pn,0);
	}
	else {
	    synthesise(Sn_,&model,Pn,1);
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

