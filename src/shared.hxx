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

#ifndef OPENAV_FABLA2_LV2_HXX
#define OPENAV_FABLA2_LV2_HXX

// for FABLA2_PORT_COUNT
#include "dsp/ports.hxx"

#include "lv2/lv2plug.in/ns/lv2core/lv2.h"

#include "lv2/lv2plug.in/ns/ext/atom/atom.h"
#include "lv2/lv2plug.in/ns/ext/atom/util.h"
#include "lv2/lv2plug.in/ns/ext/midi/midi.h"
#include "lv2/lv2plug.in/ns/ext/urid/urid.h"
#include "lv2/lv2plug.in/ns/ext/atom/forge.h"
#include "lv2/lv2plug.in/ns/ext/patch/patch.h"

#define FABLA2_URI    "http://www.openavproductions.com/fabla2"
#define FABLA2_UI_URI "http://www.openavproductions.com/fabla2#gui"

//#define EG_SAMPLER_URI          "http://lv2plug.in/plugins/eg-sampler"
//#define EG_SAMPLER__sample      EG_SAMPLER_URI "#sample"

#define FABLA2_sample           FABLA2_URI "#sample"
#define FABLA2_PadEvent         FABLA2_URI "#PadEvent"
//#define FABLA2_bank             FABLA2_URI "#bank"
#define FABLA2_pad              FABLA2_URI "#pad"
#define FABLA2_velocity         FABLA2_URI "#velocity"


typedef struct {
  LV2_URID atom_Blank;
  LV2_URID atom_Path;
  LV2_URID atom_Int;
  LV2_URID atom_Resource;
  LV2_URID atom_Sequence;
  LV2_URID atom_URID;
  LV2_URID atom_eventTransfer;

  LV2_URID midi_MidiEvent;
  
  LV2_URID patch_Set;
  LV2_URID patch_property;
  LV2_URID patch_value;
  
  LV2_URID fabla2_sample;
  LV2_URID fabla2_PadEvent;
  LV2_URID fabla2_velocity;
  LV2_URID fabla2_pad;
} URIs;

static void mapUri( URIs* uris, LV2_URID_Map* map )
{
  uris->atom_Blank         = map->map(map->handle, LV2_ATOM__Blank);
  uris->atom_Path          = map->map(map->handle, LV2_ATOM__Path);
  uris->atom_Int           = map->map(map->handle, LV2_ATOM__Int);
  uris->atom_Resource      = map->map(map->handle, LV2_ATOM__Resource);
  uris->atom_Sequence      = map->map(map->handle, LV2_ATOM__Sequence);
  uris->atom_URID          = map->map(map->handle, LV2_ATOM__URID);
  uris->atom_eventTransfer = map->map(map->handle, LV2_ATOM__eventTransfer);
  
  uris->midi_MidiEvent     = map->map(map->handle, LV2_MIDI__MidiEvent);
  
  uris->patch_Set          = map->map(map->handle, LV2_PATCH__Set);
  uris->patch_property     = map->map(map->handle, LV2_PATCH__property);
  uris->patch_value        = map->map(map->handle, LV2_PATCH__value);
  
  uris->fabla2_sample      = map->map(map->handle, FABLA2_sample);
  uris->fabla2_PadEvent    = map->map(map->handle, FABLA2_PadEvent);
  uris->fabla2_pad         = map->map(map->handle, FABLA2_pad);
  uris->fabla2_velocity    = map->map(map->handle, FABLA2_velocity);
}



/// ################################
/// DSP implementations below here #
/// ################################

namespace Fabla2
{
  class Fabla2DSP;
};

class FablaLV2
{
  public:
    FablaLV2(int rate);
    ~FablaLV2();
    static LV2_Handle instantiate(const LV2_Descriptor* descriptor,
                                  double samplerate,
                                  const char* bundle_path,
                                  const LV2_Feature* const* features);
    static void activate(LV2_Handle instance);
    static void deactivate(LV2_Handle instance);
    static void connect_port(LV2_Handle instance, uint32_t port, void *data);
    static void run(LV2_Handle instance, uint32_t n_samples);
    static void cleanup(LV2_Handle instance);
    static const void* extension_data(const char* uri);
  
  private:
    Fabla2::Fabla2DSP* dsp;
    
    // LV2 Atom Ports
    const LV2_Atom_Sequence* control;
    const LV2_Atom_Sequence* notify;
    
    // Forge for Atoms
    LV2_Atom_Forge forge;
    LV2_Atom_Forge_Frame notify_frame;
    
    // Features
    LV2_URID_Map* map;
    
    URIs uris;
    
};

#endif // OPENAV_FABLA_LV2_HXX
