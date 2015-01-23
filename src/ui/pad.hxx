
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
    
    void loaded( bool l )
    {
      loaded_ = l;
    }
    
    bool loaded_;
};

};

#endif // OPENAV_AVTK_PAD_HXX
