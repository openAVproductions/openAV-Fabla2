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

#include "number.hxx"

#include <stdio.h>
#include <sstream>
#include "ui.hxx"
#include "theme.hxx"


using namespace Avtk;

Number::Number( Avtk::UI* ui, int x_, int y_, int w_, int h_, std::string label_) :
	Widget( ui, x_, y_, w_, h_, label_ ),
	blank( 0 )
{
	valueMode( VALUE_INT, 1, 9 );
	scrollDisable = false;
	dragMode( DM_DRAG_VERTICAL );
}

void Number::blankValue( int b )
{
	blank = b;
}

void Number::draw( cairo_t* cr )
{
	cairo_save( cr );

	theme_->color( cr, HIGHLIGHT, 0.8 );
	roundedBox(cr, x_, y_, w_, h_, theme_->cornerRadius_ );
	cairo_fill_preserve(cr);
	theme_->color( cr, BG_DARK, 1 );
	cairo_set_line_width(cr, theme_->lineWidthWide() );
	cairo_stroke(cr);

	// single int digit shown
	int v = int( value() );

	std::stringstream vStr;
	if( v != blank )
		vStr << v;
	else
		vStr << '_';

	int offset = 0;
	if( v == 10 )
		offset = 0; // move 2 px left, looks better for 2 digits

	// Draw label
	cairo_text_extents_t extents;
	cairo_set_font_size(cr, 15.0);

	cairo_select_font_face(cr, "impact", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);

	cairo_text_extents(cr, vStr.str().c_str(), &extents);
	cairo_move_to(cr, (x_ + w_ / 2) - extents.width / 2 + offset,
	              (y_ + h_ / 2) + extents.height / 2 );

	theme_->color( cr, BG_DARK );
	cairo_show_text( cr, vStr.str().c_str() );

	cairo_restore( cr );
}

