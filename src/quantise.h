/*---------------------------------------------------------------------------*\
                                                                             
  FILE........: quantise.h
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

#ifndef __QUANTISE__
#define __QUANTISE__

#define WO_BITS   7
#define WO_LEVELS (1<<WO_BITS)
#define E_BITS    5
#define E_LEVELS  (1<<E_BITS)
#define E_MIN_DB -10.0
#define E_MAX_DB  40.0

void quantise_init();
float lpc_model_amplitudes(float Sn[], float w[], MODEL *model, int order,
			   int lsp,float ak[]);
void aks_to_M2(float ak[], int order, MODEL *model, float E, float *snr);
float get_gmin(void);

int   encode_Wo(float Wo);
float decode_Wo(int index);

void encode_lsps(int indexes[], float lsp[], int order);
void decode_lsps(float lsp[], int indexes[], int order);

int encode_energy(float e);
float decode_energy(int index);

void encode_amplitudes(int    lsp_indexes[], 
		       int   *lpc_correction, 
		       int   *energy_index,
		       MODEL *model, 
		       float  Sn[], 
		       float  w[]);

float decode_amplitudes(MODEL *model, 
		       int lsp_indexes[],
		       int lpc_correction, 
		       int energy_index);

void pack(char bits[], int *nbit, int index, int index_bits);
int unpack(char bits[], int *nbit, int index_bits);

#endif
