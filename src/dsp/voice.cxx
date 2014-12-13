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
#include "sampler.hxx"
#include "dsp_filters_svf.hxx"

#ifdef FABLA2_COMPONENT_TEST
#include "plotter.hxx"
#endif

namespace Fabla2
{

Voice::Voice( Fabla2DSP* d, int r ) :
  dsp( d ),
  sr ( r ),
  pad( 0 ),
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
  adsr->setReleaseRate ( 0.5 * r );
}

void Voice::play( Pad* p, int velocity )
{
  pad = p;
  active_ = true;
  phase = 0;
  
  adsr->reset();
  adsr->gate( true );
  
  sampler->play( pad, velocity );

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

void Voice::process()
{
  if( !active_ )
    return;
  
  int done = sampler->process( dsp->nframes, &voiceBuffer[0], &voiceBuffer[dsp->nframes] );
  
  // fast forward the ADSR to the after nframes-value, to respond faster
  for(int i = 0; i < dsp->nframes - 1; i++)
    adsr->process();
  
  float adsrVal = adsr->process();
  filterL->setValue( (pad->controls[Pad::FILTER_CUTOFF]+0.3) * adsrVal );
  filterR->setValue( (pad->controls[Pad::FILTER_CUTOFF]+0.3) * adsrVal );
  
  filterL->process( dsp->nframes, &voiceBuffer[           0], &voiceBuffer[           0] );
  filterR->process( dsp->nframes, &voiceBuffer[dsp->nframes], &voiceBuffer[dsp->nframes] );
  
  float* outL = dsp->controlPorts[OUTPUT_L];
  float* outR = dsp->controlPorts[OUTPUT_R];
  
  for(int i = 0; i < dsp->nframes; i++ )
  {
    *outL++ += voiceBuffer[             i] * 0.8 * adsrVal;
    *outR++ += voiceBuffer[dsp->nframes+i] * 0.8 * adsrVal;
  }
  
  if( done )
  {
    printf("Voice done\n");
    active_ = false;
  }
}

Voice::~Voice()
{
}


}; // Fabla2
