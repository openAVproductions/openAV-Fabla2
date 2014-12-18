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

#ifndef OPENAV_FABLA2_LIBRARY_HXX
#define OPENAV_FABLA2_LIBRARY_HXX

#include <list>
#include "yasper.hxx"

namespace Fabla2
{

class Bank;
class Fabla2DSP;

/** Library
 * The Library class holds all resources. When a Voice gets a play() event, the
 * appropriate resources are linked from the Library. This avoids multiple loading
 * of sample files, and allows voices to play back any sample.
 */
class Library
{
  public:
    Library( Fabla2DSP* dsp, int rate );
    ~Library();
    
    /// add resources for a certain bank/pad
    void bank( Bank* b );
    Bank* bank( int ID );
    
    /// testing function, to see if there are null pointers in the system
    void checkAll();
    
  private:
    Fabla2DSP* dsp;
    std::list< yasper::ptr<Bank> > banks;
};

};

#endif // OPENAV_FABLA2_LIBRARY_HXX
