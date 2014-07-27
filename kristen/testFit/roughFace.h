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

#include <opencv2/calib3d/calib3d.hpp>

#include "opencv2/features2d/features2d.hpp"
#include "opencv2/nonfree/features2d.hpp"

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

#define NAME_WINDOW "Rough Avatar test 1"
#define NAME_SEE_MARKERS "iMarkers2_2.jpg"
#define SVG_WIDTH 480
#define SVG_HEIGHT 480
#define SVG_CENTER 240 // 480/2
#define NUM_INTRAFACE_MARKERS 49

struct Rough
{
    Mat testImage;
    Mat intrafaceMarkers;
    string testImageName;
    int pointsX[NUM_INTRAFACE_MARKERS];
    int pointsY[NUM_INTRAFACE_MARKERS];
    
    string htmlFile;
    string markersJpg;
    string kmeansJpg;
    
    // for hair
    // anything within the triangle of (highest-leftest, highest-rightest, lowest-center)
    // is considered non-hair automatically.
    Point lowerLipCoord, centerCoord;
    Point leftestEyeCorner, highestLeftEyeCorner;
    Point rightestEyeCorner, highestRightEyeCorner;
    
    bool align;
    bool doKmeans;
    bool hair_contexts;
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

Mat cropNose(struct Rough &r);

#endif /* defined(__IntraFace__roughFace__) */

