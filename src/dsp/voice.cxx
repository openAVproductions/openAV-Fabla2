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

#include <math.h>
#include <stdio.h>

#include "fabla2.hxx"

#include "pad.hxx"
#include "sample.hxx"
#include "sampler.hxx"
#include "dsp_filters_svf.hxx"

#ifdef FABLA2_COMPONENT_TEST
#include "plotter.hxx"
#endif

namespace Fabla2
{

int Voice::privateID = 0;

Voice::Voice( Fabla2DSP* d, int r ) :
  ID( privateID++ ),
  dsp( d ),
  sr ( r ),
  pad_( 0 ),
  active_( false ),
  bankInt_( -1 ),
  padInt_( -1 )
{
  adsr = new ADSR();
  sampler = new Sampler( d, r );
  filterL = new FiltersSVF( r );
  filterR = new FiltersSVF( r );
  
  voiceBuffer.resize( 1024 );
  
  adsr->setAttackRate  ( 0.001 * r );
  adsr->setDecayRate   ( 0.25 * r );
  adsr->setSustainLevel( 0.5  );
  adsr->setReleaseRate ( 0.05 * r );
}

bool Voice::matches( int bank, int pad )
{
  return ( bank == bankInt_ && pad == padInt_ );
}

void Voice::play( int bankInt, int padInt, Pad* p, float velocity )
{
  // useful for mute groups etc
  bankInt_ = bankInt;
  padInt_ = padInt;
  
  pad_ = p;
  active_ = true;
  
  sampler->play( pad_, velocity );
  
  Sample* s = sampler->getSample();
  if( s )
  {
    //printf("Voice::play() %i, on Sample %s\n", ID, samp->getName() );
  }
  else
  {
#ifdef FABLA2_COMPONENT_TEST
    printf("Voice::play() %i, sampler->play() returns NULL sample! Setting active to false\n", ID );
#endif
    // *hard* set the sample to not play: we don't have a sample!
    active_ = false;
    return;
  }
  
  filterActive_ = true;
  
  int filterType = 0; // lowpass
  // check the value of the filter type to set voice params
       if( s->filterType < 0.25 )
    filterActive_ = false;
  else if( s->filterType < 0.5 )
    filterType = 0;
  else if( s->filterType < 0.75 )
    filterType = 1;
  else if( s->filterType < 1.0 )
    filterType = 1;
  else
    filterType = 0; // lowpass default
  
  filterL->setType( filterType );
  filterR->setType( filterType );
  
  // ADSR: add *minimal* attack / release to avoid clicks
  adsr->setAttackRate  ( (0.001+s->attack) * sr );
  adsr->setDecayRate   ( s->decay * sr );
  adsr->setSustainLevel( s->sustain  );
  adsr->setReleaseRate ( (0.05+s->release) * sr );
  
  adsr->reset();
  adsr->gate( true );
  
#ifdef FABLA2_COMPONENT_TEST
  if( false )
  {
    std::vector<float> tmp(44100 * 5);
    int i = 0;
    for( i = 0; i < 44100 * 3; i++ )
    {
      tmp.at(i) = adsr->process();
    }
    
    adsr->gate( false );
    
    for( ; i < 44100 * 5; i++ )
    {
      tmp.at(i) = adsr->process();
    }

    Plotter::plot( "adsr.dat", 44100 * 5, &tmp[0] );
  }
#endif
}

void Voice::stop()
{
  if( active_ )
  {
    if ( pad_->triggerMode() == Pad::TM_GATED )
    {
      //printf("Voice::stop() %i, GATED\n", ID );
      adsr->gate( false );
    }
    else
    {
      //printf("Voice::stop() %i, ONE-SHOT, ignoring.\n", ID );
    }
  }
}

void Voice::process()
{
  if( !active_ )
  {
    return;
  }
  
  int done = sampler->process( dsp->nframes, &voiceBuffer[0], &voiceBuffer[dsp->nframes] );
  
  float adsrVal = adsr->process();
  
  // adsr -> freq / reso / etc, we need to multiply by the adsrVal * routingAmount
  //* adsrVal );
  
  /// set filter state
  Sample* s = sampler->getSample();
  
  // filter details setup in play()
  if( filterActive_ )
  {
    filterL->setResonance( ( s->filterResonance) );
    filterR->setResonance( ( s->filterResonance) );
    
    filterL->setValue( ( s->filterFrequency + 0.3) );
    filterR->setValue( ( s->filterFrequency + 0.3) );
    
    filterL->process( dsp->nframes, &voiceBuffer[           0], &voiceBuffer[           0] );
    filterR->process( dsp->nframes, &voiceBuffer[dsp->nframes], &voiceBuffer[dsp->nframes] );
  }
  
  float* outL = dsp->controlPorts[OUTPUT_L];
  float* outR = dsp->controlPorts[OUTPUT_R];
  
  for(int i = 0; i < dsp->nframes; i++ )
  {
    *outL++ += voiceBuffer[             i] * adsrVal;
    *outR++ += voiceBuffer[dsp->nframes+i] * adsrVal;
    
    // ADSR processes first sample *before* the filter set section.
    adsrVal = adsr->process();
  }
  
  if( done || adsr->getState() == ADSR::ENV_IDLE )
  {
    //printf("Voice done\n");
    active_ = false;
    pad_ = 0;
  }
}

Voice::~Voice()
{
}


}; // Fabla2
