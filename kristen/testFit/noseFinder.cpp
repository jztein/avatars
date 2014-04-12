//
//  noseFinder.cpp
//  roughFace
//
//  Created by Kristen Aw on 4/12/14.
//

#include "noseFinder.h"

#include <stdio.h>
#include <iostream>

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
 


void NoseFinder::findNoseShape()
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
