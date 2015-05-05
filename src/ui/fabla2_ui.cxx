
#include "fabla2_ui.hxx"

#include "utils.hxx"
#include "theme.hxx"

#include "pad.hxx"
#include "fader.hxx"
#include "mixstrip.hxx"

// include the data files for the header images
#include "header_fabla.c"
#include "header_openav.c"

#include <sstream>

#include "../shared.hxx"
#include "../lv2_messaging.hxx"

// implementation of LV2 Atom writing
#include "helper.hxx"

// for file browsing
extern "C" {
#include "sofd/libsofd.h"
}

static void fabla2_widgetCB(Avtk::Widget* w, void* ud);

TestUI::TestUI( PuglNativeWindow parent ):
  Avtk::UI( 856, 322, parent ),
  currentBank( 0 ),
  currentPad( 0 ),
  currentLayer(0),
  followPad( true )
{
  themes.push_back( new Avtk::Theme( this, "orange.avtk" ) );
  themes.push_back( new Avtk::Theme( this, "green.avtk" ) );
  themes.push_back( new Avtk::Theme( this, "yellow.avtk" ) );
  themes.push_back( new Avtk::Theme( this, "red.avtk" ) );
  
  Avtk::Image* headerImage = 0;
  headerImage = new Avtk::Image( this, 0, 0, 200, 36, "Header Image - Fabla" );
  headerImage->load( header_fabla.pixel_data );
  headerImage = new Avtk::Image( this, w()-130, 0, 130, 36, "Header Image - OpenAV" );
  headerImage->load( header_openav.pixel_data );
  
  int s = 32;
  bankBtns[0] = new Avtk::Button( this, 5      , 43    , s, s, "A" );
  
  bankBtns[1] = new Avtk::Button( this, 5 + s+6, 43    , s, s, "B" );
  bankBtns[1]->theme( theme( 1 ) );
  bankBtns[2] = new Avtk::Button( this, 5      , 43+s+6, s, s, "C" );
  bankBtns[2]->theme( theme( 2 ) );
  bankBtns[3] = new Avtk::Button( this, 5 + s+6, 43+s+6, s, s, "D" );
  bankBtns[3]->theme( theme( 3 ) );
  
  for(int i = 0; i < 4; i++)
    bankBtns[i]->clickMode( Avtk::Widget::CLICK_TOGGLE );
  
  
  int wx = 5;
  int wy = 43+(s+6)*2; // bottom of bank buttons
  Avtk::Widget* waste = 0;
  
  panicButton = new Avtk::Button( this, wx, wy, s * 2 + 6, 32,  "PANIC" );
  panicButton->clickMode( Avtk::Widget::CLICK_MOMENTARY );
  panicButton->theme( theme(4) );
  wy += 32 + 10;
  
  waste = new Avtk::Box( this, wx, wy, 70, 74,  "Views" );
  waste->clickMode( Widget::CLICK_NONE );
  wy += 20;
  
  uiViewGroup = new Avtk::List( this, wx+2, wy, 70-4, 78, "UiViewGroup");
  uiViewGroup->spacing( 2 );
  uiViewGroup->mode      ( Group::WIDTH_EQUAL );
  uiViewGroup->valueMode ( Group::VALUE_SINGLE_CHILD );
  
  liveView = new Avtk::ListItem( this, wx, 10, 70, 16,  "Live" );
  liveView->clickMode( Avtk::Widget::CLICK_TOGGLE );
  
  padsView = new Avtk::ListItem( this, wx, 10, 70, 16,  "Pads" );
  padsView->clickMode( Avtk::Widget::CLICK_TOGGLE );
  padsView->value( 1 );
  
  fileView = new Avtk::ListItem( this, wx, 10, 70, 16,  "File" );
  fileView->clickMode( Avtk::Widget::CLICK_TOGGLE );
  
  uiViewGroup->end();
  wy += 78;
  
  followPadBtn = new Avtk::Button( this, wx, wy, 70, 20,  "Follow" );
  followPadBtn->clickMode( Avtk::Widget::CLICK_TOGGLE );
  wy += 26;
  
  recordOverPad = new Avtk::Button( this, wx, wy, 70, 30,  "REC" );
  recordOverPad->theme( theme( 4 ) );
  recordOverPad->clickMode( Avtk::Widget::CLICK_TOGGLE );
  
  
  waveformGroup = new Avtk::Group( this, 355, 42, FABLA2_UI_WAVEFORM_PX, 113, "WaveformGroup");
  waveform = new Avtk::Waveform( this, 355, 42, FABLA2_UI_WAVEFORM_PX, 113, "Waveform" );
  
  /// waveform overlays
  int waveX = 355;
  int waveY = 42;
  int waveTW = 120;
  sampleName = new Avtk::Text( this, waveX + 8, waveY + 8, waveTW, 14, "-" );
  
  waveformGroup->end();
  
  
  
  /// sample edit view =========================================================
  int colWidth = 90;
  const int spacer = 4;
  wx = 355;
  wy = 161;
  int divider = 35;
  sampleControlGroup = new Avtk::Group( this, wx, wy, FABLA2_UI_WAVEFORM_PX, 260, "SampleControlGroup");
  
  
  /// sample config options
  waste = new Avtk::Box( this, wx, wy, colWidth, 50,  "Mt-Of-Trg-Swt" );
  waste->clickMode( Widget::CLICK_NONE );
  wy += 14;
  muteGroup = new Avtk::Number( this, wx + 2, wy + 8, 20, 19, "Mute Group" );
  muteGroup->valueMode( Avtk::Widget::VALUE_INT, 0, 8 );
  
  offGroup = new Avtk::Number( this, wx + 24, wy + 8, 20, 19, "Off Group" );
  offGroup->valueMode( Avtk::Widget::VALUE_INT, 0, 8 );
  
  triggerMode = new Avtk::Number( this, wx + 46, wy + 8, 20, 19, "Trigger Mode" );
  triggerMode->valueMode( Avtk::Widget::VALUE_INT, 1, 1 );
  
  switchType = new Avtk::Number( this, wx + 68, wy + 8, 20, 19, "Switch Type" );
  switchType->valueMode( Avtk::Widget::VALUE_INT, 1, 2 );
  wy += 40;
  
  /// layers
  waste = new Avtk::Box( this, wx, wy, colWidth, 104,  "Layers" );
  waste->clickMode( Widget::CLICK_NONE );
  layers    = new Avtk::List( this, wx, wy+18, colWidth, 162-18, "LayersList" );
  layers->end();
  
  /// next column
  wx += colWidth + spacer;
  wy = 161;
  
  /// velocity ranges
  waste = new Avtk::Box( this, wx, wy, colWidth, 50,  "Velocity Map" );
  waste->clickMode( Widget::CLICK_NONE );
  wy += 14;
  velocityStartPoint = new Avtk::Dial( this, wx     , wy, 40, 40, "Velocity Low" );
  velocityEndPoint   = new Avtk::Dial( this, wx + 44, wy, 40, 40, "Velocity High" );
  velocityStartPoint->value( 0 );
  velocityEndPoint  ->value( 1 );
  wy += 40;
  
  /// Velocity -> Volume / Filter
  waste = new Avtk::Box( this, wx, wy, colWidth, 50,  "Vel -> Vol-Fil" );
  waste->clickMode( Widget::CLICK_NONE );
  wy += 14;
  waste = new Avtk::Dial( this, wx     , wy, 40, 40, "VelocityToVolume" );
  waste = new Avtk::Dial( this, wx + 44, wy, 40, 40, "VelocityToFilter" );
  waste->value( 0 );
  wy += 40;
  
  /// Filter
  waste = new Avtk::Box( this, wx, wy, colWidth, 50, "Filter" );
  waste->clickMode( Widget::CLICK_NONE );
  
  filterType = new Avtk::Number( this, wx + colWidth-divider, wy, divider, 14, "Filter Type" );
  filterType->valueMode( Widget::VALUE_INT, 0, 3 );
  
  wy += 14;
  filterFrequency = new Avtk::Dial( this, wx, wy, 40, 40, "Filter Frequency" );
  filterResonance = new Avtk::Dial( this, wx + divider + 10, wy, 40, 40, "Filter Resonance" );
  
  
  
  /// next col
  wx += colWidth + spacer;
  wy = 161;
  
  /// gain pan
  waste = new Avtk::Box( this, wx, wy, colWidth, 50,  "Gain / Pan" );
  waste->clickMode( Widget::CLICK_NONE );
  wy += 16;
  sampleGain = new Avtk::Dial( this, wx + 4, wy, 40, 40, "Gain" );
  sampleGain->value( 0.75 );
  samplePan  = new Avtk::Dial( this, wx + 46, wy, 40, 40, "Pan" );
  samplePan->value( 0.5 );
  wy += 38;
  
  /// start / end point dials
  waste = new Avtk::Box( this, wx, wy, colWidth, 50,  "Start / End" );
  waste->clickMode( Widget::CLICK_NONE );
  wy += 14;
  sampleStartPoint = new Avtk::Dial( this, wx     , wy, 40, 40, "Sample Start Point" );
  sampleEndPoint   = new Avtk::Dial( this, wx + 44, wy, 40, 40, "Sample End Point" );
  sampleEndPoint->value( true );
  wy += 40;
  
  /// pitch / time
  waste = new Avtk::Box( this, wx, wy, colWidth, 50,  "Pitch / Time" );
  waste->clickMode( Widget::CLICK_NONE );
  wy += 14;
  samplePitch = new Avtk::Dial( this, wx     , wy, 40, 40, "Pitch" );
  samplePitch->value( 0.5 );
  sampleTime   = new Avtk::Dial( this, wx + 44, wy, 40, 40, "Time" );
  sampleTime->clickMode( Widget::CLICK_NONE );
  sampleTime->theme( theme(2) );
  
  /// next col
  wx += colWidth + spacer;
  wy = 161;
  waste = new Avtk::Box( this, wx, wy, colWidth, 50,  "ADSR" );
  waste->clickMode( Widget::CLICK_NONE );
  wy += 12;
  adsrA = new Avtk::Dial( this, wx - 2 , wy   , 32, 32, "adsrA" );
  adsrD = new Avtk::Dial( this, wx + 21, wy+10, 32, 32, "adsrD" );
  adsrS = new Avtk::Dial( this, wx + 42, wy   , 32, 32, "adsrS" );
  adsrR = new Avtk::Dial( this, wx + 63, wy+10, 32, 32, "adsrR" );
  adsrS->value( 1.0 );
  wy += 40;
  
  waste = new Avtk::Box( this, wx, wy, colWidth, 50,  "AuxBus 1 2" );
  waste->clickMode( Widget::CLICK_NONE );
  wy += 14;
  send1 = new Avtk::Dial( this, wx     , wy, 40, 40, "AuxBus 1" );
  send1->theme( theme(1) );
  send2 = new Avtk::Dial( this, wx + 44, wy, 40, 40, "AuxBus 2" );
  send2->theme( theme(2) );
  wy += 40;
  
  waste = new Avtk::Box( this, wx, wy, colWidth, 50,  "AuxBus 3 4" );
  waste->clickMode( Widget::CLICK_NONE );
  wy += 14;
  send3 = new Avtk::Dial( this, wx     , wy, 40, 40, "AuxBus 3" );
  send3->theme( theme(3) );
  send4 = new Avtk::Dial( this, wx + 44, wy, 40, 40, "AuxBus 4" );
  send4->theme( theme(4) );
  wy += 40;
  
  ///  pad fader
  wx += colWidth + spacer;
  wy = 161;
  waste = new Avtk::Box( this, wx, wy, colWidth/2, (14+40)*3-4,  "Pad" );
  waste->clickMode( Widget::CLICK_NONE );
  wy += 18;
  padPlay = new Avtk::Button( this, wx+3.5, wy, 38, 16,  "Play" );
  //padPlay->theme( theme( 2 ) ); // green?
  wy += 18;
  padMute = new Avtk::Button( this, wx+3.5, wy, 38, 16,  "Mute" );
  padMute->clickMode( Widget::CLICK_TOGGLE );
  padMute->theme( theme( 1 ) );
  wy += 55-36;
  padVolume = new Avtk::Fader( this, wx+10, wy, 25, 100,  "PadVolume" );
  padVolume->clickMode( Widget::CLICK_NONE );
  padVolume->value( 0.75 );
  
  
  
  //sampleControlGroup->visible( false );
  sampleControlGroup->end();
  
  
  /// load defaults config dir
  loadConfigFile( defaultDir );
  currentDir = defaultDir;
  
  
  
  /// Sample Browser panes =====================================================
  wx = 82;
  wy = 43;
  sampleBrowseGroup = new Avtk::Group( this, wx, wy, 266, 276, "SampleBrowseGroup");
  sampleViewHeader = new Avtk::Box( this, wx, wy, 266, 276,  "Sample Browser" );
  wy += 20 + spacer;
  
  fileViewHome = new Avtk::Button( this, wx     , wy, 50, 23, "Home" );
  fileViewUp   = new Avtk::Button( this, wx + 55, wy, 50, 23, "Up" );
  wy += 25;
  
  // samples folder view
  sampleDirScroll = new Avtk::Scroll( this, wx, wy, 110, 166-25, "SampleFilesScroll" );
  
  listSampleDirs = new Avtk::List( this, 82, 73, 110, 216, "Folder" );
  listSampleDirs->mode      ( Group::WIDTH_EQUAL );
  listSampleDirs->valueMode ( Group::VALUE_SINGLE_CHILD );
  listSampleDirs->resizeMode( Group::RESIZE_FIT_TO_CHILDREN );
  listSampleDirs->end();
  
  sampleDirScroll->set( listSampleDirs );
  
  sampleDirScroll->end();
  
  wx = 198;
  wy = 43 + 20 + spacer;
  // samples view
  sampleFileScroll = new Avtk::Scroll( this, wx, wy, 146, 166, "SampleFilesScroll" );
  
  listSampleFiles = new Avtk::List( this, 0, 0, 126, 866, "Sample Files" );
  listSampleFiles->mode      ( Group::WIDTH_EQUAL );
  listSampleFiles->valueMode ( Group::VALUE_SINGLE_CHILD );
  listSampleFiles->resizeMode( Group::RESIZE_FIT_TO_CHILDREN );
  listSampleFiles->end();
  
  sampleFileScroll->set( listSampleFiles );
  sampleFileScroll->end();
  
  
  sampleBrowseGroup->visible(false);
  sampleBrowseGroup->end();
  
  // pads
  wx = 82;
  wy = 43;
  int xS = 57;
  int yS = 56;
  int border = 9;
  
  padsGroup = new Avtk::Group( this,  wx, wy, 266, 276, "Pads Group");
  waste = new Avtk::Box( this, wx, wy, 266, 276, "Pads" );
  waste->clickMode( Widget::CLICK_NONE );
  
  int x = 87;
  int y = (yS+border) * 4 - 1.5;
  for(int i = 0; i < 16; i++ )
  {
    if( i != 0 && i % 4 == 0 )
    {
      y -= yS + border;
      x = 87;
    }
    
    std::stringstream s;
    s << i + 1;
    pads[i] = new Avtk::Pad( this, x, y, xS, yS, s.str().c_str() );
    
    x += xS + border;
  }
  
  padsGroup->end();
  
  /// live view =======================================================
  wx = 82;
  wy = 43;
  liveGroup = new Avtk::Group( this, wx, wy, 266, 276, "SampleBrowseGroup");
  
  int livePadsX = 464;
  int livePadsY = 276;
  padsHeaderBox = new Avtk::Box( this, wx, wy, livePadsX, 14,  "16 Pads" );
  padsHeaderBox->clickMode( Widget::CLICK_NONE );
  wy += 14;
  
  for(int i = 0; i < 16; ++i)
  {
    int mx = wx + (livePadsX/16.f*i);
    int my = wy;
    int mw = (livePadsX/16.f);
    
    std::stringstream s;
    s << i + 1;
    mixStrip[i] = new Avtk::MixStrip( this, mx, my, mw, livePadsY - 14, s.str().c_str() );
    mixStrip[i]->clickMode( Widget::CLICK_NONE );
    mixStrip[i]->setNum( s.str() );
    
    // dials
    int size = mw+4;
    mw -= 6;
    auxDials[ 0+i] = new Avtk::Dial( this, mx, my       , size, size, "Aux1" );
    auxDials[16+i] = new Avtk::Dial( this, mx, my + mw  , size, size, "Aux2" );
    auxDials[32+i] = new Avtk::Dial( this, mx, my + mw*2, size, size, "Aux3" );
    auxDials[48+i] = new Avtk::Dial( this, mx, my + mw*3, size, size, "Aux4" );
    
    auxDials[ 0+i]->theme( theme( 1 ) );
    auxDials[16+i]->theme( theme( 2 ) );
    auxDials[32+i]->theme( theme( 3 ) );
    auxDials[48+i]->theme( theme( 4 ) );
    
    my += mw*5;
    
    // pad faders
    padFaders[i] = new Avtk::Fader( this, mx + 3, my, mw, 120, "Vol" );
  }
  
  wx = 82;
  wy = 43;
  wx += padsHeaderBox->w() + spacer;
  
  waste = new Avtk::Box( this, wx, wy, 228, 14, "AuxBus" );
  waste->clickMode( Widget::CLICK_NONE );
  wy += 14;
  
  for(int i = 0; i < 4; ++i)
  {
    int mx = wx + (228/4.f*i);
    int my = wy;
    int mw = 228 / 4;
    int mh = 276-14;
    std::stringstream s;
    s << i + 1;
    
    const char* names[] = 
    {
      "Reverb",
      "Compression",
      "Sidechain",
      "Delay",
    };
    
    auxbus[i] = new Avtk::MixStrip( this, mx, my, mw, mh, names[i] );
    auxbus[i]->clickMode( Widget::CLICK_NONE );
    auxbus[i]->setNum( s.str().c_str() );
    
    // buttons
    waste = new Avtk::Button( this, mx+3.5, my+2, mw - 7, 16,  "Solo" );
    waste->clickMode( Widget::CLICK_TOGGLE );
    waste->theme( theme( 3 ) );
    //waste->value( 1 );
    waste = new Avtk::Button( this, mx+3.5, my+22, mw - 7, 16,  "Mute" );
    waste->clickMode( Widget::CLICK_TOGGLE );
    waste->theme( theme( 1 ) );
    //waste->value( 1 );
    waste = new Avtk::Button( this, mx+3.5, my+42, mw - 7, 16,  "Audit" );
    waste->clickMode( Widget::CLICK_TOGGLE );
    waste->theme( theme( 5 ) );
   // waste->value( 1 );
    waste = new Avtk::Button( this, mx+3.5, my+62, mw - 7, 16,  "Meta" );
    waste->clickMode( Widget::CLICK_TOGGLE );
    waste->theme( theme( 2 ) );
   // waste->value( 1 );
    
    // fader
    mw -= 6;
    my += 115;
    auxFaders[i] = new Avtk::Fader( this, mx + 8, my, 30, 140, names[i] );
  }
  liveGroup->visible( false );
  liveGroup->end();
  
  
  /// Master view on right
  wx = 782;
  wy = 43;
  waste = new Avtk::Box( this, wx, wy, 70, 276, "Master" );
  waste->clickMode( Widget::CLICK_NONE );
  wy += 18;
  
  masterVolume = new Avtk::Fader( this, wx+4, wy+5, 70-8, 250,  "Master Volume" );
  masterVolume->clickMode( Widget::CLICK_NONE );
  masterVolume->value( 0.75 );
  
  
  // initial values
  bankBtns[0]->value( true );
  followPadBtn->value(true );
  
  /// created last, so its on top
  deleteLayer = new Avtk::Dialog( this, 0, 0, 320, 120, "Delete Sample?" );
}

