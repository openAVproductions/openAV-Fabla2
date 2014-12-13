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
#include "plotter.hxx"

#include <sndfile.h>
#include <stdio.h>

#ifdef FABLA2_COMPONENT_TEST
#include "tests/qunit.hxx"
extern QUnit::UnitTest qunit;
#endif 

namespace Fabla2
{

Sample::Sample( Fabla2DSP* d, int rate, std::string n, std::string path  ) :
  dsp( d ),
  sr(rate),
  name( n ),
  channels( 0 ),
  frames( 0 ),
  velLow( 0 ),
  velHigh( 127 )
{
  SF_INFO info;
  SNDFILE* const sndfile = sf_open( path.c_str(), SFM_READ, &info);
  if ( !sndfile )
  {
    printf("Failed to open sample '%s'\n", path.c_str() );
    return;
  }
  
  channels = info.channels;
  frames   = info.frames;
  
  if( info.channels > 2 )
  {
    printf("Error loading sample %s, channels >= 2\n", path.c_str() );
    return;
  }
  
  audio.resize( frames * channels );
  
  sf_seek(sndfile, 0ul, SEEK_SET);
  sf_read_float( sndfile, &audio[0], info.frames *channels );
  sf_close(sndfile);
  
#ifdef FABLA2_COMPONENT_TEST
  if( false )
  {
    Plotter::plot( path, frames * channels, &audio[0] );
    printf("Sample %s loaded OK: Channels = %i, Frames = %i\n", path.c_str(), channels, frames );
  }
  QUNIT_IS_TRUE( info.frames > 0 );
  QUNIT_IS_TRUE( audio.size() == info.frames );
#endif
}

void Sample::velocity( int low, int high )
{
  velLow  = low;
  velHigh = high;
}

bool Sample::velocity( int vel )
{
  if( vel > velLow &&
      vel <= velHigh )
  {
    return true;
  }
  
  return false;
}

Sample::~Sample()
{
#ifdef FABLA2_COMPONENT_TEST
  printf("%s\n", __PRETTY_FUNCTION__ );
#endif
}

};
