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

#include "image.hxx"

#include <stdio.h>

using namespace Avtk;

Image::Image( Avtk::UI* ui, int x_, int y_, int w_, int h_, std::string label_) :
	Widget( ui, x_, y_, w_, h_, label_ ),
	imgSurf(0),
	cairoImgData(0)
{
	stride = cairo_format_stride_for_width (CAIRO_FORMAT_ARGB32, w_);
	cairoImgData = malloc (stride * h_);
}

Image::~Image()
{
#ifdef AVTK_DEBUG
	printf("%s\n", __PRETTY_FUNCTION__ );
#endif
	cairo_surface_destroy( imgSurf );
	free( cairoImgData );
}

void Image::load( const unsigned char* data )
{
#ifdef AVTK_DEBUG
	printf("w = %i, stride = %i\n", w_, stride );
#endif // AVTK_DEBUG
	memcpy( cairoImgData, data, sizeof(unsigned char) * w_ * h_ * 4 );
	imgSurf = cairo_image_surface_create_for_data( (unsigned char*)cairoImgData, CAIRO_FORMAT_ARGB32, w_, h_, stride);
}

void Image::draw( cairo_t* cr )
{
	if( !imgSurf ) {
#ifdef AVTK_DEBUG
		printf("Image::draw(), this = %i, imgSurf == 0\n", this );
#endif // AVTK_DEBUG
		return;
	}

	cairo_save( cr );
	cairo_set_source_surface( cr, imgSurf, x_, y_ );
	cairo_paint( cr );
	cairo_restore( cr );
}

