
#ifndef OPENAV_AVTK_TEST_UI_HXX
#define OPENAV_AVTK_TEST_UI_HXX

#include "avtk/avtk.hxx"

#include "../shared.hxx"

#include "header.c"

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
  
    Avtk::Widget* masterVolume;
    Avtk::List* list;
    Avtk::List* list2;
    Avtk::Waveform* waveform;
    
    Avtk::Widget* loadSampleBtn;
    
    // LV2 ports
    LV2UI_Controller controller;
    LV2UI_Write_Function write_function;
    
    // LV2 Atom
    URIs uris;
    LV2_URID_Map* map;
    LV2_Atom_Forge forge;
};


#endif // OPENAV_AVTK_TEST_UI_HXX
