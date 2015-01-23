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

#include "fabla2.hxx"

// for basename
#include <libgen.h>

#include <math.h>
#include <stdio.h>
#include <string.h>

#include "pad.hxx"
#include "plotter.hxx"

#include <sndfile.h>
#include <sndfile.hh>

#include <samplerate.h>

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
  for( int i = 0; i + 1 < size / 2; i++ )
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
  
  printf("Recache waveform with %i frames\n", frames );
  
  memset( waveformData, 0 , sizeof(float) * FABLA2_UI_WAVEFORM_PX );
  
  printf("Recache returning!\n" );
  return;
  
  int sampsPerPix = frames / FABLA2_UI_WAVEFORM_PX;
  
  float highestPeak = 0.f;
  
  // counts pixels in output waveform
  int p = 0;
  
  for( int f = 0; f < frames; f++ )
  {
    float tmp = audioMono[f];
    
    if ( channels == 2 )
      tmp += audioStereoRight[f];
    
    float t = fabsf(tmp);
    
    if( t > fabsf( waveformData[p] ) )
      waveformData[p] = t;
    
    p = f / sampsPerPix;
    if( p >= FABLA2_UI_WAVEFORM_PX )
      p = FABLA2_UI_WAVEFORM_PX - 1;
  }
  
  printf("RECACHE: done, frames %i, sampsPerPx %i,  p %i\n", frames, sampsPerPix, p );
  
  Plotter::plot( "forLoop", FABLA2_UI_WAVEFORM_PX, &waveformData[0] );
  
  /*
  // loop over each pixel value we need
  for( int p = 0; p < FABLA2_UI_WAVEFORM_PX; p++ )
  {
    float average = 0.f;
    
    printf("WF px %i : sampsPerPix %i\n", p, sampsPerPix );
    
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
    
    float tmp = (average / sampsPerPix);
    
    if( fabsf(tmp) > highestPeak )
      highestPeak = fabsf(tmp);
    
    waveformData[p] = tmp;
  }
  */
  
  float normalizeFactor = 1;
  if( highestPeak > 0.001 ) // avoid divide-by-zero
    normalizeFactor = (1.f / highestPeak);
  printf("normalizing with highestPeak %f: nomalizeFactor %f\n", highestPeak, normalizeFactor );
  
  // loop over each pixel and normalize it
  for( int p = 0; p < FABLA2_UI_WAVEFORM_PX; p++ )
  {
    waveformData[p] = (waveformData[p] * normalizeFactor) - 1.0;
  }
  
  Plotter::plot( getName(), FABLA2_UI_WAVEFORM_PX, &waveformData[0] );
  
  printf("Recache waveform with %i frames : DONE!\n", frames );
}


void Sample::resample( int fromSr, std::vector<float>& buf )
{
  /// resample audio
  printf("Resampling from %i to %i\n", fromSr, sr);
  
  float resampleRatio = float( sr ) / fromSr;
  std::vector<float> resampled( buf.size() * resampleRatio );
  
  SRC_DATA data;
  data.data_in  = &buf[0];
  data.data_out = &resampled[0];
  
  data.input_frames = buf.size();
  data.output_frames = buf.size() * resampleRatio;
  
  data.end_of_input = 0;
  data.src_ratio = resampleRatio;
  
  int q = SRC_SINC_FASTEST;
  /*
  switch( resampleQuality )
  {
    case 0: q = SRC_LINEAR;             break;
    case 1: q = SRC_SINC_FASTEST;       break;
    case 2: q = SRC_SINC_BEST_QUALITY;  break;
  }
  */
  // resample quality taken from config file, 
  int ret = src_simple ( &data, q, 1 );
  if ( ret == 0 )
    printf("%s%i%s%i", "Resampling finished, from ", data.input_frames_used, " to ", data.output_frames_gen );
  else
    printf("%s%i%s%i", "Resampling finished, from ", data.input_frames_used, " to ", data.output_frames_gen );
  
  /// exchange buffers, so buf contains the resampled audio
  buf.swap( resampled );
}


