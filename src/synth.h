/*---------------------------------------------------------------------------*\
                                                                             
  FILE........: synth.h                                         
  AUTHOR......: David Rowe                                             
  DATE CREATED: 11/9/09                                                 
                                                                             
  Function for synthesising a speech signal in the frequency domain from      
  the sinusodal model parameters.                                             
                                                                             
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

#ifndef __SYNTH__
#define __SYNTH__

#include "sine.h"

void synthesise_mixed(float Pn[], MODEL *model, float Sn_[], int shift);
void synthesise_continuous_phase(float Pn[], MODEL *model, float Sn_[], 
				 int voiced, float *Wo_prev, float phi_prev[]);
#endif
