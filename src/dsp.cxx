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

#include "dsp.hxx"
#include "shared.hxx"
#include "lv2_work.hxx"
#include "lv2_state.hxx"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "dsp/ports.hxx"
#include "dsp/fabla2.hxx"

LV2_Handle FablaLV2::instantiate( const LV2_Descriptor* descriptor,
                                  double samplerate,
                                  const char* bundle_path,
                                  const LV2_Feature* const* features)
{
  LV2_Log_Log* log = 0;
  LV2_URID_Map* map = 0;
  LV2_URID_Unmap* unmap = 0;
  LV2_Worker_Schedule* schedule = 0;
  
  for (int i = 0; features[i]; ++i)
  {
    if (!strcmp(features[i]->URI, LV2_URID__map))
    {
      map = (LV2_URID_Map*)features[i]->data;
    }
    else if (!strcmp(features[i]->URI, LV2_URID__unmap))
    {
      unmap = (LV2_URID_Unmap*)features[i]->data;
    }
    else if (!strcmp(features[i]->URI, LV2_LOG__log))
    {
      log = (LV2_Log_Log*)features[i]->data;
    }
    else if (!strcmp(features[i]->URI, LV2_WORKER__schedule))
    {
      schedule = (LV2_Worker_Schedule*)features[i]->data;
    }
  }
  
  if (!map)
  {
    fprintf( stderr, "Missing feature urid:map\n");
    return 0;
  }
  else if (!schedule)
  {
    printf("Fabla2: the host does not support Work:schedule, so Fabla2 cannot load samples without glitches! Please ask your host developers to implement Work:schedule!\n");
  }
  
  FablaLV2* tmp = new FablaLV2( samplerate );
  tmp->log      = log;
  tmp->map      = map;
  tmp->unmap    = unmap;
  tmp->schedule = schedule;
  
  mapUri( &tmp->uris, map );
  lv2_atom_forge_init( &tmp->forge, map);
  lv2_log_logger_init( &tmp->logger, tmp->map, tmp->log);
  
  tmp->dsp = new Fabla2::Fabla2DSP( samplerate, &tmp->uris );
  if( !tmp->dsp )
  {
    printf("Fabla2DSP() failed in FablaLV2::instantiate() Aborting.\n");
    delete tmp;
    return 0;
  }
  
  // a "return" pointer for the DSP to access this instance
  // needed to use forge etc
  tmp->dsp->lv2 = tmp;
  
  return (LV2_Handle)tmp;
}

FablaLV2::FablaLV2(int rate)
{
}

FablaLV2::~FablaLV2()
{
  delete dsp;
}

void FablaLV2::activate(LV2_Handle instance)
{
}

void FablaLV2::deactivate(LV2_Handle instance)
{
}

void FablaLV2::connect_port(LV2_Handle instance, uint32_t port, void *data)
{
  FablaLV2* self = (FablaLV2*) instance;
  
  switch (port)
  {
    // handle Atom ports gracefully here
    case Fabla2::ATOM_IN:
        self->in_port = (const LV2_Atom_Sequence*)data;
        break;
    case Fabla2::ATOM_OUT:
        self->out_port = (LV2_Atom_Sequence*)data;
        break;
      
    // and push all other float*s for audio / control into the controlPorts
    // array. They can be retrieved using enum in ports.hxx
    default:
        self->dsp->controlPorts[port]     = (float*)data;
        break;
  }
}

int FablaLV2::atomBankPadLayer( const LV2_Atom_Object* obj, int& b, int& p, int& l, float& v )
{
  const LV2_Atom* bank = 0;
  const LV2_Atom* pad  = 0;
  const LV2_Atom* lay  = 0;
  const LV2_Atom* fl   = 0;
  lv2_atom_object_get(obj,uris.fabla2_bank , &bank,
                          uris.fabla2_pad  , &pad,
                          uris.fabla2_layer, &lay,
                          uris.fabla2_value, &fl, 0);
  if( bank && pad )
  {
    b = ((const LV2_Atom_Int*)bank)->body;
    p = ((const LV2_Atom_Int*)pad )->body;
    
    if( lay )
      l = ((const LV2_Atom_Int*)lay )->body;
    
    // not all messages have a float, so just leave its value alone
    if( fl )
      v = ((const LV2_Atom_Float*)fl)->body;
    
    return 0;
  }
  return -1;
}

