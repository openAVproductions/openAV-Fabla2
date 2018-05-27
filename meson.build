project('openav_fabla2', ['c','cpp'])

conf_data = configuration_data()
conf_data.set('version', '2.0.1')

add_project_arguments('-Wno-unused-variable', language : 'cpp')
add_project_arguments('-Wno-reorder', language : 'cpp')
add_project_arguments('-Wno-sign-compare', language : 'cpp')
add_project_arguments('-fPIC', language : 'cpp')
add_project_arguments('-shared', language : 'cpp')

subdir('src')

dep_names = [
  'threads',
  'lv2',
  'sndfile',
  'samplerate',
  'openav_ctlra',
  'openav_caira'
]
deps = []

foreach dep : dep_names
  deps += dependency(dep)
endforeach

#install_subdir('artyfx.lv2', install_dir : 'lib/lv2/') 

fabla2_dsp = library('fabla2',
    [fabla2_dsp_src],
    install : true,
    name_prefix : '', # backwards compatible with old build system
    install_dir : 'lib/lv2/fabla2.lv2/',
    dependencies: deps)


