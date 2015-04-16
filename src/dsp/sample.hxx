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

#ifndef OPENAV_FABLA2_SAMPLE_HXX
#define OPENAV_FABLA2_SAMPLE_HXX

#include "../shared.hxx"

#include <string>
#include <vector>

namespace Fabla2
{

class Fabla2DSP;

/** Sample
 * The Sample instance is a single sound-object. This sample data can be mono or
 * stereo, but the process() callback provides a stereo signal.
 */
class Sample
{
  public:
    /// normal constructor: loads an audio sample from disk using sndfile
    Sample( Fabla2DSP* dsp, int rate, std::string name, std::string filePathToLoad );
    
    /// record constructor: creates a Sample based on live-recorded audio data
    Sample( Fabla2DSP* dsp, int rate, const char* name, int size, float* data );
    
    ~Sample();
    
    /// writes this sample to disk: used by LV2 State save()
    bool write( const char* filename );
    
    /// gives the name of the sample
    const char*   getName()     {return name.c_str();}
    
    /// data get functions
    const int     getChannels() {return channels    ;}
    const long    getFrames()   {return (endPoint - startPoint)*frames;}
    /// returns the buffer for the provided channel
    const float*  getAudio(int channel);
    const int     getStartPoint(){return startPoint*frames;}
    
    /// returns the waveform buffer, a mono-mixdown resampled to fit the window
    const float* getWaveform();
    
    /// velocity functions
    bool velocity( float vel ); // returns true if vel is in this samples range
    void velocityLow ( float low  );
    void velocityHigh( float high );
    
    /// playback controls
    bool dirty;           ///< Set to true when the cache is invalid
    
    float gain;           ///< Gain of this sample
    float pan;            ///< Panning of the sample (amplitude based)
    
    float pitch;          ///< Pitch of the sample (affects playback speed)
    float time;           ///< Time-stretch sample (does *not* affect playback pitch)
    
    float startPoint;     ///< Starting point of the sample playback 0->1
    float endPoint;       ///< Ending point of the sample 0->1
    
    float filterType;     ///< Filter high/low/band-pass setting
    float filterFrequency;///< Filter cutoff/center frequency
    float filterResonance;///< Filter resonance ( Q-factor )
    
    float velLow;           ///< Velocity low mark  0-1 range
    float velHigh;          ///< Velocity high mark 0-1 range
    
    float attack;           ///< ADSR Attack
    float decay;            ///< ADSR Decay
    float sustain;          ///< ADSR Sustain
    float release;          ///< ADSR Release
  
  private:
    Fabla2DSP* dsp;
    int sr;
    
    // convienience for setting defaults after constructor
    void init();
    void resample( int fromSr, std::vector<float>& inBuffer );
    
    std::string name;
    
    /// audio variables
    int channels;
    long frames;
    std::vector<float> audioMono;
    std::vector<float> audioStereoRight;
    
    /// a low-resolution re-sample of the audio data in this Sample
    void recacheWaveform();
    float waveformData[FABLA2_UI_WAVEFORM_PX];
};

};

#endif // OPENAV_FABLA2_SAMPLE_HXX
