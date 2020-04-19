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

#include "fabla2.hxx"

#include "../dsp.hxx"

#ifdef OPENAV_PROFILE
#include "../profiny.hxx"
#endif

#include <sstream>

#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "pad.hxx"
#include "bank.hxx"
#include "voice.hxx"
#include "sample.hxx"
#include "sampler.hxx"
#include "pattern.hxx"
#include "library.hxx"
#include "sequencer.h"
#include "midi_helper.hxx"

#include "plotter.hxx"

namespace Fabla2
{

/* 64 byte message structure to pass through ring */
struct f2_msg {
	/* function to call */
	f2_msg_func func;
	/* size of data to consume */
	uint32_t data_size;
	/* passing to 16B */
	uint32_t padding;
};

void f2_print_hello(void *self, void *func_data)
{
	char *word = (char *)func_data;
	printf("f2 print hello: %p, %s\n", self, word);
}

void f2_print_uint64(void *self, void *func_data)
{
	uint64_t *d = (uint64_t *)func_data;
	for(int i = 0; i < 8; i++) {
		printf("\t%ld\n", d[i]);
	}
	printf("\n");
}

struct f2_play_pad_t {
	int bank;
	int pad;
	float velocity;
};
void f2_play_pad(void *self, void *func_data)
{
	struct f2_play_pad_t *d = (struct f2_play_pad_t *)func_data;
	Fabla2DSP *f2 = (Fabla2DSP *)self;
	f2->playPadOnNextVoice(d->bank, d->pad, d->velocity, 0);
}

struct f2_pad_duplicate_t {
	int from_bank;
	int from_pad;
	int to_bank;
	int to_pad;
};
void f2_pad_duplicate(void *self, void *data)
{
	struct f2_pad_duplicate_t *d = (struct f2_pad_duplicate_t *)data;
	Fabla2DSP *f2 = (Fabla2DSP *)self;
	f2->padDuplicateTo(d->from_bank, d->from_pad, d->to_bank, d->to_pad);
}

int
Fabla2DSP::padDuplicateTo(int from_bank, int from_pad, int to_bank, int to_pad)
{
	printf("pad dup to %d, %d, %d, %d\n", from_bank, from_pad, to_bank,
	       to_pad);
	Pad *fp = library->bank(from_bank)->pad(from_pad);
	if(!fp) { printf("!fp\n"); return -1; }

	Sample *s = fp->layer(fp->lastPlayedLayer());
	if(!s) { printf("!sample\n"); return -1; }

	Pad *tp = library->bank(to_bank)->pad(to_pad);
	if(!tp) { printf("!tp\n"); return -1; }

	tp->add(s);

	return 0;
}



struct f2_pad_record_t {
	int bank;
	int pad;
	int enable;
};
void f2_pad_record(void *self, void *func_data)
{
	struct f2_pad_record_t *d = (struct f2_pad_record_t *)func_data;
	Fabla2DSP *f2 = (Fabla2DSP *)self;
	if(d->enable) {
		f2->startRecordToPad(d->bank, d->pad);
	} else {
		f2->stopRecordToPad();
	}
}


int
Fabla2DSP::ctlra_ring_write(f2_msg_func func, void *data, uint32_t size)
{
	/* message to enqueue into ring */
	struct f2_msg m = {
		.func = func,
		.data_size = size,
	};

	/* TODO: check write space available */

	uint32_t data_w = zix_ring_write(ctlra_to_f2_data_ring, data, size);
	if(data_w != size) {
		printf("error didn't write data to ring: %d\n", data_w);
	}

	uint32_t w = zix_ring_write(ctlra_to_f2_ring, &m, sizeof(m));
	if(w != sizeof(m)) {
		printf("error didn't write full msg to ring: %d\n", w);
	}

	return 0;
}

void Fabla2DSP::ctlra_func()
{
	usleep(500 * 1000);

	while(1) {
		if(ctlra_thread_running) {
			ctlra_idle_iter(ctlra);
			usleep(1 * 1000);
		} else {
			sleep(1);
		}

		if(ctlra_thread_quit_now) {
			break;
		}
	}
}

void *ctlra_thread_func(void *ud)
{
	Fabla2DSP *self = (Fabla2DSP *)ud;
	self->ctlra_func();
	self->ctlra_thread_quit_done = 1;
	return 0;
}

void
Fabla2DSP::feedback_func(struct ctlra_dev_t *dev)
{
	/*
	int led = (3-(e->grid.pos / 4)) * 4 + (e->grid.pos % 4);
	ctlra_dev_light_set(dev, 63 + 24 + led,
			    0x020000ff * e->grid.pressed);
	*/
	int bank = 0;
	Bank *b = library->bank(bank);

	ctlra_dev_light_set(dev, 29 + bank, 0x030000ff);

	const int offset = 63 + 24;
	for(int i = 0; i < 16; i++) {
		Pad *p = b->pad(i);
		int loaded = p->loaded() ? 1 : 0;
		int led = (3-(i / 4)) * 4 + (i % 4);
		ctlra_dev_light_set(dev, offset + led, 0x020000ff * loaded);

	}
	
	/* showing sample */ {
		Pad *p = b->pad(last_pressed_pad);
		if(p) {
			int led = (3-(last_pressed_pad / 4)) * 4 + (last_pressed_pad % 4);
			int loaded = p->loaded() ? 1 : 0;
			uint32_t col = duplicate_from_pad ?  0xff0000ff : 0xff00ff00;
			ctlra_dev_light_set(dev, offset + led, col * loaded);
		}
	}

	ctlra_dev_light_set(dev,  5, 0x0400003f);
	ctlra_dev_light_set(dev, 42, record_pressed ? -1 : 0);
	ctlra_dev_light_set(dev, 54, duplicate_pressed ? -1 : 0x11111111);

	/* for each pad in the grid, light up loaded state */
	ctlra_dev_light_flush(dev, 1);
}

static void
static_feedback_func(struct ctlra_dev_t *dev, void *userdata)
{
	Fabla2DSP *self = (Fabla2DSP *)userdata;
	self->feedback_func(dev);
}

/* TODO: remove dependency on this */
#define SR_CHANNELS r, g, b, a
typedef union {
  unsigned int word;
  struct { unsigned char SR_CHANNELS; } rgba;
} sr_Pixel;

#if 0
static inline void draw_slider(caira_t *cr, int x, int y, int w, int h, float v)
{
	caira_set_source_rgb(cr, 0.2, 0.2, 0.2);
	caira_rectangle(cr, x, y, w, h);
	caira_fill(cr);

	caira_set_source_rgb(cr, 0.0, 0x51 / 255., 1.);
	caira_rectangle(cr, x, y + h - 20 - (v * (h - 20)), w, 20);
	caira_fill(cr);
}
#endif

int32_t
Fabla2DSP::screen_redraw_func(struct ctlra_dev_t *dev,
				  uint32_t screen_idx,
				  uint8_t *pixel_data,
				  uint32_t bytes,
				  struct ctlra_screen_zone_t *redraw_zone)
{
#warning TODO, implement redraw func here
	return 0;
#if 0
	caira_set_source_rgb(cr, 0, 0, 0);
	caira_rectangle(cr, 0, 0, 480, 272);
	caira_fill(cr);

