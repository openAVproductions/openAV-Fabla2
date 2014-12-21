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

#ifndef OPENAV_FABLA2_LV2_WORK_HXX
#define OPENAV_FABLA2_LV2_WORK_HXX

#include "lv2/lv2plug.in/ns/ext/atom/atom.h"
#include "lv2/lv2plug.in/ns/ext/worker/worker.h"

namespace Fabla2
{
  class Sample;
};

/// definitions of the Work:schedule LV2 extension functions
/// See lv2_work.cxx for implementations.
extern LV2_Worker_Status
fabla2_work(LV2_Handle           instance,
     LV2_Worker_Respond_Function respond,
     LV2_Worker_Respond_Handle   handle,
     uint32_t                    size,
     const void*                 data);

extern LV2_Worker_Status
fabla2_work_response(LV2_Handle  instance,
                    uint32_t    size,
                    const void* data);

/// events that can be sent to the worker thread
typedef struct
{
  LV2_Atom atom;
  int bank;
  int pad;
  Fabla2::Sample*  sample;
} SampleLoadUnload;

#endif // OPENAV_FABLA2_LV2_WORK_HXX