void TestUI::blankSampleState()
{
  padVolume       ->value( 0 );
  
  muteGroup       ->value( 0 );
  offGroup        ->value( 0 );
  triggerMode     ->value( 0 );
  switchType      ->value( 0 );
  
  sampleGain      ->value( 0 );
  samplePan       ->value( 0 );
  
  samplePitch     ->value( 0 );
  sampleTime      ->value( 0 );
  
  sampleStartPoint->value( 0 );
  sampleEndPoint  ->value( 0 );
  
  velocityStartPoint->value( 0 );
  velocityEndPoint->value( 0 );
  
  adsrA->value( 0 );
  adsrD->value( 0 );
  adsrS->value( 0 );
  adsrR->value( 0 );
  
  send1           ->value( 0 );
  send2           ->value( 0 );
  send3           ->value( 0 );
  send4           ->value( 0 );
  
  filterType      ->value( 0 );
  filterFrequency ->value( 0 );
  filterResonance ->value( 0 );
  
  sampleName->label("-");
  
  layers->clear();
  
  waveform->setStartPoint( 0 );
  
  std::vector<float> tmp(FABLA2_UI_WAVEFORM_PX);
  for(int i = 0; i < FABLA2_UI_WAVEFORM_PX; ++i)
    tmp[i] = -1.0;
  waveform->show( tmp );
  
}

