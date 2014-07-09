//
//  roughFace.cpp
//
//  Created by Kristen Aw on 4/3/14.
//

#include "roughFace.h"
#include "utils.h"
#include "noseFinder.h"

#include "alignFront.h"
#include "hairShape.h"

//
//  Some INTRAFACE setup code adapted from
//  main.cpp, IntraFace
//  Created by Marc Estruch Tena on 20/12/13.
//  Copyright (c) 2013 IntraFace. All rights reserved.
//

// for INTRAFACE::FaceAlignment::Detect()
bool compareRect(Rect r1, Rect r2) { return r1.height < r2.height; }


//
void rotateImage(const Mat &input, Mat &output, double alpha, double beta, double gamma, double dx, double dy, double dz, double f)
{
    alpha = (alpha - 90.)*CV_PI/180.;
    beta = (beta - 90.)*CV_PI/180.;
    gamma = (gamma - 90.)*CV_PI/180.;
    // get width and height for ease of use in matrices
    double w = (double)input.cols;
    double h = (double)input.rows;
    // Projection 2D -> 3D matrix
    Mat A1 = (Mat_<double>(4,3) <<
              1, 0, -w/2,
              0, 1, -h/2,
              0, 0,    0,
              0, 0,    1);
    // Rotation matrices around the X, Y, and Z axis
    Mat RX = (Mat_<double>(4, 4) <<
              1,          0,           0, 0,
              0, cos(alpha), -sin(alpha), 0,
              0, sin(alpha),  cos(alpha), 0,
              0,          0,           0, 1);
    Mat RY = (Mat_<double>(4, 4) <<
              cos(beta), 0, -sin(beta), 0,
              0, 1,          0, 0,
              sin(beta), 0,  cos(beta), 0,
              0, 0,          0, 1);
    Mat RZ = (Mat_<double>(4, 4) <<
              cos(gamma), -sin(gamma), 0, 0,
              sin(gamma),  cos(gamma), 0, 0,
              0,          0,           1, 0,
              0,          0,           0, 1);
    // Composed rotation matrix with (RX, RY, RZ)
    Mat R = RX * RY * RZ;
    // Translation matrix
    Mat T = (Mat_<double>(4, 4) <<
             1, 0, 0, dx,
             0, 1, 0, dy,
             0, 0, 1, dz,
             0, 0, 0, 1);
    // 3D -> 2D matrix
    Mat A2 = (Mat_<double>(3,4) <<
              f, 0, w/2, 0,
              0, f, h/2, 0,
              0, 0,   1, 0);
    // Final transformation matrix
    Mat trans = A2 * (T * (R * A1));
    // Apply matrix transformation
    warpPerspective(input, output, trans, input.size(), INTER_LANCZOS4);
}
//

