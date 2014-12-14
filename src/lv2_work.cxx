
#include "lv2_work.hxx"

#include "dsp.hxx"
#include "shared.hxx"

#include "dsp/sample.hxx"
#include "lv2_messaging.hxx"

// Offline thread, does not have to be RT safe
LV2_Worker_Status
fabla2_work( LV2_Handle                  instance,
             LV2_Worker_Respond_Function respond,
             LV2_Worker_Respond_Handle   handle,
             uint32_t                    size,
             const void*                 data)
{
  
  printf("Work() - RT safe printing going on!\n");
  
  if( true )
    respond(handle, 0, 0);
  
  return LV2_WORKER_SUCCESS;
}

// RT audio thread: *must* be RT safe
LV2_Worker_Status
fabla2_work_response(LV2_Handle  instance,
                    uint32_t    size,
                    const void* data)
{
  FablaLV2* self = (FablaLV2*)instance;
  
  if( false ) // currentlyLoadedSampleNeedsRemoval );
  {
    SampleLoadUnload msg = { { sizeof(Fabla2::Sample*), self->uris.fabla2_SampleUnload }, // Atom
                              false, // bool load
                              0x0 }; // data pointer
  }
  
  
  
  return LV2_WORKER_SUCCESS;
}
