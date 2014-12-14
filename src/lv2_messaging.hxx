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

#ifndef OPENAV_FABLA2_LV2_MESSAGING_HXX
#define OPENAV_FABLA2_LV2_MESSAGING_HXX

/// A helper header to read / write Atom messages

static inline LV2_Atom* fabla2_writeSampleLoadUnload(
   LV2_Atom_Forge*  forge,
   const URIs*      uris,
   const bool       load,
   const char*      filename,
   const uint32_t   filename_len)
{
  LV2_Atom_Forge_Frame frame;
  LV2_Atom* set = (LV2_Atom*)lv2_atom_forge_object( forge, &frame, 0, uris->patch_Set);
  
  lv2_atom_forge_key(forge, uris->patch_property);
  
  if( load )
    lv2_atom_forge_urid(forge, uris->fabla2_SampleLoad);
  else
    lv2_atom_forge_urid(forge, uris->fabla2_SampleUnload);
  
  lv2_atom_forge_key(forge, uris->patch_value);
  lv2_atom_forge_path(forge, filename, filename_len);
  
  lv2_atom_forge_pop(forge, &frame);
  return set;
}


#endif // OPENAV_FABLA2_LV2_MESSAGING_HXX