int main(int argc, char** argv)
{
    srand(unsigned(time(NULL)));
    
    //FlannBasedMatcher m;
    struct Rough r;
    
    r.align = false; // init here
    r.doKmeans = false;
    
    while (parseArgs(argc-1, argv+1, r))
        ;
    
    cout << r.htmlFile << ", " << r.markersJpg << ", " << r.kmeansJpg << endl;
    

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
    
    // A 2xM-matrix, where M is the number of face markers INTRAFACE gives
    // the first row holds the x-coordinates, second row holds y-coordinates
    Mat intrafaceMarkers;
    
    Mat oriIm, oriIm2;
    oriIm = r.testImage.clone();
    oriIm2 = r.testImage.clone();
    
    // cv::Mat::r.testImage is captured webcam image from user's feed
    int frameWidth = r.testImage.size[0];
    
    cascadeClassifier.detectMultiScale(r.testImage, faces, 1.2, 2, 0, Size(50, 50));
    
    ofstream svgFile(r.htmlFile.c_str());
    initSVG(svgFile);
    
    Point realCenter;
    float cFactor = SVG_WIDTH / float(frameWidth);
    float score;
    float notFaceScore = 0.5;
    int tmpI = 0;
    while (true) {
    if (faceAligner->Detect(r.testImage, faces[tmpI], intrafaceMarkers, score) == INTRAFACE::IF_OK)
    {
        r.intrafaceMarkers = intrafaceMarkers;
        if (score >= notFaceScore) {
            realCenter = drawMarkers(intrafaceMarkers, r, cFactor);
            break;
        }
        tmpI++;
        //drawFeatures(realCenter, r, svgFile);
    }
    }
    
    cleanupSVG(svgFile);
    
    if (r.doKmeans ) {
        cout << "DOING K MEANS CLUSTERING FOR HAIR" << endl;
        clustering(oriIm, intrafaceMarkers, r);
    }
    
    if (r.align)
    {
        cout << "DOING FACE ALIGN " << endl;
        cout << "Initializing..." << endl;
        
        INTRAFACE::HeadPose hp;
        faceAligner->EstimateHeadPose(intrafaceMarkers, hp);
        
        /*
        //angle[0] - roll (about x)
        //angle[1] - yaw (about z)
        //angle[2] - pitch (about y)
        float aX = hp.angles[0];
        float aY = hp.angles[2];
        float aZ = hp.angles[1];
        Mat RX = (Mat_<float>(3,3) <<
                  1,          0,           0,
                  0, cos(aX), -sin(aX),
                  0, sin(aX),  cos(aX));
        Mat RY = (Mat_<float>(3,3) <<
                  cos(aY), 0, sin(aY),
                  0, 1, 0,
                  -sin(aY), 0,  cos(aY));
        
        Mat RZ = (Mat_<float>(3,3) <<
                  1,          0,           0,
                  0, cos(aZ), -sin(aZ),
                  0, sin(aZ),  cos(aZ));
        
        
        Mat final;
        warpPerspective(oriIm2, final, hp.rot.inv(), oriIm2.size());
        //warpPerspective(final, final, RY.inv(), oriIm2.size());
        //warpPerspective(final, final, RZ.inv(), oriIm2.size());
        imshow("hey ho", final);
        waitKey();
        exit(0);
        //*/
        
        // far left eyebrow
        Point2f vf0(intrafaceMarkers.at<float>(0,0),intrafaceMarkers.at<float>(1,0));
        // far right eyebrow
        Point2f vf1(intrafaceMarkers.at<float>(0,9),intrafaceMarkers.at<float>(1,9));
        // bottom tip of nose
        Point2f vf2(intrafaceMarkers.at<float>(0,16),intrafaceMarkers.at<float>(1,16));
        Point2f src[3] = {vf0, vf1, vf2};
        
        Point2f vf02(intrafaceMarkers.at<float>(0,33),intrafaceMarkers.at<float>(1,33));
        Point2f vf12(intrafaceMarkers.at<float>(0,16),intrafaceMarkers.at<float>(1,16));
        Point2f vf22(intrafaceMarkers.at<float>(0,35),intrafaceMarkers.at<float>(1,35));
        Point2f src2[3] = {vf02, vf12, vf22};
        
        cout << "ALIGNING..." << endl;
        alignFace(oriIm2, intrafaceMarkers, cFactor, src, src2);
    }
    
	return 0;
    
}

void initSVG(ofstream& svgFile)
{
    // SVG init
    svgFile << "<!DOCTYPE html>\n<html>\n<body>\n<svg width='" << SVG_WIDTH << "' height='" << SVG_HEIGHT << "' xmlns='http://www.w3.org/2000/svg' xmlns:xlink= 'http://www.w3.org/1999/xlink' viewBox='0 0 " << SVG_WIDTH << " " << SVG_HEIGHT << "'> \n";
    
    // SVG defs
    svgFile << "<defs> \n";
    svgFile << "</defs> \n";
    
    // Background
    svgFile << "<rect x='0' y='0' width='" << SVG_WIDTH << "' height='" << SVG_HEIGHT << "' style='fill:blue;opacity:0.1'/> \n";
}

void cleanupSVG(ofstream& svgFile)
{
    svgFile << "</svg>\n</body>\n</html>\n";
    svgFile.close();
}

