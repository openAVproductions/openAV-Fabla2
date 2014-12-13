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
    Sample( Fabla2DSP* dsp, int rate, std::string name, std::string filePathToLoad );
    ~Sample();
    
    /// data get functions
    const int     getChannels() {return channels    ;}
    const long    getFrames()   {return frames      ;}
    const float*  getAudio()    {return &audio[0]   ;}
    const char*   getName()     {return name.c_str();}
    
    /// velocity functions
    bool velocity( int vel );
    void velocity( int low, int high );
  
  private:
    Fabla2DSP* dsp;
    int sr;
    
    std::string name;
    
    int channels;
    long frames;
    std::vector<float> audio;
    
    /// velocity range
    int velLow;
    int velHigh;
};

};

#endif // OPENAV_FABLA2_SAMPLE_HXX
