
#ifndef OPENAV_AVTK_MIX_STRIP_HXX
#define OPENAV_AVTK_MIX_STRIP_HXX

#include "widget.hxx"

namespace Avtk
{

class MixStrip : public Widget
{
public:
	MixStrip( Avtk::UI* ui, int x, int y, int w, int h, std::string label);

	void setNum( std::string n )
	{
		num = n;
	}

	virtual void draw( cairo_t* cr );

private:
	// for drawing mixer strip number
	std::string num;
};

};

#endif // OPENAV_AVTK_MIX_STRIP_HXX
