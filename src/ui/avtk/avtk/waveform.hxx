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

#ifndef OPENAV_AVTK_WAVEFORM_HXX
#define OPENAV_AVTK_WAVEFORM_HXX

#include "widget.hxx"

#include <vector>

namespace Avtk
{

class Waveform : public Widget
{
public:
	Waveform( Avtk::UI* ui, int x, int y, int w, int h, std::string label);
	virtual ~Waveform();

	virtual void draw( cairo_t* cr );

	void show( long samps, const float* data );
	void show( std::vector<float> data );

	/// call to change the zoom level:
	///   1.0 is normal
	///   2.0 shows half the sample data
	///   4.0 shows a quater etc
	void zoom( float zoomLevel = 1.0f );

	/// sets the center sample for the zoom, 0 is start, 0.5 mid, 1 is end
	void zoomOffset( float percentageOffset );

	void setStartPoint( float percent );

private:
	/// cache the drawn waveform for speed
	bool newWaveform;
	cairo_t*          waveformCr;
	cairo_surface_t*  waveformSurf;

	/// this ptr (when not zero) points to a vector that contains the audio data
	/// to be drawn. Its a shared ptr in order to ease memory book-keeping.
	std::vector<float> audioData;

	/// view parameters
	float zoom_;
	float zoomOffset_;
	float startPoint;
};

};

#endif // OPENAV_AVTK_WAVEFORM_HXX
