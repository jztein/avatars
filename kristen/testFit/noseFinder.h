//
//  noseFinder.h
//  roughFace
//
//  Created by Kristen Aw on 4/12/14.
//

#ifndef __roughFace__noseFinder__
#define __roughFace__noseFinder__

/*
#include <opencv2/core/core.hpp>
#include <opencv2/calib3d/calib3d.hpp>
#include <opencv2/features2d/features2d.hpp>
//#include <opencv2/flann/flann.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/nonfree/nonfree.hpp>
#include <opencv2/stitching/stitcher.hpp>
//*/
#include "roughFace.h"
/*
#include "opencv2/core/core.hpp"
#include "opencv2/features2d/features2d.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/nonfree/features2d.hpp"
 //*/

#include <iostream>

using namespace std;
using namespace cv;

// SIFT
#define SIFT_nfeatures 0
#define SIFT_nOctaveLayers 4 // 4
#define SIFT_contrastThreshold 0.03 // .03
#define SIFT_edgeThreshold 10
#define SIFT_sigma 2.5
// Outlier rejection
#define SIFT_MIN_DIST_FACTOR 7.5

struct Pt
{
    int x;
    int y;
};

class NoseFinder
{
public:

    NoseFinder(Mat targetIm);
    void findNoseShape_SIFT();
    void findNoseShape_HOG();
         /*
    vector<struct Pt> getDrawing(){ vector<struct Pt> m; return m; }
    //*/
private:
    // outlier rejections
    void rejMinDist(size_t numDescriptors, vector<DMatch> &correspondences, float multiplier);
    
    void siftMatching(vector<Point2f> &im0Points, vector<Point2f> &im1Points);
    
    //vector<DMatch> correspondences;
    Mat mTarget;
    Mat mRefNose;

};



#endif /* defined(__roughFace__noseFinder__) */
