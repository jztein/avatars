//
//  Deprecated approach:
//  Fits SVG sample to face
//  by Kristen Aw
//
//  Some INTRAFACE setup code taken from
//  main.cpp, IntraFace
//  Created by Marc Estruch Tena on 20/12/13.
//  Copyright (c) 2013 IntraFace. All rights reserved.
//

/*
#include <opencv2/core/core.hpp>
#include <opencv2/objdetect/objdetect.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <iostream>
#include <algorithm>
#include <string>
#include <vector>
#include <time.h>
#include "XXDescriptor.h"
#include "FaceAlignment.h"

#include <fstream>

using namespace std;

#define TEST_IMAGE "kristen2.jpg"
#define TEST_IMAGE2 "test_face_without_beard.png"
#define WINDOW_NAME "Toonchat avatar test k1"
#define SVG_WIDTH 547
#define SVG_HEIGHT 547
#define SVG_CENTER 274 // 547 / 2 = 273.5

// for INTRAFACE::FaceAlignment::Detect()
bool compareRect(cv::Rect r1, cv::Rect r2) { return r1.height < r2.height; }

int main()
{
    INTRAFACE::FaceAlignment *faceAligner;
	INTRAFACE::XXDescriptor xxd(4);
	faceAligner = new INTRAFACE::FaceAlignment("DetectionModel-v1.5.bin", "TrackingModel-v1.10.bin", &xxd);
	if (!faceAligner->Initialized()) {
		cerr << "INTRAFACE error: Cannot init FaceAlignment" << endl;
		exit(1);
	}
    
	cv::CascadeClassifier cascadeClassifier;
	if( !cascadeClassifier.load( string("haarcascade_frontalface_alt2.xml") ) )
	{
		cerr << "cv::CascadeClassifier error: Cannot load face detection model" << endl;
		exit(1);
	}
	vector<cv::Rect> faces;
    
    // X is a 2xM-matrix, where M is the number of face markers INTRAFACE gives
    // the first row holds the x-coordinates, second row holds y-coordinates
    cv::Mat X;
    
    // frame is captured webcam image from user's feed
    cv::Mat frame  = cv::imread(TEST_IMAGE);
    int frameWidth = frame.size[0];
    //int frameHeight = frame.size[1];
    
    cascadeClassifier.detectMultiScale(frame, faces, 1.2, 2, 0, cv::Size(50, 50));
    
    ofstream svgFile("test1.svg");
    // SVG init
    svgFile << "<!DOCTYPE html>\n<html>\n<body>\n<svg width='" << SVG_WIDTH << "' height='" << SVG_HEIGHT << "' xmlns='http://www.w3.org/2000/svg' xmlns:xlink= 'http://www.w3.org/1999/xlink' viewBox='0 0 547 547'> \n";
    svgFile << "<g transform='matrix(0.56526049,0,0,0.56526049,4.3179764,6.0134452)'> \n";
    
    // SVG defs
    svgFile << "<defs> \n";
    svgFile << "<linearGradient id='noseLinearGradientColor'><stop style='stop-color:#fd8d6f;stop-opacity:1;' offset='0' id='stop3800' /> <stop style='stop-color:#ecb8aa;stop-opacity:0;' offset='1' id='stop3802' /></linearGradient> \n";
    svgFile << "<filter id='noseSoftFillFilter' x='-0.32217589' width='1.6443518' y='-0.30753276' height='1.6150655'> <feGaussianBlur stdDeviation='8.1676793' id='feGaussianBlur4448' /> </filter> \n";
    svgFile << "<linearGradient xlink:href='#noseLinearGradientColor' id='noseLinearGradient' x1='276.18265' y1='387.0506' x2='274.29178' y2='220.21252' gradientUnits='userSpaceOnUse' /> \n";
    svgFile << "</defs> \n";
    
    // Fill background
    svgFile << "<path d='M 0,0 949,0 949,949 0,949 0,0 M 453.37,5.2 C 409.36,7.47 365.53,15.34 323.82,29.68 236.45,59.09 158.47,115.28 102.36,188.36 50.7,255.16 17.45,336.13 7.88,420.07 c -0.65,5.63 -1.71,11.21 -2.01,16.88 -0.23,9.03 -1.58,18 -1.14,27.05 -0.35,10.82 -0.52,21.68 0.56,32.48 2.39,57.7 16.2,114.82 39.49,167.63 30.18,67.77 76.39,128.42 134.13,175.03 54.65,44.72 119.7,76.58 188.44,92.67 31.68,7.46 64.16,11.3 96.65,12.52 13.35,-0.37 26.75,0.66 40.06,-0.82 70.57,-4.45 140.33,-24.49 201.84,-59.53 31.27,-17.64 60.5,-38.88 86.98,-63.11 6.48,-6.87 14.31,-12.36 20.3,-19.7 2.71,-3.27 5.71,-6.27 8.69,-9.29 58.33,-63.49 98.98,-143.2 115.28,-227.91 4.76,-26.38 8.84,-53.09 8.12,-79.97 -0.27,-15.99 0.77,-32.03 -0.92,-47.97 -1.74,-20.5 -5.1,-40.84 -9.19,-60.99 -18.38,-84.4 -60.6,-163.45 -120.59,-225.61 -6.46,-8.19 -15.04,-14.27 -22.2,-21.79 C 753.28,92.1 708.36,62.85 659.72,42.17 610.18,21 556.9,8.77 503.17,5.41 494.15,4.33 485.06,4.78 476,4.73 468.45,4.78 460.88,4.37 453.37,5.2 z' style='fill:#f2f2f2' /> \n";
    svgFile << "<path d='m 453.37,5.2 c 7.51,-0.83 15.08,-0.42 22.63,-0.47 9.06,0.05 18.15,-0.4 27.17,0.68 53.73,3.36 107.01,15.59 156.55,36.76 48.64,20.68 93.56,49.93 132.65,85.47 7.16,7.52 15.74,13.6 22.2,21.79 59.99,62.16 102.21,141.21 120.59,225.61 4.09,20.15 7.45,40.49 9.19,60.99 1.69,15.94 0.65,31.98 0.92,47.97 0.72,26.88 -3.36,53.59 -8.12,79.97 -16.3,84.71 -56.95,164.42 -115.28,227.91 -2.98,3.02 -5.98,6.02 -8.69,9.29 -5.99,7.34 -13.82,12.83 -20.3,19.7 -26.48,24.23 -55.71,45.47 -86.98,63.11 -61.51,35.04 -131.27,55.08 -201.84,59.53 -13.31,1.48 -26.71,0.45 -40.06,0.82 -32.49,-1.22 -64.97,-5.06 -96.65,-12.52 C 298.61,915.72 233.56,883.86 178.91,839.14 121.17,792.53 74.96,731.88 44.78,664.11 21.49,611.3 7.68,554.18 5.29,496.48 4.21,485.68 4.38,474.82 4.73,464 4.29,454.95 5.64,445.98 5.87,436.95 6.17,431.28 7.23,425.7 7.88,420.07 17.45,336.13 50.7,255.16 102.36,188.36 158.47,115.28 236.45,59.09 323.82,29.68 365.53,15.34 409.36,7.47 453.37,5.2 z' style='fill:#4bc9d8' /> \n";
    svgFile << "</g> \n" << endl;
    //svgFile << "<rect x='0' y='0' width='" << SVG_WIDTH << "' height='" << SVG_HEIGHT << "' style='fill:blue;opacity:0.1'/> \n";
    // End background
    
    int cx, cy;
    cv::Point realCenter, leftEye, rightEye, nose, upperNose, leftBrow, rightBrow, leftLipCorner, upperLip;
    
    int pointsX[49];
    int pointsY[49];
    
    
    float cFactor = SVG_WIDTH / float(frameWidth);
    cout << "cFactor: " << cFactor << endl;
    float score;
    float notFaceScore = 0.5;
    for (int i = 0; i < 1;i++){//faces.size(); i++) {
        if (faceAligner->Detect(frame, faces[i], X, score) == INTRAFACE::IF_OK)
        {
            if (score >= notFaceScore) {
                for (int k = 0 ; k < X.cols ; k++)
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
                            realCenter.x = (int)(X.at<float>(0,k) * cFactor);
                            realCenter.y = (int)(X.at<float>(1,k) * cFactor);
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
                        if (k == 35)
                            R = 100;
                    }
                    cx = (int)X.at<float>(0,k);
                    cy = (int)X.at<float>(1,k);
                    
                    cv::circle(frame,cv::Point(cx, cy), 2, cv::Scalar(B,G,R), -1);
                    cx = int(float(cx) * cFactor);
                    cy = int(float(cy) * cFactor);
                    //cv::circle(frame,cv::Point(cx, cy), 2, cv::Scalar(B,G,R), -1);
                    
                    pointsX[k] = cx;
                    pointsY[k] = cy;
                    
                    cout << cx << ", " << cy << " | ";
                }
            }
        }
        cout << endl;
        
        
        cout << "===================" << endl;
        
        int shiftX = SVG_CENTER - realCenter.x;
        int shiftY = SVG_CENTER - realCenter.y;
        
        // get feature positions
        leftEye.x = pointsX[20] + shiftX;
        leftEye.y = pointsY[20] + shiftY;
        rightEye.x = (pointsX[26] + pointsX[27])/2 + shiftX;
        rightEye.y = pointsY[27] + shiftY;
        nose.x = (pointsX[10] + pointsX[11])/2 + shiftX;
        nose.y = (pointsY[10] + pointsY[11])/2 + shiftY;
        upperNose.x = pointsX[10] + shiftX;
        upperNose.y = pointsY[10] + shiftY;
        // vvv need to correct these
        leftBrow.x = pointsX[1] + shiftX;
        leftBrow.y = pointsY[1] + shiftY;
        rightBrow.x = pointsX[6] + shiftX;
        rightBrow.y = pointsY[6] + shiftY;
        leftLipCorner.x = (pointsX[31] + pointsX[41])/2 + shiftX;
        leftLipCorner.y = (pointsY[31] + pointsY[32])/2 + shiftY;
        upperLip.x = pointsX[34] + shiftX;
        upperLip.y = pointsY[34] + shiftY;
        
        for (int i = 0; i < 49; ++i)
        {
            cx = pointsX[i];
            cy = pointsY[i];
            cout << cx << ", " << cy << " | ";
            
            cx = pointsX[i] + shiftX;
            cy = pointsY[i] + shiftY;
            
            svgFile << "<circle style='fill:#ff0000' cx='" << cx << "' cy='" << cy << "' r='3' />\n";
            
        }
        cout << endl;
        
        // draw left eye
        svgFile << "<path d='m "<< leftEye.x << "," << leftEye.y << "c 5.57,-3.22 13.18,2.11 11.94,8.45 -0.52,5.94 -8.16,9.21 -12.88,5.64 -4.99,-3.22 -4.52,-11.59 0.94,-14.09 z' style='fill:#2e2c2a;fill-opacity:1' transform='matrix(0.95858194,0,0,0.95858194,10.176936,15.801653)'/> \n";
        // draw right eye
        svgFile << "<path d='m "<< rightEye.x << "," << rightEye.y << "c 5.57,-3.16 13.07,2.18 11.87,8.47 -0.5,5.95 -8.15,9.23 -12.88,5.66 -5.03,-3.23 -4.51,-11.69 1.01,-14.13 z' style='fill:#2e2c2a;fill-opacity:1' transform='matrix(0.95858194,0,0,0.95858194,10.176936,15.801653)'/> \n";
        // draw nose
        svgFile << "<path d='m "<< nose.x << "," << nose.y << "c 0.31,0 0.94,0.01 1.26,0.02 4.08,5.05 6.09,11.31 8.85,17.1 3.92,9.34 7.24,18.92 10.57,28.49 2.33,8.3 5.66,16.39 6.71,24.99 0.76,4.77 -2.2,9.37 -6.46,11.34 -11.7,5.79 -25.89,6.04 -37.98,1.25 -3.77,-1.48 -7.68,-3.98 -8.79,-8.12 -1.03,-4.15 0.3,-8.41 1.27,-12.44 5.07,-18.45 11.65,-36.48 19.48,-53.93 1.31,-3.1 2.77,-6.22 5.09,-8.7 z' style='fill:url(#noseLinearGradient);fill-opacity:1' transform='matrix(0.95858194,0,0,0.95858194,10.176933,15.801651)'/> \n";
        // draw nose softfill
        svgFile << "<circle cx='"<< nose.x << "' cy='" << nose.y << "' r='30' style='fill:#f6d9c7;fill-opacity:1;stroke:#454545;stroke-width:0;stroke-linecap:round;stroke-linejoin:round;stroke-miterlimit:4;stroke-opacity:1;stroke-dasharray:none;filter:url(#noseSoftFillFilter)' transform='matrix(0.95858194,0,0,0.95858194,10.176933,15.801651)'/> \n";
        
        // TODO: change brows path code to fit user
        // draw left eyebrow
        // (were labels in original svg correct? swapped left brow with rightBrow here)
        svgFile << "<path d='m "<< leftBrow.x << "," << leftBrow.y << "c 3.5,-2.89 7.44,-5.91 12.19,-5.97 13.02,-0.47 25.78,2.89 38.06,6.85 2.2,0.86 5,1.68 5.78,4.21 0.71,1.89 -1.22,3.56 -3.01,3.37 -8.63,-0.22 -17.18,-1.68 -25.8,-2.04 -6.68,-0.23 -13.63,-1.61 -20.07,0.91 -4.76,1.28 -8.57,4.61 -13.17,6.23 -1.31,-5.39 2.14,-10.24 6.02,-13.56 z' style='fill:#2e2c2a;fill-opacity:1' transform='matrix(0.95858194,0,0,0.95858194,10.176936,15.801653)'/> \n";
        //svgFile << "<path d='m "<< rightBrow.x << "," << rightBrow.y << "c -222.19334,196.01333 -111.09667,98.00667 0,0 z' style='fill:#002c2a;fill-opacity:1' transform='matrix(0.95858194,0,0,0.95858194,10.176936,15.801653)'/> \n";
        // draw right eyebrow
        svgFile << "<path d='m "<< rightBrow.x << "," << rightBrow.y << "c 8.69,-2.59 17.77,-3.92 26.84,-3.89 5.84,-0.11 10.75,3.68 14.75,7.52 3.23,3.07 5.42,7.44 4.51,11.98 -2.91,-0.59 -5.17,-2.6 -7.77,-3.85 -4.22,-2.07 -8.78,-3.72 -13.52,-3.8 -11.68,0.28 -23.3,1.52 -34.93,2.58 -1.79,0.03 -3.9,0.44 -5.39,-0.82 -1.51,-1.79 0.01,-4.32 1.81,-5.19 4.3,-2.2 9.1,-3.15 13.7,-4.53 z' style='fill:#2e2c2a;fill-opacity:1' transform='matrix(0.95858194,0,0,0.95858194,10.176936,15.801653)'/> \n";
        
        // draw upper teeth
        svgFile << "<path d='m "<< upperLip.x << "," << upperLip.y << "-3.23475,0.4851 -4.18612,1.23193 -4.60416,2.50488 -0.18762,0.5713 1.21906,0.86521 1.95223,0.74902 1.59863,-0.25334 2.66978,-0.8491 2.64285,-2.72652 -0.0269,-1.87742 -1.68088,3.05714 2.30485,5.65602 3.91398,2.55212 11.0165,2.6955 15.10959,0.91721 2.96459,-1.28802 2.04121,-3.1714 2.30485,-6.42037 0.26364,-3.24896 -3.2302,4.70632 2.81705,7.03182 2.85189,1.09671 10.19679,2.10638 13.57301,0.30573 4.93342,-2.63114 2.89806,-5.67678 3.07313,-7.33755 0.23885,-2.26574 -2.61392,3.50966 0.7683,5.8089 4.56097,3.10057 11.70962,3.27494 16.90224,0.30573 2.55432,-1.4606 1.55683,-10.53595 1.53656,-6.26749 -0.0171,3.59448 -1.48016,4.63672 1.02439,5.96176 3.5142,1.8592 10.32303,1.52456 14.08521,-0.15287 2.72539,-1.21515 3.09504,-4.1543 3.07312,-5.80889 -0.0322,-2.43567 -0.77107,0.44923 0.70466,1.30489 2.0452,1.18587 4.88544,0.46126 5.04163,-0.65512 0.20194,-1.44344 -5.09618,-1.47402 -6.17526,-1.60734 -5.61338,-0.69351 -7.68574,-0.74744 -10.88988,-0.89324 -16.08414,-0.73189 -31.79778,-1.38282 -49.59691,-0.51318 -6.73922,0.32926 -12.56036,1.37366 -15.64746,1.83661 z' style='fill:#ffffff;fill-opacity:1;stroke:#000000;stroke-width:0;stroke-linecap:butt;stroke-linejoin:miter;stroke-miterlimit:4;stroke-opacity:1;stroke-dasharray:none;display:inline' transform='matrix(0.4,0,0,0.4, 10.176936,15.801653)'/> \n";
        
        // draw lips
        svgFile << "<path d='m "<< leftLipCorner.x << "," << leftLipCorner.y << "c -7.44648,3.49045 -13.29592,4.70643 -15.59652,13.24973 -2.3006,8.54329 5.58369,12.05817 20.40222,16.83557 14.81853,4.77738 53.54202,4.28141 69.55463,0.004 16.0126,-4.27677 24.60964,-8.35381 23.21177,-16.43403 -1.39788,-8.08022 -6.56149,-9.69446 -16.42172,-13.83011 -9.86024,-4.13565 -28.31675,-4.7575 -42.04658,-4.72301 -13.72984,0.0345 -31.65731,1.40675 -39.1038,4.89721 z m 69.30962,6.65558 c 9.87818,2.1917 12.00154,4.78607 12.58508,7.68653 0.7084,3.5211 -3.90922,5.5312 -17.26384,8.03005 -13.35461,2.49885 -34.08212,2.76522 -48.50367,-0.11163 -14.42155,-2.87684 -17.19065,-4.46458 -15.61321,-8.69873 1.57744,-4.23414 3.62442,-4.13052 12.18938,-6.66323 8.56496,-2.5327 16.75114,-2.73913 26.67607,-2.81575 15.98526,0.47128 20.31372,0.43912 29.93019,2.57276 z' style='fill:#fcd5d5;fill-opacity:1;stroke:#ffddd9;stroke-width:0;stroke-linecap:round;stroke-linejoin:round;stroke-miterlimit:4;stroke-opacity:1;stroke-dasharray:none;display:inline' transform='matrix(0.95858194,0,0,0.95858194,10.176936,15.801653)'/> \n";
        
    }
    
    svgFile << "</svg>\n</body>\n</html>\n";
    svgFile.close();
    
    cv::namedWindow(WINDOW_NAME);
    cv::imshow(WINDOW_NAME,frame);
    
    // Press any key to close
    cv::waitKey(0);
    
	return 0;
    
}

*/











