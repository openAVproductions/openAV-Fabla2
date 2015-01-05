/*
 * Author: Harry van Haaren 2014
 *         harryhaaren@gmail.com
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 */

#include "lv2_state.hxx"

#include <string>
#include <sstream>

#include "dsp.hxx"
#include "picojson.hxx"

#include "dsp/fabla2.hxx"
#include "dsp/pad.hxx"
#include "dsp/bank.hxx"
#include "dsp/sample.hxx"
#include "dsp/library.hxx"

using namespace Fabla2;

using std::string;

LV2_State_Status
fabla2_save(LV2_Handle                 instance,
        LV2_State_Store_Function   store,
        LV2_State_Handle           handle,
        uint32_t                   flags,
        const LV2_Feature *const * features)
{
  FablaLV2* self = (FablaLV2*)instance;
  
  // Analyse new features, check for map path
  LV2_State_Map_Path*  map_path  = 0;
  LV2_State_Make_Path* make_path = 0;
  
  for (int i = 0; features[i]; ++i)
  {
    if (!strcmp(features[i]->URI, LV2_STATE__mapPath))
    {
      map_path = (LV2_State_Map_Path*)features[i]->data;
    }
    if (!strcmp(features[i]->URI, LV2_STATE__makePath))
    {
      make_path = (LV2_State_Make_Path*)features[i]->data;
    }
  }
  
  if ( !map_path )
  {
    printf("Fabla2::Save() Error: map path not available! SAVE DID NOT COMPLETE!\n" );
    return LV2_STATE_ERR_NO_FEATURE;
  }
  if ( !map_path )
  {
    printf("Fabla2::Save() Error: Make path not available! SAVE DID NOT COMPLETE!\n" );
    return LV2_STATE_ERR_NO_FEATURE;
  }
  
  Library* library = self->dsp->getLibrary();
  
  picojson::object pjAll;
  
  for(int i = 0; i < 4; i++ )
  {
    picojson::object pjBank;
    
    Bank* bank = library->bank( i );
    
    /// write Bank specific stuff
    // Is there any? Keep "bank" concept anyway, for loading banks seperate
    // from an entire session.
    
    pjBank["name"]     = picojson::value( "padTestName" );
    
    for(int p = 0; p < 16; p++ )
    {
      picojson::object pjPad;
      
      Pad* pad = bank->pad( p );
      
      /// write Pad specific things
      //printf("mute group: %lf\n", pad->muteGroup() );
      pjPad["muteGroup"]     = picojson::value( (double)pad->muteGroup() );
      pjPad["triggerMode"]   = picojson::value( (double)pad->triggerMode() );
      pjPad["switchMode"]    = picojson::value( (double)pad->switchSystem() );
      
      pjPad["nLayers"]    = picojson::value( (double)pad->nLayers() );
      
      for(int l = 0; l < pad->nLayers(); l++ )
      {
        picojson::object pjLayer;
        Sample* s = pad->layer( i );
        
        /// write Layer / Sample specific things
        pjLayer["name"            ] = picojson::value( s->getName() );
        
        // save Sample audio data as <pad_num>_<layer_num>.wav
        std::stringstream padName;
        padName << "pad" << p << "_layer" << l << ".wav";
        char* savePath = make_path->path(make_path->handle, padName.str().c_str() );
        s->write( savePath );
        free( savePath );
        // write the portable <padX_layerY.wav> form to the JSON
        pjLayer["filename"        ] = picojson::value( padName.str().c_str() );
        
        pjLayer["gain"            ] = picojson::value( (double)s->gain );
        pjLayer["pan"             ] = picojson::value( (double)s->pan );
        pjLayer["pitch"           ] = picojson::value( (double)s->pitch );
        pjLayer["startPoint"      ] = picojson::value( (double)s->startPoint );
        
        pjLayer["filterType"      ] = picojson::value( (double)s->filterType );
        pjLayer["filterFrequency" ] = picojson::value( (double)s->filterFrequency );
        pjLayer["filterResonance" ] = picojson::value( (double)s->filterResonance );
        
        pjLayer["velLow"          ] = picojson::value( (double)s->velLow );
        pjLayer["velHigh"         ] = picojson::value( (double)s->velHigh );
        
        std::stringstream layer;
        layer << "layer_" << l;
        pjPad[ layer.str() ] = picojson::value( pjLayer );
      } // Layers
      
      // finally add the current bank to the whole JSON
      std::stringstream padStream;
      padStream << "pad_" << p;
      pjBank[ padStream.str() ] = picojson::value( pjPad );
    } // pads
    
    
    // finally add the current bank to the whole JSON
    std::stringstream bankStr;
    bankStr << "bank_" << char('A' + i); // hack to bank letters
    pjAll[ bankStr.str() ] = picojson::value( pjBank );
  } // banks
  
  // serialize the whole JSON string
  string str = picojson::value( pjAll ).serialize();
  printf( "Lv2:State content = %s\n" ,  str.c_str() );
  
  store(handle,
        self->uris.fabla2_StateStringJSON,
        str.c_str(),
        strlen( str.c_str() ) + 1,  // Careful!  Need space for terminator
        self->uris.atom_String,
        LV2_STATE_IS_POD | LV2_STATE_IS_PORTABLE);
  
  return LV2_STATE_SUCCESS;
}


