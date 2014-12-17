
#include "fabla2_ui.hxx"

#include "utils.hxx"
#include "theme.hxx"

#include "header.c"

#include <sstream>

#include "../shared.hxx"
#include "../lv2_messaging.hxx"

// implementation of LV2 Atom writing
#include "writer.hxx"

static void fabla2_widgetCB(Avtk::Widget* w, void* ud);

TestUI::TestUI( PuglNativeWindow parent ):
  Avtk::UI( 780, 330, parent )
{
  themes.push_back( new Avtk::Theme( this, "orange.avtk" ) );
  themes.push_back( new Avtk::Theme( this, "green.avtk" ) );
  themes.push_back( new Avtk::Theme( this, "yellow.avtk" ) );
  themes.push_back( new Avtk::Theme( this, "red.avtk" ) );
  
  // slider vert
  Avtk::Image* headerImage = new Avtk::Image( this, 0, 0, 780, 36, "Header Image" );
  headerImage->load( header.pixel_data );
  add( headerImage );
  
  int s = 32;
  
  bankBtns[0] = new Avtk::Button( this, 5, 43, s, s, "A" );
  bankBtns[0]->callback = fabla2_widgetCB;
  bankBtns[0]->callbackUD = this;
  add( bankBtns[0] );
  
  bankBtns[1] = new Avtk::Button( this, 5 + +s+6, 43, s, s, "B" );
  bankBtns[1]->callback = fabla2_widgetCB;
  bankBtns[1]->callbackUD = this;
  bankBtns[1]->theme( theme( 1 ) );
  add( bankBtns[1] );
  
  bankBtns[2] = new Avtk::Button( this, 5, 43  + +s+6, s, s, "C" );
  bankBtns[2]->callback = fabla2_widgetCB;
  bankBtns[2]->callbackUD = this;
  bankBtns[2]->theme( theme( 2 ) );
  add( bankBtns[2] );
  
  bankBtns[3] = new Avtk::Button( this, 5 + +s+6, 43 +s+6, s, s, "D" );
  bankBtns[3]->callback = fabla2_widgetCB;
  bankBtns[3]->callbackUD = this;
  bankBtns[3]->theme( theme( 3 ) );
  add( bankBtns[3] );
  
  recordOverPad = new Avtk::Button( this, 5, 43+(s+6)*2, s * 2 + 6, s*2+6,  "X-REC" );
  recordOverPad->callback = fabla2_widgetCB;
  recordOverPad->callbackUD = this;
  recordOverPad->theme( theme( 4 ) );
  recordOverPad->clickMode( Avtk::Widget::CLICK_TOGGLE );
  add( recordOverPad );
  
  masterPitch = new Avtk::Dial( this, 5, 43+(s+6)*4+6, s * 2 + 6, s*2+6,  "Pitch" );
  masterPitch->callback = fabla2_widgetCB;
  masterPitch->callbackUD = this;
  masterPitch->theme( theme( 2 ) );
  add( masterPitch );
  
  waveform = new Avtk::Waveform( this, 355, 42, 422, 113, "Waveform" );
  std::vector<float> tmp;
  Avtk::loadSample("/usr/local/lib/lv2/fabla2.lv2/test.wav", tmp);
  waveform->show( tmp );
  add( waveform );
  
  /*
  Avtk::Widget* sampleCtrls = new Avtk::Button( this, 446, 160, 332, 167, "Sample Controls" );
  add( sampleCtrls );
  */
  
  // sample edit view
  muteGroup = new Avtk::Button( this, 355, 161, 85, 52, "Mute Group" );
  add( muteGroup );
  layers    = new Avtk::Button( this, 355, 218, 85, 109, "Layers" );
  add( layers );
  adsr      = new Avtk::Button( this, 446, 161, 59, 166, "ADSR" );
  add( adsr );
  filt1     = new Avtk::Button( this, 510, 161, 59, 81, "Filter 1" );
  add( filt1 );
  filt2     = new Avtk::Button( this, 510, 246, 59, 81, "Filter 2" );
  add( filt2 );
  bitcrusDist=new Avtk::Button( this, 573, 161, 59, 81, "Bit Cr,Dist" );
  add( bitcrusDist );
  eq        = new Avtk::Button( this, 573, 247, 59, 81, "Equalizer" );
  add( eq );
  comp      = new Avtk::Button( this, 635, 161, 59, 81, "Comp" );
  add( comp );
  gainPitch = new Avtk::Button( this, 635, 247, 59, 81, "Gain/Ptc" );
  add( gainPitch );
  padSends  = new Avtk::Button( this, 699, 161, 32, 166, "Pad Sends" );
  add( padSends );
  padMaster = new Avtk::Button( this, 736, 160, 40, 166, "Pad Master" );
  add( padMaster );
  
  // pads
  int xS = 60;
  int yS = 60;
  int border = 8;
  
  int x = 82;
  int y = -18 + (yS+border) * 4;
  for(int i = 0; i < 16; i++ )
  {
    if( i != 0 && i % 4 == 0 )
    {
      y -= yS + border;
      x = 82;
    }
    
    pads[i] = new Avtk::Button( this, x, y, xS, yS, "-" );
    pads[i]->callback = fabla2_widgetCB;
    pads[i]->callbackUD = this;
    add( pads[i] );
    
    x += xS + border;
  }
  
  // initial values
  bankBtns[0]->value( true );
  
}

