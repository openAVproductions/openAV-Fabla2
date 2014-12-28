
#include "pad.hxx"

#include "avtk/avtk/ui.hxx"
#include "avtk/avtk/theme.hxx"

#include <stdio.h>

using namespace Avtk;

Pad::Pad( Avtk::UI* ui, int x_, int y_, int w_, int h_, std::string label_) :
  Widget( ui, x_, y_, w_, h_, label_ )
{
  clickMode( CLICK_MOMENTARY );
}

void Pad::draw( cairo_t* cr )
{
  cairo_save( cr );
  
  roundedBox(cr, x_, y_, w_, h_, -1 );
  
  if( value() > 0.4999 )
  {
    theme_->color( cr, HIGHLIGHT, 0.2 );
    cairo_fill_preserve(cr);
    theme_->color( cr, HIGHLIGHT );
    cairo_set_line_width(cr, theme_->lineWidthWide() );
    cairo_stroke(cr);
  }
  else
  {
    theme_->color( cr, BG_DARK );
    cairo_fill_preserve(cr);
    theme_->color( cr, FG );
    cairo_set_line_width(cr, theme_->lineWidthNorm() );
    cairo_stroke(cr);
  }
  
  // Draw label
  cairo_text_extents_t extents;
  cairo_set_font_size(cr, 15.0);
  cairo_text_extents(cr, label(), &extents);
  cairo_move_to(cr,
                (x_ + w_ / 2) - extents.width / 2,
                (y_ + h_ / 2) + extents.height / 2 - 2);
  
  if( !value() )
  {
    theme_->color( cr, FG );
  }
  else
  {
    theme_->color( cr, BG_DARK );
  }
  cairo_show_text( cr, label() );
  
  cairo_restore( cr );
}