	Pad *p = library->bank(0)->pad(last_pressed_pad);
	if(!p)
		return 0;

	Sample *s = p->layer(p->lastPlayedLayer());
	if(!s)
		return 0;

	if(screen_idx == 0) {
		/* selectors */ {
		const uint32_t x = 16;
		const uint32_t y =  2;
		const uint32_t w =  90;
		const uint32_t h =  23;
		const uint32_t xoff = w + ((480 - 4 * 90)/4);
		caira_set_source_rgb(cr, 0.1, 0.1, 0.1);
		for(int i = 0; i < 4; i++) {
			caira_rectangle(cr, x + i * xoff , y, w, h);
			caira_fill(cr);
		}
		}

		/* waveform */ {
		const uint32_t x = 6;
		const uint32_t y = 30;
		const uint32_t high = 50;
		caira_set_source_rgb(cr, 0.1, 0.1, 0.1);
		caira_rectangle(cr, x, y, FABLA2_UI_WAVEFORM_PX, high * 2);
		caira_fill(cr);
		if(s) {
			caira_set_source_rgb(cr, 0.0, 0.51, 1.);
			const float *w = s->getWaveform();
			for(int i = 0; w && i < FABLA2_UI_WAVEFORM_PX; i++) {
				float py = (w[i] * high);
				caira_move_to(cr, x + i, y + high - py);
				caira_line_to(cr, x + i, y + high + py);
			}

			/* start point line */
			caira_set_source_rgb(cr, 1, 1, 1);
			caira_rectangle(cr, x + (FABLA2_UI_WAVEFORM_PX * s->getStartPoint()),
					y, 2, high * 2);
			caira_fill(cr);

			/* adsr */
			float a = s->attack;
			float d = s->decay;
			float su= s->sustain;
			float r = s->release;
			draw_slider(cr, 30, 140, 22, 100, a);
			draw_slider(cr, 56, 140, 22, 100, d);
			draw_slider(cr, 82, 140, 22, 100, su);
			draw_slider(cr,108, 140, 22, 100, r);
		}
		}
	}

	if(screen_idx == 1) {
		float v = p->volume;
		draw_slider(cr, 460, 70, 22, 190, v);

		if(s) {
			float p = s->pan;
			draw_slider(cr, 430, 70, 22, 190, p);
		}

	}

#if 0
	float dry_wet = p->volume;
	float time = 0.8;
	float py = 20 + 222 - (dry_wet * 242);
	float px = 20 + (time * 420);

	caira_set_source_rgb(cr, 1, 0, 0);
	caira_rectangle(cr, px, py, 20, 20);
	caira_fill(cr);

	caira_set_source_rgb(cr, 1, 1, 0);
	caira_rectangle(cr, 40, 40, 10, 10);
	caira_fill(cr);

	caira_set_source_rgb(cr, 1, 1, 1);
	/* low left, top, br */
	caira_move_to(cr, 10, 252);
	caira_line_to(cr, px, py);
	caira_line_to(cr, 480, 272);
#endif

	int stride = caira_image_surface_get_stride(img);
	unsigned char * data = caira_image_surface_get_data(img);

	/*
	for(int i = 0; i < bytes; i++)
		pixel_data[i] = data[i];
	*/

	/* TODO: convert ARGB to RGB565 byte-swapped */
#if 1
	sr_Pixel *pixels = (sr_Pixel *)data;
	uint16_t *scn = (uint16_t *)pixel_data;
	for(int j = 0; j < 272; j++) {
		for(int i = 0; i < 480; i++) {
			sr_Pixel px = *pixels++;
			/* convert to BGR565 in byte-swapped LE */
			/* blue 5, green LSB 3 bits */
			uint16_t red = px.rgba.r;
			uint16_t green = px.rgba.g;
			uint16_t blue = px.rgba.b;

			/* mask and shift */
			uint16_t b = ((blue  >> 3) & 0x1f);
			uint16_t g = ((green >> 2) & 0x3f) << 5;
			uint16_t r = ((red   >> 3) & 0x1f) << 11;

			/* byte-swap and store */
			uint16_t tmp = (b | g | r);
			*scn = ((tmp & 0x00FF) << 8) | ((tmp & 0xFF00) >> 8);
			scn++;
		}
	}
#endif
	return 1;
#endif
}

static int32_t
static_screen_redraw_func(struct ctlra_dev_t *dev, uint32_t screen_idx,
			  uint8_t *pixel_data, uint32_t bytes,
			  struct ctlra_screen_zone_t *redraw_zone,
			  void *userdata)
{
	Fabla2DSP *self = (Fabla2DSP *)userdata;
	return self->screen_redraw_func(dev, screen_idx, pixel_data, bytes,
				 redraw_zone);
}

void
Fabla2DSP::event_func(struct ctlra_dev_t* dev, uint32_t num_events,
		  struct ctlra_event_t** events)
{