void TestUI::loadNewDir( std::string newDir )
{
  printf("loadNewDir() %s\n", newDir.c_str() );
  std::vector< std::string > tmp;
  int error = Avtk::directories( newDir, tmp, true, true );
  
  if( !error )
  {
    // don't navigate into a dir with only . and ..
    if( tmp.size() > 2 )
    {
      currentDir = newDir;
      printf("%s, %d : new dir : %s\n", __PRETTY_FUNCTION__, __LINE__, newDir.c_str() );
      listSampleDirs->clear();
      listSampleDirs->show( tmp );
    }
    else
    {
      printf("%s , %d : not moving to sub-dir : has no folders to cd into\n", __PRETTY_FUNCTION__, __LINE__ );
    }
    
    
    currentFilesDir = newDir;
    tmp.clear();
    listSampleFiles->clear();
    error = Avtk::directoryContents( currentFilesDir, tmp, strippedFilenameStart );
    if( !error )
    {
      if( tmp.size() )
      {
        listSampleFiles->show( tmp );
        printf("%s , %d : error showing contents of %s\n", __PRETTY_FUNCTION__, __LINE__, currentFilesDir.c_str() );
      }
      else
      {
        printf("tmp.size() == 0, not showing\n");
      }
    }
  }
  else
  {
    printf("%s , %d :  Error loading dir %s", __PRETTY_FUNCTION__, __LINE__, newDir.c_str() );
    return;
  }
}

