
#include "lv2_work.hxx"

#include "dsp.hxx"
#include "shared.hxx"

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

LV2_Worker_Status
fabla2_work_response(LV2_Handle  instance,
                    uint32_t    size,
                    const void* data)
{
  printf("Work_response() %s\n", data);
  
  return LV2_WORKER_SUCCESS;
}
