
#ifndef OPENAV_AVTK_TEST_UI_HXX
#define OPENAV_AVTK_TEST_UI_HXX

#include "avtk.hxx"

#include "../shared.hxx"

// for write_function and controller
#include "lv2/lv2plug.in/ns/extensions/ui/ui.h"

namespace Avtk
{
  class Widget;
};

class TestUI : public Avtk::UI
{
  public:
    /// Set a NativeWindow for embedding: ignore for standalone 
    TestUI(PuglNativeWindow parent = 0);
    
    /// demo function, changes all widgets to value
    void setAllWidgets( Avtk::Widget* w, float value );
    
    
    // always visible widgets
    // L
    Avtk::Widget* bankBtns[4];
    Avtk::Widget* recordOverPad;
    Avtk::Dial* masterPitch;
    
    // R
    Avtk::Widget* masterVolume;
    
    
    // sample edit view
    Avtk::Widget* muteGroup;
    Avtk::Widget* layers;
    Avtk::Widget* adsr;
    Avtk::Widget* filt1;
    Avtk::Widget* filt2;
    Avtk::Widget* bitcrusDist;
    Avtk::Widget* eq;
    Avtk::Widget* comp;
    Avtk::Widget* gainPitch;
    Avtk::Widget* padSends;
    Avtk::Widget* padMaster;
    
    // Preset loading screen
    /*
    Avtk::List* list;
    Avtk::List* list2;
    */
    
    // shared between views!
    Avtk::Waveform* waveform;
    
    Avtk::Widget* loadSampleBtn;
    
    
    Avtk::Widget* pads[16];
    
    // LV2 ports
    LV2UI_Controller controller;
    LV2UI_Write_Function write_function;
    
    // LV2 Atom
    URIs uris;
    LV2_URID_Map* map;
    LV2_Atom_Forge forge;
};


#endif // OPENAV_AVTK_TEST_UI_HXX
