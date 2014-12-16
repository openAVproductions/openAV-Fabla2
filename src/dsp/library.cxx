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

#include "library.hxx"

#include "bank.hxx"

namespace Fabla2
{

Library::Library( Fabla2DSP* d, int rate ) :
  dsp( d )
{
  // add the 4 initial banks
  bank( new Bank( d, rate, 0, "A" ) );
  bank( new Bank( d, rate, 1, "B" ) );
  bank( new Bank( d, rate, 2, "C" ) );
  bank( new Bank( d, rate, 3, "D" ) );
}

void Library::bank( Bank* b )
{
  banks.push_back( b );
}

Bank* Library::bank( int id )
{
  for (std::list< yasper::ptr<Bank> >::iterator it= banks.begin(); it != banks.end(); ++it)
  {
    if( (*it)->ID() == id )
    {
      return (*it);
    }
  }
  return 0;
}

Library::~Library()
{
}

}; // Fabla2
