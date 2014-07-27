//
//  noseFinder.cpp
//  roughFace
//
//  Created by Kristen Aw on 4/12/14.
//

#include "noseFinder.h"

#include <stdio.h>
#include <iostream>

#include <opencv2/imgproc/imgproc.hpp>

using namespace cv;

NoseFinder::NoseFinder(Mat targetIm)
{
    Mat biggerTI(targetIm.rows*2, targetIm.cols*2, CV_32FC3);
    resize(targetIm, biggerTI, biggerTI.size(), INTER_AREA);
    
    mTarget = biggerTI;
    
    mRefNose = imread("ref_nose_kr.jpg");
    if (!mRefNose.data)
    {
        cout << "Failed to read refnose" << endl;
        exit(1);
    }
    
    Mat biggerRN(mRefNose.rows*2, mRefNose.cols*2, CV_32FC3);
    resize(mRefNose, biggerRN, biggerRN.size(), INTER_AREA);
}
 
// HOG

void NoseFinder::findNoseShape_HOG()
{
    Mat grayTarget, blurredTarget;
    
    GaussianBlur(mTarget, blurredTarget, Size(3,3), 0);
    cvtColor(blurredTarget, grayTarget, CV_BGR2GRAY);
    
    Mat gradX, gradY, absGradX, absGradY;
    Sobel(grayTarget, gradX, CV_16S, 1, 0, 5, 1, 0, BORDER_DEFAULT);
    Sobel(grayTarget, gradY, CV_16S, 0, 1, 5, 1, 0, BORDER_DEFAULT);
    convertScaleAbs(gradX, absGradX);
    convertScaleAbs(gradY, absGradY);
    
    Mat grad;
    addWeighted(absGradX, 0.5, absGradY, 0.5, 0, grad);
    
    //blurredTarget = grayTarget;
    
    //Mat biggerRN(grad.rows*2, grad.cols*2, CV_8U);
    //resize(grad, biggerRN, biggerRN.size(), INTER_AREA);
    Mat biggerRN = grad;
    
    imshow("grad target", biggerRN);
    
    imshow(" target", mTarget);
    imwrite("grad_stp1.jpg", grad);
    
    waitKey(0);
}


// SIFT

void NoseFinder::findNoseShape_SIFT()
{
    vector<Point2f> im0Points, im1Points;
    siftMatching(im0Points, im1Points);
}

  //*
void NoseFinder::siftMatching(vector<Point2f> &im0Points, vector<Point2f> &im1Points)
{
    Mat im0 = mRefNose;
    Mat im1 = mTarget;
    
    vector<DMatch> correspondences;
    Mat gray0, gray1;
    cvtColor(im0, gray0, CV_BGR2GRAY);
    cvtColor(im1, gray1, CV_BGR2GRAY);
    
    if (!gray0.data || !gray1.data)
    {
        cout << "Problem loading images or grayscaling in sift Matching" << endl;
        return;
    }
    
    SIFT sift(SIFT_nfeatures, SIFT_nOctaveLayers, SIFT_contrastThreshold, SIFT_edgeThreshold, SIFT_sigma);
    
    vector<KeyPoint> im0Landmarks, im1Landmarks;
    Mat im0Descriptors, im1Descriptors;
    
    sift(im0, Mat(), im0Landmarks, im0Descriptors);
    sift(im1, Mat(), im1Landmarks, im1Descriptors);
    //*
    FlannBasedMatcher matcher;
    
    matcher.match(im0Descriptors, im1Descriptors, correspondences);
    cout << "BEFORE outlierRej num SIFT matches: " << correspondences.size() << endl;
    
    rejMinDist(im0Descriptors.rows, correspondences, SIFT_MIN_DIST_FACTOR);
    
    cout << "AFTER outlierRej num SIFT matches: " << correspondences.size() << endl;
    
    for (int i = 0; i < correspondences.size(); i++)
    {
        im0Points.push_back(im0Landmarks[correspondences[i].queryIdx].pt);
        im1Points.push_back(im1Landmarks[correspondences[i].trainIdx].pt);
    }
    
    // draw matches
    Mat matchesIm;
    drawMatches(im0, im0Landmarks, im1, im1Landmarks, correspondences, matchesIm, Scalar::all(-1), Scalar::all(-1), vector<char>(), DrawMatchesFlags::NOT_DRAW_SINGLE_POINTS);
    imshow("SIFT matches im", matchesIm);
    //imwrite("SIFT matches im.jpg", matchesIm);
    waitKey();
     //*/
}

void NoseFinder::rejMinDist(size_t numDescriptors, vector<DMatch> &correspondences, float multiplier)
{
    double minDistance = 900000000.0;
    double curDistance = 0.0;
    size_t lastSize = 1;
    size_t curSize = 0;
    
    // Compute new min-distance
    for (int i = 0; i < numDescriptors; i++)
    {
        curDistance = correspondences[i].distance;
        if (curDistance < minDistance)
            minDistance = curDistance;
    }
    // Trim correspondences by distance
    lastSize = correspondences.size();
    vector<DMatch> goodCorrespondences;
    cout << "mdf: " << multiplier << endl;
    for (int i = 0; i < correspondences.size(); i++)
    {
        if (correspondences[i].distance < minDistance*multiplier)
            goodCorrespondences.push_back(correspondences[i]);
    }
    curSize = goodCorrespondences.size();
    correspondences = goodCorrespondences;
    
    // check sizes
    //cout << "before: " << lastSize << " after-good: " << curSize << " minDist: " << minDistance << endl;

}
