//
//  alignFront.cpp
//  roughFace
//
//  Created by Kristen Aw on 6/24/14.
//  Copyright (c) 2014 IntraFace. All rights reserved.
//

#include "alignFront.h"

struct stdInfo
{
    Mat im;
    vector<Point2f> intrafaceCoords;
};

stdInfo standardFaceMarkers(Mat &im)
{
    Mat stdIm = imread("kristen2.jpg");
    
    Mat resizedStd = im.clone();
    resize(stdIm, resizedStd, im.size());
    stdIm = resizedStd;
    
    Mat stdIntrafaceMarkers;
    
    INTRAFACE::FaceAlignment *faceAligner;
	INTRAFACE::XXDescriptor xxd(4);
	faceAligner = new INTRAFACE::FaceAlignment("DetectionModel-v1.5.bin", "TrackingModel-v1.10.bin", &xxd);
	if (!faceAligner->Initialized()) {
		cerr << "INTRAFACE error: Cannot init FaceAlignment" << endl;
		exit(1);
	}
    
	CascadeClassifier cascadeClassifier;
	if( !cascadeClassifier.load( string("haarcascade_frontalface_alt2.xml") ) )
	{
		cerr << "cv::CascadeClassifier error: Cannot load face detection model" << endl;
		exit(1);
	}
	vector<Rect> faces;
    
    cascadeClassifier.detectMultiScale(stdIm, faces, 1.2, 2, 0, Size(50, 50));
    
    float cFactor = SVG_WIDTH / float(stdIm.size[0]);
    
    float score;
    assert (faceAligner->Detect(stdIm, faces[0], stdIntrafaceMarkers, score)
            == INTRAFACE::IF_OK);
    
    stdInfo si;
    si.im = stdIm.clone();
    for (int i = 0; i < NUM_INTRAFACE_MARKERS; ++i)
    {
        si.intrafaceCoords.push_back(Point2f(stdIntrafaceMarkers.at<float>(0,i),
                                             stdIntrafaceMarkers.at<float>(1,i)));
    }
    
    drawMarkersSimple(stdIntrafaceMarkers, si.im, cFactor);
    
    return si;
}


// Mat intrafaceMarkers
// A 2xM-matrix, where M is the number of face markers INTRAFACE gives
// the first row holds the x-coordinates, second row holds y-coordinates

void alignFace(Mat& im, Mat& intrafaceMarkers, float cFactor, Point2f src[3], Point2f src2[3])
{
    drawMarkersSimple(intrafaceMarkers, im, cFactor);
    vector<Point2f> inputCoords;
    for (int i = 0; i < NUM_INTRAFACE_MARKERS; ++i)
    {
        inputCoords.push_back(Point2f(intrafaceMarkers.at<float>(0,i),
                                    intrafaceMarkers.at<float>(1,i)));
    }
    
    cout << "SIZE: " << inputCoords.size() << endl;
    
    stdInfo si = standardFaceMarkers(im);
    
    cout << "debug " << endl << si.intrafaceCoords.size() << endl;;
    
    
    Point2f vt0 = si.intrafaceCoords[0];
    Point2f vt1 = si.intrafaceCoords[9];
    Point2f vt2 = si.intrafaceCoords[16];
    Point2f dst[3] = {vt0, vt1, vt2};
    Point2f vt02 = si.intrafaceCoords[33];
    Point2f vt12 = si.intrafaceCoords[16];
    Point2f vt22 = si.intrafaceCoords[35];
    Point2f dst2[3] = {vt02, vt12, vt22};
    
    Mat affineM = getAffineTransform(src, dst);
    Mat affineM2 = getAffineTransform(src2, dst2);
    
    cout << affineM << "\n===w=erwe=w====\n" << affineM2.inv(DECOMP_SVD).t() << endl;
    Mat affineDst;
    warpAffine(im, affineDst, affineM, im.size());
    
    Mat affineToStdSize;
    Mat stdIm = imread("kristen2.jpg");
    resize(affineDst, affineToStdSize, stdIm.size());
    Mat tmp2;
    pyrDown(affineToStdSize, tmp2, Size(affineToStdSize.cols/2, affineToStdSize.rows/2) );
    
    imshow("affined", tmp2);
    //waitKey();
    imwrite("warped_astro.png", tmp2);
    
    
    exit(0);
    
    
    ////////
    //angle[0] - roll (about x)
    //angle[1] - yaw (about z)
    //angle[2] - pitch (about y)
    
    
    //
    
    // Find homography
    // from inputCoords to standardCoords
    Mat H = findHomography(inputCoords, si.intrafaceCoords);;
    cout << "H H H: " << H << endl;
    Mat warpedIm;
    warpPerspective(im, warpedIm, H.inv(), im.size());
    
    
    Mat downIm, tmp;
    pyrDown(warpedIm, tmp, Size(im.cols/2, im.rows/2) );
    pyrDown(tmp, downIm, Size(tmp.cols/2, tmp.rows/2) );
    
    
    
    imshow("warped im", downIm);
    //waitKey();
    
}


Point drawMarkersSimple(Mat intrafaceMarkers, Mat im, float cFactor)
{
    Point realCenter;
    int cx, cy;
    for (int k = 0 ; k < intrafaceMarkers.cols ; k++)
    {
        int R = 0, G = 0, B = 0;
        if (k < 5)
        {
            // left eyebrow
            R = 255;
        }
        else if (k < 10)
        {
            // right eyebrow
            G = 255;
        }
        else if (k < 19)
        {
            // nose
            B = 255;
            
            if (k == 11)
            {
                R = 255;
                G = 255;
                // center of face
                realCenter.x = (int)(intrafaceMarkers.at<float>(0,k) * cFactor);
                realCenter.y = (int)(intrafaceMarkers.at<float>(1,k) * cFactor);
            }
        }
        else if (k < 25)
        {
            // left eye
            R = 255;
            B = 255;
        }
        else if (k < 31)
        {
            // right eye
            G = 255;
            B = 255;
        }
        else
        {
            // mouth
            R = 255;
            G = 255;
            
            // right corner: 37
            // upper middle: 34
            if (k == 40)
                R = 100;
        }
        cx = (int)intrafaceMarkers.at<float>(0,k);
        cy = (int)intrafaceMarkers.at<float>(1,k);
        
        cout << cx << ", " << cy << ". ";
        
        //circle(im, Point(cx, cy), 2, Scalar(B,G,R), -1);
        
    }
    cout << endl;
    
    
    return realCenter;
}
    