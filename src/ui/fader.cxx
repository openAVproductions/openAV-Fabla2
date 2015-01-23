
#include "fader.hxx"

#include <stdio.h>
#include "ui.hxx"
#include "theme.hxx"


using namespace Avtk;

Fader::Fader( Avtk::UI* ui, int x_, int y_, int w_, int h_, std::string label_) :
  Widget( ui, x_, y_, w_, h_, label_ )
{
  dragMode( DM_DRAG_VERTICAL );
  
  if ( w_ > h_ )
  {
    dragMode( DM_DRAG_HORIZONTAL );
    scrollInvert = true;
  }
  scrollDisable = false;
}

void Fader::draw( cairo_t* cr )
{
  static const int faderHeight = 16;
  
  roundedBox(cr, x_ + (w_/2)-1, y_, 3, h_, 0 );
  theme_->color( cr, BG_DARK );
  //cairo_fill_preserve(cr);
  theme_->color( cr, FG, 0.3 );
  cairo_stroke(cr);
  
  // fader
  if( dragMode() == DM_DRAG_VERTICAL )
  {
    const int range = (h_-faderHeight-2);
    roundedBox(cr, x_+ 1, y_ + 1 + range - range*value(), w_ - 2, faderHeight, theme_->cornerRadius_ );
  }
  else
  {
    const int range = (w_-faderHeight-2);
    roundedBox(cr, x_ + 1 + range*value(), y_ + 1, faderHeight, h_ - 2, theme_->cornerRadius_ );
  }
  
  float a = 0.2;
  cairo_set_source_rgb( cr, 1*a, 1*a, 1*a );
  //theme_->color( cr, HIGHLIGHT, 0.2 );
  cairo_fill_preserve(cr);
  cairo_set_source_rgba( cr, 1, 1, 1, 0.8 );
  //theme_->color( cr, HIGHLIGHT );
  cairo_set_line_width(cr, 1.2);
  cairo_stroke(cr);
}

