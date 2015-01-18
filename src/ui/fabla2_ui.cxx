
#include "fabla2_ui.hxx"

#include "utils.hxx"
#include "theme.hxx"

#include "header.c"

#include <sstream>

#include "../shared.hxx"
#include "../lv2_messaging.hxx"

// implementation of LV2 Atom writing
#include "helper.hxx"

static void fabla2_widgetCB(Avtk::Widget* w, void* ud);

TestUI::TestUI( PuglNativeWindow parent ):
  Avtk::UI( 780, 330, parent ),
  currentBank( 0 ),
  currentPad( 0 ),
  currentLayer(0),
  followPad( true )
{
  themes.push_back( new Avtk::Theme( this, "orange.avtk" ) );
  themes.push_back( new Avtk::Theme( this, "green.avtk" ) );
  themes.push_back( new Avtk::Theme( this, "yellow.avtk" ) );
  themes.push_back( new Avtk::Theme( this, "red.avtk" ) );
  
  masterVolume = 0;
  
  theme_ = themes.front();
  
  // slider vert
  Avtk::Image* headerImage = new Avtk::Image( this, 0, 0, 780, 36, "Header Image" );
  headerImage->load( header.pixel_data );
  
  int s = 32;
  
  bankBtns[0] = new Avtk::Button( this, 5, 43, s, s, "A" );
  
  bankBtns[1] = new Avtk::Button( this, 5 + s+6, 43, s, s, "B" );
  bankBtns[1]->theme( theme( 1 ) );
  
  bankBtns[2] = new Avtk::Button( this, 5, 43  + +s+6, s, s, "C" );
  bankBtns[2]->theme( theme( 2 ) );
  
  bankBtns[3] = new Avtk::Button( this, 5 + +s+6, 43 +s+6, s, s, "D" );
  bankBtns[3]->theme( theme( 3 ) );
  
  for(int i = 0; i < 4; i++)
    bankBtns[i]->clickMode( Avtk::Widget::CLICK_TOGGLE );
  
  
  followPadBtn = new Avtk::Button( this, 5, 43+(s+6)*2 + 75, s * 2 + 6, 22,  "Follow" );
  followPadBtn->clickMode( Avtk::Widget::CLICK_TOGGLE );
  followPadBtn->value( 1 );
  
  loadSample = new Avtk::Button( this, 5, 43+(s+6)*2 + 78+ 25, s * 2 + 6, 22,  "Load" );
  loadSample->clickMode( Avtk::Widget::CLICK_TOGGLE );
  
  /*
  masterPitch = new Avtk::Dial( this, 5, 43+(s+6)*4+6 +26+28, s * 2 + 6, s*2+6,  "Pitch" );
  masterPitch->theme( theme( 2 ) );
  masterPitch->value( 0.5 );
  */
  
  recordOverPad = new Avtk::Button( this, 5, 43+(s+6)*4+6 +26+28, s * 2 + 6, s*2,  "X-REC" );
  recordOverPad->theme( theme( 4 ) );
  recordOverPad->clickMode( Avtk::Widget::CLICK_TOGGLE );
  
  Avtk::Widget* waste = 0;
  
  waveformGroup = new Avtk::Group( this, 355, 42, FABLA2_UI_WAVEFORM_PX, 113, "WaveformGroup");
  waveform = new Avtk::Waveform( this, 355, 42, FABLA2_UI_WAVEFORM_PX, 113, "Waveform" );
  waveformGroup->add( waveform );
  
  /// waveform overlays
  int waveX = 355;
  int waveY = 42;
  int waveTW = 120;
  sampleName = new Avtk::Text( this, waveX + 8, waveY + 8, waveTW, 14, "-" );
  waveformGroup->add( sampleName );
  
  
  /// sample edit view
  int colWidth = 90;
  const int spacer = 4;
  int wx = 355;
  int wy = 161;
  int divider = 35;
  
  
  /// sample config options
  waste = new Avtk::Box( this, wx, wy, colWidth, 50,  "Mute-Trg-Swch" );
  waste->clickMode( Widget::CLICK_NONE );
  wy += 14;
  muteGroup = new Avtk::Number( this, wx + 5, wy + 8, 25, 20, "Mute Group" );
  muteGroup->setRange( 0, 8 );
  
  triggerMode = new Avtk::Number( this, wx + 32, wy + 8, 25, 20, "Trigger Mode" );
  triggerMode->setRange( 1, 1 );
  
  switchType = new Avtk::Number( this, wx + 60, wy + 8, 25, 20, "Switch Type" );
  switchType->setRange( 1, 2 );
  wy += 40;
  
  /// layers
  waste = new Avtk::Box( this, wx, wy, colWidth, 104,  "Layers" );
  waste->clickMode( Widget::CLICK_NONE );
  layers    = new Avtk::List( this, wx, wy+18, colWidth, 162-18, "LayersList" );
  
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
  filterType->setRange( 0, 3 );
  
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
  
  waste = new Avtk::Box( this, wx, wy, colWidth, 50,  "Sends 1 2" );
  waste->clickMode( Widget::CLICK_NONE );
  wy += 14;
  send1 = new Avtk::Dial( this, wx     , wy, 40, 40, "Send1" );
  send1->theme( theme(1) );
  send2 = new Avtk::Dial( this, wx + 44, wy, 40, 40, "Send2" );
  send2->theme( theme(2) );
  wy += 40;
  
  waste = new Avtk::Box( this, wx, wy, colWidth, 50,  "Sends 3 4" );
  waste->clickMode( Widget::CLICK_NONE );
  wy += 14;
  send3 = new Avtk::Dial( this, wx     , wy, 40, 40, "Send3" );
  send3->theme( theme(3) );
  send4 = new Avtk::Dial( this, wx + 44, wy, 40, 40, "Send4" );
  send4->theme( theme(4) );
  wy += 40;
  
  /// master
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
  padVolume = new Avtk::Slider( this, wx+10, wy, 25, 100,  "PadVolume" );
  padVolume->clickMode( Widget::CLICK_NONE );
  padVolume->value( 0.75 );
  
  /// load defaults config dir
  loadConfigFile( defaultDir );
  currentDir = defaultDir;
  
  
  /// Sample Browser panes =====================================================
  wx = 82;
  wy = 43;
  sampleViewHeader = new Avtk::Box( this, wx, wy, 266, 276,  "Sample Browser" );
  wy += 20 + spacer;
  
  // samples folder view
  sampleDirScroll = new Avtk::Scroll( this, wx, wy, 110, 166, "SampleFilesScroll" );
  
  listSampleDirs = new Avtk::List( this, 82, 73, 110, 216, "Folder" );
  listSampleDirs->mode      ( Group::WIDTH_EQUAL );
  listSampleDirs->valueMode ( Group::VALUE_SINGLE_CHILD );
  listSampleDirs->resizeMode( Group::RESIZE_FIT_TO_CHILDREN );
  
  sampleDirScroll->set( listSampleDirs );
  
  wx = 198;
  wy = 43 + 20 + spacer;
  // samples view
  sampleFileScroll = new Avtk::Scroll( this, wx, wy, 146, 166, "SampleFilesScroll" );
  
  listSampleFiles = new Avtk::List( this, 0, 0, 126, 866, "Sample Files" );
  listSampleFiles->mode      ( Group::WIDTH_EQUAL );
  listSampleFiles->valueMode ( Group::VALUE_SINGLE_CHILD );
  listSampleFiles->resizeMode( Group::RESIZE_FIT_TO_CHILDREN );
  
  sampleFileScroll->set( listSampleFiles );
  
  // pads
  int xS = 58;
  int yS = 58;
  int border = 10;
  
  int x = 82;
  int y = -18 + (yS+border) * 4;
  for(int i = 0; i < 16; i++ )
  {
    if( i != 0 && i % 4 == 0 )
    {
      y -= yS + border;
      x = 82;
    }
    
    std::stringstream s;
    s << i + 1;
    pads[i] = new Avtk::Pad( this, x, y, xS, yS, s.str().c_str() );
    
    x += xS + border;
  }
  
  // initial values
  bankBtns[0]->value( true );
  
  //showSampleBrowser( true );
  showSampleBrowser( false );
  
  /// created last, so its on top
  deleteLayer = new Avtk::Dialog( this, 0, 0, 320, 120, "Delete Sample?" );
}

