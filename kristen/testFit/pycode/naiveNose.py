import argparse
import numpy as np
from scipy import misc
import pylab
import matplotlib.pyplot as plt
import matplotlib.colors as color

parser = argparse.ArgumentParser(description="naively draw nose looking at gradients")
parser.add_argument('-n', nargs=1, required=True, help="nose input file")
parser.add_argument('-s', nargs=1, required=True, help="output svg filename")

args = parser.parse_args()
noseIm = misc.imread(args.n[0])

def setupSVG():
    outputSVG = open(args.s[0], 'w')
    outputSVG.write('')

def closeSVG():
    outputSVG.write('')
    outputSVG.close()

def calcGradX(rows, cols, gradLineVecX, grayLineVec):
    for r in xrange(rows):

        rcols = r*cols
        rcols_cols = r*cols + cols

        # special cases: first & last elements
        gradLineVecX[rcols] = grayLineVec[rcols+1]
        gradLineVecX[rcols_cols-1] = -grayLineVec[rcols_cols-2]

        for c in xrange(1, cols-1):
            rcols_c = rcols + c
            gradLineVecX[rcols_c] = -grayLineVec[rcols_c-1] + \
            grayLineVec[rcols_c+1]

    return gradLineVecX

def calcGradY(cols, rows, gradLV, grayLV):
    for c in xrange(cols):
        # special cases: first & last elements
        gradLV[c] = grayLV[c + rows]
        gradLV[c + cols*(rows-1)] = -grayLV[c + cols*(rows-2)]

        for r in xrange(1, rows-1):
            #print r, "=> ", r*cols+c, "==", (r+1)*cols+c
            gradLV[r*cols+c] = -grayLV[(r-1)*cols+c] + grayLV[(r+1)*cols+c]
    return gradLV

def makeGrayscale(noseImg):
    shape = noseImg.shape
    numElements = noseImg.size
    lineVec = np.asarray(noseIm).reshape(-1)
    grayLineVec = np.zeros(numElements/3)
    for i in xrange(0, numElements, 3):
        p1 = lineVec[i]/3
        p2 = lineVec[i+1]/3
        p3 = lineVec[i+2]/3
        avg = (p1 + p2 + p3)
        if avg > 255:
            avg = 255
        elif avg < 0:
            avg = 0
        grayLineVec[i/3] = avg

    grayIm = grayLineVec.reshape(shape[0], shape[1])
    #plt.imsave('grayed_kr.png', grayIm, cmap=pylab.gray())
    #plt.hist(grayLineVec)
    #plt.imshow(grayIm, cmap=pylab.gray())
    #plt.show()

    # grayInfo
    return {'shape':shape, 'numEles':numElements, 'grayLV':grayLineVec}

# HOG = (1) calcGradients, (2) binGradients
    
def calcGradients(grayInfo):
    shape = grayInfo['shape']
    rows = shape[0]
    cols = shape[1]
    numElements = grayInfo['numEles'] / 3
    grayLineVec = grayInfo['grayLV']

    print rows, cols, numElements
    print "====="
    print shape

    # 1D [-1, 0 , 1] and [-1, 0, 1]^T
    gradLineVecX = np.zeros(numElements)
    gradLineVecY = np.zeros(numElements)

    # apply gradient mask in X-direction
    gradLineVecX = calcGradX(rows, cols, gradLineVecX, grayLineVec)
    # apply gradient mask in Y-direction
    gradLineVecY = calcGradY(cols, rows, gradLineVecY, grayLineVec)

    gradXIm = gradLineVecY.reshape(shape[0], shape[1])
    plt.imshow(gradXIm, cmap=pylab.gray())
    plt.show()

    return

def binGradients(gradients):
    # return cells/ weighted gradients + orientations
    pass


def drawNose():
    grayResults = makeGrayscale(noseIm)
    #plt.hist(grayResults['grayLV'])
    #plt.show()

    calcGradients(grayResults)

if __name__ == '__main__':
    drawNose()
    exit(0)


# DEPRECATE ME
def makeNaiveGradients(grayInfo):
    shape = grayInfo['shape']
    numElements = grayInfo['numEles']
    grayLineVec = grayInfo['grayLV']

    # section image into cells
    numCells = 20
    cellSize = numElements / numCells
    if numElements % numCells:
        print 'TELL ME'
        #exit(1)

    # TODO: get gradient in more orientations
    # gets gradient when cell is separated normally
    for k in xrange(numCells):
        # a cell

        topHalf_dy = 0
        botHalf_dy = 0

        lefHalf_dx = 0
        rigHalf_dx = 0

        halfEles = cellSize/2
        quartEles = cellSize/4

        curStartIdx = k * cellSize

        for x in xrange(curStartIdx, curStartIdx + halfEles):
            topHalf_dy += grayLineVec[x]
            botHalf_dy += grayLineVec[x + halfEles]
            
        for x in xrange(curStartIdx, curStartIdx + quartEles):
            lefHalf_dx += grayLineVec[x]
            lefHalf_dx += grayLineVec[x + halfEles]

            rigHalf_dx += grayLineVec[x + quartEles]
            rigHalf_dx += grayLineVec[x + halfEles + quartEles]

        gradient = (topHalf_dy - botHalf_dy) / (rigHalf_dx - lefHalf_dx)
        print gradient



def calcGradLineVecMini(Xs, Ys, gradLV, grayLV):
    for x in xrange(Xs):
        xYs = x * Ys
        xYs_Ys = x*Ys + Ys
        gradLV[xYs] = grayLV[xYs+1]
        grayLV[xYs_Ys-1] = grayLV[xYs_Ys-2]
        for y in xrange(1, Ys-1):
            xYs_y = xYs + y
            gradLV[xYs_y] = -grayLV[xYs_y-1] + grayLV[xYs_y+1]
    return gradLV

