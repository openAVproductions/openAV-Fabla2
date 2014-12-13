
#ifndef OPENAV_AVTK_LV2_UI_WRITER_HXX
#define OPENAV_AVTK_LV2_UI_WRITER_HXX

#include "../shared.hxx"

static LV2_Atom* writeSetFile( LV2_Atom_Forge* forge, URIs* uris, std::string file )
{
  LV2_Atom_Forge_Frame frame;
  LV2_Atom* set = (LV2_Atom*)lv2_atom_forge_object(
    forge, &frame, 0, uris->patch_Set);

  lv2_atom_forge_key(forge, uris->patch_property);
  lv2_atom_forge_urid(forge, uris->fabla2_sample);
  lv2_atom_forge_key(forge, uris->patch_value);
  lv2_atom_forge_path(forge, file.c_str(), strlen(file.c_str()) );

  lv2_atom_forge_pop(forge, &frame);
  
  return set;
}


#endif // OPENAV_AVTK_LV2_UI_WRITER_HXX
