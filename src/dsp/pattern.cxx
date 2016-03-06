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


#include "pattern.hxx"
#include "sequencer.h"
#include "fabla2.hxx"

#include <stdio.h>
#include <stdint.h>

namespace Fabla2 {

static void pattern_seq_cb(int frame, int note, int velo, void* ud)
{
	((Pattern*)ud)->writeEvent(frame, note, velo);
}

void Pattern::writeEvent(int frame, int note, int velo)
{
	uint8_t msg[3];
	msg[0] = 0x90;
	msg[1] = note;
	msg[2] = velo;
	dsp->midi( frame, msg, 0 );
}

Pattern::Pattern( Fabla2DSP* d, int r) :
	dsp(d),
	rate(r)
{
	for(int i = 0; i < N_SEQS; i++) {
		Sequencer* s  = sequencer_new(rate);
		sequencer_set_callback(s, pattern_seq_cb, this);
		sequencer_set_note(s, 36 + i);
		sequencer_set_num_steps(s, 32);
		sequencer_set_length(s, r * 4 ); // 4 seconds = 32 steps
		seqs[i] = s;
	}
}

Pattern::~Pattern()
{
	for(int i = 0; i < N_SEQS; i++) {
		sequencer_free( seqs[i] );
	}
}

void Pattern::rewind_to_start()
{
	for(int i = 0; i < N_SEQS; i++)
		sequencer_reset_playhead(seqs[i]);
}

void Pattern::setBPM(int bpm)
{
	for(int i = 0; i < N_SEQS; i++)
		sequencer_set_length(seqs[i], rate/(bpm/2/60.f));
}

void Pattern::process(int nf)
{
	for(int i = 0; i < N_SEQS; i++) {
		sequencer_process(seqs[i], nf);
	}
}

};
