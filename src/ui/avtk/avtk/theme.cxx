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

#include "theme.hxx"

#include "common.hxx"

#include "ui.hxx"
#include "widget.hxx"

#include <fstream>
#include <sstream>
#include <fstream>
#include <iostream>

#include "picojson.hxx"

namespace Avtk
{

int Theme::privateID = 0;

Theme::Theme( Avtk::UI* ui_, std::string colour ) :
	ui( ui_ ),
	ID( privateID++ ),
	cornerRadius_( 1 ),
	lineWidthThin_( 0.9 ),
	lineWidthNorm_( 1.1 ),
	lineWidthWide_( 2.1 )
{
	int loadError = true;

	loadError = load( colour );

	if( loadError ) {
		AVTK_DEV("Theme::Theme() Load error: using hard coded defaults\n" );
		// set default values to the colors array
		colors[BG].c[0] = colors[BG].c[1] = colors[BG].c[2] = 34;
		colors[BG_DARK].c[0] = colors[BG_DARK].c[1] = colors[BG_DARK].c[2] = 17;

		colors[FG].c[0] = 76;
		colors[FG].c[1] = 80;
		colors[FG].c[2] = 83;

		colors[FG_DARK].c[0] = 35;
		colors[FG_DARK].c[1] = 87;
		colors[FG_DARK].c[2] =136;

		colors[HIGHLIGHT].c[0] =  0;
		colors[HIGHLIGHT].c[1] =128;
		colors[HIGHLIGHT].c[2] =255;
	}
}

int Theme::load( std::string theme )
{
	try {
		picojson::value v;
		std::stringstream ifs;
		ifs << theme;
		ifs >> v;

		const char* items[5] = {
			"bg",
			"bg-dark",
			"fg",
			"fg-dark",
			"highlight"
		};

		for( int i = 0; i < 5; i++ ) {
			// extract the 3 ints from the array, and store into Color array
			int colNum = 0;

			if( !v.is<picojson::object>() ) {
				printf("%s : Error  v is NOT array\n", __PRETTY_FUNCTION__);
				return -1;
			}

			picojson::array list = v.get( items[i] ).get<picojson::array>();
			//printf("array list ok\n");

			//std::cerr << picojson::get_last_error() << std::endl;

			for (picojson::array::iterator iter = list.begin();
			     iter != list.end(); ++iter) {
				double tmp = (int)(*iter).get("c").get<double>();
				//printf("%s = %lf\r\n", items[i], tmp );
				colors[i].c[colNum++] = tmp;
			}
		}
	} catch( ... ) {
		printf("Theme::load() Error loading theme from %s : falling back to"\
		       "default.Double check file-exists and JSON contents valid.\n",
		       theme.c_str() );
		return -1;
	}
	return 0;
}

void Theme::cornerRadius( int c )
{
	cornerRadius_ = c;
	ui->redraw();
}

float Theme::color( cairo_t* cr, USE_CASE uc, float alpha_ )
{
	float r = colors[uc].c[0] / 255.;
	float g = colors[uc].c[1] / 255.;
	float b = colors[uc].c[2] / 255.;
	//printf("%f, %f, %f\n", r, g, b );
	cairo_set_source_rgba(cr, r, g, b, alpha_);
	return 0;
}

}; // Avtk
