
#ifndef OPENAV_AVTK_STEP_HXX
#define OPENAV_AVTK_STEP_HXX

#include "avtk/avtk/button.hxx"
namespace Avtk {

/* Step - used in the Step sequencer UI view */
class Step : public Button 
{
public:
	Step( Avtk::UI* ui, int x, int y, int w, int h, std::string label);
	virtual ~Step() {}
	int row;
	int col;
};
};

#endif // OPENAV_AVTK_STEP_HXX
