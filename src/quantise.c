/*---------------------------------------------------------------------------*\
                                                                             
  FILE........: quantise.c
  AUTHOR......: David Rowe                                                          
  DATE CREATED: 31/5/92                                                       
                                                                             
  Quantisation functions for the sinusoidal coder.  
                                                                             
\*---------------------------------------------------------------------------*/

/*
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
#include <ctype.h>
#include "sine.h"
#include "quantise.h"
#include "lpc.h"
#include "dump.h"
#include <lsp.h>
#include <speex_bits.h>
#include <quant_lsp.h>

#define MAX_ORDER 20

#define LPC_FLOOR 0.0002        /* autocorrelation floor */
#define LSP_DELTA1 0.2          /* grid spacing for LSP root searches */

/* Speex lag window */

const float lag_window[11] = {
   1.00000, 0.99716, 0.98869, 0.97474, 0.95554, 0.93140, 0.90273, 0.86998, 
   0.83367, 0.79434, 0.75258
};

/* 10 + 9 + 9 + 9 = 37 bit quantiser (1850 bit/s with 20ms frames) */
 
#define LSP_12_K  2
#define LSP_12_M  1024
#define LSP_34_K  2
#define LSP_34_M  512
#define LSP_57_K  3
#define LSP_57_M  512
#define LSP_810_K 3
#define LSP_810_M 512

static float cb12[LSP_12_K*LSP_12_M];
static float cb34[LSP_34_K*LSP_34_M];
static float cb57[LSP_57_K*LSP_57_M];
static float cb810[LSP_810_K*LSP_810_M];

/*---------------------------------------------------------------------------*\
									      
  quantise_uniform

  Simulates uniform quantising of a float.

\*---------------------------------------------------------------------------*/

void quantise_uniform(float *val, float min, float max, int bits)
{
    int   levels = 1 << (bits-1);
    float norm;
    int   index;

    /* hard limit to quantiser range */

    printf("min: %f  max: %f  val: %f  ", min, max, val[0]);
    if (val[0] < min) val[0] = min;
    if (val[0] > max) val[0] = max;

    norm = (*val - min)/(max-min);
    printf("%f  norm: %f  ", val[0], norm);
    index = fabs(levels*norm + 0.5);

    *val = min + index*(max-min)/levels;

    printf("index %d  val_: %f\n", index, val[0]);
}

/*---------------------------------------------------------------------------*\
									      
  lspd_quantise

  Simulates differential lsp quantiser

\*---------------------------------------------------------------------------*/

void lsp_quantise(
  float lsp[], 
  float lsp_[],
  int   order
) 
{
    int   i;
    float dlsp[MAX_ORDER];
    float dlsp_[MAX_ORDER];

    dlsp[0] = lsp[0];
    for(i=1; i<order; i++)
	dlsp[i] = lsp[i] - lsp[i-1];

    for(i=0; i<order; i++)
	dlsp_[i] = dlsp[i];

    quantise_uniform(&dlsp_[0], 0.1, 0.5, 5);

    lsp_[0] = dlsp_[0];
    for(i=1; i<order; i++)
	lsp_[i] = lsp_[i-1] + dlsp_[i];
}

/*---------------------------------------------------------------------------*\

  scan_line()

  This function reads a vector of floats from a line in a text file.

\*---------------------------------------------------------------------------*/

void scan_line(FILE *fp, float f[], int n)
/*  FILE   *fp;		file ptr to text file 		*/
/*  float  f[]; 	array of floats to return 	*/
/*  int    n;		number of floats in line 	*/
{
    char   s[MAX_STR];
    char   *ps,*pe;
    int	   i;

    fgets(s,MAX_STR,fp);
    ps = pe = s;
    for(i=0; i<n; i++) {
	while( isspace(*pe)) pe++;
	while( !isspace(*pe)) pe++;
	sscanf(ps,"%f",&f[i]);
	ps = pe;
    }
}

/*---------------------------------------------------------------------------*\

  load_cb

  Quantises vec by choosing the nearest vector in codebook cb, and
  returns the vector index.  The squared error of the quantised vector
  is added to se.

\*---------------------------------------------------------------------------*/

void load_cb(char *filename, float *cb, int k, int m)
{
    FILE *ftext;
    int   lines;
    int   i;

    ftext = fopen(filename,"rt");
    if (ftext == NULL) {
	printf("Error opening text file: %s\n",filename);
	exit(1);
    }

    lines = 0;
    for(i=0; i<m; i++) {
	scan_line(ftext, &cb[k*lines++], k);
    }

    fclose(ftext);
}

void quantise_init()
{
    load_cb("../unittest/lsp12.txt", cb12,  LSP_12_K,  LSP_12_M);
    load_cb("../unittest/lsp34.txt", cb34,  LSP_34_K,  LSP_34_M);
    load_cb("../unittest/lsp57.txt", cb57, LSP_57_K, LSP_57_M);
    load_cb("../unittest/lsp810.txt", cb810, LSP_810_K, LSP_810_M);
}

/*---------------------------------------------------------------------------*\

  quantise

  Quantises vec by choosing the nearest vector in codebook cb, and
  returns the vector index.  The squared error of the quantised vector
  is added to se.

\*---------------------------------------------------------------------------*/