static void fabla2_setBank( TestUI* ui, int bank )
{
  // bank buttons highlight
  for(int i = 0; i < 4; i++ )
  {
    if( i == bank )
      ui->bankBtns[i]->value( true  );
    else
      ui->bankBtns[i]->value( false );
  }
  
  // pad theme set
  for(int i = 0; i < 16; i++)
    ui->pads[i]->theme( ui->theme( bank ) );
  
  ui->redraw();
}
static void fabla2_widgetCB(Avtk::Widget* w, void* ud)
{
  TestUI* ui = (TestUI*)ud;
  
  float tmp = w->value();
  
  printf("widgetCB : %s\n", w->label() );
  
  if( w == ui->recordOverPad )
  {
    ui->write_function( ui->controller, Fabla2::RECORD_OVER_LAST_PLAYED_PAD, sizeof(float), 0, &tmp );
  }
  else if( w == ui->masterPitch )
  {
    float scaleVal = tmp * 24 - 12;
    ui->write_function( ui->controller, Fabla2::MASTER_PITCH, sizeof(float), 0, &scaleVal );
  }
  else if( w == ui->masterVolume )
  {
    ui->write_function( ui->controller, Fabla2::MASTER_VOL, sizeof(float), 0, &tmp );
  }
  else if( w == ui->bankBtns[0] )
  {
    fabla2_setBank( ui, 0 );
  }
  else if( w == ui->bankBtns[1] )
  {
    fabla2_setBank( ui, 1 );
  }
  else if( w == ui->bankBtns[2] )
  {
    fabla2_setBank( ui, 2 );
  }
  else if( w == ui->bankBtns[3] )
  {
    fabla2_setBank( ui, 3 );
    /*
    const char* f = "/usr/local/lib/lv2/fabla2.lv2/drum_loop.wav";
    LV2_Atom* msg = fabla2_writeSampleLoadUnload( &ui->forge, &ui->uris, true, f, strlen(f) );
    printf("Lv2Atom MSG: size = %li, eventTransfer = %i\n", (long)lv2_atom_total_size(msg), ui->uris.atom_eventTransfer ); 
    ui->write_function( ui->controller, Fabla2::ATOM_IN, lv2_atom_total_size(msg), ui->uris.atom_eventTransfer, &msg );
    */
  }
  else if( w == ui->loadSampleBtn )
  {
    printf("load clicked\n");
    
#define OBJ_BUF_SIZE 1024
    uint8_t obj_buf[OBJ_BUF_SIZE];
    lv2_atom_forge_set_buffer(&ui->forge, obj_buf, OBJ_BUF_SIZE);
    
    std::string filenameToLoad = "test.wav";
    
    LV2_Atom* msg = writeSetFile(&ui->forge, &ui->uris, filenameToLoad );
    
    ui->write_function(ui->controller, 0, lv2_atom_total_size(msg),
              ui->uris.atom_eventTransfer,
              msg);
    
  }
}
