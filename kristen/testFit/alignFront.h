//
//  alignFront.h
//  roughFace
//
//  Created by Kristen Aw on 6/24/14.
//  Copyright (c) 2014 IntraFace. All rights reserved.
//

#ifndef __roughFace__alignFront__
#define __roughFace__alignFront__

#include <iostream>
#include "roughFace.h"

void alignFace(Mat& im, Mat& intrafaceMarkers, float cFactor, Point2f src[3], Point2f src2[3]);
Point drawMarkersSimple(Mat intrafaceMarkers, Mat im, float cFactor);

#endif /* defined(__roughFace__alignFront__) */