void TestUI::showLiveView()
{
  padsGroup         ->visible( false );
  waveformGroup     ->visible( false );
  sampleBrowseGroup ->visible( false );
  sampleControlGroup->visible( false );
  
  liveGroup         ->visible( true  );
}

void TestUI::showPadsView()
{
  liveGroup         ->visible( false );
  sampleBrowseGroup ->visible( false );
  
  padsGroup         ->visible( true );
  waveformGroup     ->visible( true );
  sampleControlGroup->visible( true );
  
  // info could be outdated from live view
  requestSampleState( currentBank, currentPad, currentLayer );
}

/// taken from SOFD example - thanks x42 for this awesome library!
std::string fabla2_showFileBrowser( std::string dir )
{
  Display* dpy = XOpenDisplay(0);
  if (!dpy)
  {
    return "";
  }
  //x_fib_cfg_filter_callback (sofd_filter);
  x_fib_configure (1, "Open File");
  x_fib_load_recent ("/tmp/sofd.recent");
  x_fib_show (dpy, 0, 400, 320);
  
  // stores result to return
  std::string ret;
  
  while (1)
  {
    XEvent event;
    while (XPending (dpy) > 0)
    {
      XNextEvent (dpy, &event);
      if (x_fib_handle_events (dpy, &event))
      {
        if (x_fib_status () > 0) {
          char *fn = x_fib_filename ();
          //printf ("OPEN '%s'\n", fn);
          x_fib_add_recent (fn, time (NULL));
          
          ret = fn;
          free (fn);
        }
      }
    }
    if (x_fib_status ()) {
      break;
    }
    usleep (80000);
  }
  x_fib_close (dpy);
  
  x_fib_save_recent ("/tmp/sofd.recent");
  
  x_fib_free_recent ();
  XCloseDisplay (dpy);
  
  return ret;
}

