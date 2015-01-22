
#ifndef OPENAV_AVTK_MIX_STRIP_HXX
#define OPENAV_AVTK_MIX_STRIP_HXX

#include "widget.hxx"

namespace Avtk
{

class MixStip : public Widget
{
  public:
    MixStip( Avtk::UI* ui, int x, int y, int w, int h, std::string label);
    
    virtual void draw( cairo_t* cr );
};

};

#endif // OPENAV_AVTK_MIX_STRIP_HXX
