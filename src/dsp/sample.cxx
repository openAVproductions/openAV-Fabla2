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

#include "sample.hxx"

#include "pad.hxx"
#include "plotter.hxx"

#include <sndfile.h>
#include <stdio.h>
#include <string.h>

#ifdef FABLA2_COMPONENT_TEST
#include "tests/qunit.hxx"
extern QUnit::UnitTest qunit;
#endif 

namespace Fabla2
{

static void fabla2_deinterleave( int size, const float* all, std::vector<float>& L, std::vector<float>& R )
{
  L.resize( size / 2 );
  R.resize( size / 2 );
  
  float* l = &L[0];
  float* r = &R[0];
#ifdef FABLA2_COMPONENT_TEST
  printf("deinterlacing... size = %i\n", size );
#endif
  // de-interleave samples
  for( int i = 0; i + 1 < size; i++ )
  {
    *l++ = *all++;
    *r++ = *all++;
  }
}

void Sample::recacheWaveform()
{
#ifdef FABLA2_COMPONENT_TEST
  printf("recaching waveform... \n" );
#endif
  int sampsPerPix = frames / FABLA2_UI_WAVEFORM_PX;
  
  // loop over each pixel value we need
  for( int p = 0; p < FABLA2_UI_WAVEFORM_PX; p++ )
  {
    float average = 0.f;
    
    // calc value for this pixel
    for( int i = 0; i < sampsPerPix; i++ )
    {
      int tmpIndex = (p * sampsPerPix) + i;
      
      float tmp = audioMono[tmpIndex];
      
      if ( channels == 2 )
      {
        tmp += audioStereoRight[tmpIndex];
      }
      average += tmp;
    }
    
    // downscale by the number of samples, and if stereo
    if( channels == 2 )
      average /= 2;
    
    waveformData[p] = (average / sampsPerPix);
  }
}

void Sample::init()
{
  gain  = 0.75;
  pitch = 0.5;
  pan   = 0.5;
  startPoint = 0.0;
  
  memset( waveformData, 0 , sizeof(float) * FABLA2_UI_WAVEFORM_PX );
  
  // set to true so we recacheWaveform() when requested for it
  dirty = true;
}

Sample::Sample( Fabla2DSP* d, int rate, int size, float* data ) :
  dsp( d ),
  sr(rate),
  channels( 2 ),
  frames( size / 2 ),
  velLow( 0 ),
  velHigh( 127 ),
  pitch( 0 ),
  gain ( 0.75 ),
  pan  ( 0 )
{
  //memset( waveformData, 0, sizeof(float)*FABLA2_UI_WAVEFORM_PX);
  
#ifdef FABLA2_COMPONENT_TEST
  printf("%s\n", __PRETTY_FUNCTION__ );
#endif
  
  //memcpy( &audioMono[0], data, sizeof(float) * size );
  fabla2_deinterleave( size, data, audioMono, audioStereoRight );
  
  init();
}

Sample::Sample( Fabla2DSP* d, int rate, std::string n, std::string path  ) :
  dsp( d ),
  sr(rate),
  name( n ),
  channels( 0 ),
  frames( 0 ),
  velLow( 0 ),
  velHigh( 127 ),
  pitch( 0 ),
  gain ( 0.5 ),
  pan  ( 0.5 )
{
  SF_INFO info;
  memset( &info, 0, sizeof( SF_INFO ) );
  SNDFILE* const sndfile = sf_open( path.c_str(), SFM_READ, &info);
  if ( !sndfile )
  {
    printf("Failed to open sample '%s'\n", path.c_str() );
    return;
  }
  
  channels = info.channels;
  frames   = info.frames;
  
  if( channels > 2 || channels < 0 )
  {
    printf("Error loading sample %s, channels >= 2\n", path.c_str() );
    return;
  }
  
  // tmp buffer for loading
  std::vector<float> audio;
  
  // used to load into from disk. If mono, load directly into this buffer
  // if stereo, load into audio buffer, and then de-interleave samples into
  // the two buffers
  float* loadBuffer = 0;
  
  if( channels == 1 )
  {
    audioMono.resize( frames );
    loadBuffer = &audioMono.at(0);
  }
  else if( channels == 2 )
  {
    audio.resize( frames * channels );
    loadBuffer = &audio.at(0);
  }
  
  // read from disk
  sf_seek(sndfile, 0ul, SEEK_SET);
  int samplRead = sf_read_float( sndfile, loadBuffer, info.frames *channels );
  sf_close(sndfile);
  
  if( channels == 2 )
  {
    audioMono.resize( frames );
    audioStereoRight.resize( frames );
    fabla2_deinterleave( frames, loadBuffer, audioMono, audioStereoRight );
  }
  
  init();
  
#ifdef FABLA2_COMPONENT_TEST
  if( false )
  {
    Plotter::plot( path, frames * channels, loadBuffer );
    printf("Sample %s loaded OK: Channels = %i, Frames = %i\n", path.c_str(), channels, frames );
  }
  QUNIT_IS_TRUE( info.frames > 0 );
  QUNIT_IS_TRUE( samplRead == info.frames * info.channels );
#endif
}

void Sample::velocity( int low, int high )
{
  velLow  = low;
  velHigh = high;
}

const float* Sample::getAudio( int chnl )
{
  if( channels == 2 && chnl == 1 && audioStereoRight.size() > 0 )
  {
    return &audioStereoRight[0];
  }
  return &audioMono[0];
}

bool Sample::velocity( int vel )
{
  if( vel > velLow &&
      vel <= velHigh )
  {
    return true;
  }
  
  return false;
}

Sample::~Sample()
{
#ifdef FABLA2_COMPONENT_TEST
  printf("%s\n", __PRETTY_FUNCTION__ );
#endif
}

};