void TestUI::showFileView()
{
  liveGroup->visible( false );
  padsGroup->visible( false );
  
  // sofd temp replacing in-UI browser
  sampleBrowseGroup->visible( false );
  
  waveformGroup->visible( true );
  sampleControlGroup->visible( true );
  
  ui->redraw();
  
  /*
  loadNewDir( currentDir );
  sampleFileScroll->set( listSampleFiles );
  */
  //printf("spawming SOFD now!\n");
  std::string chosen = fabla2_showFileBrowser( currentDir );
  
  if( chosen.size() > 0 )
  {
    //printf("SOFD returned %s\n", chosen.c_str() );
#define OBJ_BUF_SIZE 1024
    uint8_t obj_buf[OBJ_BUF_SIZE];
    lv2_atom_forge_set_buffer(&forge, obj_buf, OBJ_BUF_SIZE);
    LV2_Atom* msg = writeSetFile( &forge, &uris, currentBank, currentPad, chosen.c_str() );
    write_function(controller, 0, lv2_atom_total_size(msg), uris.atom_eventTransfer, msg);
    
    // return to pads view for triggering
    showPadsView();
    uiViewGroup->value( 1 );
  }
  
}

void TestUI::padEvent( int bank, int pad, int layer, bool noteOn, int velocity )
{
  if( pad < 0 || pad >= 16 )
  {
    return; // invalid pad number
  }
  
  if( noteOn && bank == currentBank )
  {
    pads[pad]->value( true );
    pads[pad]->theme( theme( bank ) );
    
    if( pad == currentPad )
    {
      if( followPad )
      {
        // light up pad play button
        padPlay->value( true );
      }
      
      // set the theme (ensure revert highlighting)
      pads[pad]->theme( theme( bank ) );
    }
    else
    {
      pads[currentPad]->theme( theme( bank ) );
    }
  }
  else
  {
    padPlay->value( false );
    pads[pad]-> value( false );
    
    // if the current note-off event is *also* the current pad:
    // then highlight it as the selected pad
    if( pad == currentPad && bank == currentBank && followPad )
    {
      pads[pad]-> value( 0.78900 );
      pads[pad]->theme( theme( bank+1%3 ) );
    }
  }
  
  layers->value( layer );
  
  bool newLayer = ( pad   != currentPad   ||
                    layer != currentLayer  );
  
  if( followPad && noteOn && newLayer )
  {
    if( bank != currentBank )
    {
      bankBtns[currentBank]->value( false );
      currentBank = bank;
      bankBtns[currentBank]->value( true );
      /*
      // all pads off if we're going to a new bank
      for(int i = 0; i < 16; i++)
      {
        if( i != pad ) // except current pad
          pads[i]->value( false );
      }
      */
      waveform->theme( theme( bank ) );
    }
    
    // if currentPad is highlighted, turn it off
    if( int(pads[currentPad]->value() * 1000) == 789 )
    {
      pads[currentPad]->value( false );
      pads[currentPad]->theme( theme(bank) );
    }
    
    currentLayer = layer;
    
    currentPad  = pad;
    pads[currentPad]->theme( theme(bank) );
    
    padPlay->value( true );
    
    // request update for state from DSP
    //printf("UI requesting %i %i %i\n", bank, pad, layer );
    requestSampleState( currentBank, currentPad, currentLayer );
  }
  
  redraw();
}


