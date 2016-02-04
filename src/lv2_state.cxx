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
#include <assert.h>

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
	assert( self );
	// Analyse new features, check for map path
	LV2_State_Map_Path*  map_path  = 0;
	LV2_State_Make_Path* make_path = 0;

	for (int i = 0; features[i]; ++i) {
		if (!strcmp(features[i]->URI, LV2_STATE__mapPath)) {
			map_path = (LV2_State_Map_Path*)features[i]->data;
		}
		if (!strcmp(features[i]->URI, LV2_STATE__makePath)) {
			make_path = (LV2_State_Make_Path*)features[i]->data;
		}
	}

	if ( !map_path ) {
		printf("Fabla2::Save() Error: map_path not available! SAVE DID NOT COMPLETE!\n" );
		return LV2_STATE_ERR_NO_FEATURE;
	}
	if ( !make_path ) {
		printf("Fabla2::Save() Error: make_path not available! SAVE DID NOT COMPLETE!\n" );
		return LV2_STATE_ERR_NO_FEATURE;
	}

	Library* library = self->dsp->getLibrary();

	picojson::object pjAll;

	for(int i = 0; i < 4; i++ ) {
		picojson::object pjBank;

		Bank* bank = library->bank( i );

		/// write Bank specific stuff
		// Is there any? Keep "bank" concept anyway, for loading banks seperate
		// from an entire session.

		pjBank["name"]      = picojson::value( "bank name test" );
		pjBank["auxbus1vol"]= picojson::value( self->dsp->auxBusVol[0] );
		pjBank["auxbus2vol"]= picojson::value( self->dsp->auxBusVol[1] );
		pjBank["auxbus3vol"]= picojson::value( self->dsp->auxBusVol[2] );
		pjBank["auxbus4vol"]= picojson::value( self->dsp->auxBusVol[3] );

		for(int p = 0; p < 16; p++ ) {
			picojson::object pjPad;

			Pad* pad = bank->pad( p );

			/// write Pad specific things
			pjPad["muteGroup"]     = picojson::value( (double)pad->muteGroup() );
			pjPad["offGroup"]      = picojson::value( (double)pad->offGroup() );
			pjPad["triggerMode"]   = picojson::value( (double)pad->triggerMode() );
			pjPad["switchMode"]    = picojson::value( (double)pad->switchSystem() );
			pjPad["volume"]        = picojson::value( (double)pad->volume );
			printf("pad volume saved as %d : %f\n", p, pad->volume);

			pjPad["auxbus1"]        = picojson::value( (double)pad->sends[0] );
			pjPad["auxbus2"]        = picojson::value( (double)pad->sends[1] );
			pjPad["auxbus3"]        = picojson::value( (double)pad->sends[2] );
			pjPad["auxbus4"]        = picojson::value( (double)pad->sends[3] );

			pjPad["nLayers"]    = picojson::value( (double)pad->nLayers() );

			for(int l = 0; l < pad->nLayers(); l++ ) {
				picojson::object pjLayer;
				Sample* s = pad->layer( l );

				/// write Layer / Sample specific things
				pjLayer["name"            ] = picojson::value( s->getName() );

				std::stringstream padName;
				padName << s->getName();

				char* savePath = make_path->path(make_path->handle, padName.str().c_str() );
				s->write( savePath );
				free( savePath );
				pjLayer["filename"        ] = picojson::value( padName.str().c_str() );

				pjLayer["gain"            ] = picojson::value( (double)s->gain );
				pjLayer["pan"             ] = picojson::value( (double)s->pan );

				pjLayer["pitch"           ] = picojson::value( (double)s->pitch );
				pjLayer["time"            ] = picojson::value( (double)s->time );

				pjLayer["startPoint"      ] = picojson::value( (double)s->startPoint );
				pjLayer["endPoint"        ] = picojson::value( (double)s->endPoint );

				pjLayer["filterType"      ] = picojson::value( (double)s->filterType );
				pjLayer["filterFrequency" ] = picojson::value( (double)s->filterFrequency );
				pjLayer["filterResonance" ] = picojson::value( (double)s->filterResonance );

				pjLayer["velLow"          ] = picojson::value( (double)s->velLow );
				pjLayer["velHigh"         ] = picojson::value( (double)s->velHigh );

				pjLayer["attack"          ] = picojson::value( (double)s->attack );
				pjLayer["decay"           ] = picojson::value( (double)s->decay );
				pjLayer["sustain"         ] = picojson::value( (double)s->sustain );
				pjLayer["release"         ] = picojson::value( (double)s->release );

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

	for (int i = 0; features[i]; ++i) {
		if (!strcmp(features[i]->URI, LV2_STATE__mapPath)) {
			map_path = (LV2_State_Map_Path*)features[i]->data;
		}
	}

	if ( !map_path ) {
		printf("Fabla2::restore() Error: map path not available! SAVE DID NOT COMPLETE!\n" );
		return LV2_STATE_ERR_NO_FEATURE;
	}

	Library* library = self->dsp->getLibrary();

	size_t   size = 0;
	uint32_t type = 0;
	uint32_t flgs = 0;

	// get the JSON description file
	const char* jsonSource = (const char*)retrieve(handle,
	                         self->uris.fabla2_StateStringJSON, &size, &type, &flgs );

	if( jsonSource ) {
		//printf("\n\n%s\n\n", jsonSource );

		char* json = (char*)malloc(strlen(jsonSource)+1);
		strcpy(json, jsonSource);

		picojson::value pjAll;
		string err = picojson::parse( pjAll, json, json + strlen(json));
		if( err.size() > 0 ) {
			printf( "PicoJSON Parser Error! %s\n", err.c_str() );
			return LV2_STATE_ERR_UNKNOWN;
		}

		//try
		{
			for(int b = 0; b < 4; b++ ) {
				Bank* bank = library->bank( b );

				//printf("Bank %i\n", b );

				std::stringstream bankStr;
				bankStr << "bank_" << char('A' + b);

				picojson::value pjBanks = pjAll.get( bankStr.str() );

				bool pjBankOk = pjBanks.is<picojson::object>();
				if( !pjBankOk ) {
					printf( "Fabla2 : Lv2 State Restore() : PjBanks is object not valid.\
							Corrupt save file? Check letter X in \"bank_X\" of the save file.\n" );
					continue;
				}

				// set auxbus values - TODO split values to each bank and load/restore
				if( pjBanks.get("auxbus1vol").get<double>() )
					self->dsp->auxBusVol[0] = (float)pjBanks.get("auxbus1vol").get<double>();
				if( pjBanks.get("auxbus2vol").get<double>() )
					self->dsp->auxBusVol[1] = (float)pjBanks.get("auxbus2vol").get<double>();
				if( pjBanks.get("auxbus3vol").get<double>() )
					self->dsp->auxBusVol[2] = (float)pjBanks.get("auxbus3vol").get<double>();
				if( pjBanks.get("auxbus4vol").get<double>() )
					self->dsp->auxBusVol[3] = (float)pjBanks.get("auxbus4vol").get<double>();

				for(int p = 0; p < 16; p ++) {
					Pad* pad = bank->pad( p );
					//printf("Pad %i\n", p );

					// kick all state out
					pad->clearAllSamples();

					std::stringstream padStr;
					padStr << "pad_" << p;
					picojson::value pjPad = pjBanks.get( padStr.str() );

					bool pjPadOk = pjPad.is<picojson::object>();
					if( !pjPadOk ) {
						printf( "Fabla2 : Lv2 State Restore() : pjPad is object not valid.\
								Corrupt save file? Check number X in \"pad_X\" of the save file.\n" );
						continue;
					}

					if( pjPad.get("muteGroup").is<double>() )
						pad->muteGroup   ( (int)pjPad.get("muteGroup").get<double>() );

					if( pjPad.get("offGroup").is<double>() )
						pad->offGroup    ( (int)pjPad.get("offGroup" ).get<double>() );

					if( pjPad.get("triggerMode").is<double>() )
						pad->triggerMode ( (Fabla2::Pad::TRIGGER_MODE)pjPad.get("triggerMode").get<double>() );

					if( pjPad.get("switchMode").is<double>() )
						pad->switchSystem( (Fabla2::Pad::SAMPLE_SWITCH_SYSTEM)pjPad.get("switchMode" ).get<double>() );

					if( pjPad.get("volume").is<double>() ) {
						pad->volume = pjPad.get("volume").get<double>();
					}
					else {
						printf("pad volume not restored on %d\n", p);
					}

					if( pjPad.get("auxbus1").is<double>() )
						pad->sends[0] = pjPad.get("auxbus1").get<double>();
					if( pjPad.get("auxbus2").is<double>() )
						pad->sends[1] = pjPad.get("auxbus2").get<double>();
					if( pjPad.get("auxbus3").is<double>() )
						pad->sends[2] = pjPad.get("auxbus3").get<double>();
					if( pjPad.get("auxbus4").is<double>() )
						pad->sends[3] = pjPad.get("auxbus4").get<double>();

					int nLayers = 0;
					if( pjPad.get("nLayers").is<double>() )
						nLayers= (int)pjPad.get("nLayers").get<double>();

					for(int i = 0; i < nLayers; i++) {
						std::stringstream layerStr;
						layerStr << "layer_" << i;

						//printf("Sample %i, %s\n", i, layerStr.str().c_str() );

						picojson::value pjLayer = pjPad.get( layerStr.str() );

						std::string filename;
						if( pjLayer.get("filename").is<std::string>() ) {
							filename = pjLayer.get("filename").get<std::string>();
						} else {
							printf("Fabla2UI: Pad %i : Sample %i : no filename in save file, skipping sample.\n", p, i );
							continue;
						}

						std::string name;
						if( !pjLayer.get("name").is<std::string>() ) {
							std::stringstream padName;
							padName << "pad" << p << "_layer" << i << ".wav";
							name = padName.str();
						} else {
							name = pjLayer.get("name").get<std::string>();
						}

						// map the short filename to the full path
						std::string path = map_path->absolute_path(map_path->handle, filename.c_str() );

						// strip the file:// from the start
						path = path.substr( 7 );

						//printf("Loading %s\n", path.c_str() );
						Sample* s = new Sample( self->dsp, self->dsp->sr, name.c_str(), path );
						if( s->getFrames() <= 0 ) {
							delete s;
							// error loading this sample!
							printf("Error Loading %s : Pad %i : Sample %i : Frames == 0, ignoring this sample!\n", path.c_str(), p, i );
							continue;
						} else {
							// write directly to Sample*
							if( pjLayer.get("gain").is<double>() )
								s->gain            = (float)pjLayer.get("gain").get<double>();
							if( pjLayer.get("pan").is<double>() )
								s->pan             = (float)pjLayer.get("pan").get<double>();

							if( pjLayer.get("pitch").is<double>() )
								s->pitch           = (float)pjLayer.get("pitch").get<double>();
							if( pjLayer.get("time").is<double>() )
								s->time            = (float)pjLayer.get("time").get<double>();

							if( pjLayer.get("startPoint").is<double>() )
								s->startPoint      = (float)pjLayer.get("startPoint").get<double>();
							if( pjLayer.get("endPoint").is<double>() )
								s->endPoint        = (float)pjLayer.get("endPoint").get<double>();

							if( pjLayer.get("filterType").is<double>() )
								s->filterType      = (float)pjLayer.get("filterType").get<double>();
							if( pjLayer.get("filterFrequency").is<double>() )
								s->filterFrequency = (float)pjLayer.get("filterFrequency").get<double>();
							if( pjLayer.get("filterResonance").is<double>() )
								s->filterResonance = (float)pjLayer.get("filterResonance").get<double>();

							if( pjLayer.get("velLow").is<double>() )
								s->velLow          = pjLayer.get("velLow").get<double>();
							if( pjLayer.get("velHigh").is<double>() )
								s->velHigh         = pjLayer.get("velHigh").get<double>();

							if( pjLayer.get("attack").is<double>() )
								s->attack          = (int)pjLayer.get("attack").get<double>();
							if( pjLayer.get("decay").is<double>() )
								s->decay           = (int)pjLayer.get("decay").get<double>();
							if( pjLayer.get("sustain").is<double>() )
								s->sustain         = (int)pjLayer.get("sustain").get<double>();
							if( pjLayer.get("release").is<double>() )
								s->release         = (int)pjLayer.get("release").get<double>();

							// add the sample to the Pad
							pad->add( s );
						}
					}
				}

			} // banks

		}
		//catch( std::exception& e )
		{
			//printf("Fabla2 : LV2 State restore() Runtime exception thrown. Please send the preset you attempted to load to harryhaaren@gmail.com so I can fix a bug in Fabla2! Thanks, -Harry. PicoJSON says: %s\n", e.what() );
		}

	} else {
		printf("Fabla2:State() Warning, no JSON : not loading preset.\n" );
	}

	for(int i = 0; i < 16; i++)
		self->dsp->padRefreshLayers(0,i);

	return LV2_STATE_SUCCESS;
}

