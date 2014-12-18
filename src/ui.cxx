
/// lv2 core / ui includes
#include "lv2/lv2plug.in/ns/lv2core/lv2.h"
#include "lv2/lv2plug.in/ns/extensions/ui/ui.h"


#include "shared.hxx"

#include "ui/fabla2_ui.hxx"

static LV2UI_Handle fabla2_instantiate(const struct _LV2UI_Descriptor * descriptor,
                              const char * plugin_uri,
                              const char * bundle_path,
                              LV2UI_Write_Function write_function,
                              LV2UI_Controller controller,
                              LV2UI_Widget * widget,
                              const LV2_Feature * const * features)
{
  if (strcmp(plugin_uri, FABLA2_URI) != 0)
  {
    fprintf(stderr, "FABLA2_UI_URI error: this GUI does not support plugin with URI %s\n", plugin_uri);
    return NULL;
  }
  
  LV2_URID_Map* map = 0;
  LV2UI_Resize* resize = 0;
  PuglNativeWindow parentXwindow = 0;
  
  for (int i = 0; features[i]; ++i)
  {
    //printf("Feature %s\n", features[i]->URI );
    if (!strcmp(features[i]->URI, LV2_UI__parent))
    {
      parentXwindow = (PuglNativeWindow)features[i]->data;
    }
    else if (!strcmp(features[i]->URI, LV2_UI__resize))
    {
      resize = (LV2UI_Resize*)features[i]->data;
    }
    else if (!strcmp(features[i]->URI, LV2_URID__map))
    {
      map = (LV2_URID_Map*)features[i]->data;
    }
  }
  
  // ensure we have the LV2 requirements
  if( !map || !write_function || !controller || !parentXwindow )
  {
    return 0;
  }
  
  TestUI* t = new TestUI( parentXwindow );
  
  t->map = map;
  t->write_function = write_function;
  t->controller     = controller;
  
  mapUri( &t->uris, map );
  lv2_atom_forge_init( &t->forge, map );
  
  *widget = (void*)t->getNativeHandle();
  
  if (resize)
  {
    resize->ui_resize(resize->handle, 780, 330 );
  }
  else
  {
    printf("Your host does not support LV2:Resize, please ask the developers to implement it!\n");
  }
  
  t->init();
  
  return t;
}

static void fabla2_cleanup(LV2UI_Handle ui)
{
  delete (TestUI*)ui;
}

static void fabla2_port_event(LV2UI_Handle handle,
               uint32_t port_index,
               uint32_t buffer_size,
               uint32_t format,
               const void * buffer)
{
  TestUI* ui = (TestUI*)handle;
  
  /* Check type of data received
   *  - format == 0: Control port event (float)
   *  - format > 0:  Message (atom)
   */
   
  if (format == ui->uris.atom_eventTransfer)
  {
    const LV2_Atom* atom = (const LV2_Atom*)buffer;
  
    const LV2_Atom_Object* obj = (const LV2_Atom_Object*)atom;
    
    bool padStop = (obj->body.otype == ui->uris.fabla2_PadStop);
    if (obj->body.otype == ui->uris.fabla2_PadPlay || padStop)
    {
      //printf("UI: Fabla Pad Event\n");
      const LV2_Atom* bank = NULL;
      const LV2_Atom* pad  = NULL;
      const LV2_Atom* vel  = NULL;
      const int n_props  = lv2_atom_object_get( obj,
          ui->uris.fabla2_bank    , &bank,
          ui->uris.fabla2_pad     , &pad,
          ui->uris.fabla2_velocity, &vel,
          NULL);
      
      if (n_props != 3 ||
          bank->type != ui->uris.atom_Int ||
          pad->type != ui->uris.atom_Int  ||
          vel->type != ui->uris.atom_Int )
      {
        printf("Fabla2::port_event() error: Corrupt state message\n");
        return;
      }
      else
      {
        const int32_t b  = ((const LV2_Atom_Int*)bank)->body;
        const int32_t p  = ((const LV2_Atom_Int*)pad)->body;
        int32_t v  = ((const LV2_Atom_Int*)vel)->body;
        int layer = 0;
        ui->padEvent( b, p, layer, !padStop, v );
      }
    }
    else if( obj->body.otype == ui->uris.fabla2_ReplyUiSampleState )
    {
      /*
      const LV2_Atom* bank = NULL;
      const LV2_Atom* pad  = NULL;
      const LV2_Atom* layer= NULL;
      
      const int n_props  = lv2_atom_object_get( obj,
            ui->uris.fabla2_bank , &bank,
            ui->uris.fabla2_pad  , &pad,
            ui->uris.fabla2_layer, &layer,
            NULL);
      if( bank && pad && layer )
      {
      }
      */
      // atoms to represent the data
      const LV2_Atom* aGain       = 0;
      const LV2_Atom* aPitch      = 0;
      const LV2_Atom* aPan        = 0;
      const LV2_Atom* aStartPoint = 0;
      
      const int n_props  = lv2_atom_object_get( obj,
            ui->uris.fabla2_SampleGain        , &aGain,
            ui->uris.fabla2_SamplePitch       , &aPitch,
            ui->uris.fabla2_SamplePan         , &aPan,
            ui->uris.fabla2_SampleStartPoint  , &aStartPoint,
            //ui->uris.fabla2_Sample  , &pad,
            0 );
      if( aGain && aPan && aPitch && aStartPoint )
      {
        ui->sampleGain      ->value( ((const LV2_Atom_Float*)aGain)->body );
        ui->samplePan       ->value( ((const LV2_Atom_Float*)aPan )->body );
        ui->samplePitch     ->value( ((const LV2_Atom_Float*)aPitch)->body);
        ui->sampleStartPoint->value( ((const LV2_Atom_Float*)aStartPoint)->body);
      }
    }
    else
    {
      fprintf(stderr, "Unknown message type.\n");
    }
  }
  
  ui->redraw();
}

static int fabla2_idle(LV2UI_Handle handle)
{
  //printf("idle()\n");
  TestUI* ui = (TestUI*)handle;
  ui->idle();
  return 0;
}

static const LV2UI_Idle_Interface idle_iface = { fabla2_idle };

static const void*
fabla2_extension_data(const char* uri)
{
	if (!strcmp(uri, LV2_UI__idleInterface)) {
		return &idle_iface;
	}
	return NULL;
}

static const LV2UI_Descriptor descriptor =
{
  FABLA2_UI_URI,
  fabla2_instantiate,
  fabla2_cleanup, 
  fabla2_port_event, 
  fabla2_extension_data
};

LV2_SYMBOL_EXPORT const LV2UI_Descriptor* lv2ui_descriptor(uint32_t index)
{
	switch (index) {
	case 0:
		return &descriptor;
	default:
		return NULL;
	}
}

