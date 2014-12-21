
#include "lv2_work.hxx"

#include "dsp.hxx"
#include "shared.hxx"

#include "dsp/fabla2.hxx"
#include "dsp/library.hxx"
#include "dsp/bank.hxx"
#include "dsp/pad.hxx"
#include "dsp/sample.hxx"
#include "lv2_messaging.hxx"


static inline const LV2_Atom* read_set_file(FablaLV2* self,
                                            const URIs*     uris,
                                            const LV2_Atom_Object* obj)
{
  if (obj->body.otype != uris->patch_Set) {
    fprintf(stderr, "Ignoring unknown message type %d\n", obj->body.otype);
    return NULL;
  }
  
  /* Get property URI. */
  const LV2_Atom* property = NULL;
  lv2_atom_object_get(obj, uris->patch_property, &property, 0);
  if (!property) {
    fprintf(stderr, "Malformed set message has no body.\n");
    return NULL;
  } else if (property->type != uris->atom_URID) {
    fprintf(stderr, "Malformed set message has non-URID property.\n");
    return NULL;
  } else if (((const LV2_Atom_URID*)property)->body != uris->fabla2_sample) {
    fprintf(stderr, "Set message for unknown property.\n");
    printf("Unmapped: %s\n", self->unmap->unmap( self->unmap->handle, ((const LV2_Atom_URID*)property)->body ) );
    return NULL;
  }
  
  /* Get value. */
  const LV2_Atom* file_path = NULL;
  lv2_atom_object_get(obj, uris->patch_value, &file_path, 0);
  if (!file_path) {
    fprintf(stderr, "Malformed set message has no value.\n");
    return NULL;
  } else if (file_path->type != uris->atom_Path) {
    fprintf(stderr, "Set message value is not a Path.\n");
    return NULL;
  }
  
  return file_path;
}

// Offline thread, does not have to be RT safe
LV2_Worker_Status
fabla2_work( LV2_Handle                  instance,
             LV2_Worker_Respond_Function respond,
             LV2_Worker_Respond_Handle   handle,
             uint32_t                    size,
             const void*                 data)
{
  FablaLV2* self = (FablaLV2*)instance;
  
  const LV2_Atom* atom = (const LV2_Atom*)data;
  if( atom->type == self->uris.patch_Set )
  {
    
  }
  else
  {
    printf("Unmapped: %s\n", self->unmap->unmap( self->unmap->handle, atom->type ) );
    const LV2_Atom_Object* obj = (const LV2_Atom_Object*)data;
    
    const LV2_Atom* file_path = read_set_file( self, &self->uris, obj);
    
    if (!file_path) {
            return LV2_WORKER_ERR_UNKNOWN;
    }
    
    std::string file = (const char*)LV2_ATOM_BODY_CONST(file_path);
    Fabla2::Sample* s = new Fabla2::Sample( 0x0, 44100, "LoadedSample", file );
    printf("Work() - Sample() has %i frames\n", s->getFrames() );
    
    if ( s )
    {
      SampleLoadUnload msg;
      msg.atom.size = sizeof(SampleLoadUnload*);
      msg.atom.type = self->uris.fabla2_SampleLoad;
      
      msg.bank   = 0;
      msg.pad    = 0;
      msg.sample = s;
      
      // Loaded sample, send it to run() to be applied.
      respond( handle, sizeof(msg), &msg );
    }
  }
  
  return LV2_WORKER_SUCCESS;
}

// RT audio thread: *must* be RT safe
LV2_Worker_Status
fabla2_work_response(LV2_Handle  instance,
                    uint32_t    size,
                    const void* data)
{
  FablaLV2* self = (FablaLV2*)instance;
  
  const LV2_Atom* atom = (const LV2_Atom*)data;
  
  printf("Work:resonse() Got type : %s\n", self->unmap->unmap( self->unmap->handle, atom->type ) );
  
  if( atom->type == self->uris.fabla2_SampleLoad )
  {
    // we just recieved a newly loaded sample from the worker thread
    //  1. Swap new sample in
    //  2. send old sample to be free-d (using SampleLoadUnload* we just recieved)
    
    const SampleLoadUnload* msg = (const SampleLoadUnload*)data;
    printf( "Work Response, sample load/unload. Name: %s. Frames: %i \n", msg->sample->getName(), msg->sample->getFrames() );
    
    int bank = msg->bank;
    int pad  = msg->pad;
    
    if(bank < 0 || bank >=  4){ delete msg->sample; return LV2_WORKER_ERR_UNKNOWN; }
    if(pad  < 0 || pad  >= 16){ delete msg->sample; return LV2_WORKER_ERR_UNKNOWN; }
    
    self->dsp->getLibrary()->bank( bank )->pad( pad )->add( msg->sample );
  }
  
  /*
  if( false ) // currentlyLoadedSampleNeedsRemoval );
  {
    SampleLoadUnload msg = { { sizeof(SampleLoadUnload*), self->uris.fabla2_SampleUnload }, // Atom
                              0, // target bank
                              0, // target pad
                              0x0 }; // data pointer
  }
  */
  
  
  return LV2_WORKER_SUCCESS;
}
