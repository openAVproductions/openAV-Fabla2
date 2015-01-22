
#ifndef OPENAV_AVTK_PAD_HXX
#define OPENAV_AVTK_PAD_HXX

#include "widget.hxx"

namespace Avtk
{

class Pad : public Widget
{
  public:
    Pad( Avtk::UI* ui, int x, int y, int w, int h, std::string label);
    
    virtual void draw( cairo_t* cr );
    
    bool loaded;
};

};

#endif // OPENAV_AVTK_PAD_HXX
