#!/usr/bin/env python3
# This is a script to convert the avlinux drumkits into fabla2 drumkits.
# Get them from http://bandshed.net/avldrumkits/index.html

import argparse
import json


def processFile(filename_in, filename_out):
    # open the template & input file
    with open(filename_in, 'r') as sfz:
        jsonData = '''{\n "bank_A":{"name":"AvLinux Red Zepplin 5pc",'''

        layer = ""

        boolFirstGroup = 1
        boolFirstRegion = 1
        groupData = ""

        # is in <group> in sfz, but "layer" in Fabla,
        # hence its declared here so we can
        # transport it between the <group> and <region> parts.
        pan = 0

        for l in sfz:
            if l.startswith("//"):
                # Comment: ignore
                pass

            # handle <group> key=56 pan=-63 loop_mode=one_shot
            if l.startswith("<group>"):

                if boolFirstGroup != 1:
                    groupData += ''',"nLayers": ''' + str(layer)
                    groupData += '},'

                # reset default group options
                pan = 0
                layer = 0
                boolFirstRegion = 1

                # print( "Processsing: " + l )
                items = l.split(' ')

                key = -1
                muteGroup = -1
                offGroup = -1

                for i in items:
                    # print( i )
                    if i.startswith('key='):
                        key = i[4:]
                        pass

                    if i.startswith('pan='):
                        pan = i[4:]
                        pass

                    if i.startswith('loop_mode='):
                        # print( i[10:] )
                        pass

                    if i.startswith('group='):
                        muteGroup = i[6:]
                        pass

                    if i.startswith('offby='):
                        offGroup = i[6:]
                        pass

                if int(key)-36 < 0:
                    print("WARNING: input file: " + filename_in +
                          "  Key out of range!" + str(items))

                # "pad_0": {
                groupData += ('''\n\n "pad_''' + str(int(key)-36) +
                              '''" : \n''' + '{')

                groupData += '''"triggerMode": 1,\n'''
                groupData += '''"volume": 0.75,\n'''
                groupData += '''"switchMode": 2,\n'''

                if int(muteGroup) != -1:
                    groupData += '''"muteGroup": ''' + muteGroup + ''',\n'''

                if int(offGroup) != -1:
                    groupData += '''"offGroup": ''' + offGroup + ''',\n'''

                boolFirstGroup = 0

                # print(groupData)

            if l.startswith("<region>"):
                if boolFirstRegion != 1:
                    groupData += ','
                else:
                    boolFirstRegion = 0

                if key == 49:
                    print(l)

                items = l.split(' ')
                filename = ""
                lowVel = 0.0
                highVel = 1.0
                for i in items:
                    if i.startswith('lovel='):
                        lowVel = int(i[6:]) / 127.
                    if i.startswith('hivel='):
                        highVel = int(i[6:]) / 127.
                    if i.startswith('sample='):
                        filename = i[7:].rstrip()

                # print("Low: " + str(lowVel) + "  High: " +
                #       str(highVel) + "  File: "+ filename)

                # "velHigh": 127,
                # "velLow": 0
                # "filename": "pad0_layer0.wav",
                groupData += '''"layer_''' + str(layer) + '''":  '''
                groupData += '{'
                groupData += '''"filename": "''' + filename + '''",'''
                groupData += '''"velHigh": ''' + str(highVel) + ''','''
                groupData += '''"velLow": ''' + str(lowVel) + ''','''
                # -100,0,100 to 0.0, 0.5, 1.0 mapping
                groupData += '''"pan": ''' + str(int(pan)/200.+0.5)
                groupData += '}\n'

                layer = layer + 1

        # No new <group> to trigger writing Nlayers to this pad,
        # so we do it here
        groupData += ''',"nLayers": ''' + str(layer)

        groupData += '}'  # pad

        jsonData += groupData

        jsonData += '}'  # bank

        jsonData += '}'  # json closer

        # print( "FINAL:\n" + jsonData )
        with open(filename_out, 'w') as out:
            out.write(jsonData)


def parseArgs():
    parser = argparse.ArgumentParser(description='Convert avlinux drumkits ' +
                                                 'into fabla2 drumkits.')
    parser.add_argument('-i', dest='input', metavar='IN',
                        help='path to the input sfz', required=True)
    parser.add_argument('-o', dest='output', metavar='OUT', nargs='?',
                        help='path the fabla drumkit should be written to, ' +
                             'default is "<IN>_fabla2.json"')
    args = parser.parse_args()
    if not args.output:
        args.output = '{0}_fabla2.json'.format(args.input)
    return args


def main():
    args = parseArgs()
    processFile(args.input, args.output)

if __name__ == "__main__":
    main()
