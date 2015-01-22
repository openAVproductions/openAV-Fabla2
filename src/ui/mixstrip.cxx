
#include "mixstrip.hxx"

#include "avtk/avtk/ui.hxx"
#include "avtk/avtk/theme.hxx"

#include <stdio.h>

using namespace Avtk;

MixStrip::MixStrip( Avtk::UI* ui, int x_, int y_, int w_, int h_, std::string label_) :
  Widget( ui, x_, y_, w_, h_, label_ )
{
}

void MixStrip::draw( cairo_t* cr )
{
  cairo_save( cr );
  
  cairo_rectangle( cr, x_+1, y_, w_-2, h_ );
  theme_->color( cr, BG_DARK );
  cairo_fill( cr );
  
  if( value() > 0.4999 )
  {
    cairo_set_source_rgba( cr, 1,1,1, 0.1 );
    //theme_->color( cr, HIGHLIGHT, 0.2 );
    cairo_fill_preserve(cr);
    cairo_set_source_rgba( cr, 1,1,1, 0.75 );
    //theme_->color( cr, HIGHLIGHT, 0.8 );
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
  cairo_set_font_size(cr, 12.0);
  cairo_text_extents(cr, label(), &extents);
  
  int labelPosY = y_ + h_*6/17.f+4;
  
  int textX = (x_ + w_ / 2) - extents.width / 2;
  int textY = labelPosY     + extents.height + 2;
  
  // square behind text
  cairo_rectangle( cr, x_+1, labelPosY, w_-2, 14 );
  theme_->color( cr, HIGHLIGHT, 0.8 );
  cairo_fill( cr );
  
  // text
  if( !value() )
    theme_->color( cr, BG_DARK );
  else
    theme_->color( cr, BG );
  cairo_move_to(cr, textX, textY );
  cairo_show_text( cr, num.c_str() );
  
  {
    cairo_save(cr);
    cairo_move_to(cr, x_ + w_ + 5 - extents.height, y_ + h_ - 5 );
    cairo_rotate( cr, -3.1415/2.f );
    
    //theme_->color( cr, HIGHLIGHT, 0.8 );
    cairo_set_source_rgb( cr, 1,1,1 );
    cairo_show_text( cr, label() );
    
    cairo_restore(cr);
  }
  
  cairo_restore( cr );
}

