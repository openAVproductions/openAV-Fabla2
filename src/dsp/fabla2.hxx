/*
 * Author: Harry van Haaren 2014 - 2016
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

#include "pad.hxx"
#include "midi.hxx"
#include "dsp_dbmeter.hxx"

#include <map>
#include <vector>

#include <ctlra/ctlra.h>
#include "zix/thread.h"
#include "zix/ring.h"

#include <caira.h>

// for accessing forge to write ports
class FablaLV2;

namespace Fabla2
{

typedef struct {
	int note;
	int velocity;
} MidiNote ;

class Voice;
class Sample;
class Library;

typedef void (*f2_msg_func)(void *self, void *func_data);

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

	/// turns off all voices, silencing output
	void panic();

	/// audition a sample, free the one being returned.
	Fabla2::Sample* auditionPlay( Fabla2::Sample* sample );
	void auditionPlay( int bank, int pad, int layer );
	void auditionStop();

	/// control values
	float* controlPorts[PORT_COUNT];

	/// main process callback
	void process( int nframes );

	/// plugin format wrapper calls this for each MIDI event that arrives
	/// if fromUI == 1, do not send updates back to UI
	void midi( int frame, const uint8_t*, bool fromUI = 0 );

	/// called with UI Atom data
	void uiMessage( int bank, int pad, int layer, int URI, float value );

	// Push loaded preset / state to UI
	void refreshUI();

	/// called with AuxBus messages
	void auxBus( int bus, float value );

	/// called when a sample is removed or added to a Pad, and the UI needs the
	/// update the layer info
	void padRefreshLayers( int bank, int pad );

	/// step sequencer control functions
	void stepSeq(int bank, int pad, int step, int value);

	/// play a sample
	int playPadOnNextVoice(int bank, int pad, float vel, int event_time);

	/// lv2 convienience function to write a samples state to the UI
	void writePadsState( int b, int p, Pad* pad );
	void writeSampleState( int b, int p, int l, Pad* pad, Sample* );
	void tx_waveform( int bank, int pad, int layer, const float* data );

	Library* getLibrary() {return library;}

	float auxBusVol[4];

	void writeMidiNote(int b1, int note, int velo);

	std::vector<MidiNote>* getMidiNotes();

	/// record buffer: when a record operation begins, it uses this buffer
	void startRecordToPad(int bank, int pad);
	void stopRecordToPad();

	void ctlra_func();
	volatile uint32_t ctlra_thread_running;
	volatile uint32_t ctlra_thread_quit_now;
	volatile uint32_t ctlra_thread_quit_done;

	/* ctlra static function converters */
	void event_func(struct ctlra_dev_t* dev, uint32_t num_events,
			struct ctlra_event_t** events);
	void feedback_func(struct ctlra_dev_t *dev);
	int32_t screen_redraw_func(struct ctlra_dev_t *dev,
				   uint32_t screen_idx,
				   uint8_t *pixel_data,
				   uint32_t bytes,
				   struct ctlra_screen_zone_t *redraw_zone);

private:
	URIs* uris;

	/// when true, AuxBus audio ports can be used
	bool useAuxbus;

	/// when true, the internal patterns will be played back
	bool patternPlay;
	int  patternBank;
	int  patternChoice;

	/// when true, the UI should be updated of pad stats / refreshed
	bool refresh_UI;
	int uiDbUpdateCounter; 

	/// used to audition samples, and deal with layer-playing from UI
	Voice* auditionVoice;
	Pad* auditionPad;

	DBMeter dbMeter;

	/// Vector to store outgoing MIDI messages
	std::vector<MidiNote> outMidiNotes;

	/// voices store all the voices available for use
	std::vector<Voice*> voices;

	/// Library stores all data
	Library* library;

	/// map from MIDI number to pad instance
	std::map<int, Pad*> midiToPad;

	bool recordEnable;
	int  recordBank;
	int  recordPad;
	long recordIndex;
	std::vector<float> recordBuffer;

	// Ctlra related stuffA
	struct ctlra_t *ctlra;
	ZixThread ctlra_thread;
	/* pass msg through ring, pass data to match in _data */
	ZixRing *ctlra_to_f2_ring;
	ZixRing *ctlra_to_f2_data_ring;

	ZixRing *f2_to_ctlra_ring;
	ZixRing *f2_to_ctlra_data_ring;

	int ctlra_ring_write(f2_msg_func func, void *data, uint32_t size);

	/* caira for screen drawing */
	caira_surface_t *img;
	caira_t *cr;
	int last_pressed_pad;
	int record_pressed;
};

}; // Fabla2

#endif // OPENAV_FABLA2_HXX
