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

#ifndef OPENAV_FABLA2_PAD_HXX
#define OPENAV_FABLA2_PAD_HXX

#include "yasper.hxx"

#include <vector>
#include <string>

namespace Fabla2
{

class Sample;
class Fabla2DSP;

/** Pad
 * The Pad class is a collection of resources that apply to this pad. As multiple
 * samples can be hosted on one pad, it is essential to have a clean structure to
 * maintain these resources. The Pad class takes care of this.
 * 
 * Multiple samples are contained here, as well as the necessary options to play
 * them back: round-robin or velocity mapped? Both?
 * 
 * The Library class loads / saves data from the Pads, providing a clean method
 * to interact with the Pad contents in an RT-safe way.
 */
class Pad
{
  public:
    Pad( Fabla2DSP* dsp, int rate, int ID );
    ~Pad();
    
    int ID(){return ID_;}
    
    enum PAD_CONTROLS {
      FILTER_CUTOFF,
      CONTROLS_COUNT, // always last
    };
    
    float controls[CONTROLS_COUNT];
    
    /// library functions
    void add( Sample* );
    void clearAllSamples();
    
    void muteGroup( int mg ){muteGroup_ = mg;}
    int  muteGroup(){return muteGroup_;}
    
    /// get a layer: wether its velocity or Round-robin doesn't matter: this
    /// is for UI interaction
    Sample* layer( int id );
    
    /// playback functions
    int lastPlayedLayer();
    Sample* getPlaySample( int velocity );
    
    /// Sets the switch system between samples
    enum SAMPLE_SWITCH_SYSTEM {
      SS_NONE = 0,        /// always plays first sample added
      SS_ROUND_ROBIN,     /// iterates over all samples 1 by 1
      SS_VELOCITY_LAYERS, /// takes velocity into account, and plays a sample
    };
    void switchSystem( SAMPLE_SWITCH_SYSTEM sss );
    
    /// testing func
    void checkAll();
  
  private:
    Fabla2DSP* dsp;
    int sr;
    int ID_; // pad place within Bank
    int muteGroup_;
    
    SAMPLE_SWITCH_SYSTEM sampleSwitchSystem;
    int sampleLayerCounter;
    
    /// shared pointer to each of the samples available on this pad
    std::vector< yasper::ptr<Sample> > samples;
    
};

};

#endif // OPENAV_FABLA2_PAD_HXX
