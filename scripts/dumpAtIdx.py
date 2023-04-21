#!/usr/bin/python3

import sys
import binascii

#32768

if 2 >= len(sys.argv):
    print (f"usage: {sys.argv[0]} <infile> <byte 0> <byte 1> ...")
    exit(1)

inFile = sys.argv[1]

idxs = []
for i in sys.argv[2:]:
    idxs.append(int(i))

f = open(inFile, "rb")
inFileData = f.read()
f.close()

NUM_BYTES_AFTER = 12
NUM_BYTES_AFTER = 32
NUM_BYTES_AFTER = 128

for searchIdx in idxs:
    print (f"{inFile}::%s" % binascii.hexlify(inFileData[searchIdx:searchIdx+NUM_BYTES_AFTER], ' '))
    #print (binascii.hexlify(inFileData[searchIdx:searchIdx+NUM_BYTES_AFTER], ' '))




