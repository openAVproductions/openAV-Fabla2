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
  
  playheadDelta(1),
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
  
  // playheadDelta with sample and master pitch offset: can be +- 12.
  float mstr = sample->pitch * 24 -12 + *dsp->controlPorts[Fabla2::MASTER_PITCH];
  float pd = playheadDelta + mstr / 12.f; // 1 -> 2 range (double pitch)
  if( mstr < 0.000 )
    pd = playheadDelta + mstr / 24.f; // 1 -> 0.5 range (half pitch)
  
  if( chans == 1 )
  {
    for(int i = 0; i < nframes; i++ )
    {
      // linear interpolation between samples
      float x0 = playIndex - int(playIndex);
      int x1 = int(playIndex);
      int x2 = x1 + 1; // next sample
      float y1 = audio[x1];
      float y2 = audio[x2];
      float out = y1 + ( y2 - y1 ) * x0;
      
      *L++ = out;
      *R++ = out;
      playIndex += pd;
      
      if( playIndex > frames * chans )
        return 1;
    }
  }
  else if( chans == 2 )
  {
    for(int i = 0; i < nframes; i++ )
    {
      // stereo linear interpolatation between samples
      float x0 = playIndex - int(playIndex);
      
      int l1 = int(playIndex);
      int r1 = int(playIndex);
      
      int l2 = l1 + 2; // next sample
      int r2 = r1 + 2; // and after sample
      
      // FIXME: Optimize this
      float ly1 = audio[l1];
      float ly2 = audio[l2];
      
      float ry1 = audio[r1];
      float ry2 = audio[r2];
      
      float lOut = ly1 + ( ly2 - ly1 ) * x0;
      float rOut = ry1 + ( ry2 - ry1 ) * x0;
      
      *L++ = lOut;//audio[(int)playIndex];
      *R++ = rOut;//audio[(int)playIndex];
      
      //playIndex += pd;
      //playIndex +=pd;
      playIndex += pd * 2; // move to next frame
      
      if( playIndex + 4 > frames * chans)
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

