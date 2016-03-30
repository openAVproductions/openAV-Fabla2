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

#include "button.hxx"

#include "ui.hxx"
#include "theme.hxx"

#include <stdio.h>

using namespace Avtk;

Button::Button( Avtk::UI* ui, int x_, int y_, int w_, int h_, std::string label_) :
	Widget( ui, x_, y_, w_, h_, label_ )
{
	clickMode( CLICK_MOMENTARY );
}

void Button::draw( cairo_t* cr )
{
	cairo_save( cr );

	roundedBox(cr, x_, y_, w_, h_, theme_->cornerRadius_ );

	if( value() > 0.4999 ) {
		theme_->color( cr, HIGHLIGHT, 0.2 );
		cairo_fill_preserve(cr);
		theme_->color( cr, HIGHLIGHT );
		cairo_set_line_width(cr, theme_->lineWidthNorm() );
		cairo_stroke(cr);
	} else {
		theme_->color( cr, BG_DARK );
		cairo_fill_preserve(cr);
		theme_->color( cr, FG );
		cairo_set_line_width(cr, theme_->lineWidthNorm() );
		cairo_stroke(cr);
	}


	// Draw label
	if( label_visible ) {
		cairo_set_font_size(cr, 15.0);
		{
			cairo_text_extents_t extents;
			cairo_text_extents(cr, label(), &extents);
			cairo_move_to(cr,
			              (x_ + w_ / 2) - extents.width / 2,
			              (y_ + h_ / 2) + extents.height / 2 - 2);
		}
		if( !value() ) {
			theme_->color( cr, FG );
		} else {
			theme_->color( cr, BG_DARK );
		}
		cairo_show_text( cr, label() );
	}

	cairo_restore( cr );
}

