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

#ifndef OPENAV_FABLA2_SHARED_HXX
#define OPENAV_FABLA2_SHARED_HXX

// for FABLA2_PORT_COUNT
#include "dsp/ports.hxx"

#include "lv2/lv2plug.in/ns/lv2core/lv2.h"

#include "lv2/lv2plug.in/ns/ext/atom/atom.h"
#include "lv2/lv2plug.in/ns/ext/atom/util.h"
#include "lv2/lv2plug.in/ns/ext/midi/midi.h"
#include "lv2/lv2plug.in/ns/ext/urid/urid.h"
#include "lv2/lv2plug.in/ns/ext/atom/forge.h"
#include "lv2/lv2plug.in/ns/ext/patch/patch.h"
#include "lv2/lv2plug.in/ns/ext/log/log.h"
#include "lv2/lv2plug.in/ns/ext/log/logger.h"
#include "lv2/lv2plug.in/ns/ext/worker/worker.h"

#define FABLA2_URI    "http://www.openavproductions.com/fabla2"
#define FABLA2_UI_URI "http://www.openavproductions.com/fabla2#gui"

//#define EG_SAMPLER_URI          "http://lv2plug.in/plugins/eg-sampler"
//#define EG_SAMPLER__sample      EG_SAMPLER_URI "#sample"

/// Atom Event types
#define FABLA2_PadPlay            FABLA2_URI "#PadPlay"
#define FABLA2_PadStop            FABLA2_URI "#PadStop"

#define FABLA2_SampleAdsrAttack   FABLA2_URI "#SampleAdsrAttack"
#define FABLA2_SampleAdsrDecay    FABLA2_URI "#SampleAdsrDecay"
#define FABLA2_SampleAdsrSustain  FABLA2_URI "#SampleAdsrSustain"
#define FABLA2_SampleAdsrRelease  FABLA2_URI "#SampleAdsrRelease"
#define FABLA2_SampleGain         FABLA2_URI "#SampleGain"
#define FABLA2_SamplePan          FABLA2_URI "#SamplePan"
#define FABLA2_SamplePitch        FABLA2_URI "#SamplePitch"
#define FABLA2_SampleStartPoint   FABLA2_URI "#SampleStartPoint"
#define FABLA2_SampleLoad         FABLA2_URI "#SampleLoad"
#define FABLA2_SampleUnload       FABLA2_URI "#SampleUnload"
#define FABLA2_SampleAudioData    FABLA2_URI "#SampleAudioData"

/// "Inside Atoms" data types
#define FABLA2_sample             FABLA2_URI "#sample"
#define FABLA2_bank               FABLA2_URI "#bank"
#define FABLA2_pad                FABLA2_URI "#pad"
#define FABLA2_layer              FABLA2_URI "#layer"
#define FABLA2_velocity           FABLA2_URI "#velocity"
#define FABLA2_value              FABLA2_URI "#value"


typedef struct {
  LV2_URID atom_Blank;
  LV2_URID atom_Path;
  LV2_URID atom_Int;
  LV2_URID atom_Float;
  LV2_URID atom_Resource;
  LV2_URID atom_Sequence;
  LV2_URID atom_URID;
  LV2_URID atom_eventTransfer;

  LV2_URID midi_MidiEvent;
  
  LV2_URID patch_Set;
  LV2_URID patch_property;
  LV2_URID patch_value;
  
  LV2_URID fabla2_PadPlay;
  LV2_URID fabla2_PadStop;
  
  LV2_URID fabla2_SampleAdsrAttack;
  LV2_URID fabla2_SampleAdsrDecay;
  LV2_URID fabla2_SampleAdsrSustain;
  LV2_URID fabla2_SampleAdsrRelease;
  LV2_URID fabla2_SampleGain;
  LV2_URID fabla2_SamplePan;
  LV2_URID fabla2_SamplePitch;
  LV2_URID fabla2_SampleStartPoint;
  LV2_URID fabla2_SampleLoad;
  LV2_URID fabla2_SampleUnload;
  LV2_URID fabla2_SampleAudioData;
  
  LV2_URID fabla2_sample;
  LV2_URID fabla2_velocity;
  LV2_URID fabla2_bank;
  LV2_URID fabla2_pad;
  LV2_URID fabla2_layer;
  LV2_URID fabla2_value;
} URIs;

static void mapUri( URIs* uris, LV2_URID_Map* map )
{
  uris->atom_Blank                  = map->map(map->handle, LV2_ATOM__Blank);
  uris->atom_Path                   = map->map(map->handle, LV2_ATOM__Path);
  uris->atom_Int                    = map->map(map->handle, LV2_ATOM__Int);
  uris->atom_Float                  = map->map(map->handle, LV2_ATOM__Float);
  uris->atom_Resource               = map->map(map->handle, LV2_ATOM__Resource);
  uris->atom_Sequence               = map->map(map->handle, LV2_ATOM__Sequence);
  uris->atom_URID                   = map->map(map->handle, LV2_ATOM__URID);
  uris->atom_eventTransfer          = map->map(map->handle, LV2_ATOM__eventTransfer);
  
  uris->midi_MidiEvent              = map->map(map->handle, LV2_MIDI__MidiEvent);
  
  uris->patch_Set                   = map->map(map->handle, LV2_PATCH__Set);
  uris->patch_property              = map->map(map->handle, LV2_PATCH__property);
  uris->patch_value                 = map->map(map->handle, LV2_PATCH__value);
  
  uris->fabla2_PadPlay              = map->map(map->handle, FABLA2_PadPlay);
  uris->fabla2_PadStop              = map->map(map->handle, FABLA2_PadStop);
  
  uris->fabla2_SampleAdsrAttack     = map->map(map->handle, FABLA2_SampleAdsrAttack);
  uris->fabla2_SampleAdsrDecay      = map->map(map->handle, FABLA2_SampleAdsrDecay);
  uris->fabla2_SampleAdsrSustain    = map->map(map->handle, FABLA2_SampleAdsrSustain);
  uris->fabla2_SampleAdsrRelease    = map->map(map->handle, FABLA2_SampleAdsrRelease);
  
  uris->fabla2_SampleGain           = map->map(map->handle, FABLA2_SampleGain);
  uris->fabla2_SamplePan            = map->map(map->handle, FABLA2_SamplePan );
  uris->fabla2_SamplePitch          = map->map(map->handle, FABLA2_SamplePitch);
  uris->fabla2_SampleStartPoint     = map->map(map->handle, FABLA2_SampleStartPoint);
  uris->fabla2_SampleLoad           = map->map(map->handle, FABLA2_SampleLoad);
  uris->fabla2_SampleUnload         = map->map(map->handle, FABLA2_SampleUnload);
  uris->fabla2_SampleAudioData      =map->map(map->handle,FABLA2_SampleAudioData);
  
  uris->fabla2_sample               = map->map(map->handle, FABLA2_sample);
  uris->fabla2_velocity             = map->map(map->handle, FABLA2_velocity);
  uris->fabla2_bank                 = map->map(map->handle, FABLA2_bank);
  uris->fabla2_pad                  = map->map(map->handle, FABLA2_pad);
  uris->fabla2_layer                = map->map(map->handle, FABLA2_layer);
  uris->fabla2_value                = map->map(map->handle, FABLA2_value);
}

#endif // OPENAV_FABLA2_SHARED_HXX
