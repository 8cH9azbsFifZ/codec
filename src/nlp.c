/*---------------------------------------------------------------------------*\
                                                 
  FILE........: nlp.c                                                   
  AUTHOR......: David Rowe                                      
  DATE CREATED: 23/3/93                                    
                                                         
  Non Linear Pitch (NLP) estimation functions.			  
                                                               
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

#include "nlp.h"
#include "sine.h"
#include "dump.h"
#include <assert.h>

/*---------------------------------------------------------------------------*\
                                                                             
 				DEFINES                                       
                                                                             
\*---------------------------------------------------------------------------*/

#define PMAX_M      600		/* maximum NLP analysis window size     */
#define COEFF       0.95	/* notch filter parameter               */
#define PE_FFT_SIZE 512		/* DFT size for pitch estimation        */
#define DEC         5		/* decimation factor                    */
#define SAMPLE_RATE 8000
#define PI          3.141592654	/* mathematical constant                */
#define T           0.1         /* threshold for local minima candidate */
#define F0_MAX      500

/*---------------------------------------------------------------------------*\
                                                                            
 				GLOBALS                                       
                                                                             
\*---------------------------------------------------------------------------*/

/* 48 tap 600Hz low pass FIR filter coefficients */

float nlp_fir[] = {
  -1.0818124e-03,
  -1.1008344e-03,
  -9.2768838e-04,
  -4.2289438e-04,
   5.5034190e-04,
   2.0029849e-03,
   3.7058509e-03,
   5.1449415e-03,
   5.5924666e-03,
   4.3036754e-03,
   8.0284511e-04,
  -4.8204610e-03,
  -1.1705810e-02,
  -1.8199275e-02,
  -2.2065282e-02,
  -2.0920610e-02,
  -1.2808831e-02,
   3.2204775e-03,
   2.6683811e-02,
   5.5520624e-02,
   8.6305944e-02,
   1.1480192e-01,
   1.3674206e-01,
   1.4867556e-01,
   1.4867556e-01,
   1.3674206e-01,
   1.1480192e-01,
   8.6305944e-02,
   5.5520624e-02,
   2.6683811e-02,
   3.2204775e-03,
  -1.2808831e-02,
  -2.0920610e-02,
  -2.2065282e-02,
  -1.8199275e-02,
  -1.1705810e-02,
  -4.8204610e-03,
   8.0284511e-04,
   4.3036754e-03,
   5.5924666e-03,
   5.1449415e-03,
   3.7058509e-03,
   2.0029849e-03,
   5.5034190e-04,
  -4.2289438e-04,
  -9.2768838e-04,
  -1.1008344e-03,
  -1.0818124e-03
};

float test_candidate_mbe(COMP Sw[], float f0);
extern int frames;

/*---------------------------------------------------------------------------*\
                                                                             
  void nlp()                                                                  
                                                                             
  Determines the pitch in samples using the NLP algorithm. Returns the
  fundamental in Hz.  Note that the actual pitch estimate is for the
  centre of the M sample Sn[] vector, not the current N sample input
  vector.  This is (I think) a delay of 2.5 frames with N=80 samples.
  You should align further analysis using this pitch estimate to be
  centred on the middle of Sn[].
                                                                             
\*---------------------------------------------------------------------------*/

