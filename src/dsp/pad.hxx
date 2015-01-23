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
#include <stdio.h>

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
    
    /// position of pad (0 - 15) drum pads layout where 0 is bottom left
    int ID(){return ID_;}
    
    // arbitrary limit of 20 chars max for names
    void setName( const char* n );
    const char* getName(){return name;}
    
    /// the bank this Pad is on
    void bank(int b){bank_ = b;}
    int bank(){return bank_;}
    
    /// library functions
    void add( Sample* );
    void remove( Sample* s );
    
    void clearAllSamples();
    bool loaded(){return loaded_;}
    
    void muteGroup( int mg ){muteGroup_ = mg; }
    int  muteGroup(){ return muteGroup_;}
    
    void offGroup( int og ){offGroup_ = og;}
    int  offGroup(){ return offGroup_;}
    
    /// Sets play method for samples: gated or one-shot
    enum TRIGGER_MODE {
      TM_GATED = 0,       /// Note on starts, note off does ADSR->release()
      TM_ONE_SHOT,        /// Always plays samples until end
    };
    
    void triggerMode( TRIGGER_MODE tm ){ triggerMode_ = tm; }
    int triggerMode(){ return triggerMode_; }
    
    /// get a layer: wether its velocity or Round-robin doesn't matter: this
    /// is for UI interaction
    int nLayers(){return samples.size();}
    Sample* layer( int id );
    
    /// playback functions
    int lastPlayedLayer();
    Sample* getPlaySample( int velocity );
    
    /// Sets the switch system between samples
    enum SAMPLE_SWITCH_SYSTEM {
      SS_NONE = 0,        /// always plays selected sample
      SS_ROUND_ROBIN,     /// iterates over all samples incrementally
      SS_VELOCITY_LAYERS, /// takes velocity into account, and plays a sample
    };
    void switchSystem( SAMPLE_SWITCH_SYSTEM sss );
    int switchSystem(){ return sampleSwitchSystem; }
    
    /// testing func
    void checkAll();
  
    /// volume: is used by Sampler to multiply into each sample. Default 0.75.
    float volume;
    float sends[4];
  
  private:
    Fabla2DSP* dsp;
    int sr;
    int bank_;// pad bank
    int ID_; // pad place within Bank
    int muteGroup_;
    int offGroup_;
    int triggerMode_;
    bool loaded_;
    
    char name[21];
    
    SAMPLE_SWITCH_SYSTEM sampleSwitchSystem;
    int sampleLayerCounter;
    
    /// shared pointer to each of the samples available on this pad
    std::vector< yasper::ptr<Sample> > samples;
};

};

#endif // OPENAV_FABLA2_PAD_HXX
