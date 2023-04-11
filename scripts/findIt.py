#!/usr/bin/python3

import sys
import binascii

#01 43 44 30 30 31
#consistently found at offset 32768 
#files that don't have it are 00 42 45 41 30 31 01 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00

if 2 >= len(sys.argv):
    print (f"usage: {sys.argv[0]} <infile> <byte 0> <byte 1> ...")
    exit(1)

inFile = sys.argv[1]

search = []
for i in sys.argv[2:]:
    search.append(int(i, 16))

f = open(inFile, "rb")
inFileData = f.read()
f.close()

tot = 0
idxs = []
while True:
    idx = inFileData[tot:].find(bytes(search))
    if -1 == idx:
        break
    tot += idx

    print (f"Found at {tot}")
    idxs.append(tot)

    tot += 1


#NUM_BYTES_AFTER = 12
#NUM_BYTES_AFTER = 255
#
#for searchIdx in idxs:
#    print (f"Dumping data around {searchIdx}")
#    print (binascii.hexlify(inFileData[searchIdx:searchIdx+NUM_BYTES_AFTER], ' '))




