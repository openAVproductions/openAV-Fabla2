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

#ifndef OPENAV_FABLA2_LV2_STATE_HXX
#define OPENAV_FABLA2_LV2_STATE_HXX

#include "lv2/lv2plug.in/ns/ext/state/state.h"

LV2_State_Status
fabla2_save(LV2_Handle                 instance,
            LV2_State_Store_Function   store,
            LV2_State_Handle           handle,
            uint32_t                   flags,
            const LV2_Feature *const * features);

LV2_State_Status
fabla2_restore(LV2_Handle                  instance,
               LV2_State_Retrieve_Function retrieve,
               LV2_State_Handle            handle,
               uint32_t                    flags,
               const LV2_Feature *const *  features);

#endif // OPENAV_FABLA2_LV2_STATE_HXX

