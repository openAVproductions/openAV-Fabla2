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

#include "bank.hxx"

#include "pad.hxx"
#include "pattern.hxx"
#include <assert.h>
#include <stdio.h>
#include <cstring>

namespace Fabla2
{

Bank::Bank( Fabla2DSP* d, int rate, int ID, const char* name ) :
	dsp( d ),
	ID_( ID )
{
	pattern = new Pattern(d, rate);
}

void Bank::name( const char* name )
{
	memcpy( name_, name, 20 );
	name_[20] = '\n';
}

void Bank::pad( Pad* p )
{
	assert( p );
#ifdef FABLA2_DEBUG
	//printf("%s : %i : Adding pad %i\n", __PRETTY_FUNCTION__, ID_, p->ID() );
#endif // FABLA2_DEBUG
	pads.push_back( p );
}

void Bank::checkAll()
{
	printf("%s : Starting...\n", __PRETTY_FUNCTION__ );
	for(int bi = 0; bi < 4; bi++ ) {
		Pad* p = pad( bi );
		if( !p ) {
			printf("%s : Pad %i == 0\n", __PRETTY_FUNCTION__, bi );
		} else {

		}
	}
	printf("%s : Done.\n", __PRETTY_FUNCTION__ );
}

Pad* Bank::pad( int n )
{
	if( n < pads.size() && n >= 0 ) {
		return pads.at(n);
	}

	return 0;
}

Bank::~Bank()
{
	for(int i = 0; i < pads.size(); i++) {
		delete pads.at(i);
	}
}

}; // Fabla2
