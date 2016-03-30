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

#include "dial.hxx"

#include <stdio.h>
#include "ui.hxx"
#include "theme.hxx"


using namespace Avtk;

Dial::Dial( Avtk::UI* ui, int x_, int y_, int w_, int h_, std::string l) :
	Widget( ui, x_, y_, w_, h_, l )
{
	dragMode( DM_DRAG_VERTICAL );
	scrollDisable = false;
}

void Dial::draw( cairo_t* cr )
{
	cairo_save( cr );

	theme_->color( cr, FG, 0.5 );
	cairo_new_sub_path( cr );
	cairo_arc(cr, x_+w_/2,y_+h_/2,  w_/2.f - 8, 2.46, 2.46 + 4.54 );
	cairo_set_line_width(cr, w_ / 20.f);
	cairo_stroke(cr);

	cairo_new_sub_path( cr );
	cairo_arc(cr, x_+w_/2,y_+h_/2, w_/2.f - 8, 2.46, 2.46 + 4.54*value() );

	theme_->color( cr, HIGHLIGHT, 0.2 );
	//cairo_fill_preserve(cr);
	theme_->color( cr, HIGHLIGHT, 0.8 );
	cairo_set_line_width(cr, w_ / 7.f);
	cairo_stroke(cr);

	if( label_visible ) {
		cairo_text_extents_t ext;
		cairo_text_extents( cr, label(), &ext );

		cairo_move_to( cr, x_+w_/2-ext.width/2., y_+h_+ext.height/2.-4);
		cairo_set_source_rgb( cr, 1,1,1 );
		cairo_show_text( cr, label() );
	}

	cairo_restore( cr );
}