void TestUI::requestSampleState( int bank, int pad, int layer )
{
  uint8_t obj_buf[UI_ATOM_BUF_SIZE];
  lv2_atom_forge_set_buffer(&forge, obj_buf, UI_ATOM_BUF_SIZE);
  
  // write message
  LV2_Atom_Forge_Frame frame;
  LV2_Atom* msg = (LV2_Atom*)lv2_atom_forge_object( &forge, &frame, 0, uris.fabla2_RequestUiSampleState);
  
  lv2_atom_forge_key(&forge, uris.fabla2_bank);
  lv2_atom_forge_int(&forge, currentBank );
  
  lv2_atom_forge_key(&forge, uris.fabla2_pad);
  lv2_atom_forge_int(&forge, currentPad );
  
  lv2_atom_forge_key(&forge, uris.fabla2_layer);
  lv2_atom_forge_int(&forge, currentLayer );
  
  //printf("UI writes requestSampleState %i, %i, %i\n", currentBank, currentPad, currentLayer );
  
  lv2_atom_forge_pop(&forge, &frame);
  
  // send it
  write_function(controller, 0, lv2_atom_total_size(msg), uris.atom_eventTransfer, msg);
}

void TestUI::setBank( int bank )
{
  bankBtns[currentBank]->value( false );
  currentBank = bank;
  bankBtns[currentBank]->value( true );
  
  Avtk::Theme* t = theme( bank );
  waveform->theme( t );
  
  for(int i = 0; i < 16; i++)
    mixStrip[i]->theme( t );
  
  //padsHeaderBox->theme( t );
  
  
  /*
  for(int i = 0; i < 16; i++)
  {
    pads[i]->theme( t );
  }
  */
}

void TestUI::writePadPlayStop( bool noteOn, int bank, int pad, int layer )
{
  uint8_t obj_buf[UI_ATOM_BUF_SIZE];
  lv2_atom_forge_set_buffer(&forge, obj_buf, UI_ATOM_BUF_SIZE);
  
  LV2_Atom_Forge_Frame frame;
  int uri = uris.fabla2_PadStop;
  if( noteOn )
    uri = uris.fabla2_PadPlay;
  
  LV2_Atom* msg = (LV2_Atom*)lv2_atom_forge_object( &forge, &frame, 0, uri);
  
  lv2_atom_forge_key(&forge, uris.fabla2_bank);
  lv2_atom_forge_int(&forge, bank );
  
  lv2_atom_forge_key(&forge, uris.fabla2_pad);
  lv2_atom_forge_int(&forge, pad );
  
  lv2_atom_forge_key(&forge, uris.fabla2_layer);
  lv2_atom_forge_int(&forge, layer );
  
  lv2_atom_forge_pop(&forge, &frame);
  
  // send it
  write_function(controller, 0, lv2_atom_total_size(msg), uris.atom_eventTransfer, msg);
}