void FablaLV2::run(LV2_Handle instance, uint32_t nframes)
{
  FablaLV2* self = (FablaLV2*) instance;
  
  const uint32_t space = self->out_port->atom.size;
  //printf("Atom space = %i\n", space );
  
  // Prepare forge buffer and initialize atom-sequence
  lv2_atom_forge_set_buffer(&self->forge, (uint8_t*)self->out_port, space);
  lv2_atom_forge_sequence_head(&self->forge, &self->notify_frame, 0);
  
  int midiMessagesIn = 0;
  // handle incoming MIDI
  LV2_ATOM_SEQUENCE_FOREACH(self->in_port, ev)
  {
    if (ev->body.type == self->uris.midi_MidiEvent)
    {
      midiMessagesIn++;
      
      const uint8_t* const msg = (const uint8_t*)(ev + 1);
      self->dsp->midi( ev->time.frames, msg );
    }
    else if (lv2_atom_forge_is_object_type(&self->forge, ev->body.type))
    {
      const LV2_Atom_Object* obj = (const LV2_Atom_Object*)&ev->body;
      
      // deal with Atoms from UI
      bool noteOn  = (obj->body.otype == self->uris.fabla2_PadPlay);
      bool noteOff = (obj->body.otype == self->uris.fabla2_PadStop);
      if ( noteOn || noteOff )
      {
        int bank, pad;
        int layer = -1;
        float v;
        // convienience func to get bank, pad, and check OK
        if( self->atomBankPadLayer( obj, bank, pad, layer, v ) == 0 )
        {
          // check if we have layer info
          if( layer != -1 )
          {
            //printf("audition voice play()\n");
            if( noteOn )
            {
              self->dsp->auditionPlay( bank, pad, layer );
            }
            else
              self->dsp->auditionStop();
          }
          else
          {
            uint8_t msg[3];
            if( noteOn )
              msg[0] = 0x90 + bank;
            if( noteOff )
              msg[0] = 0x80 + bank;
            msg[1] = 36 + pad;
            msg[2] = 90;
            // use normal MIDI function for playing notes
            self->dsp->midi( 0, msg );
          }
        }
      }
      // handle *ALL* UI message types here!
      else if (obj->body.otype == self->uris.fabla2_SampleGain            ||
               obj->body.otype == self->uris.fabla2_SamplePitch           ||
               obj->body.otype == self->uris.fabla2_SamplePan             ||
               
               obj->body.otype == self->uris.fabla2_SampleStartPoint      ||
               obj->body.otype == self->uris.fabla2_SampleEndPoint        ||
               
               obj->body.otype == self->uris.fabla2_SampleVelStartPnt     ||
               obj->body.otype == self->uris.fabla2_SampleVelEndPnt       ||
               
               obj->body.otype == self->uris.fabla2_SampleFilterType      ||
               obj->body.otype == self->uris.fabla2_SampleFilterFrequency ||
               obj->body.otype == self->uris.fabla2_SampleFilterResonance ||
               
               obj->body.otype == self->uris.fabla2_RequestUiSampleState  ||
               
               obj->body.otype == self->uris.fabla2_SampleAdsrAttack      ||
               obj->body.otype == self->uris.fabla2_SampleAdsrDecay       ||
               obj->body.otype == self->uris.fabla2_SampleAdsrSustain     ||
               obj->body.otype == self->uris.fabla2_SampleAdsrRelease     ||
               
               obj->body.otype == self->uris.fabla2_PadMuteGroup          ||
               obj->body.otype == self->uris.fabla2_PadOffGroup           ||
               obj->body.otype == self->uris.fabla2_PadTriggerMode        ||
               obj->body.otype == self->uris.fabla2_PadSwitchType         ||
               
               obj->body.otype == self->uris.fabla2_PadAuxBus1            ||
               obj->body.otype == self->uris.fabla2_PadAuxBus2            ||
               obj->body.otype == self->uris.fabla2_PadAuxBus3            ||
               obj->body.otype == self->uris.fabla2_PadAuxBus4            ||
               
               obj->body.otype == self->uris.fabla2_SampleUnload          ||
               
               obj->body.otype == self->uris.fabla2_PadVolume             ||
               obj->body.otype == self->uris.fabla2_Panic                 ||
               false )
      {
        int bank, pad, layer;
        float value = -1;
        if( self->atomBankPadLayer( obj, bank, pad, layer, value ) == 0 )
        {
          self->dsp->uiMessage( bank, pad, layer, obj->body.otype, value );
        }
      }
      else if (obj->body.otype == self->uris.patch_Set)
      {
        // Received a set message, send it to the worker.
        //lv2_log_trace(&self->logger, "Queueing set message\n");
        self->schedule->schedule_work(self->schedule->handle,
                                      lv2_atom_total_size(&ev->body),
                                      &ev->body);
      }
      else
      {
        lv2_log_trace(&self->logger, "Unknown object type %d\n", self->unmap->unmap( self->unmap->handle, obj->body.otype) );
      }
    }
    else
    {
      lv2_log_trace(&self->logger, "Fabla2DSP: Unknown event type %d\n", self->unmap->unmap( self->unmap->handle, ev->body.type) );
    }
  }
  
  
  self->dsp->process( nframes );
  
  return;
}

void FablaLV2::cleanup(LV2_Handle instance)
{
  delete ((FablaLV2*) instance);
}

const void* FablaLV2::extension_data(const char* uri)
{
  static const LV2_Worker_Interface worker = { fabla2_work, fabla2_work_response, NULL };
  if (!strcmp(uri, LV2_WORKER__interface)) {
    return &worker;
  }
  
  static const LV2_State_Interface state_iface = { fabla2_save, fabla2_restore };
  if (!strcmp(uri, LV2_STATE__interface)) {
      return &state_iface;
  }
  
  return NULL;
}



static const LV2_Descriptor descriptor = {
	FABLA2_URI,
	FablaLV2::instantiate,
	FablaLV2::connect_port,
	FablaLV2::activate,
	FablaLV2::run,
	FablaLV2::deactivate,
	FablaLV2::cleanup,
	FablaLV2::extension_data
};

LV2_SYMBOL_EXPORT const LV2_Descriptor* lv2_descriptor(uint32_t index)
{
  if( index == 0 )
    return &descriptor;
  
  return 0;
}
