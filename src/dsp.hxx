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

#include "shared.hxx"

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
    
    // LV2 Atom Ports
    const LV2_Atom_Sequence* in_port;
    LV2_Atom_Sequence*       out_port;
    
    // Forge for Atoms
    LV2_Atom_Forge forge;
    LV2_Atom_Forge_Frame notify_frame;
    
    // Log feature and convenience API
    LV2_Log_Log*   log;
    LV2_Log_Logger logger;
    
    // Worker thread
    LV2_Worker_Schedule* schedule;
    
    // Features
    LV2_URID_Map*   map;
    LV2_URID_Unmap* unmap;
    
    URIs uris;
    
    /// the actual DSP instance: public for LV2 Work Response, LV2 State Save
    Fabla2::Fabla2DSP* dsp;
  
  private:
    /// Sample rate
    int sr;
    
    /// A buffer for the AuxBus ports if the host doesn't provide them
    float* auxBusBuffer;

    /// convienience functions to extract bank/pad info from an Atom
    /// @return 0 on success, non-zero on error
    int atomBankPadLayer( const LV2_Atom_Object* obj,
                          int& bank,
                          int& pad,
                          int& layer,
                          float& value );
    
};

#endif // OPENAV_FABLA2_LV2_HXX