void TestUI::blankSampleState()
{
  muteGroup       ->value( 0 );
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
  waveform->show( tmp );
  
}

void TestUI::loadNewDir( std::string newDir )
{
  std::vector< std::string > tmp;
  int error = Avtk::directories( newDir, tmp );
  
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

void TestUI::showSampleBrowser( bool show )
{
  // toggle other widgets
  for(int i = 0; i < 16; i++)
    pads[i]->visible( !show );
  
  loadNewDir( currentDir );
  
  sampleFileScroll->set( listSampleFiles );
  
  sampleDirScroll ->visible( show );
  sampleFileScroll->visible( show );
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
    
    if( pad == currentPad && bank == currentBank )
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
  
  bool newPad = ( pad   != currentPad   ||
                  layer != currentLayer  );
  
  if( followPad && noteOn && newPad )
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
    //requestSampleState( bank, pad, layer );
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
  
  printf("widgetCB : %s, value: %f\n", w->label(), tmp );
  
  printf("widget mousebutton %i\n", w->mouseButton() );
  
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
      if( deleteLayer->run("Are you sure you want to delete layer!?",
          Avtk::Dialog::OK_CANCEL, mx, my ) == 1 )
      {
        printf("UI writing sampleUnload\n");
        writeAtom( uris.fabla2_SampleUnload, true );
      }
    }
    else
    {
      int lay = int( layers->value() );
      printf("click on layer %i : value() %f\n", lay, tmp );
      if( tmp > 0.4999 )
        writePadPlayStop( true, currentBank, currentPad, lay );
      else
        writePadPlayStop( false, currentBank, currentPad, lay );
    }
  }
  else if( w == loadSample )
  {
    showSampleBrowser( tmp );
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
    std::string selected = listSampleFiles->selectedString();
    std::stringstream s;
    s << currentFilesDir << "/" << strippedFilenameStart << selected;
    printf("UI sending sample load: %s\n", s.str().c_str() );

#define OBJ_BUF_SIZE 1024
    uint8_t obj_buf[OBJ_BUF_SIZE];
    lv2_atom_forge_set_buffer(&forge, obj_buf, OBJ_BUF_SIZE);
    LV2_Atom* msg = writeSetFile( &forge, &uris, currentBank, currentPad, s.str() );
    write_function(controller, 0, lv2_atom_total_size(msg), uris.atom_eventTransfer, msg);
  }
  else if( w == muteGroup )
  {
    float fin = tmp * 8;
    //printf("MuteGroup %i\n", (int)fin);
    writeAtom( uris.fabla2_PadMuteGroup, fin );
  }
  else if( w == triggerMode )
  {
    float fin = tmp;
    //printf("TriggerMode %i\n", (int)fin);
    writeAtom( uris.fabla2_PadTriggerMode, fin );
  }
  else if( w == switchType )
  {
    float fin = tmp * 2.99;
    //printf("switchType %f\n", fin);
    writeAtom( uris.fabla2_PadSwitchType, fin );
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
  else if( w == followPadBtn )
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
    
    // check all the pads
    for(int i = 0; i < 16; i++)
    {
      if( w == pads[i] )
      {
        if( currentPad != i )
        {
          currentPad = i;
          requestSampleState( currentBank, currentPad, currentLayer );
        }
        
        writeAtom( uris.fabla2_PadPlay, w->value() );
        
        return;
      }
    }
  }
}
