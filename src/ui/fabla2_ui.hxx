
#ifndef OPENAV_AVTK_TEST_UI_HXX
#define OPENAV_AVTK_TEST_UI_HXX

#include "avtk.hxx"

#include "../shared.hxx"

// for write_function and controller
#include "lv2/lv2plug.in/ns/extensions/ui/ui.h"

namespace Avtk
{
  class Pad;
  class Widget;
  class MixStrip;
};

#define UI_ATOM_BUF_SIZE 128

class TestUI : public Avtk::UI
{
  public:
    /// Set a NativeWindow for embedding: ignore for standalone 
    TestUI(PuglNativeWindow parent = 0);
    
    /// init function, called by LV2 UI wrapper after setting map, forge etc
    void init()
    {
      setBank( 0 );
      currentLayer = 1; // invalidate, so request updates
      requestSampleState( 0, 0, 0 );
      blankSampleState();
    }
    
    void blankSampleState();
    
    /// widget value callback
    void widgetValueCB( Avtk::Widget* widget);
    
    
    // left: always visible widgets
    Avtk::Widget* bankBtns[4];
    
    Avtk::Button* panicButton;
    
    Avtk::List*  uiViewGroup;
    Avtk::ListItem* liveView;
    Avtk::ListItem* padsView;
    Avtk::ListItem* fileView;
    
    Avtk::Widget* followPadBtn;
    Avtk::Widget* recordOverPad;
    
    Avtk::Dial* masterPitch; // not shown
    
    // Right
    Avtk::Slider* masterVolume;
    
    
    // sample info
    Avtk::Text* sampleName;
    
    // delete layer dialog
    Avtk::Dialog* deleteLayer;
    
    // sample edit view
    Avtk::Number* muteGroup;
    Avtk::Number* offGroup;
    Avtk::Number* triggerMode;
    Avtk::Number* switchType;
    Avtk::List* layers;
    Avtk::Widget* adsr;
    
    Avtk::Widget* filt1;
    Avtk::Number* filterType;
    Avtk::Widget* filterFrequency;
    Avtk::Widget* filterResonance;
    
    Avtk::Widget* filt2;
    Avtk::Widget* bitcrusDist;
    Avtk::Widget* eq;
    Avtk::Widget* comp;
    Avtk::Widget* gainPitch;
    Avtk::Dial* sampleGain;
    Avtk::Dial* samplePan;
    Avtk::Dial* samplePitch;
    Avtk::Dial* sampleTime;
    Avtk::Dial* sampleStartPoint;
    Avtk::Dial* sampleEndPoint;
    
    Avtk::Dial* velocityStartPoint;
    Avtk::Dial* velocityEndPoint;
    
    Avtk::Dial* send1;
    Avtk::Dial* send2;
    Avtk::Dial* send3;
    Avtk::Dial* send4;
    
    Avtk::Dial* adsrA;
    Avtk::Dial* adsrD;
    Avtk::Dial* adsrS;
    Avtk::Dial* adsrR;
    
    Avtk::Button* padPlay;
    Avtk::Button* padMute;
    Avtk::Slider* padVolume;
    
    Avtk::Widget* padSends;
    Avtk::Widget* padMaster;
    
    // Sample / File loading screen
    Avtk::Box* sampleViewHeader;
    Avtk::Scroll* sampleDirScroll;
    Avtk::List* listSampleDirs;
    Avtk::Scroll* sampleFileScroll;
    Avtk::List* listSampleFiles;
    
    // Live view
    Avtk::Group* liveGroup;
    Avtk::Widget* padsHeaderBox;
    
    // shared between views!
    /// holds all waveform related widgets: sample name, sample duration etc
    Avtk::Group*    waveformGroup;
    Avtk::Waveform* waveform;
    
    Avtk::Group*    sampleBrowseGroup;
    Avtk::Group*    sampleControlGroup;
    
    void padEvent( int bank, int pad, int layer, bool noteOn, int velocity );
    
    Avtk::Group* padsGroup;
    Avtk::Pad* pads[16];
    
    Avtk::MixStrip* mixStrip[16];
    Avtk::Dial* aux1[16];
    Avtk::Dial* aux2[16];
    Avtk::Dial* aux3[16];
    Avtk::Dial* aux4[16];
    
    // LV2 ports
    LV2UI_Controller controller;
    LV2UI_Write_Function write_function;
    
    // LV2 Atom
    URIs uris;
    LV2_URID_Map* map;
    LV2_Atom_Forge forge;
  
  private:
    /// default directories / file loading
    std::string defaultDir;
    std::string currentDir;
    std::string currentFilesDir;
    
    /// holds the stripped start of the filename, as presented in List. To build
    /// the loadable /path/filename, we do << currentDir << strippedFilenameStart;
    std::string strippedFilenameStart;
    
    /// followPad allows the UI to update to the last played PAD.
    bool followPad;
    
    int currentBank;
    int currentPad;
    int currentLayer;
    
    /// shows the sample browser window instead of the pads
    void showLiveView();
    void showPadsView();
    void showFileView();
    
    /// updates the UI to a specifc bank
    void setBank( int bank );
    /// writes event/value identified by eventURI using currentBank / currentPad
    void writeAtom( int eventURI, float value );
    /// writes a pad play/stop event
    void writePadPlayStop( bool noteOn, int bank, int pad, int layer );
    /// request the state of a sample from the DSP, to show in the UI
    void requestSampleState( int bank, int pad, int layer );
    /// list sample dirs
    void loadNewDir( std::string newDir );
};


#endif // OPENAV_AVTK_TEST_UI_HXX
