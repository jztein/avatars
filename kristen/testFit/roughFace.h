//
//  roughFace.h
//
//  Created by Kristen Aw on 4/3/14.
//

#ifndef __IntraFace__roughFace__
#define __IntraFace__roughFace__

#include <opencv2/core/core.hpp>
#include <opencv2/objdetect/objdetect.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <iostream>
#include <fstream>
#include <algorithm>
#include <string>
#include <vector>
#include <time.h>

#include "XXDescriptor.h"
#include "FaceAlignment.h"

using namespace std;
using namespace cv;

#define TEST_IMAGE "kristen2.jpg"
#define NAME_WINDOW "Rough Avatar test 1"
#define NAME_SEE_MARKERS "iMarkers1.jpg"
#define SVG_WIDTH 480
#define SVG_HEIGHT 480
#define SVG_CENTER 240 // 480/2
#define NUM_INTRAFACE_MARKERS 49

struct Rough
{
    Mat testImage;
    string testImageName;
    int pointsX[NUM_INTRAFACE_MARKERS];
    int pointsY[NUM_INTRAFACE_MARKERS];
};

void initSVG(ofstream& svgFile);
void cleanupSVG(ofstream& svgFile);

Point drawMarkers(Mat intrafaceMarkers, struct Rough& r, float cFactor);
void drawFeatures(Point realCenter, struct Rough& r, ofstream& svgFile);

void drawEyebrow(struct Rough r, int start, int end, ofstream& svgFile);
void drawNose(struct Rough r, int curveStart, int curveEnd, ofstream& svgFile);
void drawEyes(struct Rough r, int left_tl, int left_tr, int left_bl, int right_tl, int right_tr, int right_bl, ofstream& svgFile);
void drawMouth(struct Rough r, int leftCorner, int rightCorner, int upperMiddle, int lowerMiddle, ofstream& svgFile);
void drawFace(struct Rough r, int mid, ofstream& svgFile);

#endif /* defined(__IntraFace__roughFace__) */

