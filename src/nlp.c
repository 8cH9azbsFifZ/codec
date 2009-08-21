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

/*---------------------------------------------------------------------------*\
                                                                             
 				DEFINES                                       
                                                                             
\*---------------------------------------------------------------------------*/

#define PMAX_M 600		/* maximum NLP analysis window size */
#define COEFF 0.95		/* noth filter parameter */
#define NTAP 48			/* Decimation LPF order */
#define PE_FFT_SIZE 512		/* DFT size for pitch estimation */
#define DEC 5			/* decimation factor */
#define SAMPLE_RATE 8000
#define PI 3.141592654		/* mathematical constant */
#define CNLP 0.5		/* post processor constant */

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

/*---------------------------------------------------------------------------*\
                                                                             
  void nlp()                                                                  
                                                                             
  Determines the pitch in samples using the NLP algorithm. Returns the 	      
  fundamental in Hz.						   	      
                                                                             
\*---------------------------------------------------------------------------*/

float nlp(Sn,n,m,d,pmin,pmax,pitch)
float Sn[];			/* input speech vector */
int n;				/* frames shift (no. new samples in Sn[]) */
int m;				/* analysis window size */
int d;				/* additional delay (used for testing) */
int pmin;			/* minimum pitch value */
int pmax;			/* maximum pitch value */
float *pitch;			/* estimated pitch */
{
  static float sq[PMAX_M];	/* squared speech samples */
  float notch;			/* current notch filter output */
  static float mem_x,mem_y;     /* memory for notch filter */
  static float mem_fir[NTAP];	/* decimation FIR filter memory */
  COMP Fw[PE_FFT_SIZE];		/* DFT of squared signal */

  int gmax_bin;			/* DFT bin where global maxima occurs */
  float gmax;			/* global maxima value */
  float lmax;			/* current local maxima value */
  int lmax_bin;			/* bin of current local maxima */
  float cmax;			/* chosen local maxima value */
  int cmax_bin;			/* bin of chosen local maxima */

  int mult;			/* current submultiple */
  int min_bin;			/* lowest possible bin */
  int bmin,bmax;		/* range of local maxima search */
  float thresh;			/* threshold for submultiple selection */

  float F0;			/* fundamental frequency */
  int i,j,b;

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

    for(j=0; j<NTAP-1; j++)
      mem_fir[j] = mem_fir[j+1];
    mem_fir[NTAP-1] = sq[i];

    sq[i] = 0.0;
    for(j=0; j<NTAP; j++)
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

  /* find global peak within limits, this corresponds to F0 estimate */

  gmax = 0.0;
  for(i=PE_FFT_SIZE*DEC/pmax; i<=PE_FFT_SIZE*DEC/pmin; i++) {
    if (Fw[i].real > gmax) {
      gmax = Fw[i].real;
      gmax_bin = i;
    }
  }

  /* Now post process estimate by searching submultiples */

  mult = 2;
  min_bin = PE_FFT_SIZE*DEC/pmax;
  thresh = CNLP*gmax;
  cmax_bin = gmax_bin;

  while(gmax_bin/mult >= min_bin) {

    b = gmax_bin/mult;			/* determine search interval */
    bmin = 0.8*b;
    bmax = 1.2*b;
    if (bmin < min_bin)
      bmin = min_bin;
      
    lmax = 0;
    for (b=bmin; b<=bmax; b++) 		/* look for maximum in interval */
      if (Fw[b].real > lmax) {
        lmax = Fw[b].real;
	lmax_bin = b;
      }

    if (lmax > thresh)
      if (lmax > Fw[lmax_bin-1].real && lmax > Fw[lmax_bin+1].real) {
	cmax = lmax;
	cmax_bin = lmax_bin;
      }

    mult++;
  }

  F0 = (float)cmax_bin*SAMPLE_RATE/(PE_FFT_SIZE*DEC);
  *pitch = SAMPLE_RATE/F0;

  /* Shift samples in buffer to make room for new samples */

  for(i=0; i<m-n+d; i++)
    sq[i] = sq[i+n];

  return(F0);  
}
    

