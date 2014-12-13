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

#include "sample.hxx"

#include "pad.hxx"

namespace Fabla2
{

Sample::Sample( Fabla2DSP* d, int rate, std::string n, std::string filePathToLoad  ) :
  dsp( d ),
  sr(rate),
  name( n ),
  isMono_( true )
{
  // load the audio data from disk here
  // filePathToLoad
  
  // isMono_
}

/// the process function: explicitly passed in voice buffers for FX later
void Sample::process(int nframes, float* L, float* R)
{
  //if( 
}

bool Sample::isMono()
{
  return isMono_;
}

Sample::~Sample()
{
}

};
