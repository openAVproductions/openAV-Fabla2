
#include "test_ui.hxx"
#include "avtk/utils.hxx"
#include "avtk/theme.hxx"

#include <sstream>

#include "../shared.hxx"

// implementation of LV2 Atom writing
#include "writer.hxx"

static void widgetCB(Avtk::Widget* w, void* ud);

TestUI::TestUI( PuglNativeWindow parent ):
  Avtk::UI( 610, 430, parent )
{
  // slider vert
  masterVolume = new Avtk::Slider( this, 520, 40, 22, 220, "Master Volume" );
  masterVolume->callback = widgetCB;
  masterVolume->callbackUD = this;
  add( masterVolume );
  
  /*
  loadSampleBtn = new Avtk::Button( this, 70, 70, 120, 25, "Load Sample" );
  loadSampleBtn->callback = widgetCB;
  loadSampleBtn->callbackUD = this;
  add( loadSampleBtn );
  */
  
  // pads
  int x = 5;
  int y = 41 + (45+2) * 4;
  for(int i = 0; i < 16; i++ )
  {
    if( i != 0 && i % 4 == 0 )
    {
      y -= 45 + 2;
      x = 5;
    }
    
    pads[i] = new Avtk::Button( this, x, y, 52, 44, "-" );
    pads[i]->callback = widgetCB;
    pads[i]->callbackUD = this;
    add( pads[i] );
    
    x += 55;
    
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