	for(uint32_t i = 0; i < num_events; i++) {
		struct ctlra_event_t *e = events[i];
		printf("event %d\n", e->type);
		Pad *p = library->bank(0)->pad(last_pressed_pad);
		Sample *s = p->layer(p->lastPlayedLayer());
		switch(e->type) {

		case CTLRA_EVENT_BUTTON: {
			switch(e->button.id) {
			case 30: /* duplicate */
				duplicate_pressed = e->button.pressed;
				if(duplicate_pressed == 0) {
					duplicate_from_pad = -1;
				}
				break;
			case 42: /* record */
				record_pressed = e->button.pressed;
				printf("button %d, %d\n", e->button.id,
				       record_pressed);
				break;
			default:
				printf("button %d\n", e->button.id);
				break;
			}
			}
			break;

		case CTLRA_EVENT_SLIDER: {
			if(s) {
				s->pitch = e->slider.value;
			}
			//printf("slider %d %f\n", e->slider.id, e->slider.value);
			}
			break;

		case CTLRA_EVENT_ENCODER: {
			if(s) {
			switch(e->encoder.id) {
			case 1: s->attack  += e->encoder.delta_float * 1.25; break;
			case 2: s->decay   += e->encoder.delta_float * 1.25; break;
			case 3: s->sustain += e->encoder.delta_float * 1.25; break;
			case 4: s->release += e->encoder.delta_float * 1.25; break;
			case 5: s->startPoint += e->encoder.delta_float * 0.85; break;
			}

			if(s->attack < 0.f) s->attack = 0.f;
			if(s->decay < 0.f) s->decay = 0.f;
			if(s->sustain < 0.f) s->sustain = 0.f;
			if(s->release < 0.f) s->release = 0.f;
			if(s->startPoint < 0.f) s->startPoint = 0.f;

			}

			if(e->encoder.id == 8)
			case 8: p->volume += e->encoder.delta_float * 1.25; break;
			printf("encoder %d %f\n", e->encoder.id, e->encoder.delta_float);
			}
			break;

		case CTLRA_EVENT_GRID: {
			if(record_pressed) {
				struct f2_pad_record_t d = {
					.bank = 0,
					.pad = e->grid.pos,
					.enable = (int)e->grid.pressed,
				};
				ctlra_ring_write(f2_pad_record, &d, sizeof(d));
			} else {
				if(e->grid.pressed) {
					if(duplicate_from_pad > 0) {
						/* duplicate from already set,
						 * so do the dup action */
						struct f2_pad_duplicate_t d = {
							.from_bank = 0,
							.from_pad = duplicate_from_pad, 
							.to_bank = 0,
							.to_pad = e->grid.pos,
						};
						ctlra_ring_write(f2_pad_duplicate, &d, sizeof(d));
						duplicate_from_pad = -1;
						duplicate_pressed = 0;
					}
					if(duplicate_pressed) {
						duplicate_from_pad = e->grid.pos;
					}
					struct f2_play_pad_t d = {
						.bank = 0,
						.pad = e->grid.pos,
						.velocity = 1.0f,
					};
					ctlra_ring_write(f2_play_pad, &d, sizeof(d));
				}
			}
			last_pressed_pad = e->grid.pos;
			}
		}
	}
}

static void
static_event_func(struct ctlra_dev_t* dev, uint32_t num_events,
		  struct ctlra_event_t** events, void *userdata)
{
	Fabla2DSP *self = (Fabla2DSP *)userdata;
	if (self->ctlra_is_quitting)
		return;

