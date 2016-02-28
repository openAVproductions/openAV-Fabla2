
#include "step.hxx"

#include "ui.hxx"
#include "theme.hxx"

#include <stdio.h>

using namespace Avtk;

Step::Step( Avtk::UI* ui, int x_, int y_, int w_, int h_, std::string label_) :
	Button( ui, x_, y_, w_, h_, label_ )
{
}