LV2_State_Status
fabla2_restore(LV2_Handle                  instance,
               LV2_State_Retrieve_Function retrieve,
               LV2_State_Handle            handle,
               uint32_t                    flags,
               const LV2_Feature *const *  features)
{
  FablaLV2* self = (FablaLV2*)instance;
  
  // Analyse new features, check for map path
  LV2_State_Map_Path*  map_path  = 0;
  LV2_State_Make_Path* make_path = 0;
  
  for (int i = 0; features[i]; ++i)
  {
    if (!strcmp(features[i]->URI, LV2_STATE__mapPath))
    {
      map_path = (LV2_State_Map_Path*)features[i]->data;
    }
    if (!strcmp(features[i]->URI, LV2_STATE__makePath))
    {
      make_path = (LV2_State_Make_Path*)features[i]->data;
    }
  }
  
  if ( !map_path )
  {
    printf("Fabla2::restore() Error: map path not available! SAVE DID NOT COMPLETE!\n" );
    return LV2_STATE_ERR_NO_FEATURE;
  }
  if ( !map_path )
  {
    printf("Fabla2::restore() Error: Make path not available! SAVE DID NOT COMPLETE!\n" );
    return LV2_STATE_ERR_NO_FEATURE;
  }
  
  Library* library = self->dsp->getLibrary();
  
  size_t   size = 0;
  uint32_t type = 0;
  uint32_t flgs = 0;
  
  // get the JSON description file
  const char* jsonSource = (const char*)retrieve(handle, self->uris.fabla2_StateStringJSON, &size, &type, &flgs );
  
  if( jsonSource )
  {
    printf("\n\n%s\n\n", jsonSource );
    
    char* json = (char*)malloc(strlen(jsonSource)+1);
    strcpy(json, jsonSource);
    
    picojson::value pjAll;
    string err = picojson::parse( pjAll, json, json + strlen(json));
    if( err.size() > 0 )
    {
      printf( "PicoJSON Parser Error! %s\n", err.c_str() );
      return LV2_STATE_ERR_UNKNOWN;
    }
    
    //try
    {
      for(int b = 0; b < 4; b++ )
      {
        Bank* bank = library->bank( b );
        
        printf("Bank %i\n", b );
        
        std::stringstream bankStr;
        bankStr << "bank_" << char('A' + b);
        picojson::value pjBanks = pjAll.get( bankStr.str() );
        
        for(int p = 0; p < 16; p ++)
        {
          Pad* pad = bank->pad( p );
          printf("Pad %i\n", p );
          
          // kick all state out
          pad->clearAllSamples();
          
          std::stringstream padStr;
          padStr << "pad_" << p;
          picojson::value pjPad = pjBanks.get( padStr.str() );
          
          int muteGroup     = (int)pjPad.get("muteGroup").get<double>();
          int triggerMode   = (int)pjPad.get("triggerMode").get<double>();
          int nLayers       = (int)pjPad.get("nLayers").get<double>();
          
          for(int i = 0; i < nLayers; i++)
          {
            printf("Sample %i\n", i );
            
            std::stringstream layerStr;
            layerStr << "layer_" << i;
            picojson::value pjLayer = pjPad.get( layerStr.str() );
            
            // create new audio sample from the file on disk here
            // save Sample audio data as <pad_num>_<layer_num>.wav
            std::stringstream padName;
            padName << "pad" << p << "_layer" << i << ".wav";
            /*
            char* savePath = make_path->path(make_path->handle, padName.str().c_str() );
            s->write( savePath );
            free( savePath );
            */
            
            std::string filename = pjLayer.get("filename").get<std::string>();
            // map the short filename to the full path
            
            std::string path = map_path->absolute_path(map_path->handle, filename.c_str() );
            // strip the file:// from the start
            path = path.substr( 7 );
            printf("Loading %s\n", path.c_str() );
            Sample* s = new Sample( self->dsp, self->dsp->sr, padName.str().c_str(), path );
            if( s->getFrames() <= 0 )
            {
              delete s;
              // error loading this sample!
              printf("Pad %i : Sample %i : Frames == 0, ignoring this sample!\n", p, i );
              continue;
            }
            else
            {
              // write directly to Sample*
              s->gain            = (float)pjLayer.get("gain").get<double>();
              printf("pitch\n");
              s->pitch           = (float)pjLayer.get("pitch").get<double>();
              printf("pan\n");
              s->pan             = (float)pjLayer.get("pan").get<double>();
              printf("startPoint\n");
              s->startPoint      = (float)pjLayer.get("startPoint").get<double>();
              printf("filterType\n");
              s->filterType      = (float)pjLayer.get("filterType").get<double>();
              printf("filterFrequency\n");
              s->filterFrequency = (float)pjLayer.get("filterFrequency").get<double>();
              printf("filterResonance\n");
              s->filterResonance = (float)pjLayer.get("filterResonance").get<double>();
              
              printf("velLow\n");
              s->velLow          = (int)pjLayer.get("velLow").get<double>();
              printf("velHigh\n");
              s->velHigh         = (int)pjLayer.get("velHigh").get<double>();
              printf("done\n");
              
              // add the sample to the Pad
              pad->add( s );
            }
          }
        }
        
      } // banks
    
    }
    //catch( std::exception& e )
    {
      //printf("PicoJSON : Runtime exception thrown! %s\n", e.what() );
    }
    
  }
  else
  {
    printf("Fabla2:State load ERROR, no JSON file available: aborting preset load!\n" );
  }
  
  return LV2_STATE_SUCCESS;
}

