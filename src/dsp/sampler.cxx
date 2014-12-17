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
#include "fabla2.hxx"
#include "ports.hxx"
#include "sample.hxx"
#include <stdio.h>

namespace Fabla2
{

Sampler::Sampler( Fabla2DSP* d, int rate ) :
  dsp( d ),
  sr(rate),
  
  pad( 0 ),
  sample( 0 ),
  
  playheadDelta(2),
  playIndex(0)
{
}

void Sampler::play( Pad* p, int velocity )
{
  pad = p;
  
  sample = pad->getPlaySample( velocity );
  
  if( !sample )
  {
#ifdef FABLA2_COMPONENT_TEST
  printf("%s ERROR : pad->getPlaySample() returned 0!\n", __PRETTY_FUNCTION__ );
#endif
    return;
  }
  
  // trigger audio playback here
  playIndex = 0;
}

int Sampler::process(int nframes, float* L, float* R)
{
  if( !sample )
  {
    // no sample loaded on the Pad that this Sampler represents
    return 1;
  }
  const int    chans = sample->getChannels();
  const int    frames= sample->getFrames();
  const float* audio = sample->getAudio();
  
  // return immidiatly if we are finished playing the sample
  if( playIndex > frames * chans )
    return 1;
  
  // playheadDelta with master-pitch offset: can be +- 12.
  float mstr = *dsp->controlPorts[Fabla2::MASTER_PITCH];
  float pd = playheadDelta + mstr / 12.f; // 1 -> 2 range (double pitch)
  if( mstr < 0.000 )
    pd = playheadDelta + mstr / 24.f; // 1 -> 0.5 range (half pitch)
  
  if( chans == 1 )
  {
    for(int i = 0; i < nframes; i++ )
    {
      *L++ = audio[(int)playIndex];
      *R++ = audio[(int)playIndex];
      playIndex += pd;
      
      if( playIndex > frames * chans )
        return 1;
    }
  }
  else if( chans == 2 )
  {
    for(int i = 0; i < nframes; i++ )
    {
      *L++ = audio[(int)playIndex];
      playIndex +=pd;
      
      *R++ = audio[(int)playIndex];
      playIndex += pd;
      
      if( playIndex > frames * chans)
      {
        return 1;
      }
    }
  }
  else
  {
    // return if we don't know how to deal with this channel count
    return 1;
  }
  
  // normal return path: not done, keep calling this
  return 0;
}

Sampler::~Sampler()
{
#ifdef FABLA2_COMPONENT_TEST
  printf("%s\n", __PRETTY_FUNCTION__ );
#endif
}

};

