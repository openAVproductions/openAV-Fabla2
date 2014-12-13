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

#ifndef OPENAV_FABLA2_VOICE_HXX
#define OPENAV_FABLA2_VOICE_HXX

#include "yasper.hxx"
#include "dsp_adsr.hxx"

#include <vector>

namespace Fabla2
{

class Pad;
class FxUnit;
class Sampler;
class FiltersSVF;

class Fabla2DSP;

/** Voice
 * The Voice class is a currently active sound-sample / synth object that is
 * being processed. It accepts audio two inputs, one MIDI input, and generates
 * two main outs, along with 4 submix outputs.
 * 
 * The Voice class itself has sub-objects:
 * 1) Synth   - a small but flexible drum-synth module
 * 2) Sampler - a typical sample-playback engine
 * 3) FxUnit  - applied to the output of the Synth and Sampler
 * 4) Mixer   - mix down the audio streams to master / submix busses.
 */
class Voice
{
  public:
    Voice( Fabla2DSP* dsp, int rate );
    ~Voice();
    
    bool active(){return active_;}
    
    /// start playing a sample on this voice
    void play( Pad*, int velocity );
    
    /// start recording a sample on this voice
    
    /// the main audio callback: since we have the dsp pointer, we can access the
    /// audio buffers etc from there: no need to pass them around.
    void process();
    
    //ADSR* getADSR(){return adsr.GetRawPointer();}
    //Sampler* getSampler(){return sampler.GetRawPointer();}
    
    float* getVoiceBuffer(){return &voiceBuffer[0];}
    
  
  private:
    Fabla2DSP* dsp;
    int sr;
    
    Pad* pad;
    
    bool active_;
    
    float phase;
    
    yasper::ptr<ADSR> adsr;
    yasper::ptr<Sampler> sampler;
    yasper::ptr<FiltersSVF> filterL;
    yasper::ptr<FiltersSVF> filterR;
    
    std::vector<float> voiceBuffer;
    
};

};

#endif // OPENAV_FABLA2_VOICE_HXX
