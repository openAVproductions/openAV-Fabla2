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
#include "bank.hxx"
#include "voice.hxx"
#include "sample.hxx"
#include "sampler.hxx"
#include "library.hxx"
#include "midi_helper.hxx"

namespace Fabla2
{

Fabla2DSP::Fabla2DSP( int rate ) :
  sr( rate ),
  recordEnable( false ),
  recordBank( 0 ),
  recordPad( 0 )
{
  // init the library
  library = new Library( this, rate );
  
  for( int i = 0; i < 16; i++ )
    voices.push_back( new Voice( this, rate ) );
  
  recordBuffer.resize( rate * 10 );
  
  memset( controlPorts, 0, sizeof(float*) * PORT_COUNT );
  
  int bankID = 0;
  for(int i = 0; i < 16 * 4; i++)
  {
    // move to next bank
    if( i > 0 && i % 16 == 0 )
    {
      bankID++;
    }
    
    Pad* tmpPad = new Pad( this, rate, i % 16 );
    
    // TODO: Fixme to use Library & RT-safe loading
    // hack code load sample for now
    if ( i == 0 )
    {
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
      Sample* tmp = new Sample( this, rate, "Two", "/usr/local/lib/lv2/fabla2.lv2/test2.wav");
      tmpPad->add( tmp );
      tmpPad->muteGroup( 1 );
    }
    if ( i == 2 )
    {
      Sample* tmp = new Sample( this, rate, "Three", "/usr/local/lib/lv2/fabla2.lv2/test3.wav");
      tmpPad->add( tmp );
      tmpPad->muteGroup( 1 );
    }
    if ( i == 3 )
    {
      Sample* tmp = new Sample( this, rate, "Four", "/usr/local/lib/lv2/fabla2.lv2/test4.wav");
      tmpPad->add( tmp );
      tmpPad->muteGroup( 1 );
    }
    if ( i == 4 )
    {
      Sample* tmp = new Sample( this, rate, "Kick", "/usr/local/lib/lv2/fabla2.lv2/kick.wav");
      tmpPad->add( tmp );
    }
    if ( i == 16 )
    {
      Sample* tmp = new Sample( this, rate, "Kick", "/usr/local/lib/lv2/fabla2.lv2/kick.wav");
      tmpPad->add( tmp );
    }
    
    library->bank( bankID )->pad( tmpPad );
  }
}

void Fabla2DSP::process( int nf )
{
  nframes = nf;
  
  float recordOverLast = *controlPorts[RECORD_OVER_LAST_PLAYED_PAD];
  if( recordEnable != (int)recordOverLast )
  {
    // update based on control port value
    recordEnable = (int)recordOverLast;
    
    if( recordEnable )
    {
      printf("recording switch! %i\n", recordEnable );
      startRecordToPad( recordBank, recordPad );
    }
    else
    {
      stopRecordToPad();
    }
  }
  
  
  memset( controlPorts[OUTPUT_L], 0, sizeof(float) * nframes );
  memset( controlPorts[OUTPUT_R], 0, sizeof(float) * nframes );
  
  if( recordEnable && recordPad != -1 && recordIndex + nframes < sr * 4 )
  {
    printf("recording...\n" );
    for(int i = 0; i < nframes; i++)
    {
      recordBuffer[recordIndex++] = controlPorts[INPUT_L][i];
      recordBuffer[recordIndex++] = controlPorts[INPUT_R][i];
    }
  }
  else if( recordEnable && recordPad != -1 )
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
  
  switch( lv2_midi_message_type( msg ) )
  {
    case LV2_MIDI_MSG_NOTE_ON:
        //printf("MIDI : Note On received\n");
        // valid MIDI note on for a sampler
        if( msg[1] >= 36 && msg[1] < 36 + 16 )
        {
          int chnl = (msg[0] & 0x0F);
          if( chnl < 0 && chnl >= 4 ) { return; }
          
          recordBank = chnl;
          recordPad  = msg[1] - 36; // 0 - 15 based Pad num
          
          //printf("midi channel %i\n", chnl );
          // get pad, and push to a voice
          Pad* p = library->bank( chnl )->pad( msg[1] - 36 );
          
          bool allocd = false;
          for(int i = 0; i < voices.size(); i++)
          {
            if( !voices.at(i)->active() && !allocd )
            {
              voices.at(i)->play( p, msg[2] );
              allocd = true; // don't set more voices to play the pad
              // don't return: scan all voices for mute-groups!
              continue;
            }
            
            if( voices.at(i)->active() )
            {
              Pad* vp = voices.at(i)->getPad();
              if( vp && vp->muteGroup() == p->muteGroup() )
              {
                printf("Mute Group %i, stopped voice #%i\n", p->muteGroup(), i );
                voices.at(i)->stop();
              }
            }
          }
        }
        break;
    
    case LV2_MIDI_MSG_NOTE_OFF:
        //printf("MIDI : Note Off received\n");
        for(int i = 0; i < voices.size(); i++)
        {
          Voice* v = voices.at(i);
          if( v->active() )
          {
            int chnl = (msg[0] & 0x0F);
            if( chnl < 0 && chnl >= 4 ) { return; }
            if( msg[1] < 36 && msg[1] >= 36 + 16 ){ return; }
            
            Pad* p = library->bank( chnl )->pad( msg[1] - 36 );
            
            if ( v->getPad() == p );
            {
              voices.at(i)->stop();
              return;
            }
          }
        }
        break;
    
    case LV2_MIDI_MSG_CONTROLLER:
        printf("MIDI : Controller received\n");
        if( msg[1] == 119 ) 
        {
          startRecordToPad( recordBank, recordPad );
        }
        else if( msg[1] == 117 )
        {
          stopRecordToPad();
        }
        break;
    
    case LV2_MIDI_MSG_PGM_CHANGE:
        printf("MIDI : Program Change received\n");
        break;
    
  }
  
  /// OLD midi handling code, doesn't use Library. Fixing
  /*
  if( msg[0] == 144 )
  {
    if( recordEnable && recordPad == -1 )
    {
      recordPad = msg[1] - 36;
      recordIndex = 0;
    }
    
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
        }
      }
    }
    
  }
  else if ( msg[0] == 128 )
  {
    if( recordEnable )
    {
      
    }
    else
    {
      // lookup which Pad* gets *played* when a note on arrives
      Pad* padOnPtr = 0;
      for (std::map< int, yasper::ptr<Pad> >::iterator it= midiToPad.begin(); it != midiToPad.end(); ++it)
      {
        if( it->first == msg[1] )
        {
          padOnPtr = it->second;
        }
      }
      
      if( padOnPtr )
      {
        // normal note off event, so kill voice
        for(int i = 0; i < voices.size(); i++)
        {
          if( voices.at(i)->getPad() == padOnPtr )
          {
            voices.at(i)->stop();
            break;
          }
        }
      }
    }
  }
  
  else if( msg[0] == 176 )
  {
    // STOP on MPD32 "ctrl" mode for transport
    if( msg[1] == 117 )
    {
      recordEnable = false;
      recordIndex = 0;
      recordPad = -1;
      printf("record disabled!\n");
    }
    if( msg[1] == 119 ) // record
    {
      recordEnable = true;
      recordIndex = 0;
      recordPad = -1;
      printf("record enabled!\n");
    }
    
    if( midiToPad.find( 36 + msg[1] ) != midiToPad.end() )
    {
      midiToPad[ 36 + msg[1] - 1 ]->controls[Pad::FILTER_CUTOFF] = msg[2] / 127.f;
    }
    
  }
  **/
  
}

void Fabla2DSP::uiMessage(int bank, int pad, int layer, int URI, float value)
{
  
}

void Fabla2DSP::startRecordToPad( int b, int p )
{
  recordBank  = b;
  recordPad   = p;
  recordIndex = 0;
  recordEnable = true;
  printf("record starting, bank %i, pad %i\n", recordBank, recordPad );
}

void Fabla2DSP::stopRecordToPad()
{
  printf("record finished, pad # %i\n", recordPad );
  
  Pad* pad = library->bank( recordBank )->pad( recordPad );
  
  printf("%s : NON RT SAFE NEW SAMPLE()\n", __PRETTY_FUNCTION__ );
  Sample* s = new Sample( this, sr, recordIndex, &recordBuffer[0] );
  
  // reset the pad
  pad->clearAllSamples();
  pad->add( s );
  
  recordIndex = 0;
  recordEnable = false;
}

Fabla2DSP::~Fabla2DSP()
{
}

}; // Fabla2

