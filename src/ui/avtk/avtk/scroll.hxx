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

#ifndef OPENAV_AVTK_SCROLL_HXX
#define OPENAV_AVTK_SCROLL_HXX

#include "group.hxx"

#include <string>
#include <vector>

// for the childs cairo_t
#include <cairo/cairo.h>

namespace Avtk
{

// for the scroll bars
class Slider;

/** Scroll
 * A widget that wraps a group, and functions as a scrollable area. This seems
 * a simple task, however in order to appropriately and quickly draw the entire
 * child widget on scroll-movements, this becomes non-trivial.
 *
 * The solution is to have a local cached cairo_t, which the child draws to.
 * The cairo_t context for the child is matched to the childs size, and scales
 * up and down with the child widget.
 *
 * The actual drawing of the child cairo_t is done in Scroll::draw(), where the
 * co-ordinates of the child cairo_t are moved around on paint().
 */
class Scroll : public Group
{
public:
	Scroll( Avtk::UI* ui, int x, int y, int w, int h, std::string label);
	virtual ~Scroll();

	virtual void draw( cairo_t* cr );

	/// sets widget to be the child of this scroll. Only one widget can be inside
	/// a scroll at a time: add a group if multiple child-widgets need to scroll
	void set( Widget* child );

	/// sets the  and horizontal scroll position;
	/// - vertical  : 0 is top,  1 is bottom
	/// - horizontal: 0 is left, 1 is right
	void vertical  ( float v );
	void horizontal( float v );

	/// choose scroll action: pass Ctrl + Scroll wheel on to child, or zoom widget
	void setCtrlZoom( bool zoom );

	/// called by child widgets when thier size changes
	virtual void childResize( Widget* w );

	/// handles an event, propagating it to the integrated scroll bars: and if
	/// not handled, pass on to the child widget
	virtual int handle( const PuglEvent* event );

protected:
	/// when true, child widget is bigger than Scroll, so there is a possibility
	/// to scroll trough the widget.
	bool scrollV_;
	bool scrollH_;

	/// number of pixels that the child is bigger than the Scroll view
	int scrollVamount;
	int scrollHamount;

	bool newChildCr;
	cairo_t* childCr;

	bool redrawChild_;

	int scrollX_;
	int scrollY_;

	bool setCtrlZoom_;

	void redrawChild( cairo_t* cr );

	Avtk::Slider* vSlider;
	Avtk::Slider* hSlider;

	// sliderCB functions
	void sliderCB( Avtk::Widget* w );
	static void staticSliderCB( Avtk::Widget* w, void* ud )
	{
		((Scroll*)ud)->sliderCB( w );
	}

	// convienience function to handle offset from scroll widget position, to
	// the child widgets co-ordinates on the child-cairo canvas
	void offsetEvent( const PuglEvent* inEvent, PuglEvent* outEvent );
};

};

#endif // OPENAV_AVTK_SCROLL_HXX
