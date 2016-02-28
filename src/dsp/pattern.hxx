/*
 * Author: Harry van Haaren 2016
 *         harryhaaren@gmail.com
 *         OpenAV Productions
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

#ifndef OPENAV_FABLA2_PATTERN_HXX
#define OPENAV_FABLA2_PATTERN_HXX

#include <vector>

struct Sequencer;

namespace Fabla2
{

class Fabla2DSP;

#define N_SEQS 16

/** Pattern
 * The pattern class allows creating and playback of sequences.
 */
class Pattern
{
public:
	Pattern(Fabla2DSP* dsp, int rate);
	~Pattern();

	void process(int nframes);

	// Used by static sequencer write callback to send events to F2
	void writeEvent(int frame, int note, int velo);

private:
	Fabla2DSP* dsp;
	int rate; /// samplerate - used for timing
	Sequencer* seqs[N_SEQS];

};

};

#endif // OPENAV_FABLA2_PATTERN_HXX
