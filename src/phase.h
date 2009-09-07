/*---------------------------------------------------------------------------*\
                                                                             
  FILE........: phase.h                                          
  AUTHOR......: David Rowe                                             
  DATE CREATED: 1/2/09                                                 
                                                                             
  Functions for modelling phase.
                                                                             
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

#ifndef __PHASE__
#define __PHASE__

#define PHASE_LPC_ORD 10

float phase_model_first_order(float aks[], COMP H[], int *i_min, COMP *min_Am);
void phase_synth_zero_order(float snr, COMP H[], float *prev_Wo, float *ex_phase);
void phase_synth_first_order(float snr, COMP H[], int i_min, COMP min_Am);

#endif