void TestUI::writeAtomForPad( int eventURI, int pad, float value )
{
  uint8_t obj_buf[UI_ATOM_BUF_SIZE];
  lv2_atom_forge_set_buffer(&forge, obj_buf, UI_ATOM_BUF_SIZE);
  LV2_Atom_Forge_Frame frame;
  LV2_Atom* msg = (LV2_Atom*)lv2_atom_forge_object( &forge, &frame, 0, eventURI);
  
  lv2_atom_forge_key(&forge, uris.fabla2_bank);
  lv2_atom_forge_int(&forge, currentBank );
  lv2_atom_forge_key(&forge, uris.fabla2_pad);
  lv2_atom_forge_int(&forge, pad );
  lv2_atom_forge_key(&forge, uris.fabla2_value);
  lv2_atom_forge_float(&forge, value );
  lv2_atom_forge_pop(&forge, &frame);
  
  write_function(controller, 0, lv2_atom_total_size(msg), uris.atom_eventTransfer, msg);
}

void TestUI::writeAtom( int eventURI, float value )
{
  uint8_t obj_buf[UI_ATOM_BUF_SIZE];
  lv2_atom_forge_set_buffer(&forge, obj_buf, UI_ATOM_BUF_SIZE);
  
  //printf("Fabla2:UI writeAtom %i, %f\n", eventURI, value );
  LV2_Atom_Forge_Frame frame;
  LV2_Atom* msg = (LV2_Atom*)lv2_atom_forge_object( &forge, &frame, 0, eventURI);
  
  lv2_atom_forge_key(&forge, uris.fabla2_bank);
  lv2_atom_forge_int(&forge, currentBank );
  
  lv2_atom_forge_key(&forge, uris.fabla2_pad);
  lv2_atom_forge_int(&forge, currentPad );
  
  // don't write layer if its a pad play event
  if( eventURI != uris.fabla2_PadPlay && eventURI != uris.fabla2_PadStop )
  {
    lv2_atom_forge_key(&forge, uris.fabla2_layer);
    lv2_atom_forge_int(&forge, currentLayer );
  }
  
  lv2_atom_forge_key  (&forge, uris.fabla2_value);
  lv2_atom_forge_float(&forge, value );
  
  lv2_atom_forge_pop(&forge, &frame);
  
  // send it
  write_function(controller, 0, lv2_atom_total_size(msg), uris.atom_eventTransfer, msg);
}

