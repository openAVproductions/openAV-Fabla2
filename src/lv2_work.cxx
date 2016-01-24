
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
        const LV2_Atom_Object* obj,
        int& bank,
        int& pad )
{
    if (obj->body.otype != uris->patch_Set) {
        lv2_log_note(&self->logger,"Ignoring unknown message type %d\n", obj->body.otype);
        return NULL;
    }

    // get data from atom
    const LV2_Atom_Int* b = 0;
    const LV2_Atom_Int* p = 0;
    const LV2_Atom* property = NULL;

    lv2_atom_object_get(obj,
                        uris->patch_property, &property,
                        uris->fabla2_bank   , &b,
                        uris->fabla2_pad    , &p,
                        0);

    //printf(" %i, %i, %i\n", property, b, p );

    if( property && b && p ) {
        lv2_log_note(&self->logger,"Work() : Getting Bank / Pad data from Atom.\n");

        bank = ((const LV2_Atom_Int*)b)->body;
        printf("got bank %i\n", bank);
        pad = ((const LV2_Atom_Int*)p)->body;
        printf("got pad %i\n", pad );

        // get file path
        const LV2_Atom* file_path = NULL;
        lv2_atom_object_get(obj, uris->patch_value, &file_path, 0);
        if (!file_path) {
            lv2_log_note(&self->logger,"Malformed set message has no value.\n");
            return 0;
        } else if (file_path->type != uris->atom_Path) {
            lv2_log_note(&self->logger,"Set message value is not a Path.\n");
            return 0;
        }
        return file_path;
    } else {
        lv2_log_error(&self->logger,"Fabla2: Work() sample-load: error parsting Atom: abort.\n");
        printf("Unmapped: %s\n", self->unmap->unmap( self->unmap->handle, ((const LV2_Atom_URID*)property)->body ) );
        return 0;
    }

    return 0;
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
    if( atom->type == self->uris.patch_Set ) {

    } else {
        //printf("Unmapped: %s\n", self->unmap->unmap( self->unmap->handle, atom->type ) );
        const LV2_Atom_Object* obj = (const LV2_Atom_Object*)data;

        int bank = 0;
        int pad  = 0;
        const LV2_Atom* file_path = read_set_file( self, &self->uris, obj, bank, pad );

        if (!file_path || bank == -1 || pad == -1 ) {
            lv2_log_note(&self->logger,"Fabla2: Work() !file_path || !bank || !pad: aborting sample load.\n" );
            return LV2_WORKER_ERR_UNKNOWN;
        }

        std::string file = (const char*)LV2_ATOM_BODY_CONST(file_path);
        Fabla2::Sample* s = new Fabla2::Sample( self->dsp, 44100, "LoadedSample", file );

        lv2_log_note(&self->logger,"Work() - B: %i, P %i: Loading %s: Sample() has %ld frames\n",
                     bank, pad, file.c_str(), s->getFrames() );

        if ( s && s->getFrames() ) {
            SampleLoadUnload msg;
            msg.atom.size = sizeof(SampleLoadUnload*);
            msg.atom.type = self->uris.fabla2_SampleLoad;

            msg.bank   = bank;
            msg.pad    = pad;
            msg.sample = s;

            // Loaded sample, send it to run() to be applied.
            respond( handle, sizeof(msg), &msg );
        } else {
            lv2_log_error(&self->logger,"Work() - ERROR Loading %s: Sample has 0 frames? Check file path/name!\n", file.c_str());
            delete s;
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

    //printf("Work:resonse() Got type : %s\n", self->unmap->unmap( self->unmap->handle, atom->type ) );

    if( atom->type == self->uris.fabla2_SampleLoad ) {
        // we just recieved a newly loaded sample from the worker thread
        //  1. Swap new sample in
        //  2. send old sample to be free-d (using SampleLoadUnload* we just recieved)

        const SampleLoadUnload* msg = (const SampleLoadUnload*)data;
        //printf( "Work Response, sample load/unload. Name: %s. Frames: %i \n", msg->sample->getName(), msg->sample->getFrames() );

        int bank = msg->bank;
        int pad  = msg->pad;

        if(bank < 0 || bank >=  4) {
            delete msg->sample;
            return LV2_WORKER_ERR_UNKNOWN;
        }
        if(pad  < 0 || pad  >= 16) {
            delete msg->sample;
            return LV2_WORKER_ERR_UNKNOWN;
        }

        // add() of pad writes LV2 update: we don't have layer information here yet.
        self->dsp->getLibrary()->bank( bank )->pad( pad )->add( msg->sample );
    }


    return LV2_WORKER_SUCCESS;
}