// get other features and draw
void drawFeatures(Point realCenter, struct Rough& r, ofstream& svgFile)
{
    // features wanted
    Point leftEye, rightEye, nose, upperNose, leftBrow, rightBrow, leftLipCorner, upperLip;
    
    int shiftX = SVG_CENTER - realCenter.x;
    int shiftY = SVG_CENTER - realCenter.y;
    
    // get feature positions
    leftEye.x = r.pointsX[20] + shiftX;
    leftEye.y = r.pointsY[20] + shiftY;
    rightEye.x = (r.pointsX[26] + r.pointsX[27])/2 + shiftX;
    rightEye.y = r.pointsY[27] + shiftY;
    nose.x = (r.pointsX[10] + r.pointsX[11])/2 + shiftX;
    nose.y = (r.pointsY[10] + r.pointsY[11])/2 + shiftY;
    upperNose.x = r.pointsX[10] + shiftX;
    upperNose.y = r.pointsY[10] + shiftY;
    // vvv need to correct these
    leftBrow.x = r.pointsX[1] + shiftX;
    leftBrow.y = r.pointsY[1] + shiftY;
    rightBrow.x = r.pointsX[6] + shiftX;
    rightBrow.y = r.pointsY[6] + shiftY;
    leftLipCorner.x = (r.pointsX[31] + r.pointsX[41])/2 + shiftX;
    leftLipCorner.y = (r.pointsY[31] + r.pointsY[32])/2 + shiftY;
    upperLip.x = r.pointsX[34] + shiftX;
    upperLip.y = r.pointsY[34] + shiftY;
    
    // draw features, cout compare to ensure correctness
    cout << "===================" << endl;
    
    int cx, cy;
    for (int i = 0; i < NUM_INTRAFACE_MARKERS; ++i)
    {
        cx = r.pointsX[i];
        cy = r.pointsY[i];
        cout << cx << ", " << cy << " | ";
        
        cx = r.pointsX[i] + shiftX;
        cy = r.pointsY[i] + shiftY;
        
        // intraface markers scaled to svg canvas
        //svgFile << "<circle style='fill:#000000' cx='" << cx << "' cy='" << cy << "' r='3' />\n";
        
        r.pointsX[i] = cx;
        r.pointsY[i] = cy;
        
    }
    cout << endl;
    
    // draw face things
    
    // left eyebrow
    drawEyebrow(r, 0, 5, svgFile);
    // right eyebrow
    drawEyebrow(r, 5, 10, svgFile);
    
    // nose
    drawNose(r, 14, 19, svgFile);
    
    // left eye
    drawEyes(r, 20, 21, 24, 26, 27, 30, svgFile);
    
    drawMouth(r, 31, 37, 44, 47, svgFile);
    
    drawFace(r, 11, svgFile);
    
//    Mat croppedToNose = cropNose(r);
//    NoseFinder nf(croppedToNose);
//    //nf.findNoseShape_SIFT();
//    nf.findNoseShape_HOG();
    
    // see things
    //drawEyebrow(r, 10, 19, svgFile);
}

Mat cropNose(struct Rough &r)
{
    int x0, y0, x1, y1;
    x0 = (int)r.intrafaceMarkers.at<float>(0,3);
    y0 = (int)r.intrafaceMarkers.at<float>(1,3);
    x1 = (int)r.intrafaceMarkers.at<float>(0,37) - x0;
    y1 = (int)r.intrafaceMarkers.at<float>(1,37) - y0;
    
    cout << "##:\n" << x0 << "," << y0 << " || " << x1 << "," << y1 << endl;
    cout << "Bluhfesht: " << r.testImage.cols << "," << r.testImage.rows << endl;
    
    Rect roi(x0, y0, x1, y1);
    
    Mat croppedIm = r.testImage(roi);
    //imshow("hello cropped!", croppedIm);
    //imwrite("cropped1.jpg", croppedIm);
    //waitKey(0);
    return croppedIm;
    
}

void drawEyebrow(struct Rough r, int start, int end, ofstream& svgFile)
{
    svgFile << "<path d='M" << r.pointsX[start] << " " << r.pointsY[start] << " ";
    for (int i = start+1; i < end; ++i)
    {
        svgFile << "L " << r.pointsX[i] << " " << r.pointsY[i] << " ";
    }
    svgFile << "' fill='transparent' stroke='black' />\n";
}