void TestUI::widgetValueCB( Avtk::Widget* w)
{
  float tmp = w->value();
  
  //printf("widgetCB : %s, value: %f\n", w->label(), tmp );
  
  if( w == recordOverPad )
  {
    write_function( controller, Fabla2::RECORD_OVER_LAST_PLAYED_PAD, sizeof(float), 0, &tmp );
  }
  /*
  else if( w == masterPitch )
  {
    float scaleVal = tmp * 24 - 12;
    write_function( controller, Fabla2::MASTER_PITCH, sizeof(float), 0, &scaleVal );
  }
  */
  else if( w == layers )
  {
    currentLayer = int( layers->value() );
    if( w->mouseButton() == 3 )
    {
      int mx = w->mouseX();
      int my = w->mouseY();
      printf("%i %i\n", mx, my );
      
      std::stringstream s;
      s << "Delete layer " << layers->selectedString() << "?";
      
      deleteLayer->run("Delete Layer", s.str().c_str(), Avtk::Dialog::OK_CANCEL, mx, my );
    }
    else
    {
      int lay = int( layers->value() );
      printf("click on layer %i : value() %f\n", lay, tmp );
      if( true ) //;;tmp > 0.4999 )
        writePadPlayStop( true, currentBank, currentPad, lay );
      else
        writePadPlayStop( false, currentBank, currentPad, lay );
    }
  }
  else if( w == deleteLayer )
  {
    // user clicked OK on delete layer dialog
    if( int(tmp) == 1 )
    {
      printf("UI writing sampleUnload\n");
      writeAtom( uris.fabla2_SampleUnload, true );
      requestSampleState( currentBank, currentPad, currentLayer );
    }
  }
  else if( w == fileViewUp )
  {
    std::string newDir;
    std::string current = listSampleDirs->selectedString();
    Avtk::fileUpLevel( current, newDir );
    loadNewDir( newDir );
  }
  else if( w == fileViewHome )
  {
    std::string newDir = getenv("HOME");
    loadNewDir( newDir );
  }
  else if( w == panicButton )
  {
    writeAtom( uris.fabla2_Panic , true );
  }
  else if( w == followPadBtn )
  {
    followPad = int(tmp);
    if( !followPad )
    {
      pads[currentPad]->value(0);
    }
  }
  else if( w == liveView )
  {
    showLiveView();
  }
  else if( w == padsView )
  {
    showPadsView();
  }
  else if( w == fileView )
  {
    showFileView();
  }
  else if( w == listSampleDirs )
  {
    std::string selected = listSampleDirs->selectedString();
    std::stringstream s;
    s << currentDir << "/" << selected;
    // load the new dir
    loadNewDir( s.str().c_str() );
  }
  else if( w == listSampleFiles )
  {
    /*
    // TMP Replaced by SOFD
    std::string selected = listSampleFiles->selectedString();
    std::stringstream s;
    s << currentFilesDir << "/" << strippedFilenameStart << selected;
    printf("UI sending sample load: %s\n", s.str().c_str() );
    
#define OBJ_BUF_SIZE 1024
    uint8_t obj_buf[OBJ_BUF_SIZE];
    lv2_atom_forge_set_buffer(&forge, obj_buf, OBJ_BUF_SIZE);
    LV2_Atom* msg = writeSetFile( &forge, &uris, currentBank, currentPad, s.str() );
    write_function(controller, 0, lv2_atom_total_size(msg), uris.atom_eventTransfer, msg);
    */
  }
  else if( w == offGroup )
  {
    writeAtom( uris.fabla2_PadOffGroup, tmp );
  }
  else if( w == muteGroup )
  {
    writeAtom( uris.fabla2_PadMuteGroup, tmp );
  }
  else if( w == triggerMode )
  {
    writeAtom( uris.fabla2_PadTriggerMode, tmp );
  }
  else if( w == switchType )
  {
    writeAtom( uris.fabla2_PadSwitchType, tmp );
  }
  else if( w == padVolume )
  {
    writeAtom( uris.fabla2_PadVolume, tmp );
  }
  else if( w == velocityStartPoint )
  {
    writeAtom( uris.fabla2_SampleVelStartPnt, tmp );
  }
  else if( w == velocityEndPoint )
  {
    writeAtom( uris.fabla2_SampleVelEndPnt, tmp );
  }
  else if( w == sampleGain )
  {
    writeAtom( uris.fabla2_SampleGain, tmp );
  }
  else if( w == samplePitch )
  {
    writeAtom( uris.fabla2_SamplePitch, tmp );
  }
  else if( w == samplePan )
  {
    writeAtom( uris.fabla2_SamplePan, tmp );
  }
  else if( w == sampleStartPoint )
  {
    float fin = tmp * 0.5;
    waveform->setStartPoint( fin );
    writeAtom( uris.fabla2_SampleStartPoint, fin );
  }
  else if( w == filterType )
  {
    writeAtom( uris.fabla2_SampleFilterType, tmp );
  }
  else if( w == filterFrequency )
  {
    writeAtom( uris.fabla2_SampleFilterFrequency, tmp );
  }
  else if( w == filterResonance )
  {
    writeAtom( uris.fabla2_SampleFilterResonance, tmp );
  }
  else if( w == masterVolume )
  {
    write_function( controller, Fabla2::MASTER_VOL, sizeof(float), 0, &tmp );
  }
  else if( w == padsView )
  {
    followPad = (int)tmp;
    
    // reset current "followed" pad to normal color
    if( !followPad )
    {
      pads[currentPad]->value( 0 );
      pads[currentPad]->theme( theme( currentBank ) );
    }
  }
  else if( w == adsrA )
  {
    writeAtom( uris.fabla2_SampleAdsrAttack, tmp );
  }
  else if( w == adsrD )
  {
    writeAtom( uris.fabla2_SampleAdsrDecay, tmp );
  }
  else if( w == adsrS )
  {
    writeAtom( uris.fabla2_SampleAdsrSustain, tmp );
  }
  else if( w == adsrR )
  {
    writeAtom( uris.fabla2_SampleAdsrRelease, tmp );
  }
  else if( w == send1 )
    writeAtom( uris.fabla2_PadAuxBus1, tmp );
  else if( w == send2 )
    writeAtom( uris.fabla2_PadAuxBus2, tmp );
  else if( w == send3 )
    writeAtom( uris.fabla2_PadAuxBus3, tmp );
  else if( w == send4 )
    writeAtom( uris.fabla2_PadAuxBus4, tmp );
  else
  {
    // check bank buttons
    for(int i = 0; i < 4; i++)
    {
      if( w == bankBtns[i] )
      {
        setBank( i );
        return;
      }
    }
    
    for(int i = 0; i < 16; i++)
    {
      // check the Aux dials in live view
      for(int aux = 0; aux  < 4; ++aux)
      {
        if( w == auxDials[aux*16+i] ) {
          currentBank = i/16;
          currentPad  = i%16;
          if( aux == 0 )
            writeAtomForPad( uris.fabla2_PadAuxBus1, i, tmp );
          if( aux == 1 )
            writeAtomForPad( uris.fabla2_PadAuxBus2, i, tmp );
          if( aux == 2 )
            writeAtomForPad( uris.fabla2_PadAuxBus3, i, tmp );
          if( aux == 3 )
            writeAtomForPad( uris.fabla2_PadAuxBus4, i, tmp );
        }
      }
      
      // check padFaders
      if( w == padFaders[i] )
      {
        writeAtomForPad( uris.fabla2_PadVolume, i, tmp );
      }
      
      // check pads
      if( w == pads[i] )
      {
        if( w->mouseButton() == 3 )
        {
          // rename pad
          
        }
        else
        {
          if( tmp )
          {
            currentPad = i;
            requestSampleState( currentBank, currentPad, currentLayer );
            writeAtom( uris.fabla2_PadPlay, w->value() );
          }
          else
          {
            writeAtom( uris.fabla2_PadStop, 0 );
          }
        }
        return;
      }
    }
  }
}
