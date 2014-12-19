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

#include "../dsp.hxx"

#include <sstream>

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

Fabla2DSP::Fabla2DSP( int rate, URIs* u ) :
  sr( rate ),
  uris( u ),
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
    
    if ( i == 9 )
    {
      Sample* tmp = new Sample( this, rate, "One", "/usr/local/lib/lv2/fabla2.lv2/stereoTest.wav");
      tmpPad->add( tmp );
    }
    
    if( i < 16 )
    {
      std::stringstream s;
      s << i << ".wav";
      
      std::stringstream path;
      path << "/usr/local/lib/lv2/fabla2.lv2/" << i << ".wav";
      
      Sample* tmp = new Sample( this, rate, s.str(), path.str() );
      tmp->velocity( 0, 128 );
      tmpPad->add( tmp );
    }
    
    // TODO: Fixme to use Library & RT-safe loading
    // hack code load sample for now
    if ( i == 16 )
    {
      Sample* tmp = new Sample( this, rate, "One", "/usr/local/lib/lv2/fabla2.lv2/test.wav");
      tmp->velocity( 0, 32 );
      tmpPad->add( tmp );
      
      tmp = new Sample( this, rate, "Two", "/usr/local/lib/lv2/fabla2.lv2/test2.wav");
      tmp->velocity( 32, 64 );
      tmpPad->add( tmp );
      
      tmp = new Sample( this, rate, "Three", "/usr/local/lib/lv2/fabla2.lv2/test3.wav");
      tmp->velocity( 64, 96 );
      tmpPad->add( tmp );
      
      tmp = new Sample( this, rate, "Four", "/usr/local/lib/lv2/fabla2.lv2/test4.wav");
      tmp->velocity( 96, 128 );
      tmpPad->add( tmp );
      
      tmpPad->switchSystem( Pad::SS_ROUND_ROBIN );
    }
    
    library->bank( bankID )->pad( tmpPad );
  }
  
  library->checkAll();
  
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

static void fabla2_dsp_getDetailsFromNote( const uint8_t* msg, int& bank, int& pad )
{
  // check MIDI note is valid in downwards direction
  if( msg[1] < 36 )
    return;
  
  // get midi bank (from message, or from MIDI note num?)
  bank = 0; //(msg[0] & 0x0F);
  
  // select pad number from midi note (0-15)
  pad = (msg[1] - 36) % 16;
  
  // convert higher midi notes ( > 36 + 16, aka first bank) to next bank
  if( msg[1] >= 36 + 16 )
  {
    int nMoreBanks = (msg[1] - 36) / 16;
    //printf( "more banks %i\n", nMoreBanks );
    bank += nMoreBanks;
  }
}

void Fabla2DSP::midi( int eventTime, const uint8_t* msg )
{
  //printf("MIDI: %i, %i, %i\n", (int)msg[0], (int)msg[1], (int)msg[2] );
  
  switch( lv2_midi_message_type( msg ) )
  {
    case LV2_MIDI_MSG_NOTE_ON:
        {
          // check MIDI note is valid in downwards direction
          if( msg[1] < 36 )
            return;
          
          int bank = 0;
          int pad  = 0;
          fabla2_dsp_getDetailsFromNote( msg, bank, pad );
          if( bank < 0 || bank >=  4 ) { return; }
          if( pad  < 0 || pad  >= 16 ) { return; }
          
          // update the recording pad
          recordBank = bank;
          recordPad  = pad;
          
          bool allocd = false;
          for(int i = 0; i < voices.size(); i++)
          {
            // current voice pointer
            Voice* v = voices.at(i);
            // the pad that's going to be allocated to play
            Pad* p = library->bank( bank )->pad( pad );
            
            // check mute-group to stop the voice first
            if( v->active() )
            {
              // current voice mutegroup = pad-to-play muteGroup, stop it.
              if( v->getPad()->muteGroup() != 0 &&
                  v->getPad()->muteGroup() == p->muteGroup() )
              {
                // note that this triggers ADSR off, so we can *NOT* re-purpose
                // the voice right away to play the new note.
                v->stop();
              }
            }
            else
            {
              // only allocate voice if we haven't already done so
              if( !allocd )
              {
                // play pad
                voices.at(i)->play( bank, pad, p, msg[2] );
                
                // write note on MIDI events to UI
                LV2_Atom_Forge_Frame frame;
                lv2_atom_forge_frame_time( &lv2->forge, eventTime );
                lv2_atom_forge_object( &lv2->forge, &frame, 0, uris->fabla2_PadPlay );
                
                lv2_atom_forge_key(&lv2->forge, uris->fabla2_bank);
                lv2_atom_forge_int(&lv2->forge, bank );
                lv2_atom_forge_key(&lv2->forge, uris->fabla2_pad);
                lv2_atom_forge_int(&lv2->forge, pad );
                lv2_atom_forge_key(&lv2->forge, uris->fabla2_layer);
                lv2_atom_forge_int(&lv2->forge, p->lastPlayedLayer() );
                lv2_atom_forge_key(&lv2->forge, uris->fabla2_velocity);
                lv2_atom_forge_int(&lv2->forge, msg[2] );
                
                lv2_atom_forge_pop(&lv2->forge, &frame);
                
                allocd = true;
                // don't return: scan all voices for mute-groups!
                continue;
              }
            }
          }
        }
        break;
    
    case LV2_MIDI_MSG_NOTE_OFF:
      {
        int bank = 0;
        int pad  = 0;
        fabla2_dsp_getDetailsFromNote( msg, bank, pad );
        if( bank < 0 || bank >=  4 ) { return; }
        if( pad  < 0 || pad  >= 16 ) { return; }
        
        // write note on MIDI events to UI
        LV2_Atom_Forge_Frame frame;
        lv2_atom_forge_frame_time( &lv2->forge, eventTime );
        lv2_atom_forge_object( &lv2->forge, &frame, 0, uris->fabla2_PadStop );
        
        lv2_atom_forge_key(&lv2->forge, uris->fabla2_bank);
        lv2_atom_forge_int(&lv2->forge, bank );
        lv2_atom_forge_key(&lv2->forge, uris->fabla2_pad);
        lv2_atom_forge_int(&lv2->forge, pad );
        lv2_atom_forge_key(&lv2->forge, uris->fabla2_layer);
        lv2_atom_forge_int(&lv2->forge, -1 );
        lv2_atom_forge_key(&lv2->forge, uris->fabla2_velocity);
        lv2_atom_forge_int(&lv2->forge, msg[2] );
        
        lv2_atom_forge_pop(&lv2->forge, &frame);
        
        for(int i = 0; i < voices.size(); i++)
        {
          Voice* v = voices.at(i);
          
          if( v->active() )
          {
            if( v->matches( bank, pad ) )
            {
              // this voice was started by the Pad that now sent corresponding
              // note off event: so stop() the voice
              //printf("Voice.matces() NOTE_OFF -> Stop()\n" );
              v->stop();
              return;
            }
          }
        }
      }
      break;
    
    case LV2_MIDI_MSG_CONTROLLER:
        //printf("MIDI : Controller received\n");
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
        //printf("MIDI : Program Change received\n");
        break;
    
  }
  
}

