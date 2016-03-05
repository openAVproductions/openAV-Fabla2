
#ifndef OPENAV_AVTK_VOLUME_HXX
#define OPENAV_AVTK_VOLUME_HXX

#include "avtk/avtk/avtk.hxx"

namespace Avtk {

class Volume : public Widget
{
public:
	Volume( Avtk::UI* ui, int x, int y, int w, int h, std::string label);
	virtual ~Volume() {}

	virtual void draw( cairo_t* cr );
};
};

#endif // OPENAV_AVTK_VOLUME_HXX
