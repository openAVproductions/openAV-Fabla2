/*
 * Author: Harry van Haaren 2014
 *         harryhaaren@gmail.com
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 */

#include "voice.hxx"

#include "fabla2.hxx"
#include <math.h>

namespace Fabla2
{

Voice::Voice( Fabla2DSP* d, int r ) :
  dsp( d ),
  sr ( r ),
  active_( false )
{
}

void Voice::play()
{
  active_ = true;
  phase = 0;
}

void Voice::process()
{
  float* outL = dsp->outL;
  float* outR = dsp->outR;
  
  for( int i = 0; i < dsp->nframes; i++ )
  {
    const float freq = 110;
    const float sampsPerCycle = sr / freq;
    const float phaseInc = (1.f / sampsPerCycle);
    
    *outL++ += sin( phase * 3.1415 ) * 0.2;
    *outR++ += sin( phase * 3.1415 ) * 0.2;
    
    phase += phaseInc;
  }
}

Voice::~Voice()
{
}


}; // Fabla2
