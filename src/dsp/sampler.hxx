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

#ifndef OPENAV_FABLA2_SAMPLER_HXX
#define OPENAV_FABLA2_SAMPLER_HXX

namespace Fabla2
{

class Pad;
class Sample;
class Fabla2DSP;

/** Sampler
 * The Sampler class handles sample file playback. Its main functionality is to
 * accept MIDI input, map velocity layers, and produce audio.
 * 
 * The secondary duty of the Sampler is to record incoming audio and store it
 * using the Library class for playback later.
 */
class Sampler
{
  public:
    Sampler( Fabla2DSP* dsp, int rate );
    ~Sampler();
    
    /// the Pad to play
    void play( Pad*, int velocity );
    
    /// process function, passing in the voice buffers for FX
    int process(int nframes, float* L, float* R);
    
    Pad*    getPad()    {return pad   ;}
    Sample* getSample() {return sample;}
  
  private:
    Fabla2DSP* dsp;
    int sr;
    
    /// Pad pointer, which contains the audio data and triggering types that are
    /// needed in order to process this sampler object
    Pad* pad;
    
    /// Sample pointer, retrieved from Pad when the note started playing
    Sample* sample;
    
    /// audio playback variables
    long playIndex;
    
    
};

};

#endif // OPENAV_FABLA2_SAMPLER_HXX
