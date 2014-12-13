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

#include "pad.hxx"

#include "sample.hxx"
#ifdef FABLA2_COMPONENT_TEST
#include <stdio.h>
#include "tests/qunit.hxx"
extern QUnit::UnitTest qunit;
#endif 


namespace Fabla2
{

Pad::Pad( Fabla2DSP* d, int rate ) :
  dsp( d ),
  sr(rate)
{
}

void Pad::add( Sample* s )
{
  samples.push_back( s );
}

Sample* Pad::getPlaySample( int velocity )
{
#ifdef FABLA2_COMPONENT_TEST
  QUNIT_IS_TRUE( samples.size() > 0 );
#endif
  /// Logic to do round-robin / velocity mapping here
  if( samples.size() )
    return samples.at( 0 );
  
  return 0;
}

Pad::~Pad()
{
#ifdef FABLA2_COMPONENT_TEST
  printf("%s\n", __PRETTY_FUNCTION__ );
#endif
}

};
