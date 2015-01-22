
/// lv2 core / ui includes
#include "lv2/lv2plug.in/ns/lv2core/lv2.h"
#include "lv2/lv2plug.in/ns/extensions/ui/ui.h"


#include "shared.hxx"

#include "ui/fabla2_ui.hxx"

#include "plotter.hxx"

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
    resize->ui_resize(resize->handle, t->w(), t->h() );
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
      const LV2_Atom* lay  = NULL;
      const LV2_Atom* vel  = NULL;
      const int n_props  = lv2_atom_object_get( obj,
          ui->uris.fabla2_bank    , &bank,
          ui->uris.fabla2_pad     , &pad,
          ui->uris.fabla2_layer   , &lay,
          ui->uris.fabla2_velocity, &vel,
          NULL);
      
      if ( bank->type != ui->uris.atom_Int ||
           pad->type != ui->uris.atom_Int  ||
           lay->type != ui->uris.atom_Int   )
      {
        //printf("Fabla2-UI::port_event() error: Corrupt state message\n");
        return;
      }
      else
      {
        const int32_t b  = ((const LV2_Atom_Int*)bank)->body;
        const int32_t p  = ((const LV2_Atom_Int*)pad)->body;
        int32_t layer    = ((const LV2_Atom_Int*)lay)->body;
        
        int32_t v = 127;
        if( vel )
          v = ((const LV2_Atom_Int*)vel)->body;
        
        //printf("UI pad event %i, %i, %i\n", b, p, layer );
        ui->padEvent( b, p, layer, !padStop, v );
      }
    }
    else if( obj->body.otype == ui->uris.fabla2_SampleAudioData )
    {
      const LV2_Atom* data_val = NULL;
      const int n_props  = lv2_atom_object_get( obj,
                ui->uris.fabla2_audioData, &data_val, NULL);

      if (data_val->type != ui->uris.atom_Vector) {
        // Object does not have the required properties with correct types
        fprintf(stderr, "Fabla2 UI error: Corrupt audio message\n");
        return;
      }
      
      // Get the values we need from the body of the property value atoms
      const LV2_Atom_Vector* vec = (const LV2_Atom_Vector*)data_val;
      if (vec->body.child_type != ui->uris.atom_Float) {
        fprintf(stderr, "Fabla2 UI error: Corrupt audio message, incorrect element type!\n");
              return;  // Vector has incorrect element type
      }
      
      // Number of elements = (total size - header size) / element size
      const size_t n_elem = ((data_val->size - sizeof(LV2_Atom_Vector_Body))
                             / sizeof(float));
      
      // Float elements immediately follow the vector body header
      const float* data = (const float*)(&vec->body + 1);
      
      //printf("Fabla UI got %i elements for waveform data\n", n_elem );
      
      Plotter::plot( "waveformArrived.dat", FABLA2_UI_WAVEFORM_PX, data );
      ui->waveform->show( FABLA2_UI_WAVEFORM_PX, data );
      ui->redraw();
    }
    else if( obj->body.otype == ui->uris.fabla2_PadRefreshLayers )
    {
      const LV2_Atom* bank = 0;
      const LV2_Atom* pad  = 0;
      const LV2_Atom* lay  = 0;
      const LV2_Atom* name = 0;
      const int n_props  = lv2_atom_object_get( obj,
          ui->uris.fabla2_bank    , &bank,
          ui->uris.fabla2_pad     , &pad,
          ui->uris.fabla2_layer   , &lay,
          ui->uris.fabla2_name    , &name,
          NULL);
      
      if (n_props != 4 ||
          bank->type != ui->uris.atom_Int  ||
          pad->type  != ui->uris.atom_Int  ||
          lay->type  != ui->uris.atom_Int  )
          //name->type != ui->uris.atom_String )
      {
        printf("Fabla2::port_event() error: Corrupt state message\n");
        return;
      }
      else
      {
        const int32_t b  = ((const LV2_Atom_Int*)bank)->body;
        const int32_t p  = ((const LV2_Atom_Int*)pad)->body;
        int32_t layer    = ((const LV2_Atom_Int*)lay)->body;
        std::string n    = (const char*) LV2_ATOM_BODY_CONST( name );
        
        //printf("UI got PadRefresh: layer = %i, name = %s\n", layer, n.c_str() );
        
        if( layer == 0 ) // starting from start: reset
        {
          ui->layers->clear();
        }
        ui->layers->addItem( n );
        ui->pads[p]->loaded = true;
      }
    }
    else if( obj->body.otype == ui->uris.fabla2_ReplyUiSampleState )
    {
      // atoms to represent the data
      const LV2_Atom* aPad        = 0;
      
      const LV2_Atom* aPadVolume  = 0;
      
      const LV2_Atom* aPadOffGrp  = 0;
      const LV2_Atom* aPadMuteGrp = 0;
      const LV2_Atom* aPadTrigMode= 0;
      const LV2_Atom* aPadSwtchSys= 0;
      
      const LV2_Atom* aName       = 0;
      
      const LV2_Atom* aGain       = 0;
      const LV2_Atom* aPan        = 0;
      
      const LV2_Atom* aPitch      = 0;
      const LV2_Atom* aTime       = 0;
      
      const LV2_Atom* aStartPoint = 0;
      const LV2_Atom* aEndPoint   = 0;
      
      const LV2_Atom* aVelLow     = 0;
      const LV2_Atom* aVelHigh    = 0;
      
      const LV2_Atom* aFiltType   = 0;
      const LV2_Atom* aFiltFreq   = 0;
      const LV2_Atom* aFiltReso   = 0;
      
      const LV2_Atom* aAttack     = 0;
      const LV2_Atom* aDecay      = 0;
      const LV2_Atom* aSustain    = 0;
      const LV2_Atom* aRelease    = 0;
      
      const int n_props  = lv2_atom_object_get( obj,
            ui->uris.fabla2_pad               , &aPad,
            ui->uris.fabla2_PadVolume         , &aPadVolume,
            ui->uris.fabla2_PadOffGroup       , &aPadOffGrp,
            ui->uris.fabla2_PadMuteGroup      , &aPadMuteGrp,
            ui->uris.fabla2_PadTriggerMode    , &aPadTrigMode,
            ui->uris.fabla2_PadSwitchType     , &aPadSwtchSys,
            ui->uris.fabla2_name              , &aName,
            ui->uris.fabla2_SampleGain        , &aGain,
            ui->uris.fabla2_SamplePitch       , &aPitch,
            ui->uris.fabla2_SampleTime        , &aTime,
            ui->uris.fabla2_SampleStartPoint  , &aStartPoint,
            ui->uris.fabla2_SampleEndPoint    , &aEndPoint,
            ui->uris.fabla2_SampleVelStartPnt , &aVelLow,
            ui->uris.fabla2_SampleVelEndPnt   , &aVelHigh,
            ui->uris.fabla2_SamplePan         , &aPan,
            ui->uris.fabla2_SampleFilterType  , &aFiltType,
            ui->uris.fabla2_SampleFilterFrequency,&aFiltFreq,
            ui->uris.fabla2_SampleFilterResonance,&aFiltReso,
            
            ui->uris.fabla2_SampleAdsrAttack  ,&aAttack,
            ui->uris.fabla2_SampleAdsrDecay   ,&aDecay,
            ui->uris.fabla2_SampleAdsrSustain ,&aSustain,
            ui->uris.fabla2_SampleAdsrRelease ,&aRelease,
            0 );
      
      int pad = -1;
      if( aPad )
      {
        pad = ((const LV2_Atom_Int*)aPad)->body;
      } 
      
      if( aGain && aPan && aPitch && aStartPoint )
      {
        float tmp = ((const LV2_Atom_Float*)aPadVolume)->body;
        //printf("UI got ReplyUiSampleState from DSP : volume %f\n", tmp);
        ui->padVolume       ->value( tmp );
        
        int mute = ((const LV2_Atom_Int*)aPadMuteGrp)->body;
        int off  = ((const LV2_Atom_Int*)aPadOffGrp)->body;
        int trig = ((const LV2_Atom_Int*)aPadTrigMode)->body;
        int swtc = ((const LV2_Atom_Int*)aPadSwtchSys)->body;
        //printf("UI numbers: %i, %i, %i, %i\n", mute, off, trig, swtc );
        
        ui->muteGroup   ->value( mute );
        ui->offGroup    ->value( off  );
        ui->triggerMode ->value( trig );
        ui->switchType  ->value( swtc );
        
        // sample string up top of waveform
        const char* n = (const char*) LV2_ATOM_BODY_CONST( aName );
        ui->sampleName->label( n );
        
        ui->sampleGain      ->value( ((const LV2_Atom_Float*)aGain)->body );
        ui->samplePan       ->value( ((const LV2_Atom_Float*)aPan )->body );
        ui->samplePitch     ->value( ((const LV2_Atom_Float*)aPitch)->body);
        
        ui->sampleStartPoint->value( ((const LV2_Atom_Float*)aStartPoint)->body*2); // 2* as dial offsets on write too!
        ui->waveform->setStartPoint( ((const LV2_Atom_Float*)aStartPoint)->body);
        
        ui->velocityStartPoint->value(((const LV2_Atom_Float*)aVelLow)->body  );
        ui->velocityEndPoint  ->value(((const LV2_Atom_Float*)aVelHigh)->body );
        
        ui->filterType      ->value( ((const LV2_Atom_Float*)aFiltType)->body);
        ui->filterFrequency ->value( ((const LV2_Atom_Float*)aFiltFreq)->body);
        ui->filterResonance ->value( ((const LV2_Atom_Float*)aFiltReso)->body);
        
        ui->adsrA ->value( ((const LV2_Atom_Float*)aAttack)->body);
        ui->adsrD ->value( ((const LV2_Atom_Float*)aDecay)->body);
        ui->adsrS ->value( ((const LV2_Atom_Float*)aSustain)->body);
        ui->adsrR ->value( ((const LV2_Atom_Float*)aRelease)->body);
      }
      else
      {
        //printf("UI NOT setting from DSP ReplyUiSampleState, %i, %i, %i, %i\n", aGain, aPan, aPitch, aStartPoint );
        ui->blankSampleState();
        if( pad != -1 )
          ui->pads[pad]->loaded = false;
        else
          printf("Fabla2 UI pad == -1");
      }
    }
    else
    {
      fprintf(stderr, "Unknown message type.\n");
    }
  }
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

