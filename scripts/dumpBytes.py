#!/usr/bin/python3

import binascii
import sys


#FILENAME = "1-5988092708"

def dumpFile(fileName, cnt):
    f = open(fileName, "rb")
    data = f.read()
    f.close()

    print (binascii.hexlify(data[0:cnt], ' '))


try:
    cnt = int(sys.argv[2])

    dumpFile(sys.argv[1], cnt)

except:
    print (f"usage: {sys.argv[0]} <infile> <bytes to print>")
