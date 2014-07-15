//
//  hairShape.h
//  roughFace
//
//  Created by Kristen Aw on 7/8/14.
//  Copyright (c) 2014 IntraFace. All rights reserved.
//

#ifndef __roughFace__hairShape__
#define __roughFace__hairShape__

#include <iostream>
#include "roughFace.h"

struct Pixel{
    Vec3b bgr;
    int x, y;
};

int distKmeans(struct Pixel a, struct Pixel centroid);

void clustering(Mat im, Mat& intrafaceMarkers, struct Rough& rough);
Mat getContours(Mat src);
void drawHairSvg(Mat& contourIm);

// deprecated
void contouring2(Mat im, Mat& intrafaceMarkers);

#endif /* defined(__roughFace__hairShape__) */
