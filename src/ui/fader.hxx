
#ifndef OPENAV_AVTK_FADER_HXX
#define OPENAV_AVTK_FADER_HXX

#include "widget.hxx"

namespace Avtk
{

class Fader : public Widget
{
public:
    Fader( Avtk::UI* ui, int x, int y, int w, int h, std::string label);
    virtual ~Fader() {}

    virtual void draw( cairo_t* cr );
};

};

#endif // OPENAV_AVTK_FADER_HXX