void Fabla2DSP::writeSampleState( int b, int p, int l, Sample* s )
{
  LV2_Atom_Forge_Frame frame;
  lv2_atom_forge_frame_time( &lv2->forge, 0 );
  
  lv2_atom_forge_object( &lv2->forge, &frame, 0, uris->fabla2_ReplyUiSampleState );
  
  lv2_atom_forge_key(&lv2->forge, uris->fabla2_bank);
  lv2_atom_forge_int(&lv2->forge, b );
  
  lv2_atom_forge_key(&lv2->forge, uris->fabla2_pad);
  lv2_atom_forge_int(&lv2->forge, p );
  
  lv2_atom_forge_key(&lv2->forge, uris->fabla2_layer);
  lv2_atom_forge_int(&lv2->forge, l );
  
  lv2_atom_forge_key(&lv2->forge, uris->fabla2_SampleGain);
  lv2_atom_forge_float(&lv2->forge, s->gain );
  
  lv2_atom_forge_key(&lv2->forge, uris->fabla2_SamplePitch);
  lv2_atom_forge_float(&lv2->forge, s->pitch );
  
  lv2_atom_forge_key(&lv2->forge, uris->fabla2_SamplePan );
  lv2_atom_forge_float(&lv2->forge, s->pan );
  
  lv2_atom_forge_key(&lv2->forge, uris->fabla2_SampleStartPoint );
  lv2_atom_forge_float(&lv2->forge, s->startPoint / s->getFrames() );
  
  lv2_atom_forge_pop(&lv2->forge, &frame);
}

void Fabla2DSP::tx_waveform( int b, int p, int l, const float* data )
{
  LV2_Atom_Forge_Frame frame;
  
  lv2_atom_forge_frame_time(&lv2->forge, 0);
  lv2_atom_forge_object(&lv2->forge, &frame, 0, uris->fabla2_SampleAudioData);
  
  lv2_atom_forge_key(&lv2->forge, uris->fabla2_bank);
  lv2_atom_forge_int(&lv2->forge, b );
  
  lv2_atom_forge_key(&lv2->forge, uris->fabla2_pad);
  lv2_atom_forge_int(&lv2->forge, p );
  
  lv2_atom_forge_key(&lv2->forge, uris->fabla2_layer);
  lv2_atom_forge_int(&lv2->forge, l );
  
  // Add vector of floats 'audioData' property
  lv2_atom_forge_key(&lv2->forge, uris->fabla2_audioData);
  lv2_atom_forge_vector( &lv2->forge, sizeof(float), uris->atom_Float, FABLA2_UI_WAVEFORM_PX, data);
  
  // Close off object
  lv2_atom_forge_pop(&lv2->forge, &frame);
}

void Fabla2DSP::uiMessage(int b, int p, int l, int URI, float v)
{
  //printf("Fabla2:uiMessage bank %i, pad %i, layer %i: %f\n", b, p, l, v );
  
  Pad* pad = library->bank( b )->pad( p );
  Sample* s = pad->layer( l );
  if( !s )
  {
    printf("Fabla2:uiMessage *ERROR* bank %i, pad %i, layer %i, sample == NULL\n", b, p, l );
    return;
  }
  
  if(       URI == uris->fabla2_SamplePitch ) {
    s->dirty = 1; s->pitch = v;
  }
  else if(  URI == uris->fabla2_SampleGain ) {
    //printf("setting gain to %f\n", v );
    s->dirty = 1; s->gain = v;
  }
  else if(  URI == uris->fabla2_SamplePan ) {
    //printf("setting pan to %f\n", v );
    s->dirty = 1; s->pan = v;
  }
  else if(  URI == uris->fabla2_SampleStartPoint ) {
    //printf("setting start point to %f\n", v );
    s->dirty = 1; s->startPoint = v * s->getFrames();
  }
  else if(  URI == uris->fabla2_PadMuteGroup ) {
    //printf("setting start point to %f\n", v );
    pad->muteGroup( int(v) );
  }
  else if(  URI == uris->fabla2_RequestUiSampleState ) {
    //printf("UI requested %i, %i, %i\n", b, p, l );
    {
      writeSampleState( b, p, l, s );
      
      // causes double-frees / corruption somewhere
      //tx_waveform( b, p, l, s->getWaveform() );
    }
  }
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

