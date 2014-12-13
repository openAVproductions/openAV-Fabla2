
#include "test_ui.hxx"
#include "avtk/utils.hxx"
#include "avtk/theme.hxx"

#include <sstream>

#include "../shared.hxx"

static void widgetCB(Avtk::Widget* w, void* ud);

TestUI::TestUI( PuglNativeWindow parent ):
  Avtk::UI( 610, 430, parent )
{
  mapUri( &uris, map );
  
  // slider vert
  masterVolume = new Avtk::Slider( this, 520, 40, 22, 220, "Master Volume" );
  masterVolume->callback = widgetCB;
  masterVolume->callbackUD = this;
  add( masterVolume );
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
  if( false ) // load widget clicked
  {
    /*
#define OBJ_BUF_SIZE 1024
    uint8_t obj_buf[OBJ_BUF_SIZE];
    lv2_atom_forge_set_buffer(&ui->forge, obj_buf, OBJ_BUF_SIZE);
    
    std::string filenameToLoad;
    
    LV2_Atom* msg = writeSetFile(&ui->forge, &ui->uris,
        filenameToLoad.c_str(), strlen(filenameToLoad.c_str()) );
    
    ui->write(ui->controller, 0, lv2_atom_total_size(msg),
              ui->uris.atom_eventTransfer,
              msg);
    */
  }
}
