#!/usr/bin/env python3
# This is a script to convert the avlinux drumkits into fabla2 drumkits.
# Get them from http://bandshed.net/avldrumkits/index.html

import argparse
import json


def processFile(filename_in, filename_out):
    # open the template & input file
    with open(filename_in, 'r') as sfz:
        jsonData = {'bank_A': {'name': 'AvLinux Red Zepplin 5pc'}}

        boolFirstGroup = True
        currentPad = None
        layer_count = 0

        # is in <group> in sfz, but "layer" in Fabla,
        # hence its declared here so we can
        # transport it between the <group> and <region> parts.
        pan = 0
        for l in sfz:
            if l.startswith('//'):
                # Comment: ignore
                pass

            # handle <group> key=56 pan=-63 loop_mode=one_shot
            if l.startswith("<group>"):

                if not boolFirstGroup:
                    # we have seen all layers of a group
                    jsonData['bank_A'][currentPad]['nLayers'] = layer_count

                # reset default group options
                pan = 0
                layer_count = 0

                items = l.split(' ')

                key = -1
                muteGroup = -1
                offGroup = -1

                for i in items:
                    if i.startswith('key='):
                        key = i[4:]
                        pass

                    if i.startswith('pan='):
                        pan = i[4:]
                        pass

                    if i.startswith('loop_mode='):
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

                pad = {"triggerMode": 1, "volume": 0.75, "switchMode": 2}

                if int(muteGroup) != -1:
                    pad['muteGroup'] = int(muteGroup)

                if int(offGroup) != -1:
                    pad['offGroup'] = int(offGroup)

                currentPad = 'pad_{0}'.format(int(key)-36)
                jsonData['bank_A'][currentPad] = pad

                boolFirstGroup = False

            if l.startswith("<region>"):
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

                layerDict = {'filename': filename, 'velHigh': highVel,
                             'velLow': lowVel,
                             # -100,0,100 to 0.0, 0.5, 1.0 mapping
                             'pan': int(pan)/200. + 0.5}
                jsonData['bank_A'][currentPad]['layer_{0}'.
                                               format(layer_count)] = layerDict
                layer_count += 1

        # No new <group> to trigger writing Nlayers to this pad,
        # so we do it here
        jsonData["bank_A"][currentPad]["nLayers"] = layer_count

        with open(filename_out, 'w') as out:
            out.write(json.dumps(jsonData, indent=4, sort_keys=True))


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
