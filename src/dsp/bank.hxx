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

#ifndef OPENAV_FABLA2_BANK_HXX
#define OPENAV_FABLA2_BANK_HXX

#include <vector>
#include "yasper.hxx"

namespace Fabla2
{

class Pad;
class Fabla2DSP;

/** Bank
 * The Bank class contains resources: 16 pads, and some settings like MIDI
 * channel for control-change values etc.
 */
class Bank
{
  public:
    Bank( Fabla2DSP* dsp, int rate, int ID, const char* name );
    ~Bank();
    
    int ID(){return ID_;}
    
    /// add resources for a certain bank/pad
    void pad( Pad* p );
    
    /// get a pad based on its location in the grid
    Pad* pad( int num );
    
    void name( const char* name );
    
    /// testing function, to see if there are null pointers in the system
    void checkAll();
    
  private:
    Fabla2DSP* dsp;
    int ID_; ///< Bank ABCD id
    char name_[21]; // 20 letters + \n
    
    
    std::vector< yasper::ptr<Pad> > pads;
};

};

#endif // OPENAV_FABLA2_BANK_HXX