	self->event_func(dev, num_events, events);
}


static int
accept_dev_func(struct ctlra_t *ctlra,
		const struct ctlra_dev_info_t *info,
		struct ctlra_dev_t *dev,
		void *userdata)
{
	Fabla2DSP *self = (Fabla2DSP *)userdata;

	printf("Fabla2: accept dev %s %s\n", info->vendor, info->device);

	ctlra_dev_set_event_func(dev, static_event_func);
	ctlra_dev_set_feedback_func(dev, static_feedback_func);
	ctlra_dev_set_screen_feedback_func(dev, static_screen_redraw_func);

	ctlra_dev_set_callback_userdata(dev, userdata);
	self->ctlra_is_quitting = 0;

	return 1;
}

Fabla2DSP::Fabla2DSP( int rate, URIs* u ) :
	sr( rate ),
	uris( u ),
	useAuxbus( false ),
	patternPlay( false ),
	patternBank( 0 ),
	patternChoice( 0 ),
	dbMeter( rate ),
	recordEnable( false ),
	recordBank( 0 ),
	recordPad( 0 ),
	uiDbUpdateCounter(rate/30),
	duplicate_pressed(0),
	duplicate_from_pad(-1)
{
	library = new Library( this, rate );

	auditionVoice = new Voice( this, rate );
	auditionPad = new Pad( this, rate, -1 );

	for( int i = 0; i < 16; i++ )
		voices.push_back( new Voice( this, rate ) );

	recordBuffer.resize( rate * 10 );

	memset( controlPorts, 0, sizeof(float*) * PORT_COUNT );

	int bankID = 0;
	for(int i = 0; i < 16 * 4; i++) {
		if( i > 0 && i % 16 == 0 ) {
			bankID++;
		}

		Pad* tmpPad = new Pad( this, rate, i % 16 );
		tmpPad->bank( i / 16 );

		library->bank( bankID )->pad( tmpPad );
	}

	/* initialize Ctlra library / thread */
	ctlra = ctlra_create(NULL);
	int num_devs = ctlra_probe(ctlra, accept_dev_func, this);
	printf("connected devices %d\n", num_devs);

	ZixStatus status = zix_thread_create(&ctlra_thread,
					     80000,
					     ctlra_thread_func,
					     this);
	if(status != ZIX_STATUS_SUCCESS) {
		printf("ERROR launching zix thread!!\n");
	}

	ctlra_to_f2_ring = zix_ring_new(4096);
	if(!ctlra_to_f2_ring) {
		printf("ERROR creating zix ctlra->f2 ring\n");
	}
	ctlra_to_f2_data_ring = zix_ring_new(4 * 4096);
	if(!ctlra_to_f2_data_ring) {
		printf("ERROR creating zix ctlra->f2 DATA ring\n");
	}


	f2_to_ctlra_ring = zix_ring_new(4096);
	if(!f2_to_ctlra_ring) {
		printf("ERROR creating zix f2->ctlra ring\n");
	}
	f2_to_ctlra_data_ring = zix_ring_new(4 * 4096);
	if(!f2_to_ctlra_data_ring) {
		printf("ERROR creating zix f2->ctlra data ring\n");
	}

	/* initialize caira for screen drawing */
#warning TODO: initialize screens here
#if 0
	img = caira_image_surface_create(CAIRA_FORMAT_ARGB32, 480, 272);
	cr = caira_create(img);
#endif


	ctlra_thread_running = 1;

	// for debugging null pointers etc
	//library->checkAll();
}

void Fabla2DSP::writeMidiNote(int b1, int note, int velo)
{
	uint8_t msg[3];
	msg[0] = b1;
	msg[1] = note;
	msg[2] = velo;
	lv2->writeMIDI(0, msg);
}


void Fabla2DSP::stepSeq(int bank, int pad, int step, int value)
{
	Sequencer* s = library->bank(bank)->getPattern()->getSequencer(pad);
	sequencer_set_step(s, step, value);
}

void Fabla2DSP::process( int nf )
{
#ifdef OPENAV_PROFILE
	PROFINY_SCOPE
#endif
	nframes = nf;

	/* pull ctlra input */
	struct f2_msg m = {0};
	uint32_t r = zix_ring_read(ctlra_to_f2_ring, &m, sizeof(m));
	if(r == 0) {
		/* empty ring == no events available */
	} else if(r != sizeof(m)) {
		/* TODO: error handle here? Programming issue detected */
	} else {
		/* we have a f2_msg now, with a function pointer and a
		 * size of data to consume. Call the function with the
		 * read head of the ringbuffer, allowing function to use
		 * data from the ring, then consume data_size from the data
		 * ring so the next message pulls the next block of info.
		 */
		/* TODO: suboptimal usage here, because we copy the data
		 * out and into a linear array - but it probably already is
		 * Use a bip-buffer mechanism to avoid the copy here */
		const uint32_t ds = 256;
		char buf[ds];
		uint32_t read_size = m.data_size > ds ? ds : m.data_size;
		uint32_t data_r = zix_ring_read(ctlra_to_f2_data_ring,
						buf, read_size);
		if(m.func)
			m.func(this, buf);
	}

	/*
	float recordOverLast = *controlPorts[RECORD_OVER_LAST_PLAYED_PAD];
	if( recordEnable != (int)recordOverLast ) {
		// update based on control port value
		recordEnable = (int)recordOverLast;

		if( recordEnable ) {
			printf("recording switch! %i\n", recordEnable );
			startRecordToPad( recordBank, recordPad );
		} else {
			stopRecordToPad();
		}
	}
	*/

	// clear the audio buffers
	memset( controlPorts[OUTPUT_L],  0, sizeof(float) * nframes );
	memset( controlPorts[OUTPUT_R],  0, sizeof(float) * nframes );

	// check if the host connected the AuxBus buffers - if so, set
	// useAuxbus to true
	if( !useAuxbus && controlPorts[AUXBUS1_L] && controlPorts[AUXBUS4_R] ) {
		useAuxbus = true;
	}

	// aux buffers if set
	if( useAuxbus ) {
		memset( controlPorts[AUXBUS1_L], 0, sizeof(float) * nframes );
		memset( controlPorts[AUXBUS1_R], 0, sizeof(float) * nframes );
		memset( controlPorts[AUXBUS2_L], 0, sizeof(float) * nframes );
		memset( controlPorts[AUXBUS2_R], 0, sizeof(float) * nframes );
		memset( controlPorts[AUXBUS3_L], 0, sizeof(float) * nframes );
		memset( controlPorts[AUXBUS3_R], 0, sizeof(float) * nframes );
		memset( controlPorts[AUXBUS4_L], 0, sizeof(float) * nframes );
		memset( controlPorts[AUXBUS4_R], 0, sizeof(float) * nframes );
	}

	if( refresh_UI ) {
		int bank = 0;
		for(int i = 0; i < 16; i++) {
			// TODO FIXME: Bank here needs to be the loaded one
			Pad* p = library->bank( bank )->pad( i );
			if(p) {
				LV2_Atom_Forge_Frame frame;
				lv2_atom_forge_frame_time( &lv2->forge, 0 );
				lv2_atom_forge_object( &lv2->forge, &frame, 0, uris->fabla2_PadHasSample );
				lv2_atom_forge_key(&lv2->forge, uris->fabla2_bank);
				lv2_atom_forge_int(&lv2->forge, bank );
				lv2_atom_forge_key(&lv2->forge, uris->fabla2_pad);
				lv2_atom_forge_int(&lv2->forge, i );
				lv2_atom_forge_key(&lv2->forge, uris->fabla2_PadHasSample);
				lv2_atom_forge_int(&lv2->forge, p->loaded() );
				lv2_atom_forge_pop(&lv2->forge, &frame);
			} else {
				//printf("%s - pad not valid!\n", __PRETTY_FUNCTION__);
			}
		}
		// FIXME Pad = 0
		Pad* p = library->bank( bank )->pad( 0 );
		writePadsState(bank, 0, p );
		writeSampleState(bank, 0, 0, p, p->layer(0) );

		for(int i = 0; i < 4; i++) {
			LV2_Atom_Forge_Frame frame;
			lv2_atom_forge_frame_time( &lv2->forge, 0 );
			lv2_atom_forge_object( &lv2->forge, &frame, 0, uris->fabla2_AuxBus);
			lv2_atom_forge_key(&lv2->forge, uris->fabla2_auxBusNumber);
			lv2_atom_forge_int(&lv2->forge, i );
			lv2_atom_forge_key(&lv2->forge, uris->fabla2_value);
			lv2_atom_forge_float(&lv2->forge, auxBusVol[i]);
			lv2_atom_forge_pop(&lv2->forge, &frame);
		}

		refresh_UI = 0;
	}

	if( recordEnable && recordPad != -1 && recordIndex + nframes < sr * 4 ) {
		//printf("recording...\n" );
		for(int i = 0; i < nframes; i++) {
			recordBuffer[recordIndex++] = controlPorts[INPUT_L][i];
			recordBuffer[recordIndex++] = controlPorts[INPUT_R][i];
		}
	} else if( recordEnable && recordPad != -1 ) {
		recordEnable = false;
		printf("record stopped: out of space! %li\n", recordIndex );
	}


	float transport_play = *controlPorts[TRANSPORT_PLAY];
	if( transport_play ) {
		// TODO: Refactor to use patternChoice
		Pattern* p = library->bank(patternBank)->getPattern();
		if(patternPlay == false) {
			// start playing, so reset to start of loop/pattern
			p->rewind_to_start();
		}
		p->setBPM( *controlPorts[TRANSPORT_BPM] );
		p->process( nf );
		// keep state of playback
		patternPlay = true;
	} else {
		patternPlay = false;
	}

	// Process voices
	for( int i = 0; i < voices.size(); i++ ) {
		Voice* v = voices.at(i);
		if( v->active() ) {
			//printf("voice %i playing\n", i);
			v->process();
		}
	}

	// Process audition voice
	auditionVoice->process();


	uiDbUpdateCounter -= nframes;
	if( uiDbUpdateCounter < 0 ) {
		float* buf[2];
		buf[0] = controlPorts[OUTPUT_L];
		buf[1] = controlPorts[OUTPUT_R];
		dbMeter.process(nf, &buf[0], &buf[0] );
		// send UI message
		LV2_Atom_Forge_Frame frame;
		lv2_atom_forge_frame_time( &lv2->forge, 0 );
		lv2_atom_forge_object( &lv2->forge, &frame, 0, uris->fabla2_dbMeter );
		lv2_atom_forge_key(&lv2->forge, uris->fabla2_dbMeter);
		lv2_atom_forge_int(&lv2->forge, 0 );

		lv2_atom_forge_key(&lv2->forge, uris->fabla2_value);
		lv2_atom_forge_float(&lv2->forge, dbMeter.getLeftDB() );

		lv2_atom_forge_pop(&lv2->forge, &frame);
		uiDbUpdateCounter = sr / 30;
	}
}

void Fabla2DSP::auditionStop()
{
	auditionVoice->stop();
}

Fabla2::Sample* Fabla2DSP::auditionPlay( Fabla2::Sample* sample )
{
	auditionVoice->stop();
	auditionPad->clearAllSamples();
	auditionPad->add(sample);
	auditionVoice->playLayer( auditionPad, 0 );
	return 0;
}

void Fabla2DSP::auditionPlay( int bank, int pad, int layer )
{
	if ( bank < 0 || bank >=  4 ) return;
	if ( pad  < 0 || pad  >= 16 ) return;

	Pad* p = library->bank( bank )->pad( pad );
	Sample* s = p->layer( layer );

	if( !s )
		return;

	writeSampleState( bank, pad, layer, p, s );

	auditionVoice->stop();

	auditionVoice->playLayer( p, layer );
}

static void fabla2_dsp_getDetailsFromNote( const uint8_t* msg, int& bank, int& pad )
{
	// check MIDI note is valid in downwards direction
	if( msg[1] < 36 )
		return;

	// get midi bank (from message, or from MIDI note num?)
	bank = 0; //(msg[0] & 0x0F);

	// select pad number from midi note (0-15)
	pad = (msg[1] - 36) % 16;

	// convert higher midi notes ( > 36 + 16, aka first bank) to next bank
	if( msg[1] >= 36 + 16 ) {
		int nMoreBanks = (msg[1] - 36) / 16;
		//printf( "more banks %i\n", nMoreBanks );
		bank += nMoreBanks;
	}
}

void Fabla2DSP::refreshUI()
{
	refresh_UI = true;
}

int Fabla2DSP::playPadOnNextVoice(int bank, int pad, float vel, int eventTime)
{
	// the pad that's going to be allocated to play
	Pad* p = library->bank( bank )->pad( pad );

	bool allocd = false;
	for(int i = 0; i < voices.size(); i++) {
		// current voice pointer
		Voice* v = voices.at(i);

		// check mute-group to stop the voice first
		if( v->active() ) {
			int mg = p->muteGroup();
			// note-on mute-group is valid && == to current voice off-group
			if( mg != 0 &&
			    mg == v->getPad()->offGroup() ) {
				// note that this triggers ADSR off, so we can *NOT* re-purpose
				// the voice right away to play the new note.
				//printf("note-on muteGroup %i : turning off %i\n", mg, v->getPad()->offGroup() );
				v->kill();
			}
		} else {
			// only allocate voice if we haven't already done so
			if( !allocd ) {
				// play pad
				voices.at(i)->play( eventTime, bank, pad, p, vel);

				// write note on MIDI events to UI
				/*
				LV2_Atom_Forge_Frame frame;
				lv2_atom_forge_frame_time( &lv2->forge, eventTime );
				lv2_atom_forge_object( &lv2->forge, &frame, 0, uris->fabla2_PadPlay );

				lv2_atom_forge_key(&lv2->forge, uris->fabla2_bank);
				lv2_atom_forge_int(&lv2->forge, bank );
				lv2_atom_forge_key(&lv2->forge, uris->fabla2_pad);
				lv2_atom_forge_int(&lv2->forge, pad );
				lv2_atom_forge_key(&lv2->forge, uris->fabla2_layer);
				lv2_atom_forge_int(&lv2->forge, p->lastPlayedLayer() );
				lv2_atom_forge_key(&lv2->forge, uris->fabla2_velocity);
				lv2_atom_forge_int(&lv2->forge, velocity * 127.f );

				lv2_atom_forge_pop(&lv2->forge, &frame);
				*/

				padRefreshLayers(bank, pad);
				int l = p->lastPlayedLayer();
				Sample* s = p->layer( l );
				if(s) {
					writeSampleState(bank, pad, l, p, s);
					tx_waveform(bank, pad, l, s->getWaveform());
				}
				allocd = true;
			}
		}
	}

	/// if all voices are full, we steal the first one
	if( allocd == false ) {
		voices.at( 0 )->play( eventTime, bank, pad, p, vel );
	}

	return 0;
}

void Fabla2DSP::midi( int eventTime, const uint8_t* msg, bool fromUI )
{
	//printf("MIDI: %i, %i, %i\n", (int)msg[0], (int)msg[1], (int)msg[2] );


	switch( lv2_midi_message_type( msg ) ) {
	case LV2_MIDI_MSG_NOTE_ON: {
		// check MIDI note is valid in downwards direction
		if( msg[1] < 36 )
			return;

		int bank = 0;
		int pad  = 0;
		fabla2_dsp_getDetailsFromNote( msg, bank, pad );
		if( bank < 0 || bank >=  4 ) {
			return;
		}
		if( pad  < 0 || pad  >= 16 ) {
			return;
		}

		if( recordEnable ) {
			printf("recording note %d %d %d to pad %d\n", msg[0],
			       msg[1], msg[2], recordPad);
			Pad* p = library->bank(recordBank)->pad(recordPad);
			p->midiNoteAdd(msg[1], msg[2]);
			return;
		} else {
			// update the recording pad
			recordBank = bank;
			recordPad  = pad;
		}

		playPadOnNextVoice(bank, pad, msg[2] / 127.f, eventTime);

		}
		break;

	case LV2_MIDI_MSG_NOTE_OFF: {
		int bank = 0;
		int pad  = 0;
		fabla2_dsp_getDetailsFromNote( msg, bank, pad );
		if( bank < 0 || bank >=  4 ) {
			return;
		}
		if( pad  < 0 || pad  >= 16 ) {
			return;
		}

		// write note on MIDI events to UI
		LV2_Atom_Forge_Frame frame;
		lv2_atom_forge_frame_time( &lv2->forge, eventTime );
		lv2_atom_forge_object( &lv2->forge, &frame, 0, uris->fabla2_PadStop );

		lv2_atom_forge_key(&lv2->forge, uris->fabla2_bank);
		lv2_atom_forge_int(&lv2->forge, bank );
		lv2_atom_forge_key(&lv2->forge, uris->fabla2_pad);
		lv2_atom_forge_int(&lv2->forge, pad );
		lv2_atom_forge_key(&lv2->forge, uris->fabla2_layer);
		lv2_atom_forge_int(&lv2->forge, -1 );
		lv2_atom_forge_key(&lv2->forge, uris->fabla2_velocity);
		lv2_atom_forge_int(&lv2->forge, msg[2] );

		lv2_atom_forge_pop(&lv2->forge, &frame);

		for(int i = 0; i < voices.size(); i++) {
			Voice* v = voices.at(i);

			if( v->active() ) {
				if( v->matches( bank, pad ) ) {
					// this voice was started by the Pad that now sent corresponding
					// note off event: so stop() the voice
					//printf("Voice.matces() NOTE_OFF -> Stop()\n" );
					v->stop();
					return;
				}
			}
		}
	}
	break;

	case LV2_MIDI_MSG_CONTROLLER:
		if( msg[1] == 120 ) // all sounds off
			panic();
		if( msg[1] == 123 ) // all notes off
			panic();
		break;

	case LV2_MIDI_MSG_PGM_CHANGE:
		//printf("MIDI : Program Change received\n");
		break;

	default:
		// ALL UN-USED TYPES OF MIDI MESSAGES
		break;
	}

}

void Fabla2DSP::writePadsState( int b, int p, Pad* pad )
{
	assert( pad );

	LV2_Atom_Forge_Frame frame;
	lv2_atom_forge_frame_time( &lv2->forge, 0 );

	lv2_atom_forge_object( &lv2->forge, &frame, 0, uris->fabla2_UiPadsState );

	lv2_atom_forge_key(&lv2->forge, uris->fabla2_bank);
	lv2_atom_forge_int(&lv2->forge, b );

	lv2_atom_forge_key(&lv2->forge, uris->fabla2_pad);
	lv2_atom_forge_int(&lv2->forge, p );

	lv2_atom_forge_key(&lv2->forge, uris->fabla2_name);
	const char* padName = pad->getName();
	if(!padName) {
		padName = "Pad-Name-Error";
	}
	lv2_atom_forge_string(&lv2->forge, padName, strlen(padName));

	lv2_atom_forge_key(&lv2->forge, uris->fabla2_PadVolume );
	lv2_atom_forge_float(&lv2->forge, pad->volume );

	lv2_atom_forge_key(&lv2->forge, uris->fabla2_value );
	lv2_atom_forge_float(&lv2->forge, pad->loaded() );

	lv2_atom_forge_key(&lv2->forge, uris->fabla2_PadAuxBus1 );
	lv2_atom_forge_float(&lv2->forge, pad->sends[0] );
	lv2_atom_forge_key(&lv2->forge, uris->fabla2_PadAuxBus2 );
	lv2_atom_forge_float(&lv2->forge, pad->sends[1] );
	lv2_atom_forge_key(&lv2->forge, uris->fabla2_PadAuxBus3 );
	lv2_atom_forge_float(&lv2->forge, pad->sends[2] );
	lv2_atom_forge_key(&lv2->forge, uris->fabla2_PadAuxBus4 );
	lv2_atom_forge_float(&lv2->forge, pad->sends[3] );

	lv2_atom_forge_pop(&lv2->forge, &frame);
}

void Fabla2DSP::writeSampleState( int b, int p, int l, Pad* pad, Sample* s )
{
	if( !pad || !s )
		return;

	assert( pad );
	assert( s );

	LV2_Atom_Forge_Frame frame;
	lv2_atom_forge_frame_time( &lv2->forge, 0 );

	lv2_atom_forge_object( &lv2->forge, &frame, 0, uris->fabla2_ReplyUiSampleState );

	lv2_atom_forge_key(&lv2->forge, uris->fabla2_bank);
	lv2_atom_forge_int(&lv2->forge, b );

	lv2_atom_forge_key(&lv2->forge, uris->fabla2_pad);
	lv2_atom_forge_int(&lv2->forge, p );

	lv2_atom_forge_key(&lv2->forge, uris->fabla2_layer);
	lv2_atom_forge_int(&lv2->forge, l );

	lv2_atom_forge_key(&lv2->forge, uris->fabla2_name);
	lv2_atom_forge_string(&lv2->forge, s->getName(), strlen( s->getName() ) );

	lv2_atom_forge_key(&lv2->forge, uris->fabla2_PadVolume );
	lv2_atom_forge_float(&lv2->forge, pad->volume );

	lv2_atom_forge_key(&lv2->forge, uris->fabla2_PadAuxBus1 );
	lv2_atom_forge_float(&lv2->forge, pad->sends[0] );
	lv2_atom_forge_key(&lv2->forge, uris->fabla2_PadAuxBus2 );
	lv2_atom_forge_float(&lv2->forge, pad->sends[1] );
	lv2_atom_forge_key(&lv2->forge, uris->fabla2_PadAuxBus3 );
	lv2_atom_forge_float(&lv2->forge, pad->sends[2] );
	lv2_atom_forge_key(&lv2->forge, uris->fabla2_PadAuxBus4 );
	lv2_atom_forge_float(&lv2->forge, pad->sends[3] );

	lv2_atom_forge_key(&lv2->forge, uris->fabla2_PadMuteGroup);
	lv2_atom_forge_int(&lv2->forge, pad->muteGroup() );

	lv2_atom_forge_key(&lv2->forge, uris->fabla2_PadOffGroup);
	lv2_atom_forge_int(&lv2->forge, pad->offGroup() );

	lv2_atom_forge_key(&lv2->forge, uris->fabla2_PadTriggerMode);
	lv2_atom_forge_int(&lv2->forge, pad->triggerMode() );

	lv2_atom_forge_key(&lv2->forge, uris->fabla2_PadSwitchType);
	lv2_atom_forge_int(&lv2->forge, pad->switchSystem() );

	lv2_atom_forge_key(&lv2->forge, uris->fabla2_SampleGain);
	lv2_atom_forge_float(&lv2->forge, s->gain );

	lv2_atom_forge_key(&lv2->forge, uris->fabla2_SamplePitch);
	lv2_atom_forge_float(&lv2->forge, s->pitch );

	lv2_atom_forge_key(&lv2->forge, uris->fabla2_SamplePan );
	lv2_atom_forge_float(&lv2->forge, s->pan );

	lv2_atom_forge_key(&lv2->forge, uris->fabla2_SampleStartPoint );
	lv2_atom_forge_float(&lv2->forge, s->startPoint);

	lv2_atom_forge_key(&lv2->forge, uris->fabla2_SampleVelStartPnt );
	lv2_atom_forge_float(&lv2->forge, s->velLow );

	lv2_atom_forge_key(&lv2->forge, uris->fabla2_SampleVelEndPnt );
	lv2_atom_forge_float(&lv2->forge, s->velHigh );

	lv2_atom_forge_key(&lv2->forge, uris->fabla2_SampleFilterType );
	lv2_atom_forge_float(&lv2->forge, s->filterType );

	lv2_atom_forge_key(&lv2->forge, uris->fabla2_SampleFilterFrequency );
	lv2_atom_forge_float(&lv2->forge, s->filterFrequency );

	lv2_atom_forge_key(&lv2->forge, uris->fabla2_SampleFilterResonance );
	lv2_atom_forge_float(&lv2->forge, s->filterResonance );

	lv2_atom_forge_key(&lv2->forge, uris->fabla2_SampleAdsrAttack );
	lv2_atom_forge_float(&lv2->forge, s->attack );
	lv2_atom_forge_key(&lv2->forge, uris->fabla2_SampleAdsrDecay );
	lv2_atom_forge_float(&lv2->forge, s->decay );
	lv2_atom_forge_key(&lv2->forge, uris->fabla2_SampleAdsrSustain );
	lv2_atom_forge_float(&lv2->forge, s->sustain );
	lv2_atom_forge_key(&lv2->forge, uris->fabla2_SampleAdsrRelease );
	lv2_atom_forge_float(&lv2->forge, s->release );

	lv2_atom_forge_pop(&lv2->forge, &frame);
}

void Fabla2DSP::padRefreshLayers( int bank, int pad)
{
	//printf("%s, %d %d\n", __PRETTY_FUNCTION__, bank, pad);
	Bank* b = library->bank(bank);
	if( !b ) {
		//printf("%s no bank\n", __PRETTY_FUNCTION__);
		return;
	}
	Pad* p = b->pad( pad );
	if( !p ) {
		//printf("%s  no pad\n", __PRETTY_FUNCTION__);
		return;
	}

	for(int i = 0; i < p->nLayers(); i++) {
		LV2_Atom_Forge_Frame frame;

		lv2_atom_forge_frame_time(&lv2->forge, 0);
		lv2_atom_forge_object(&lv2->forge, &frame, 0, uris->fabla2_PadRefreshLayers);

		lv2_atom_forge_key(&lv2->forge, uris->fabla2_bank);
		lv2_atom_forge_int(&lv2->forge, bank );

		lv2_atom_forge_key(&lv2->forge, uris->fabla2_pad);
		lv2_atom_forge_int(&lv2->forge, pad );

		lv2_atom_forge_key(&lv2->forge, uris->fabla2_layer);
		lv2_atom_forge_int(&lv2->forge, i );

		/// write this layers sample details:
		/// 1. Name
		/// 2. if Velocity layered mode: Velocity ranges
		const char* name = p->layer( i )->getName();
		lv2_atom_forge_key(&lv2->forge, uris->fabla2_name);
		lv2_atom_forge_string(&lv2->forge, name, strlen( name ) );
		lv2_atom_forge_pop(&lv2->forge, &frame);
	}

	// now *re-write* the note-on event to highlight the played layer in the UI
	LV2_Atom_Forge_Frame frame;
	lv2_atom_forge_frame_time( &lv2->forge, 0 );
	lv2_atom_forge_object( &lv2->forge, &frame, 0, uris->fabla2_PadPlay );

	lv2_atom_forge_key(&lv2->forge, uris->fabla2_bank);
	lv2_atom_forge_int(&lv2->forge, bank );
	lv2_atom_forge_key(&lv2->forge, uris->fabla2_pad);
	lv2_atom_forge_int(&lv2->forge, pad );
	lv2_atom_forge_key(&lv2->forge, uris->fabla2_layer);
	lv2_atom_forge_int(&lv2->forge, p->lastPlayedLayer() );

	lv2_atom_forge_pop(&lv2->forge, &frame);
}

void Fabla2DSP::tx_waveform( int b, int p, int l, const float* data )
{
	assert( data );

	LV2_Atom_Forge_Frame frame;

	lv2_atom_forge_frame_time(&lv2->forge, 0);
	lv2_atom_forge_object(&lv2->forge, &frame, 0,
	                      uris->fabla2_SampleAudioData);

	lv2_atom_forge_key(&lv2->forge, uris->fabla2_bank);
	lv2_atom_forge_int(&lv2->forge, b );

	lv2_atom_forge_key(&lv2->forge, uris->fabla2_pad);
	lv2_atom_forge_int(&lv2->forge, p );

	lv2_atom_forge_key(&lv2->forge, uris->fabla2_layer);
	lv2_atom_forge_int(&lv2->forge, l );

	//Plotter::plot( "tx_waveform", FABLA2_UI_WAVEFORM_PX, data );

	// Add vector of floats 'audioData' property
	lv2_atom_forge_key(&lv2->forge, uris->fabla2_audioData);
	lv2_atom_forge_vector( &lv2->forge, sizeof(float), uris->atom_Float,
	                       FABLA2_UI_WAVEFORM_PX, data);

	// Close off object
	lv2_atom_forge_pop(&lv2->forge, &frame);
}

void Fabla2DSP::panic()
{
	for(int i = 0; i < voices.size(); ++i) {
		voices.at(i)->stop();
	}
	auditionStop();
}

void Fabla2DSP::uiMessage(int b, int p, int l, int URI, float v)
{
	//printf("Fabla2:uiMessage bank %i, pad %i, layer %i: %f\n", b, p, l, v );

	/*
	if( URI == uris->fabla2_PadPlay )
	{
	  printf("DSP has note on from UI: %i, %i, %i\n", b, p, l);
	}
	*/
	Pad* pad = library->bank( b )->pad( p );
	Sample* s = pad->layer( l );
	if( !s ) {
		// abuse the error handling in UI to blank the sample view of UI
		//printf("%s : sample not valid! Fix this.\n", __PRETTY_FUNCTION__);
		LV2_Atom_Forge_Frame frame;
		lv2_atom_forge_frame_time( &lv2->forge, 0 );
		lv2_atom_forge_object( &lv2->forge, &frame, 0, uris->fabla2_ReplyUiSampleState );
		lv2_atom_forge_key(&lv2->forge, uris->fabla2_bank);
		lv2_atom_forge_int(&lv2->forge, b );
		lv2_atom_forge_key(&lv2->forge, uris->fabla2_pad);
		lv2_atom_forge_int(&lv2->forge, p );
		lv2_atom_forge_pop(&lv2->forge, &frame);
		return;
	}

	tx_waveform( b, p, l, s->getWaveform() );

	if( URI == uris->fabla2_SampleUnload ) {
		// remove a sample from the engine
		//printf("Fabla2-DSP *Deleteing* sample %s now!\n", s->getName() );

		// tell all voices / pads / samplers that the sample is gone
		for(int i = 0; i < voices.size(); ++i) {
			voices.at(i)->stopIfSample( s );
		}
		pad->remove( s );
		pad->checkAll();

		padRefreshLayers( b, p );
		writePadsState( b, p, pad );

		// TODO - refactor away yasper<ptr> stuff, to manually de-alloc
		//delete s;
	} else if(       URI == uris->fabla2_Panic ) {
		panic();
	} else if(       URI == uris->fabla2_PadVolume ) {
		pad->volume = v;
		writePadsState( b, p, pad );
	} else if(       URI == uris->fabla2_PadAuxBus1 ) {
		pad->sends[0] = v;
	} else if(       URI == uris->fabla2_PadAuxBus2 ) {
		pad->sends[1] = v;
	} else if(       URI == uris->fabla2_PadAuxBus3 ) {
		pad->sends[2] = v;
	} else if(       URI == uris->fabla2_PadAuxBus4 ) {
		pad->sends[3] = v;
	} else if(       URI == uris->fabla2_SamplePitch ) {
		s->dirty = 1;
		s->pitch = v;
	} else if(  URI == uris->fabla2_SampleGain ) {
		//printf("setting gain to %f\n", v );
		s->dirty = 1;
		s->gain = v;
	} else if(  URI == uris->fabla2_SamplePan ) {
		//printf("setting pan to %f\n", v );
		s->dirty = 1;
		s->pan = v;
	} else if(  URI == uris->fabla2_SampleStartPoint ) {
		s->dirty = 1;
		s->startPoint = v;
	} else if(  URI == uris->fabla2_SampleEndPoint ) {
		// TODO FIXME printf("%s: STUB\n", __PRETTY_FUNCTION__);
	} else if(  URI == uris->fabla2_SampleVelStartPnt ) {
		s->dirty = 1;
		s->velocityLow( v );
	} else if(  URI == uris->fabla2_SampleVelEndPnt ) {
		s->dirty = 1;
		s->velocityHigh( v );
	} else if(  URI == uris->fabla2_SampleFilterType ) {
		s->dirty = 1;
		s->filterType = v;
	} else if(  URI == uris->fabla2_SampleFilterFrequency ) {
		s->dirty = 1;
		s->filterFrequency = v;
	} else if(  URI == uris->fabla2_SampleFilterResonance ) {
		s->dirty = 1;
		s->filterResonance = v;
	} else if(  URI == uris->fabla2_SampleAdsrAttack ) {
		s->dirty = 1;
		s->attack = v;
	} else if(  URI == uris->fabla2_SampleAdsrDecay ) {
		s->dirty = 1;
		s->decay = v;
	} else if(  URI == uris->fabla2_SampleAdsrSustain ) {
		s->dirty = 1;
		s->sustain = v;
	} else if(  URI == uris->fabla2_SampleAdsrRelease ) {
		s->dirty = 1;
		s->release = v;
	} else if(  URI == uris->fabla2_PadMuteGroup ) {
		int i = int(v);
		//printf("setting mute group to %d\n", i );
		pad->muteGroup( i );
	} else if(  URI == uris->fabla2_PadOffGroup ) {
		int i = int(v);
		//printf("setting off group to %d\n", i );
		pad->offGroup( i );
	} else if(  URI == uris->fabla2_PadSwitchType ) {
		int c = int(v);
		//printf("pad switch type: %i\n", c );
		if( c < Pad::SS_MIN || c > Pad::SS_MAX ) {
			printf("bad pad switch type: %i\n", c);
			c = int(Pad::SS_NONE);
		}
		pad->switchSystem(Pad::SAMPLE_SWITCH_SYSTEM(c));
	} else if(  URI == uris->fabla2_PadTriggerMode ) {
		int i = int(v);
		//printf("pad switch type: %d\n", i );
		pad->triggerMode( (Pad::TRIGGER_MODE) i );
	} else if(  URI == uris->fabla2_RequestUiSampleState ) {
		tx_waveform( b, p, l, s->getWaveform() );
		padRefreshLayers( b, p );
		writePadsState( b, p, pad );
		writeSampleState( b, p, l, pad, s );
	}

}

void Fabla2DSP::auxBus( int bus, float value )
{
	auxBusVol[bus] = value;
}

void Fabla2DSP::startRecordToPad( int b, int p )
{
	recordBank  = b;
	recordPad   = p;
	recordIndex = 0;
	recordEnable = true;

	Pad* pad = library->bank( recordBank )->pad( recordPad );
	pad->midiNotesClear();
	//printf("record starting, bank %i, pad %i\n", recordBank, recordPad );
}

void Fabla2DSP::stopRecordToPad()
{
	//printf("record finished, pad # %i\n", recordPad );

	Pad* pad = library->bank( recordBank )->pad( recordPad );

	printf("%s : NON RT SAFE NEW SAMPLE()\n", __PRETTY_FUNCTION__ );
	Sample* s = new Sample( this, sr, "Recorded", recordIndex, &recordBuffer[0] );

	// reset the pad
	pad->clearAllSamples();
	pad->add( s );

	recordIndex = 0;
	recordEnable = false;
}

Fabla2DSP::~Fabla2DSP()
{
	/* first stop ctlra thread */
	ctlra_thread_quit_now = 1;
	while(!ctlra_thread_quit_done) {
		usleep(1000);
	}

	void *ret;
	ZixStatus stat = zix_thread_join(ctlra_thread, &ret);

	/* flag to ignore read messages etc from now on */
	ctlra_is_quitting = 1;

	ctlra_exit(ctlra);
	usleep(1000);


	for(int i = 0; i < voices.size(); i++) {
		delete voices.at(i);
	}
	delete library;
	delete auditionPad;
	delete auditionVoice;
}

}; // Fabla2

