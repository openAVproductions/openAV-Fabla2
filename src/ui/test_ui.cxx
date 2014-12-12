
#include "test_ui.hxx"
#include "avtk/utils.hxx"
#include "avtk/theme.hxx"

#include <sstream>

#include "../shared.hxx"

static void widgetCB(Avtk::Widget* w, void* ud);

TestUI::TestUI( PuglNativeWindow parent ):
  Avtk::UI( 610, 430, parent )
{
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
}

