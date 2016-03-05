
#include "volume.hxx"

#include "avtk/avtk/ui.hxx"
#include "avtk/avtk/theme.hxx"

#include <stdio.h>

using namespace Avtk;

Volume::Volume( Avtk::UI* ui, int x_, int y_, int w_, int h_, std::string label_) :
	Widget( ui, x_, y_, w_, h_, label_ )
{
}

void Volume::draw( cairo_t* cr )
{
	cairo_save( cr );

	cairo_rectangle( cr, x_, y_, w_, h_ );
	theme_->color( cr, BG_DARK );
	cairo_fill(cr);

	int offset = h_ * (value());
	int wide = w_ / 3.;
	int tmpx = x_ + 20;
	cairo_rectangle( cr, tmpx + 4, y_ + h_ - offset, wide, offset );
	cairo_rectangle( cr, tmpx - 4 - wide, y_ + h_ - offset, wide, offset );
	theme_->color( cr, HIGHLIGHT, 0.2 );
	cairo_fill_preserve(cr);
	theme_->color( cr, HIGHLIGHT, 0.8 );
	cairo_set_line_width(cr, theme_->lineWidthWide() );
	cairo_stroke(cr);

	cairo_restore( cr );
}

