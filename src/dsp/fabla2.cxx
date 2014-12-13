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

#include <stdio.h>
#include <string.h>

#include "voice.hxx"
#include "library.hxx"

namespace Fabla2
{

Fabla2DSP::Fabla2DSP( int rate )
{
  voices.push_back( new Voice( this, rate ) );
  voices.back()->play();
  
  memset( controlPorts, 0, sizeof(float*) * PORT_COUNT );
  
  midiMessages.resize( 1024 );
}

void Fabla2DSP::process( int nf )
{
  nframes = nf;
  
  memset( controlPorts[OUTPUT_L], 0, sizeof(float) * nframes );
  memset( controlPorts[OUTPUT_R], 0, sizeof(float) * nframes );
  
  for( int i = 0; i < voices.size(); i++ )
  {
    Voice* v = voices.at(i);
    if( v->active() )
    {
      v->process();
    }
  }
}

void Fabla2DSP::midi( int f, const uint8_t* msg )
{
  printf("MIDI: %i, %i, %i\n", (int)msg[0], (int)msg[1], (int)msg[2] );
  voices.at(0)->play();
  
  /// Logic for incoming MIDI -> Pad mapping
  
  /// Logic for fetch-pad-data from Library
  
  /// Logic for set-pad-data to available Voice
  
}

Fabla2DSP::~Fabla2DSP()
{
}

}; // Fabla2

