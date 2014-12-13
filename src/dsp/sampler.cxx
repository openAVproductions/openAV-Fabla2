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

#include "sampler.hxx"

#include "pad.hxx"

namespace Fabla2
{

Sampler::Sampler( Fabla2DSP* d, int rate ) :
  dsp( d ),
  sr(rate),
  
  pad( 0 ),
  sample( 0 ),
  
  playIndex(0)
{
}

void Sampler::play( int velocity, Pad* p )
{
  pad = p;
  
  sample = pad->getPlaySample( velocity );
  
  // trigger audio playback here
  playIndex = 0;
}

void Sampler::process(int nframes, float* L, float* R)
{
  
}

Sampler::~Sampler()
{
}

};

