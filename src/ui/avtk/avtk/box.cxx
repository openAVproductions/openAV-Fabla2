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

#include "box.hxx"

#include "ui.hxx"
#include "theme.hxx"

#include <stdio.h>

using namespace Avtk;

Box::Box( Avtk::UI* ui, int x_, int y_, int w_, int h_, std::string label_) :
	Widget( ui, x_, y_, w_, h_, label_ )
{
	clickMode( CLICK_MOMENTARY );
}

void Box::draw( cairo_t* cr )
{
	cairo_save( cr );


	// draw dark BG
	cairo_rectangle( cr, x_, y_, w_, h_ );
	//roundedBox(cr, x_, y_, w_, h_, theme_->cornerRadius_ );
	theme_->color( cr, BG_DARK, 1 );
	cairo_fill_preserve( cr );
	cairo_stroke( cr );
	theme_->color( cr, BG_DARK, 0.8 );
	cairo_select_font_face(cr, "impact", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);

	// draw header
	cairo_rectangle( cr, x_, y_, w_, 14 );
	theme_->color( cr, HIGHLIGHT, 0.8 );
	cairo_fill( cr );

	// show the text
	cairo_text_extents_t extents;
	cairo_set_font_size(cr, 10.0);
	cairo_text_extents(cr, label(), &extents);
	cairo_move_to(cr, x_ + 4, y_ + 7 + (extents.height / 2) );
	theme_->color( cr, BG_DARK, 1 );
	cairo_show_text( cr, label() );

	cairo_restore( cr );
}

