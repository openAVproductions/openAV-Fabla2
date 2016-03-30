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

#ifndef OPENAV_AVTK_EVENT_EDITOR_HXX
#define OPENAV_AVTK_EVENT_EDITOR_HXX

#include "widget.hxx"

#include <string>
#include <vector>
#include <valarray>

#include "midi.hxx"

namespace Avtk
{

class EventEditor : public Widget
{
public:
	EventEditor( Avtk::UI* ui, int x, int y, int w, int h, std::string label);
	virtual ~EventEditor();

	virtual void draw( cairo_t* cr );

	void zoom( float z )
	{
		if( z > 0.9 ) {
			w_ *= 1.5;
			h_ *= 1.5;
		} else {
			w_ *= 0.75;
			h_ *= 0.75;
		}
	}

	/// sets the data to be displayed in the Editor
	void setEvents( SeqEventList* events );

private:
	// current view
	int startKey;
	int keyCount;

	// event vector:
	SeqEventList* events;

	/// convienience functions
	void drawKeyboard( cairo_t* cr );
};

};

#endif // OPENAV_AVTK_EVENT_EDITOR_HXX
