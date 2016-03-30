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

#ifndef OPENAV_AVTK_THEME_HXX
#define OPENAV_AVTK_THEME_HXX

#include <string>
#include <stdio.h>
#include <cairo/cairo.h>

namespace Avtk
{

/// forward declaration of the class
class UI;

/// each color gets a use-case. To draw using said color
enum USE_CASE {
	BG = 0,
	BG_DARK,
	FG,
	FG_DARK,

	HIGHLIGHT,

	LINE_WIDTH_THIN,
	LINE_WIDTH_WIDE,

	CORNER_RADIUS,

	USE_CASE_COUNT,
};

struct Color {
	/// r, g, b
	float c[3];
};

/// a Theme instance is a set color swatch that can be applied
class Theme
{
public:
	Theme( Avtk::UI* ui_, std::string path );
	virtual ~Theme() {}

	float lineWidthThin()
	{
		return lineWidthThin_;
	}
	float lineWidthNorm()
	{
		return lineWidthNorm_;
	}
	float lineWidthWide()
	{
		return lineWidthWide_;
	}

	float color( cairo_t* cr, USE_CASE uc, float alpha = 1.0 );

	void cornerRadius( int c );

	int cornerRadius_;
	float lineWidthThin_;
	float lineWidthNorm_;
	float lineWidthWide_;

	Avtk::UI* ui;

private:
	static int privateID;
	int ID;

	Color colors[USE_CASE_COUNT];

	int load( std::string jsonTheme );
};

};

#endif // OPENAV_AVTK_THEME_HXX