float nlp(
  float  Sn[],			/* input speech vector */
  int    n,			/* frames shift (no. new samples in Sn[]) */
  int    m,			/* analysis window size */
  int    d,			/* additional delay (used for testing) */
  int    pmin,			/* minimum pitch value */
  int    pmax,			/* maximum pitch value */
  float *pitch,			/* estimated pitch period in samples */
  COMP   Sw[]                   /* Freq domain version of Sn[] */
)
{
  static float sq[PMAX_M];	/* squared speech samples */
  float  notch;			/* current notch filter output */
  static float mem_x,mem_y;     /* memory for notch filter */
  static float mem_fir[NLP_NTAP];/* decimation FIR filter memory */
  COMP   Fw[PE_FFT_SIZE];	/* DFT of squared signal */
  float  gmax;
 
  float candidate_f0;
  float f0,best_f0;		/* fundamental frequency */
  float e,e_min;                /* MBE cost function */
  int i,j;
  float e_hz[F0_MAX];
  int bin;

  /* Square, notch filter at DC, and LP filter vector */

  for(i=0; i<n; i++) 		/* square speech samples */
    sq[i+d+m-n] = Sn[i]*Sn[i];

  for(i=m-n+d; i<m+d; i++) {	/* notch filter at DC */
    notch = sq[i] - mem_x;
    notch += COEFF*mem_y;
    mem_x = sq[i];
    mem_y = notch;
    sq[i] = notch;
  }

  for(i=m-n+d; i<m+d; i++) {	/* FIR filter vector */

    for(j=0; j<NLP_NTAP-1; j++)
      mem_fir[j] = mem_fir[j+1];
    mem_fir[NLP_NTAP-1] = sq[i];

    sq[i] = 0.0;
    for(j=0; j<NLP_NTAP; j++)
      sq[i] += mem_fir[j]*nlp_fir[j];
  }

  /* Decimate and DFT */

  for(i=0; i<PE_FFT_SIZE; i++) {
    Fw[i].real = 0.0;
    Fw[i].imag = 0.0;
  }
  for(i=0; i<m/DEC; i++)
    Fw[i].real = sq[i*DEC]*(0.5 - 0.5*cos(2*PI*i/(m/DEC-1)));
  four1(&Fw[-1].imag,PE_FFT_SIZE,1);
  for(i=0; i<PE_FFT_SIZE; i++)
    Fw[i].real = Fw[i].real*Fw[i].real + Fw[i].imag*Fw[i].imag;

  dump_Fw(Fw);

  /* find global peak */

  gmax = 0.0;
  for(i=PE_FFT_SIZE*DEC/pmax; i<=PE_FFT_SIZE*DEC/pmin; i++) {
    if (Fw[i].real > gmax) {
      gmax = Fw[i].real;
    }
  }

  /* Now look for local maxima.  Each local maxima is a candidate
     that we test using the MBE pitch estimation algotithm */

  for(i=0; i<F0_MAX; i++)
      e_hz[i] = -1;
  e_min = 1E32;
  best_f0 = 50;
  for(i=PE_FFT_SIZE*DEC/pmax; i<=PE_FFT_SIZE*DEC/pmin; i++) {
    if ((Fw[i].real > Fw[i-1].real) && (Fw[i].real > Fw[i+1].real)) {

	/* local maxima found, lets test if it's big enough */

	if (Fw[i].real > T*gmax) {

	    /* OK, sample MBE cost function over +/- 10Hz range in 2.5Hz steps */

	    candidate_f0 = (float)i*SAMPLE_RATE/(PE_FFT_SIZE*DEC);
	    if (frames == 29)
	      printf("candidate F0: %f\n", candidate_f0);
	    for(f0=candidate_f0-20; f0<=candidate_f0+20; f0+= 2.5) {
		e = test_candidate_mbe(Sw, f0);
		bin = floor(f0); assert((bin > 0) && (bin < F0_MAX));
		e_hz[bin] = e;
		if (frames == 29)
		    printf("f0: %f e: %f e_min: %f best_f0: %f\n", 
			   f0, e, e_min, best_f0);
		if (e < e_min) {
		    e_min = e;
		    best_f0 = f0;
		}
	    }

	}
    }
  }
  dump_e(e_hz);

  /* Shift samples in buffer to make room for new samples */

  for(i=0; i<m-n+d; i++)
    sq[i] = sq[i+n];

  /* return pitch and F0 estimate */

  *pitch = (float)SAMPLE_RATE/best_f0;
  return(best_f0);  
}
    
/*---------------------------------------------------------------------------*\
                                                                             
  test_candidate_mbe()          
                                                                             
  Returns the error of the MBE cost function for the input f0.  

  Note: I think a lot of the operations below can be simplified as
  W[].imag = 0 and has been normalised such that den always equals 1.
                                                                             
\*---------------------------------------------------------------------------*/

float test_candidate_mbe(
    COMP  Sw[],
    float f0
)
{
    COMP  Sw_[FFT_ENC];   /* DFT of all voiced synthesised signal */
    int   l,al,bl,m;      /* loop variables */
    COMP  Am;             /* amplitude sample for this band */
    int   offset;         /* centers Hw[] about current harmonic */
    float den;            /* denominator of Am expression */
    float error;          /* accumulated error between originl and synthesised */
    float Wo;             /* current "test" fundamental freq. */
    int   L;              /* number of bands */

    Wo = f0*(2*PI/SAMPLE_RATE);
    L = floor(PI/Wo);;   

    error = 0.0;

    /* Just test across the harmonics in the first 1000 Hz (L/4) */

    for(l=1; l<L/4; l++) {
	Am.real = 0.0;
	Am.imag = 0.0;
	den = 0.0;
	al = ceil((l - 0.5)*Wo*FFT_ENC/TWO_PI);
	bl = ceil((l + 0.5)*Wo*FFT_ENC/TWO_PI);

	/* Estimate amplitude of harmonic assuming harmonic is totally voiced */

	for(m=al; m<bl; m++) {
	    offset = FFT_ENC/2 + m - l*Wo*FFT_ENC/TWO_PI + 0.5;
	    Am.real += Sw[m].real*W[offset].real + Sw[m].imag*W[offset].imag;
	    Am.imag += Sw[m].imag*W[offset].real - Sw[m].real*W[offset].imag;
	    den += W[offset].real*W[offset].real + W[offset].imag*W[offset].imag;
        }

        Am.real = Am.real/den;
        Am.imag = Am.imag/den;

        /* Determine error between estimated harmonic and original */

        for(m=al; m<bl; m++) {
	    offset = FFT_ENC/2 + m - l*Wo*FFT_ENC/TWO_PI + 0.5;
	    Sw_[m].real = Am.real*W[offset].real - Am.imag*W[offset].imag;
	    Sw_[m].imag = Am.real*W[offset].imag + Am.imag*W[offset].real;
	    error += (Sw[m].real - Sw_[m].real)*(Sw[m].real - Sw_[m].real);
	    error += (Sw[m].imag - Sw_[m].imag)*(Sw[m].imag - Sw_[m].imag);
	}
    }

    return error;
}


