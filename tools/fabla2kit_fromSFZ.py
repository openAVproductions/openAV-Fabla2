
# This is a very simple text file parser:
# -which will auto-generate .html files
# -from input HTML files
# -and sort the pages into directories
import os, sys, time
import json

pages = []


def processFile( filenameIn ):
  # open the template & input file
  sfz = open(filenameIn)
  
  jsonData = '''{\n "bank_A":{"name":"AvLinux Red Zepplin 5pc",'''
  
  layer = ""
  
  boolFirstGroup = 1
  boolFirstRegion = 1
  
  groupData = ""
  
  for l in sfz:
    if l.startswith("//"):
      # Comment: ignore
      pass
    
    # handle <group> key=56 pan=-63 loop_mode=one_shot
    if l.startswith("<group>"):
      
      if boolFirstGroup != 1:
        groupData += ''',"nLayers": ''' + str(layer)
        groupData += '},'
      
      
      layer = 0
      boolFirstRegion = 1
      
      
      #print( "Processsing: " + l )
      items = l.split(' ')
      
      key = -1
      muteGroup = -1
      offGroup = -1
      
      for i in items:
        #print( i )
        if i.startswith( 'key=' ):
          key = i[4:]
          pass
        
        if i.startswith( 'pan=' ):
          #print( i[4:] )
          pass
        
        if i.startswith( 'loop_mode=' ):
          #print( i[10:] )
          pass
        
        if i.startswith( 'group=' ):
          muteGroup = i[6:]
          pass
        
        if i.startswith( 'offby=' ):
          offGroup = i[6:]
          pass
      
      if int(key)-36 < 0:
        print("WARNING: input file: " + filenameIn + "  Key out of range!" + str(items) )
        time.sleep(3)
      
      
      # "pad_0": {
      groupData += '''\n\n "pad_''' + str( int(key)-36 ) + '''" : \n''' + '{'
      
      groupData += '''"triggerMode": 1,\n'''
      groupData += '''"volume": 0.75,\n'''
      groupData += '''"switchMode": 2,\n'''
      
      if int(muteGroup) != -1:
        groupData += '''"muteGroup": '''+ muteGroup +''',\n'''
      
      if int(offGroup) != -1:
        groupData += '''"offGroup": '''+ offGroup +''',\n'''
      
      
      boolFirstGroup = 0
      
      #print(groupData)
      
      
      
    
    if l.startswith("<region>"):
      if boolFirstRegion != 1:
        groupData += ','
      else:
        boolFirstRegion = 0
      
      if key == 49:
        print( l )
      
      items = l.split(' ')
      filename = ""
      lowVel   = 1
      highVel  = 127
      for i in items:
        if i.startswith( 'lovel=' ):
          lowVel = i[6:]
        if i.startswith( 'hivel=' ):
          highVel = i[6:]
        if i.startswith( 'sample=' ):
          filename = i[7:].rstrip()
      
      #print( "Low: " + str(lowVel) + "  High: " + str(highVel) + "  File: "+ filename )
      
      # "velHigh": 127,
      # "velLow": 0
      # "filename": "pad0_layer0.wav",
      groupData += '''"layer_''' + str(layer) + '''":  '''
      groupData += '{'
      groupData += '''"filename": "''' + filename + '''",'''
      groupData += '''"velHigh": ''' + str(highVel) + ''','''
      groupData += '''"velLow": ''' + str(lowVel) + ''''''
      groupData += '}\n'
      
      layer = layer + 1
  
  
  # No new <group> to trigger writing Nlayers to this pad, so we do it here
  groupData += ''',"nLayers": ''' + str(layer)
  
  groupData += '}' #pad
  
  jsonData += groupData
  
  jsonData += '}' # bank
  
  jsonData += '}' # json closer
  
  #print( "FINAL:\n" + jsonData )
  
  outfile = open( filenameIn + "_fabla2.json", 'w' );
  outfile.write( jsonData )
  
  

processFile( 'Red_Zeppelin_5pc.sfz' )
#processFile( 'Red_Zeppelin_4pc.sfz' )
#processFile( 'Black_Pearl_4pc.sfz' )
#processFile( 'Black_Pearl_4pc_Alt.sfz' )
