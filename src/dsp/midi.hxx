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

#ifndef OPENAV_FABLA2_MIDI_HXX
#define OPENAV_FABLA2_MIDI_HXX

#include <stdint.h>
#include <string.h>

class MidiMessage
{
  public:
    MidiMessage() :
      frame( -1 )
    {
      memset( msg, 0, sizeof(uint8_t)*3 );
    }
    
    MidiMessage( int frame_, uint8_t* msg_ ) :
      frame ( frame_ )
    {
      memcpy( msg, msg_, sizeof(uint8_t)*3 );
    }
    
    int frame;
    uint8_t msg[3];
};

#endif // OPENAV_FABLA2_MIDI_HXX
