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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "dsp/ports.hxx"
#include "dsp/fabla2.hxx"
using namespace Fabla2;


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
  
  if (!map)
    return 0;
  
  FablaLV2* tmp = new FablaLV2( samplerate );
  tmp->log      = log;
  tmp->map      = map;
  tmp->unmap    = unmap;
  tmp->schedule = schedule;
  
  mapUri( &tmp->uris, map );
  lv2_atom_forge_init( &tmp->forge, map);
  lv2_log_logger_init( &tmp->logger, tmp->map, tmp->log);
  
  return (LV2_Handle)tmp;
}

FablaLV2::FablaLV2(int rate)
{
  dsp = new Fabla2::Fabla2DSP( rate );
  if( !dsp )
  {
    printf("Fabla2DSP() failed in FablaLV2::instantiate() Aborting.\n");
  }
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
      case ATOM_IN:
          self->control = (const LV2_Atom_Sequence*)data;
          break;
      
      case ATOM_OUT:
          self->notify  = (LV2_Atom_Sequence*)data;
          break;
      
      // and push all other float*s for audio / control into the controlPorts
      // array. They can be retrieved using enum in ports.hxx
      default:
          self->dsp->controlPorts[port]     = (float*)data;
          break;
  }
}

void FablaLV2::run(LV2_Handle instance, uint32_t nframes)
{
  FablaLV2* self = (FablaLV2*) instance;
  
  // fifths.c example from book
  // Get the capacity
  const uint32_t notify_capacity = self->notify->atom.size;
  // Write an empty Sequence header to the output
  lv2_atom_sequence_clear(self->notify);
  self->notify->atom.type = self->control->atom.type;
  
  printf("notify capacity %i\n", notify_capacity );
  
  lv2_atom_forge_set_buffer(&self->forge,
                            (uint8_t*)self->notify,
                            notify_capacity);
  
  //LV2_Atom_Forge_Frame notify_frame;
  lv2_atom_forge_sequence_head(&self->forge, &self->notify_frame, 0);
  /*
  // sampler.c example from /book
  // Set up forge to write directly to notify output port.
  const uint32_t notify_capacity = self->notify->atom.size;
  lv2_atom_forge_set_buffer(&self->forge,
                            (uint8_t*)self->notify,
                            notify_capacity);

  // Start a sequence in the notify output port.
  LV2_Atom_Forge_Frame notify_frame;
  lv2_atom_forge_sequence_head(&self->forge, &notify_frame, 0);
  */
  
  
  /*
   * MY OLD CODE
   * 
  // setup Forge for Atom output (to UI)
  const uint32_t notify_capacity = self->notify->atom.size;
  lv2_atom_forge_set_buffer(&self->forge, (uint8_t*)self->notify, notify_capacity);
  lv2_atom_sequence_clear(self->notify);
  // Start a sequence in the notify output port.
  lv2_atom_forge_sequence_head(&self->forge, &self->notify_frame, 0);
  */
  
  int midiMessagesIn = 0;
  // handle incoming MIDI
  LV2_ATOM_SEQUENCE_FOREACH(self->control, ev)
  {
    if (ev->body.type == self->uris.midi_MidiEvent)
    {
      midiMessagesIn++;
      if( midiMessagesIn > 1 )
      {
        printf("MidiMessage IN %d\n", midiMessagesIn );
        //lv2_log_note(&self->logger, "MidiMessage IN %d\n", midiMessagesIn );
      }
      const uint8_t* const msg = (const uint8_t*)(ev + 1);
      self->dsp->midi( ev->time.frames, msg );
      
      
      bool send = ((msg[0] & 0xF0) == 0x90 || (msg[0] & 0xF0) == 0x80 );
      int chnl  =  (msg[0] & 0x0F);
      
      if( send && chnl >= 0 && chnl < 4 )
      {
        int pad = msg[1] - 36;
        printf("sending %i %i %i\n", msg[0], pad, msg[2] );
        
        // write note on/off MIDI events to UI
        
        LV2_Atom_Forge_Frame frame;
        lv2_atom_forge_frame_time( &self->forge, ev->time.frames );
        lv2_atom_forge_object( &self->forge, &frame, 0, self->uris.fabla2_PadEvent );
        
        // Add UI state as properties
        lv2_atom_forge_key(&self->forge, self->uris.fabla2_bank);
        lv2_atom_forge_int(&self->forge, 0 );
        
        lv2_atom_forge_key(&self->forge, self->uris.fabla2_pad);
        lv2_atom_forge_int(&self->forge, pad );
        
        lv2_atom_forge_key(&self->forge, self->uris.fabla2_velocity);
        lv2_atom_forge_int(&self->forge, msg[2] );
      }
      else if( (msg[0] & 0xF0) == 0xB0 ) // control change
      {
        
      }
      
    }
    else if (lv2_atom_forge_is_object_type(&self->forge, ev->body.type))
    {
        const LV2_Atom_Object* obj = (const LV2_Atom_Object*)&ev->body;
        if (obj->body.otype == self->uris.patch_Set)
        {
          // Received a set message, send it to the worker.
          printf("Queueing set message\n");
          lv2_log_trace(&self->logger, "Queueing set message\n");
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
      lv2_log_trace(&self->logger, "Unknown event type %d\n", ev->body.type);
    }
  }
  
  /*
  for(int i = 0; i < 5; i++)
  {
    LV2_Atom_Forge_Frame& frame = self->notify_frame;
    lv2_atom_forge_frame_time( &self->forge, 0 ); // frame time of 0
    lv2_atom_forge_object( &self->forge, &frame, 0, self->uris.fabla2_SampleAudioData );
    
    lv2_atom_forge_key(&self->forge, self->uris.fabla2_pad);
    lv2_atom_forge_int(&self->forge, -1 );
  }
  */
  
  self->dsp->process( nframes );
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
