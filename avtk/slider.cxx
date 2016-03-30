/*
 * Copyright(c) 2016, OpenAV
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in
 *       the documentation and/or other materials provided with the
 *       distribution.
 *     * Neither the name of the <organization> nor the
 *       names of its contributors may be used to endorse or promote
 *       products derived from this software without specific prior
 *       written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL OPENAV BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "slider.hxx"

#include <stdio.h>
#include "ui.hxx"
#include "theme.hxx"


using namespace Avtk;

Slider::Slider( Avtk::UI* ui, int x_, int y_, int w_, int h_, std::string label_) :
	Widget( ui, x_, y_, w_, h_, label_ )
{
	dragMode( DM_DRAG_VERTICAL );

	if ( w_ > h_ ) {
		dragMode( DM_DRAG_HORIZONTAL );
		scrollInvert = true;
	}
	scrollDisable = false;
}

void Slider::draw( cairo_t* cr )
{
	static const int faderHeight = 16;

	roundedBox(cr, x_, y_, w_, h_, theme_->cornerRadius_ );
	theme_->color( cr, BG_DARK );
	cairo_fill_preserve(cr);
	theme_->color( cr, FG );
	cairo_stroke(cr);

	// fader
	if( dragMode() == DM_DRAG_VERTICAL ) {
		const int range = (h_-faderHeight-2);
		roundedBox(cr, x_+ 1, y_ + 1 + range - range*value(), w_ - 2, faderHeight, theme_->cornerRadius_ );
	} else {
		const int range = (w_-faderHeight-2);
		roundedBox(cr, x_ + 1 + range*value(), y_ + 1, faderHeight, h_ - 2, theme_->cornerRadius_ );
	}

	theme_->color( cr, HIGHLIGHT, 0.2 );
	cairo_fill_preserve(cr);
	theme_->color( cr, HIGHLIGHT );
	cairo_set_line_width(cr, 1.2);
	cairo_stroke(cr);
}

