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
  active_( false )
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

void Voice::play( Pad* p, int velocity )
{
  pad_ = p;
  active_ = true;
  phase = 0;
  
  sampler->play( pad_, velocity );
  
  Sample* samp = sampler->getSample();
  if( samp )
  {
    printf("Voice::play() %i, on Sample %s\n", ID, samp->getName() );
  }
  else
  {
    printf("Voice::play() %i, sampler->play() returns NULL sample! Setting active to false\n", ID );
    // *hard* set the sample to not play: we don't have a sample!
    active_ = false;
  }
  
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
    /*
    Sample* samp = sampler->getSample();
    if( samp )
      printf("Voice::stop() %i, on Sample %s\n", ID, samp->getName() );
    */
    adsr->gate( false );
  }
}

void Voice::process()
{
  if( !active_ )
  {
    return;
  }
  
  
  int done = sampler->process( dsp->nframes, &voiceBuffer[0], &voiceBuffer[dsp->nframes] );
  
  // fast forward the ADSR to the after nframes-value, to respond faster
  //for(int i = 0; i < dsp->nframes - 1; i++)
  
  float adsrVal = adsr->process();
  filterL->setValue( (pad_->controls[Pad::FILTER_CUTOFF]+0.3) );//* adsrVal );
  filterR->setValue( (pad_->controls[Pad::FILTER_CUTOFF]+0.3) );//* adsrVal );
  
  bool filterOn = true;
  if( filterOn )
  {
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
