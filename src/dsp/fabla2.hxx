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

#ifndef OPENAV_FABLA2_HXX
#define OPENAV_FABLA2_HXX

#include "ports.hxx"
#include "../dsp.hxx"
#include "../shared.hxx"

#include "midi.hxx"
#include "yasper.hxx"

#include <map>
#include <vector>

// for accessing forge to write ports
class FablaLV2;

namespace Fabla2
{

class Pad;
class Voice;
class Sample;
class Library;

/** Fabla2DSP
 * This class contains the main DSP functionality of Fabla2. It handles incoming
 * audio and MIDI streams, controls voice-allocation, interacts with the host
 * for save() and worker-thread() implementations etc.
 * 
 */
class Fabla2DSP
{
  public:
    /// URIs pointer for understanding messages sent in from UI
    Fabla2DSP( int rate, URIs* uris );
    ~Fabla2DSP();
    
    // set by DSP
    FablaLV2* lv2;
    
    /// public read / write, plugin format wrapper writes audio port pointers
    /// while each voice can access incoming audio
    int sr;
    int nframes;
    
    /// control values
    float* controlPorts[PORT_COUNT];
    
    /// main process callback
    void process( int nframes );
    
    /// plugin format wrapper calls this for each MIDI event that arrives
    void midi( int frame, const uint8_t* );
    
    /// called with UI Atom data
    void uiMessage( int bank, int pad, int layer, int URI, float value );
    
    /// called when a sample is removed or added to a Pad, and the UI needs the
    /// update the layer info
    void padRefreshLayers( int bank, int pad );
    
    /// lv2 convienience function to write a samples state to the UI
    void writeSampleState( int b, int p, int l, Pad* pad, Sample* );
    void tx_waveform( int bank, int pad, int layer, const float* data );
    
    Library* getLibrary(){return library.GetRawPointer();}
    
  private:
    URIs* uris;
    
    /// voices store all the voices available for use
    std::vector< yasper::ptr<Voice> > voices;
    
    /// Library stores all data
    yasper::ptr<Library> library;
    
    /// map from MIDI number to pad instance
    std::map< int, yasper::ptr<Pad> > midiToPad;
    
    /// record buffer: when a record operation begins, it uses this buffer
    void startRecordToPad(int bank, int pad);
    void stopRecordToPad();
    
    bool recordEnable;
    int  recordBank;
    int  recordPad;
    long recordIndex;
    std::vector<float> recordBuffer;
    
    
};

}; // Fabla2

#endif // OPENAV_FABLA2_HXX