void Sample::init()
{
  gain  = 0.75;
  pitch = 0.5;
  pan   = 0.5;
  
  startPoint = 0.0;
  endPoint = 1.0;
  
  attack  = 0;
  decay   = 0.05;
  sustain = 1;
  release = 0;
  
  // full range
  velLow  = 0;
  velHigh = 1;
  
  filterType = 0;
  filterFrequency = 1.0;
  filterResonance = 0.4;
  
  // set to true so we recacheWaveform() when requested for it
  dirty = true;
}

Sample::Sample( Fabla2DSP* d, int rate, const char* nme, int size, float* data ) :
  dsp( d ),
  sr(rate),
  name( nme ),
  channels( 2 ),
  frames( size / 2 ),
  velLow( 0 ),
  velHigh( 1 ),
  pitch( 0 ),
  gain ( 0.75 ),
  pan  ( 0 )
{
#ifdef FABLA2_COMPONENT_TEST
  printf("%s\n", __PRETTY_FUNCTION__ );
#endif
  
  init();
  
  fabla2_deinterleave( size, data, audioMono, audioStereoRight );
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
  
  char* tmp = strdup( path.c_str() );
  name = basename( tmp );
  free( tmp );
  
  channels = info.channels;
  frames   = info.frames;
  
  if( frames == 0 )
  {
    // bad file path?
    printf("Error loading sample %s, frames == 0\n", path.c_str() );
    return;
  }
  
  printf("Loading sample with %i frames\n", frames );
  
  if( channels > 2 || channels <= 0 )
  {
    printf("Error loading sample %s, channels > 2 || <= 0\n", path.c_str() );
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
  int samplRead = sf_read_float( sndfile, loadBuffer, info.frames * channels );
  sf_close(sndfile);
  
  if( channels == 2 )
  {
    audioMono.resize( frames );
    audioStereoRight.resize( frames );
    fabla2_deinterleave( frames, loadBuffer, audioMono, audioStereoRight );
  }
  
  if( sr != info.samplerate )
  {
    resample( info.samplerate, audioMono );
    
    if( channels == 2 )
    {
      resample( info.samplerate, audioStereoRight );
    }
    
    // since we've resampled, the size will have changed!
    frames = audioMono.size();
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

void Sample::velocityLow( float low )
{
  velLow  = low;
  printf("sample vel low %f\n", velLow );
}
void Sample::velocityHigh( float high )
{
  velHigh = high;
  printf("sample vel high %f\n", velHigh );
}

const float* Sample::getAudio( int chnl )
{
  if( channels == 2 && chnl == 1 && audioStereoRight.size() > 0 )
  {
    return &audioStereoRight[0];
  }
  return &audioMono[0];
}

bool Sample::write( const char* filename )
{
  printf("%s Start: %s : %s\n", __PRETTY_FUNCTION__, __TIME__, filename );
  
  SndfileHandle outfile( filename, SFM_WRITE, SF_FORMAT_WAV | SF_FORMAT_FLOAT, channels, dsp->sr );
  if( channels == 1 )
  {
    int written = outfile.write( &audioMono[0], frames );
    //printf(" wrote %i frames!\n", written );
  }
  else
  {
    // hack to write stereo interleaved channels
    std::vector<float> tmp;
    for(int i = 0; i < frames; i++)
    {
      tmp.push_back( audioMono[i] );
      tmp.push_back( audioStereoRight[i] );
    }
    int wrtn = outfile.write( &tmp[0], frames * channels );
    printf("Stere: wrote %i frames!\n", wrtn );
  }
  
  printf("%s Done: %s\n", __PRETTY_FUNCTION__, __TIME__ );
}

bool Sample::velocity( float vel )
{
  if( vel > 1.0f ) vel = 0.99996;
  
  if( vel >= velLow &&
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