// curve refers to bottom of nose
void drawNose(struct Rough r, int curveStart, int curveEnd, ofstream& svgFile)
{    
    // mark nose sides y point
    // this is approximately between the two points before curve
    // CHECK this assumption:
    int midx, midy;
    midx = (r.pointsX[curveStart - 1] + r.pointsX[curveStart - 2]) / 2;
    midy = r.pointsY[curveStart - 1];//(r.pointsY[curveStart - 1] + r.pointsY[curveStart - 2]) / 2;
    //svgFile << "<circle style='fill:#0000ff' cx='" << midx << "' cy='" << midy << "' r='3' />\n";
    
    int rightx = r.pointsX[curveEnd - 1] + (r.pointsX[curveEnd - 1] - r.pointsX[curveEnd - 2]);
    int right2x = rightx + (r.pointsX[curveEnd - 1] - r.pointsX[curveEnd - 2]);
    int leftx = r.pointsX[curveStart] - (r.pointsX[curveStart + 1] - r.pointsX[curveStart]);
    int left2x = leftx - (r.pointsX[curveStart + 1] - r.pointsX[curveStart]);
    
    int lowery = r.pointsY[(curveStart + curveEnd) / 2] - ((midy - r.pointsY[(curveStart + curveEnd) / 2])/2);
    
    svgFile << "<path d='M" << leftx << " " << midy << " ";
    svgFile << "C " << left2x << " " << lowery << ", " << right2x << " " << lowery << ", " << rightx << " " << midy;
    svgFile << "' fill='transparent' stroke='black' />\n";
    
    // draw curve + control points
    svgFile << "<circle style='fill:#0000ff' cx='" << leftx << "' cy='" << midy << "' r='3' />\n";
    svgFile << "<circle style='fill:#0000ff' cx='" << left2x << "' cy='" << lowery << "' r='3' />\n";
    svgFile << "<circle style='fill:#0000ff' cx='" << right2x << "' cy='" << lowery << "' r='3' />\n";
    svgFile << "<circle style='fill:#0000ff' cx='" << rightx << "' cy='" << midy << "' r='3' />\n";
    
}

// top left, top right, bottom right, bottom left
//
//  0      1
//
//   *    *
// *   O    *
//   *    *
//
//  4      3
void drawEyes(struct Rough r, int left_tl, int left_tr, int left_bl, int right_tl, int right_tr, int right_bl, ofstream& svgFile)
{
    int leftx = (r.pointsX[left_tl] + r.pointsX[left_tr]) / 2;
    int rightx = (r.pointsX[right_tl] + r.pointsX[right_tr]) / 2;
    int y = (((r.pointsY[left_tl] + r.pointsY[left_bl]) / 2) + ((r.pointsY[right_tl] + r.pointsY[right_bl]) / 2)) / 2;
    
    svgFile << "<circle style='fill:#330055' cx='" << leftx << "' cy='" << y << "' r='7' /> \n";
    svgFile << "<circle style='fill:#330055' cx='" << rightx << "' cy='" << y << "' r='7' /> \n";
}

