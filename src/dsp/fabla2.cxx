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

#include "fabla2.hxx"

#include <stdio.h>
#include <string.h>

#include "pad.hxx"
#include "voice.hxx"
#include "sample.hxx"
#include "sampler.hxx"
#include "library.hxx"

namespace Fabla2
{

Fabla2DSP::Fabla2DSP( int rate ) :
  sr( rate ),
  recordEnable( false )
{
  voices.push_back( new Voice( this, rate ) );
  voices.push_back( new Voice( this, rate ) );
  voices.push_back( new Voice( this, rate ) );
  voices.push_back( new Voice( this, rate ) );
  
  recordBuffer.resize( rate * 4 );
  
  memset( controlPorts, 0, sizeof(float*) * PORT_COUNT );
  
  for(int i = 0; i < 16; i++)
  {
    Pad* tmpPad = new Pad( this, rate );
    
    if ( i == 0 )
    {
      // TODO: Fixme to use Library & RT-safe loading
      // hack code load sample for now
      Sample* tmp = new Sample( this, rate, "One", "/usr/local/lib/lv2/fabla2.lv2/test.wav");
      tmp->velocity( 0, 32 );
      tmpPad->add( tmp );
      /*
      tmp = new Sample( this, rate, "Two", "/usr/local/lib/lv2/fabla2.lv2/test2.wav");
      tmp->velocity( 32, 64 );
      tmpPad->add( tmp );
      
      tmp = new Sample( this, rate, "Three", "/usr/local/lib/lv2/fabla2.lv2/test3.wav");
      tmp->velocity( 64, 96 );
      tmpPad->add( tmp );
      
      tmp = new Sample( this, rate, "Four", "/usr/local/lib/lv2/fabla2.lv2/test4.wav");
      tmp->velocity( 96, 128 );
      tmpPad->add( tmp );
      */
    }
    if ( i == 1 )
    {
      // TODO: Fixme to use Library & RT-safe loading
      // hack code load sample for now
      Sample* tmp = new Sample( this, rate, "Two", "/usr/local/lib/lv2/fabla2.lv2/test2.wav");
      tmpPad->add( tmp );
    }
    if ( i == 2 )
    {
      // TODO: Fixme to use Library & RT-safe loading
      // hack code load sample for now
      Sample* tmp = new Sample( this, rate, "Three", "/usr/local/lib/lv2/fabla2.lv2/test3.wav");
      tmpPad->add( tmp );
    }
    if ( i == 3 )
    {
      // TODO: Fixme to use Library & RT-safe loading
      // hack code load sample for now
      Sample* tmp = new Sample( this, rate, "Four", "/usr/local/lib/lv2/fabla2.lv2/test4.wav");
      tmpPad->add( tmp );
    }
    
    midiToPad.insert( std::pair< int,yasper::ptr<Pad> >( i + 36, tmpPad ) );
  }
}

void Fabla2DSP::process( int nf )
{
  nframes = nf;
  
  memset( controlPorts[OUTPUT_L], 0, sizeof(float) * nframes );
  memset( controlPorts[OUTPUT_R], 0, sizeof(float) * nframes );
  
  if( recordEnable && recordIndex + nframes < sr * 4 )
  {
    for(int i = 0; i < nframes; i++)
    {
      recordBuffer[recordIndex++] = controlPorts[INPUT_L][i];
      recordBuffer[recordIndex++] = controlPorts[INPUT_R][i];
    }
  }
  else if( recordEnable )
  {
    recordEnable = false;
    printf("record stopped: out of space! %li\n", recordIndex );
  }
  
  for( int i = 0; i < voices.size(); i++ )
  {
    Voice* v = voices.at(i);
    if( v->active() )
    {
      //printf("voice %i playing\n", i);
      v->process();
    }
  }
  
  // master outputs
  
  /*
  for(int i = 0; i < nframes; i++ )
  {
    float* outL = controlPorts[OUTPUT_L];
    float* outR = controlPorts[OUTPUT_R];
    
    for(int i = 0; i < voices.size(); i++ )
    {
      float* v = voices.at(i)->getVoiceBuffer();
      *outL++ += v[ i ];
      *outR++ += v[ nframes + i ];
    }
  }
  */
  
}

void Fabla2DSP::midi( int f, const uint8_t* msg )
{
  printf("MIDI: %i, %i, %i\n", (int)msg[0], (int)msg[1], (int)msg[2] );
  
  if( msg[0] == 144 )
  {
    if( !recordEnable )
    {
      /// Logic for incoming MIDI -> Pad mapping
      for (std::map< int, yasper::ptr<Pad> >::iterator it= midiToPad.begin(); it != midiToPad.end(); ++it)
      {
        if( it->first == msg[1] )
        {
          
          for(int i = 0; i < voices.size(); i++)
          {
            if( !voices.at(i)->active() )
            {
              voices.at(i)->play( it->second, msg[2] );
              break;
            }
          }
          
          /// Logic for fetch-pad-data from Library
          voices.at(0)->play( it->second, msg[2] );
        }
      }
    }
    else
    {
      // record on this Pad
      recordPad = msg[1] - 36;
      recordIndex = 0;
    }
    
  }
  else if ( msg[0] == 240 && msg[1] == 127 )
  {
    if( recordEnable )
    {
      printf("record disabled, pad # %i\n", recordPad );
      
      if( midiToPad.find( 36 + recordPad ) != midiToPad.end() )
      {
        printf("%s : NON RT SAFE NEW SAMPLE()\n", __PRETTY_FUNCTION__ );
        Sample* s = new Sample( this, sr, recordIndex, &recordBuffer[0] );
        
        // s is auto-released when shared_ptr goes out of scope
        midiToPad[ 36 + recordPad ]->clearAllSamples();
        midiToPad[ 36 + recordPad ]->add( s );
      }
      recordPad = -1;
      recordIndex = 0;
      recordEnable = false;
    }
    else
    {
      recordEnable = true;
      recordIndex = 0;
      recordPad = -1;
      printf("record enabled!\n");
    }
    
  }
  
  else if( msg[0] == 176 )
  {
    if( midiToPad.find( 36 + msg[1] ) != midiToPad.end() )
    {
      midiToPad[ 36 + msg[1] - 1 ]->controls[Pad::FILTER_CUTOFF] = msg[2] / 127.f;
    }
    
    
    
    /*
    /// library to get Pad
    for (std::map< int, yasper::ptr<Pad> >::iterator it= midiToPad.begin(); it != midiToPad.end(); ++it)
    {
      if( msg[2] < 63 )
        it->second->switchSystem( Pad::SS_VELOCITY_LAYERS );
      else
        it->second->switchSystem( Pad::SS_ROUND_ROBIN );
    }
    */
  }
  
}

Fabla2DSP::~Fabla2DSP()
{
}

}; // Fabla2

