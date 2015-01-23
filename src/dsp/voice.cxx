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

#include "dsp_adsr.hxx"
#include "dsp_filters_svf.hxx"

#include "plotter.hxx"

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
  
  int releaseSamps = 0.05 * r;
  adsr->setReleaseRate ( releaseSamps );
  adsrOffCounter = releaseSamps;
}

bool Voice::matches( int bank, int pad )
{
  return ( bank == bankInt_ && pad == padInt_ );
}

void Voice::playLayer( Pad* p, int layer )
{
  pad_ = p;
  
  sampler->playLayer( p, layer );
  
  Sample* s = sampler->getSample();
  if( s )
  {
    //printf("Voice::playLayer() %i, on Sample %s\n", ID, s->getName() );
    active_ = true;
  }
  else
  {
    printf("Voice::playLayer() %i, sampler->play() returns NULL sample! Setting active to false\n", ID );
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
}

void Voice::play( int time, int bankInt, int padInt, Pad* p, float velocity )
{
  // useful for mute groups etc
  bankInt_ = bankInt;
  padInt_ = padInt;
  
  pad_ = p;
  
  active_ = true;
  activeCountdown = time;
  
  sampler->play( pad_, velocity );
  
  Sample* s = sampler->getSample();
  if( s )
  {
    //printf("Voice::play() %i, on Sample %s @ time : %i, padVol %f\n", ID, s->getName(), time, pad_->volume );
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
       if( s->filterType < 0.1)
    filterActive_ = false;
  else if( s->filterType < 1.1 )
    filterType = 0;
  else if( s->filterType < 2.1 )
    filterType = 1;
  else if( s->filterType < 3.1 )
    filterType = 2;
  else
    filterActive_ = false; // default: off
  
  filterL->setType( filterType );
  filterR->setType( filterType );
  
  // ADSR: add *minimal* attack / release to avoid clicks
  int attackSamps  = (0.005+s->attack ) * sr;
  int decaySamps   = (0.005+s->decay  ) * sr;
  int releaseSamps = (0.05 +s->release) * sr;
  int totalSamps   = s->getFrames();
  // sanitize ADSR values:
  
  // shorten release if needed
  if( attackSamps + decaySamps + releaseSamps > totalSamps )
  {
    releaseSamps = totalSamps - attackSamps - decaySamps;
    
    // ensure release has min length
    if( releaseSamps < 0.05 * sr )
    {
      releaseSamps = 0.05 * sr;
      printf("too long: clipped release to %i : NOT OK YET\n", releaseSamps);
    }
    else
    {
      printf("too long: clipped release to %i : now OK\n", releaseSamps);
    }
  }
  
  // shorten decay if needed
  if( attackSamps + decaySamps + releaseSamps > totalSamps )
  {
    decaySamps = totalSamps - attackSamps - releaseSamps;
    
    // ensure release has min length
    if( decaySamps < 0.005 * sr )
    {
      decaySamps = 0.005 * sr;
      printf("too long: clipped decay to %i : NOT OK YET\n", decaySamps);
    }
    else
    {
      printf("too long: clipped decay to %i : now OK\n", decaySamps);
    }
  }
  
  // shorten attack if needed
  if( attackSamps + decaySamps + releaseSamps > totalSamps )
  {
    attackSamps = totalSamps - decaySamps - releaseSamps;
    
    // ensure release has min length
    if( attackSamps < 0.005 * sr )
    {
      attackSamps = 0.005 * sr;
      printf("too long: clipped attack to %i : NOT OK YET\n", attackSamps);
    }
    else
    {
      printf("too long: clipped attack to %i : now OK\n", attackSamps);
    }
  }
  
  if( false ) // extreme ADSR testing at note-on stage
  {
    adsr->reset();
    adsr->gate(true);
    
    float array[totalSamps];
    float outAudio[totalSamps];
    for( int i = 0; i < totalSamps; i++ )
    {
      if( i == totalSamps - adsrOffCounter )
        adsr->gate(false);
      
      array[i] = adsr->process();
      outAudio[i] = s->getAudio(0)[i] * array[i];
    }
    
    Plotter::plot( "adsr.dat" , 2500, &array[totalSamps-2501] );
    Plotter::plot( "audio.dat", 2500, &outAudio[totalSamps-2501] );
    
    Plotter::plot( "adsr_s.dat" , 500, &array[0] );
    Plotter::plot( "audio_s.dat", 500, &outAudio[0] );
    adsr->reset();
  }
  
  
  
  
  
  /*
  if( attackSamps > totalSamps - releaseSamps )
  {
    // attack gets reduced to max lenght, with 0 decay and minimal release.
    attackSamps = totalSamps - releaseSamps;
    decaySamps  = 0;
    printf("ADR > total! new attack : %i\n", attackSamps );
  }
  else if( attackSamps + decaySamps > totalSamps - releaseSamps )
  {
    // decay reduced to max given 
    decaySamps = totalSamps - attackSamps - releaseSamps;
    printf("ADR > total! new decay : %i\n", decaySamps );
  }
  /*
  else if( attackSamps + decaySamps + releaseSamps >totalSamps )
  {
    // release reduced to max length
    releaseSamps = s->getFrames() - attackSamps - decaySamps;
    if( releaseSamps < 0 )
       releaseSamps = (0.05) * sr;
    
    printf("ADR > total! new release : %i\n", releaseSamps );
  }
  */
  
  
  adsrOffCounter = releaseSamps;
  printf("voice playing with start-end %i,  adsrOffCounter %i\n", totalSamps, adsrOffCounter );
  
  adsr->setAttackRate  ( attackSamps );
  adsr->setDecayRate   ( decaySamps  );
  adsr->setSustainLevel( s->sustain  );
  adsr->setReleaseRate ( releaseSamps);
  
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

void Voice::stopIfSample( Sample* s )
{
  Sample* vs = sampler->getSample();
  if( s == vs )
  {
    //printf("Voice::stopIfSample() %s : KILLED VOICE.\n", s->getName() );
    active_ = false;
  }
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
  
  int nframes = dsp->nframes;
  if( activeCountdown )
  {
    nframes = nframes - activeCountdown;
    //printf("process() with activeCountdown = %i\n", activeCountdown ); 
  }
  
  // check if we need to trigger ADSR off
  if( sampler->getRemainingFrames() + nframes < adsrOffCounter )
  {
    if( adsr->getState() != ADSR::ENV_RELEASE )
    {
      printf("remaining frames + nframes < adsrOffCounter : ADSR OFF\n");
      adsr->gate( false );
    }
  }
  
  int done = sampler->process( nframes, &voiceBuffer[0+activeCountdown], &voiceBuffer[dsp->nframes+activeCountdown] );
  
  float adsrVal = adsr->process();
  
  /// set filter state
  Sample* s = sampler->getSample();
  
  if( !s )
  {
    //printf("Fabla2 DSP: Voice process() with invalid Sample* : WARNING!");
  }
  
  if( done || adsr->getState() == ADSR::ENV_IDLE )
  {
    //printf("Voice done\n");
    active_ = false;
    pad_ = 0;
    return;
  }
  
  // filter details setup in play()
  if( filterActive_ )
  {
    filterL->setResonance( ( s->filterResonance) );
    filterR->setResonance( ( s->filterResonance) );
    
    filterL->setValue( ( s->filterFrequency + 0.3) );
    filterR->setValue( ( s->filterFrequency + 0.3) );
    
    filterL->process( nframes, &voiceBuffer[           0+activeCountdown], &voiceBuffer[           0+activeCountdown] );
    filterR->process( nframes, &voiceBuffer[dsp->nframes+activeCountdown], &voiceBuffer[dsp->nframes+activeCountdown] );
  }
  
  float* outL = dsp->controlPorts[OUTPUT_L];
  float* outR = dsp->controlPorts[OUTPUT_R];
  
  float aux1s = pad_->sends[0];
  float* aux1L = dsp->controlPorts[AUXBUS1_L];
  float* aux1R = dsp->controlPorts[AUXBUS1_R];
  
  float aux2s = pad_->sends[1];
  float* aux2L = dsp->controlPorts[AUXBUS2_L];
  float* aux2R = dsp->controlPorts[AUXBUS2_R];
  
  float aux3s = pad_->sends[2];
  float* aux3L = dsp->controlPorts[AUXBUS3_L];
  float* aux3R = dsp->controlPorts[AUXBUS3_R];
  
  float aux4s = pad_->sends[3];
  float* aux4L = dsp->controlPorts[AUXBUS4_L];
  float* aux4R = dsp->controlPorts[AUXBUS4_R];
  
  for(int i = activeCountdown; i < dsp->nframes; i++ )
  {
    float pfL = voiceBuffer[             i] * adsrVal;
    float pfR = voiceBuffer[dsp->nframes+i] * adsrVal;
    
    aux1L[i] += pfL * aux1s;
    aux1R[i] += pfR * aux1s;
    
    aux2L[i] += pfL * aux2s;
    aux2R[i] += pfR * aux2s;
    
    aux3L[i] += pfL * aux3s;
    aux3R[i] += pfR * aux3s;
    
    aux4L[i] += pfL * aux4s;
    aux4R[i] += pfR * aux4s;
    
    outL[i] += pfL;
    outR[i] += pfR;
    
    // ADSR processes first sample *before* the filter set section.
    adsrVal = adsr->process();
  }
  
  
  // for testing sample-accurate voice note-on
  if( activeCountdown )
  {
    //Plotter::plot( "active.dat", dsp->nframes, dsp->controlPorts[OUTPUT_L] );
  }
  
  
  activeCountdown = 0;
}

Voice::~Voice()
{
}


}; // Fabla2