long quantise(float cb[], float vec[], int k, int m, float *se)
/* float   cb[][K];	current VQ codebook		*/
/* float   vec[];	vector to quantise		*/
/* int	   k;		dimension of vectors		*/
/* int     m;		size of codebook		*/
/* float   *se;		accumulated squared error 	*/
{
   float   e;		/* current error		*/
   long	   besti;	/* best index so far		*/
   float   beste;	/* best error so far		*/
   long	   j;
   int     i;

   besti = 0;
   beste = 1E32;
   for(j=0; j<m; j++) {
	e = 0.0;
	for(i=0; i<k; i++)
	    e += pow(cb[j*k+i]-vec[i],2.0);
	if (e < beste) {
	    beste = e;
	    besti = j;
	}
   }

   *se += beste;

   return(besti);
}

/*---------------------------------------------------------------------------*\
									      
  lpc_model_amplitudes

  Derive a LPC model for amplitude samples then estimate amplitude samples
  from this model with optional LSP quantisation.

  Returns the spectral distortion for this frame.

\*---------------------------------------------------------------------------*/

float lpc_model_amplitudes(
  float  Sn[],			/* Input frame of speech samples */
  MODEL *model,			/* sinusoidal model parameters */
  int    order,                 /* LPC model order */
  int    lsp_quantisation,      /* optional LSP quantisation if non-zero */
  float  ak[]                   /* output aks */
)
{
  float Wn[M];
  float R[MAX_ORDER+1];
  float E;
  int   i;
  float snr;	
  float lsp[MAX_ORDER];
  int   roots;            /* number of LSP roots found */
  int   index;
  float se;

  for(i=0; i<M; i++)
      Wn[i] = Sn[i]*w[i];
  autocorrelate(Wn,R,M,order);

  levinson_durbin(R,ak,order);
  E = 0.0;
  for(i=0; i<=order; i++)
      E += ak[i]*R[i];
  
  if (lsp_quantisation) {
    roots = lpc_to_lsp(&ak[1], order, lsp, 10, LSP_DELTA1, NULL);

    index = quantise(cb12, &lsp[0], LSP_12_K, LSP_12_M, &se);
    lsp[0] = cb12[index*LSP_12_K+0];
    lsp[1] = cb12[index*LSP_12_K+1];
    
    index = quantise(cb34, &lsp[2], LSP_34_K, LSP_34_M, &se);
    lsp[2] = cb34[index*LSP_34_K+0];
    lsp[3] = cb34[index*LSP_34_K+1];
    
    index = quantise(cb57, &lsp[4], LSP_57_K, LSP_57_M, &se);
    lsp[4] = cb57[index*LSP_57_K+0];
    lsp[5] = cb57[index*LSP_57_K+1];
    lsp[6] = cb57[index*LSP_57_K+2];
    
    index = quantise(cb810, &lsp[7], LSP_810_K, LSP_810_M, &se);
    lsp[7] = cb810[index*LSP_810_K+0];
    lsp[8] = cb810[index*LSP_810_K+1];
    lsp[9] = cb810[index*LSP_810_K+2];

    lsp_to_lpc(lsp, &ak[1], order, NULL);
    dump_lsp(lsp);
  }

  aks_to_M2(ak,order,model,E,&snr);   /* {ak} -> {Am} LPC decode */

  return snr;
}

/*---------------------------------------------------------------------------*\
                                                                         
   aks_to_M2()                                                             
                                                                         
   Transforms the linear prediction coefficients to spectral amplitude    
   samples.  This function determines A(m) from the average energy per    
   band using an FFT.                                                     
                                                                        
\*---------------------------------------------------------------------------*/

void aks_to_M2(
  float ak[],	/* LPC's */
  int   order,
  MODEL *model,	/* sinusoidal model parameters for this frame */
  float E,	/* energy term */
  float *snr	/* signal to noise ratio for this frame in dB */
)
{
  COMP Pw[FFT_DEC];	/* power spectrum */
  int i,m;		/* loop variables */
  int am,bm;		/* limits of current band */
  float r;		/* no. rads/bin */
  float Em;		/* energy in band */
  float Am;		/* spectral amplitude sample */
  float signal, noise;

  r = TWO_PI/(FFT_DEC);

  /* Determine DFT of A(exp(jw)) --------------------------------------------*/

  for(i=0; i<FFT_DEC; i++) {
    Pw[i].real = 0.0;
    Pw[i].imag = 0.0;
  }

  for(i=0; i<=order; i++)
    Pw[i].real = ak[i];
  four1(&Pw[-1].imag,FFT_DEC,1);

  /* Determine power spectrum P(w) = E/(A(exp(jw))^2 ------------------------*/

  for(i=0; i<FFT_DEC/2; i++)
    Pw[i].real = E/(Pw[i].real*Pw[i].real + Pw[i].imag*Pw[i].imag);
  dump_Pw(Pw);

  /* Determine magnitudes by linear interpolation of P(w) -------------------*/

  signal = noise = 0.0;
  for(m=1; m<=model->L; m++) {
    am = floor((m - 0.5)*model->Wo/r + 0.5);
    bm = floor((m + 0.5)*model->Wo/r + 0.5);
    Em = 0.0;

    for(i=am; i<bm; i++)
      Em += Pw[i].real;
    Am = sqrt(Em);

    signal += pow(model->A[m],2.0);
    noise  += pow(model->A[m] - Am,2.0);
    model->A[m] = Am;
  }
  *snr = 10.0*log10(signal/noise);
}
