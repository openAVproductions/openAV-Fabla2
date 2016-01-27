
#ifndef OPENAV_AVTK_LV2_UI_HELPER_HXX
#define OPENAV_AVTK_LV2_UI_HELPER_HXX

#include "../shared.hxx"

#include "../picojson.hxx"

#include <iostream>
#include <fstream>
#include <sstream>

static LV2_Atom* writePadPlay( LV2_Atom_Forge* forge, URIs* uris, int bank, int pad, int layer, bool noteOn )
{
	LV2_Atom_Forge_Frame frame;
	LV2_Atom* set;
	if( noteOn )
		set = (LV2_Atom*)lv2_atom_forge_object( forge, &frame, 0, uris->fabla2_PadPlay);
	else
		set = (LV2_Atom*)lv2_atom_forge_object( forge, &frame, 0, uris->fabla2_PadStop);

	lv2_atom_forge_key(forge, uris->fabla2_bank);
	lv2_atom_forge_int(forge, bank );

	lv2_atom_forge_key(forge, uris->fabla2_pad);
	lv2_atom_forge_int(forge, pad );

	lv2_atom_forge_key(forge, uris->fabla2_layer);
	lv2_atom_forge_int(forge, layer );

	lv2_atom_forge_pop(forge, &frame);

	return set;
}

static LV2_Atom* writeSetFile( LV2_Atom_Forge* forge, URIs* uris, int bank, int pad, std::string file )
{
	LV2_Atom_Forge_Frame frame;
	LV2_Atom* set = (LV2_Atom*)lv2_atom_forge_object( forge, &frame, 0, uris->patch_Set);

	lv2_atom_forge_key(forge, uris->fabla2_bank);
	lv2_atom_forge_int(forge, bank );

	lv2_atom_forge_key(forge, uris->fabla2_pad);
	lv2_atom_forge_int(forge, pad );

	lv2_atom_forge_key(forge, uris->patch_property);
	lv2_atom_forge_urid(forge, uris->fabla2_sample);

	lv2_atom_forge_key(forge, uris->patch_value);
	lv2_atom_forge_path(forge, file.c_str(), strlen(file.c_str()) );

	lv2_atom_forge_pop(forge, &frame);

	return set;
}

bool loadConfigFile( std::string& defaultDir )
{
	std::stringstream configFile;
	configFile << getenv("HOME") << "/.config/openAV/fabla2/fabla2.prfs";

	std::ifstream ifs;
	ifs.open ( configFile.str().c_str(), std::ifstream::in);

	if( ifs.fail() ) {
		printf("Fabla2UI %s : File doesn't exist, aborting.\n", configFile.str().c_str() );
		return 0;
	}

	picojson::value v;
	ifs >> v;

	try {
		defaultDir = v.get("defaultDir").to_str();
		printf("%s\n", defaultDir.c_str() );
	} catch( ... ) {
		printf("Fabla2UI: Error parsing config file.\n" );
	}
	return true;
}

void writeConfigFile()
{
	std::stringstream f;
	f << getenv("HOME") << "/.config/openAV/fabla2/fabla2.prfs";

	std::ofstream outFile;
	outFile.open ( f.str().c_str() );
	//outFile << cJSON_Print( prfs );
	outFile.close();
}


#endif // OPENAV_AVTK_LV2_UI_HELPER_HXX
