
#include "test_ui.hxx"
#include "avtk/utils.hxx"
#include "avtk/theme.hxx"

#include <sstream>

#include "../shared.hxx"

// implementation of LV2 Atom writing
#include "writer.hxx"

static void widgetCB(Avtk::Widget* w, void* ud);

TestUI::TestUI( PuglNativeWindow parent ):
  Avtk::UI( 780, 330, parent )
{
  // slider vert
  Avtk::Image* headerImage = new Avtk::Image( this, 0, 0, 780, 36, "Header Image" );
  headerImage->load( header.pixel_data );
  add( headerImage );
  
  bankA = new Avtk::Button( this, 5, 43, 50, 65, "A" );
  bankA->callback = widgetCB;
  bankA->callbackUD = this;
  bankA->value( true );
  add( bankA );
  
  bankB = new Avtk::Button( this, 5, 115, 50, 65, "B" );
  bankB->callback = widgetCB;
  bankB->callbackUD = this;
  add( bankB );
  
  bankC = new Avtk::Button( this, 5, 187, 50, 65, "C" );
  bankC->callback = widgetCB;
  bankC->callbackUD = this;
  add( bankC );
  
  bankD = new Avtk::Button( this, 5, 258, 50, 65, "D" );
  bankD->callback = widgetCB;
  bankD->callbackUD = this;
  add( bankD );
  
  waveform = new Avtk::Waveform( this, 355, 42, 422, 113, "Waveform" );
  std::vector<float> tmp;
  Avtk::loadSample("/usr/local/lib/lv2/fabla2.lv2/test.wav", tmp);
  waveform->show( tmp );
  add( waveform );
  
  
  Avtk::Widget* sampleCtrls = new Avtk::Button( this, 446, 160, 332, 167, "Sample Controls" );
  add( sampleCtrls );
  
  Avtk::Widget* layers = new Avtk::Button( this, 355, 160, 85, 167, "Layers" );
  add( layers );
  
  
  // pads
  int xS = 65;
  int yS = 64;
  int border = 8;
  
  int x = 62;
  int y = -29 + (yS+border) * 4;
  for(int i = 0; i < 16; i++ )
  {
    if( i != 0 && i % 4 == 0 )
    {
      y -= yS + border;
      x = 62;
    }
    
    pads[i] = new Avtk::Button( this, x, y, xS, yS, "-" );
    pads[i]->callback = widgetCB;
    pads[i]->callbackUD = this;
    add( pads[i] );
    
    x += xS + border;
  }
  
}

static void widgetCB(Avtk::Widget* w, void* ud)
{
  TestUI* ui = (TestUI*)ud;
  
  float tmp = w->value();
  
  if( w == ui->masterVolume )
  {
    printf("master volume\n");
    ui->write_function( ui->controller, Fabla2::MASTER_VOL, sizeof(float), 0, &tmp );
  }
  if( w == ui->loadSampleBtn )
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
