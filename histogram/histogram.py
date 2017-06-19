#!/bin/python

from PIL import Image
import numpy as np

maxintensity = 4095
numbuckets = 20;
numFG = 0
numBG = 0
FGhistogram = np.zeros(numbuckets)
BGhistogram = np.zeros(numbuckets)

numchannels = 7

for z in range(0, 149):
    scanimage = Image.open("injured01original/CNTF_6wpc_OpticNerve_10337_z%04d.tif" % z)
    scanarray = np.array(scanimage)
    gt = np.empty(numchannels)
    for channel in range(0, numchannels):
        gt[channel] = np.array(Image.open("injured01gt/_z%04d_c%04d.tif" % z, channel + 1))
    for y in range(0, scanarray.shape[0]):
        for x in range(0, scanarray.shape[1]):
            inFG = False
            for channel in range(0, numchannels):
                if gt[channel][x][y] == 0:
                    inFG = True
                    break
            bucket = scanarray[x][y] * 20 / (maxintensity + 1)
            if inFG:
                numFG += 1
                FGhistogram[bucket] += 1
            else:
                numBG += 1
                BGhistogram[bucket] += 1
    print "done with image %d" % z
