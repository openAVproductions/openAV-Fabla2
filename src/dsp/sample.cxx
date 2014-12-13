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
  isMono_( true )
{
  SF_INFO info;
  SNDFILE* const sndfile = sf_open( path.c_str(), SFM_READ, &info);
  if ( !sndfile )
  {
    printf("Failed to open sample '%s'\n", path.c_str() );
    return;
  }
  
  if( info.channels != 1 )
  {
    printf("Error loading sample %s, channels != 1\n", path.c_str() );
    return;
  }
  
  audio.resize( info.frames );
  
  sf_seek(sndfile, 0ul, SEEK_SET);
  sf_read_float( sndfile, &audio[0], info.frames );
  sf_close(sndfile);

#ifdef FABLA2_COMPONENT_TEST
  QUNIT_IS_TRUE( info.frames > 0 );
  QUNIT_IS_TRUE( audio.size() == info.frames );
#endif
}

/// the process function: explicitly passed in voice buffers for FX later
void Sample::process(int nframes, int& playhead, const float& resample, float* L, float* R)
{
  if( isMono_ )
  {
    for(int i = 0; i < nframes; i++ )
    {
      *L++ = audio.at( playhead   );
      *R++ = audio.at( playhead++ );
    }
  }
  else
  {
    for(int i = 0; i < nframes; i++ )
    {
      *L++ = audio.at( playhead++ );
      *R++ = audio.at( playhead++ );
    }
  }
}

Sample::~Sample()
{
#ifdef FABLA2_COMPONENT_TEST
  printf("%s\n", __PRETTY_FUNCTION__ );
#endif
}

};
