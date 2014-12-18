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

#include <math.h>
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
#ifdef FABLA2_COMPONENT_TEST
  printf("%s\n", __PRETTY_FUNCTION__ );
#endif
}

void Sampler::play( Pad* p, int velocity )
{
#ifdef FABLA2_COMPONENT_TEST
  printf("%s : Pad ID %i\n", __PRETTY_FUNCTION__, p->ID() );
#endif
  pad = p;
  
  sample = pad->getPlaySample( velocity );
  
  // trigger audio playback here
  playIndex = sample->startPoint;
  
  if( !sample )
  {
#ifdef FABLA2_COMPONENT_TEST
  printf("%s ERROR : pad->getPlaySample() returned 0!\n", __PRETTY_FUNCTION__ );
#endif
    return;
  }
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
  
  // return immidiatly if we are finished playing the sample
  if( playIndex >= frames )
  {
    return 1;
  }
  
  // playheadDelta with sample and master pitch offset: can be +- 12.
#ifdef FABLA2_COMPONENT_TEST
  float mstr = sample->pitch * 24.f - 12; // ignore DSP, for testing it is 0x0
#else
  float mstr = sample->pitch * 24.f - 12 + *dsp->controlPorts[Fabla2::MASTER_PITCH];
#endif 
  float pd = playheadDelta + mstr / 24.f; // 1 -> 2 range (double pitch)
  if( mstr < 0.000 )
    pd = playheadDelta + mstr / 48.f; // 1 -> 0.5 range (half pitch)

//#define DB_CO(g) ((g) > -90.0f ? powf(10.0f, (g) * 0.05f) : 0.0f)
  //const float volMultiply = DB_CO(sample->gain);
  float volMultiply = pow( sample->gain, 3 );
  
  float panL = cos(sample->pan * 3.14/2.f);
  float panR = sin(sample->pan * 3.14/2.f);
  
  panL *= volMultiply;
  panR *= volMultiply;
  
  //printf("%f, %f, volMultiply = %f\n", panL, panR, volMultiply );
  
  if( chans == 1 )
  {
    const float* audio = sample->getAudio(0);
    for(int i = 0; i < nframes; i++ )
    {
      // cubic 4-point Hermite-curve interpolation:
      // http://musicdsp.org/showone.php?id=49
      
      int inpos = playIndex;
      float finpos = playIndex - (int)playIndex;
      float xm1 = audio[inpos    ];
      float x0  = audio[inpos + 1];
      float x1  = audio[inpos + 2];
      float x2  = audio[inpos + 3];
      float a = (3 * (x0-x1) - xm1 + x2) / 2;
      float b = 2*x1 + xm1 - (5*x0 + x2) / 2;
      float c = (x1 - xm1) / 2;
      float out = (((a * finpos) + b) * finpos + c) * finpos + x0;
      
      *L++ = out * panL;
      *R++ = out * panR;
      playIndex += pd;
      
      if( playIndex + 4 > frames )
        return 1;
    }
  }
  else if( chans == 2 )
  {
    const float* audioL = sample->getAudio(0);
    const float* audioR = sample->getAudio(1);
    
    for(int i = 0; i < nframes; i++ )
    {
      // cubic 4-point Hermite-curve interpolation:
      // http://musicdsp.org/showone.php?id=49
      {
        int inpos = playIndex;
        float finpos = playIndex - (int)playIndex;
        float xm1 = audioL[inpos    ];
        float x0  = audioL[inpos + 1];
        float x1  = audioL[inpos + 2];
        float x2  = audioL[inpos + 3];
        float a = (3 * (x0-x1) - xm1 + x2) / 2;
        float b = 2*x1 + xm1 - (5*x0 + x2) / 2;
        float c = (x1 - xm1) / 2;
        *L++ = ((((a * finpos) + b) * finpos + c) * finpos + x0) * panL;
      }
      {
        int inpos = playIndex;
        float finpos = playIndex - (int)playIndex;
        float xm1 = audioR[inpos    ];
        float x0  = audioR[inpos + 1];
        float x1  = audioR[inpos + 2];
        float x2  = audioR[inpos + 3];
        float a = (3 * (x0-x1) - xm1 + x2) / 2;
        float b = 2*x1 + xm1 - (5*x0 + x2) / 2;
        float c = (x1 - xm1) / 2;
        *R++ = ((((a * finpos) + b) * finpos + c) * finpos + x0) * panR;
      }
      
      playIndex += pd;
      
      if( playIndex + 4 > frames )
      {
        return 1;
      }
      
      
      
      
      /*
      int inpos = (int)playIndex;
      float finpos = playIndex - inpos;
      
      // Left channel
      {
        float xm1 = audioL[inpos + 0];
        float x0  = audioL[inpos + 1];
        float x1  = audioL[inpos + 2];
        float x2  = audioL[inpos + 3];
        float a = (3 * (x0-x1) - xm1 + x2) / 2;
        float b = 2*x1 + xm1 - (5*x0 + x2) / 2;
        float c = (x1 - xm1) / 2;
        *L++ = (((a * finpos) + b) * finpos + c) * finpos + x0;
      }
      // right channel
      {
        float xm1 = audioR[inpos + 0];
        float x0  = audioR[inpos + 1];
        float x1  = audioR[inpos + 2];
        float x2  = audioR[inpos + 3];
        float a = (3 * (x0-x1) - xm1 + x2) / 2;
        float b = 2*x1 + xm1 - (5*x0 + x2) / 2;
        float c = (x1 - xm1) / 2;
        *L++ = (((a * finpos) + b) * finpos + c) * finpos + x0;
      }
      
      // move forward 2 frames
      playIndex += pd;
      */
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