void drawMouth(struct Rough r, int leftCorner, int rightCorner, int upperMiddle, int lowerMiddle, ofstream& svgFile)
{
    // upper middle lip
    svgFile << "<circle style='fill:#ffff00' cx='" << r.pointsX[upperMiddle] << "' cy='" << r.pointsY[upperMiddle] << "' r='3' />\n";
    // lower middle lip
    svgFile << "<circle style='fill:#ffff00' cx='" << r.pointsX[lowerMiddle] << "' cy='" << r.pointsY[lowerMiddle] << "' r='3' />\n";
    // left lip corner
    svgFile << "<circle style='fill:#ffff00' cx='" << r.pointsX[leftCorner] << "' cy='" << r.pointsY[leftCorner] << "' r='3' />\n";
    // right lip corner
    svgFile << "<circle style='fill:#ffff00' cx='" << r.pointsX[rightCorner] << "' cy='" << r.pointsY[rightCorner] << "' r='3' />\n";
    
    // upper lip
    svgFile << "<path d='M" << r.pointsX[leftCorner] << " " << r.pointsY[leftCorner] << " ";
    svgFile << "L " << r.pointsX[upperMiddle] << " " << r.pointsY[upperMiddle] << ", " << r.pointsX[rightCorner] << " " << r.pointsY[rightCorner];
    svgFile << "' fill='transparent' stroke='red' /> \n";

    // lower lip
    svgFile << "<path d='M" << r.pointsX[leftCorner] << " " << r.pointsY[leftCorner] << " ";
    svgFile << "L " << r.pointsX[lowerMiddle] << " " << r.pointsY[lowerMiddle] << ", " << r.pointsX[rightCorner] << " " << r.pointsY[rightCorner];
    svgFile << "' fill='transparent' stroke='red' /> \n";

    
}

void drawFace(struct Rough r, int mid, ofstream& svgFile)
{
    int vertRad;
    int horzRad;
    // top of nose: 10
    // bot of nose: 13
    vertRad = 2.4*(r.pointsY[13] - r.pointsY[10]);
    horzRad = (r.pointsX[9] - r.pointsX[13]) * 1.4;
    
    svgFile << "<circle style='fill:#ff00ff' cx='" << r.pointsX[13] << "' cy='" << r.pointsY[13] << "' r='3' />\n";
    svgFile << "<circle style='fill:#cc00cc' cx='" << r.pointsX[10] << "' cy='" << r.pointsY[10] << "' r='3' />\n";
    svgFile << "<circle style='fill:#990099' cx='" << r.pointsX[9] << "' cy='" << r.pointsY[9] << "' r='3' />\n";
    svgFile << "<circle style='fill:#550055' cx='" << r.pointsX[0] << "' cy='" << r.pointsY[0] << "' r='3' />\n";
    
    svgFile << "<ellipse style='opacity:0.3;fill:#ee9955' cx='" << r.pointsX[11] << "' cy='" << r.pointsY[11] << "' ";
    svgFile << "ry='" << vertRad << "' rx='" << horzRad << "'/> \n";
}



// Draws intraface markers
// return real center
Point drawMarkers(Mat intrafaceMarkers, struct Rough& r, float cFactor)
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
        
        switch (k)
        {
            case 11:
                r.centerCoord = Point(cx, cy);
                break;
            case 19:
                r.leftestEyeCorner = Point(cx, cy);
                cout << "$: " << cx << "," << cy << endl;
                break;
            case 20:
                r.highestLeftEyeCorner = Point(cx, cy);
                cout << "$: " << cx << "," << cy << endl;
                break;
            case 28:
                r.rightestEyeCorner = Point(cx, cy);
                cout << "$: " << cx << "," << cy << endl;
                break;
            case 27:
                r.highestRightEyeCorner = Point(cx, cy);
                cout << "$: " << cx << "," << cy << endl;
                break;
            case 40:
                r.lowerLipCoord = Point(cx, cy);
                cout << "$: " << cx << "," << cy << endl;
                break;
        }
        
        circle(r.testImage, Point(cx, cy), 2, Scalar(B,G,R), -1);
        cx = int(float(cx) * cFactor);
        cy = int(float(cy) * cFactor);
        
        r.pointsX[k] = cx;
        r.pointsY[k] = cy;
        
        //cout << cx << ", " << cy << " | ";
    }
    
    //cv::namedWindow(WINDOW_NAME);
    //cv::imshow(WINDOW_NAME, r.testImage);
    
    imwrite(r.markersJpg, r.testImage);
    cout << "blatantly" << endl;
    
    //Rect roi(intrafaceMarkers.at<float>(0,3), intrafaceMarkers.at<float>(1,3), intrafaceMarkers.at<float>(0,37), intrafaceMarkers.at<float>(1,37));
    //Mat croppedIm = r.testImage(roi);
    //imshow("croppedyo", croppedIm);
    //waitKey(0);
    
    // Press any key to close
    //cv::waitKey(0);
    
    return realCenter;
}






