//
//  roughFace.cpp
//
//  Created by Kristen Aw on 4/3/14.
//

#include "roughFace.h"
#include "utils.h"
#include "noseFinder.h"

//
//  Some INTRAFACE setup code adapted from
//  main.cpp, IntraFace
//  Created by Marc Estruch Tena on 20/12/13.
//  Copyright (c) 2013 IntraFace. All rights reserved.
//

// for INTRAFACE::FaceAlignment::Detect()
bool compareRect(Rect r1, Rect r2) { return r1.height < r2.height; }
void contouring(Mat im, Mat& intrafaceMarkers);

int main(int argc, char** argv)
{
    srand(unsigned(time(NULL)));
    
    //FlannBasedMatcher m;
    struct Rough r;
    
    while (parseArgs(argc-1, argv+1, r))
        ;

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
    
    Mat oriIm;
    r.testImage.copyTo(oriIm);
    
    // cv::Mat::r.testImage is captured webcam image from user's feed
    int frameWidth = r.testImage.size[0];
    
    cascadeClassifier.detectMultiScale(r.testImage, faces, 1.2, 2, 0, Size(50, 50));
    
    ofstream svgFile("rough_hair1.html");
    initSVG(svgFile);
    
    Point realCenter;
    float cFactor = SVG_WIDTH / float(frameWidth);
    float score;
    float notFaceScore = 0.5;
    if (faceAligner->Detect(r.testImage, faces[0], intrafaceMarkers, score) == INTRAFACE::IF_OK)
    {
        r.intrafaceMarkers = intrafaceMarkers;
        if (score >= notFaceScore) {
            realCenter = drawMarkers(intrafaceMarkers, r, cFactor);
        }
        
        drawFeatures(realCenter, r, svgFile);
    }
    
    cleanupSVG(svgFile);
    
    cout << "dungus" << endl;
    
    contouring(oriIm, intrafaceMarkers);
    
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
        
        circle(r.testImage, Point(cx, cy), 2, Scalar(B,G,R), -1);
        cx = int(float(cx) * cFactor);
        cy = int(float(cy) * cFactor);
        
        r.pointsX[k] = cx;
        r.pointsY[k] = cy;
        
        cout << cx << ", " << cy << " | ";
    }
    cout << endl;
    
    //cv::namedWindow(WINDOW_NAME);
    //cv::imshow(WINDOW_NAME, r.testImage);
    
    imwrite(NAME_SEE_MARKERS, r.testImage);
    
    //Rect roi(intrafaceMarkers.at<float>(0,3), intrafaceMarkers.at<float>(1,3), intrafaceMarkers.at<float>(0,37), intrafaceMarkers.at<float>(1,37));
    //Mat croppedIm = r.testImage(roi);
    //imshow("croppedyo", croppedIm);
    //waitKey(0);
    
    // Press any key to close
    //cv::waitKey(0);
    
    return realCenter;
}

struct Pixel{
    Vec3b bgr;
    int x, y;
};

int distKmeans(struct Pixel a, struct Pixel centroid)
{
    int b, g, r;
    b = int(a.bgr[0] - centroid.bgr[0]);
    g = int(a.bgr[1] - centroid.bgr[1]);
    r = int(a.bgr[2] - centroid.bgr[2]);

//    int x2 = a.x - centroid.x;
//    int y2 = a.y - centroid.y;
//    return ((b*b + g*g + r*r) + x2*x2 + y2*y2);
    return ((b*b + g*g + r*r));
}

void contouring(Mat im, Mat& intrafaceMarkers)
{
    // do k-means
    
    // 1) background, hair, facae
//    int k = 3;
    
    // downsample image for k-means clustering speed up.
    Mat downIm, tmp;
    pyrDown(im, tmp, Size(im.cols/2, im.rows/2) );
    pyrDown(tmp, tmp, Size(tmp.cols/2, tmp.rows/2) );
    pyrDown(tmp, downIm, Size(tmp.cols/2, tmp.rows/2) );
    
    imshow("downsampled im", downIm);
    waitKey();
    
    // convert image to struct Pixels for easy dist calculation
    int numPixels = downIm.cols * downIm.rows;
    Pixel* pixels = new Pixel[numPixels];
    Pixel* cur = NULL;
    int downImCols = downIm.cols;
    int c;
    for (int r = 0; r < downIm.rows; ++r)
    {
        for (c = 0; c < downImCols; ++c)
        {
            cur = pixels + (r*downImCols + c);
            cur->x = c;
            cur->y = r;
            cur->bgr = downIm.at<Vec3b>(r,c);
        }
    }
    
    // debug by printing stuff
//    cout << "PIXEL LAST\n" << cur->x << ", " << cur->y << ": (" << cur->bgr << endl;
//    cur = pixels;
//    cout << "PIXEL FIRST\n" << cur->x << ", " << cur->y << ": (" << cur->bgr << endl;
//    
//    Mat reassembledIm(downIm.rows, downIm.cols, downIm.type());
//    cur = pixels;
//    for (int r = 0; r < downIm.rows; ++r)
//    {
//        for (int c = 0; c < downImCols; ++c)
//        {
//            reassembledIm.at<Vec3b>(r,c) = cur->bgr;
//            cur++;
//        }
//    }
//    
//    imshow("reassembled im", reassembledIm);
//    waitKey();
    
    // debug distKmeans()
//    Pixel p1, p2;
//    p1.bgr = Vec3b(2,0,0); p2.bgr = Vec3b(3,0,0);
//    p1.x = 0; p2.x = 1;
//    p1.y = 0; p2.y = 1;
    
    // initialize k random centroids
    
    // seed rand in start of main()
    const int K = 8;
    Pixel centroids[K];
    int centroidIndexes[K];
    int randIdx = 0;
    for (int k = 0; k < K; ++k)
    {
        randIdx = rand() % numPixels;
        centroidIndexes[k] = randIdx;
        centroids[k] = pixels[randIdx];
    }
    
    // get clusters
    vector<Pixel*> clusters[K];
    size_t sizes[K]; // cluster sizes
    
    int iterations = 0;
    
    while (1)
    {
        ++iterations;
        
        // save old
        vector<Pixel*> oldClusters0(clusters[0]);
        vector<Pixel*> oldClusters1(clusters[1]);
        vector<Pixel*> oldClusters2(clusters[2]);
        vector<Pixel*> oldClusters3(clusters[3]);
        vector<Pixel*> oldClusters4(clusters[4]);
        vector<Pixel*> oldClusters5(clusters[5]);
        vector<Pixel*> oldClusters6(clusters[6]);
        vector<Pixel*> oldClusters7(clusters[7]);
        vector<Pixel*> oldClusters[K];
        oldClusters[0] = oldClusters0;
        oldClusters[1] = oldClusters1;
        oldClusters[2] = oldClusters2;
        oldClusters[3] = oldClusters3;
        oldClusters[4] = oldClusters4;
        oldClusters[5] = oldClusters5;
        oldClusters[6] = oldClusters6;
        oldClusters[7] = oldClusters7;
        
        for (int k = 0; k < K; ++k)
        {
            clusters[k].clear();
        }
        
        Pixel cs[K];
        int dists[K];
        for (int k = 0; k < K; ++k)
        {
            cs[k] = centroids[k];
        }
        
        for (int i = 0; i < numPixels; ++i)
        {
            // current pixel
            cur = pixels + i;
            
            // find nearest centroid to current pixel
            for (int k = 0; k < K; ++k)
            {
                dists[k] = distKmeans(*cur, cs[k]);
            }
            
            if (dists[0] < dists[1] && dists[0] < dists[2] && dists[0] < dists[3] && dists[0] < dists[4]&& dists[0] < dists[5] && dists[0] < dists[6]&& dists[0] < dists[7])
            {
                clusters[0].push_back(cur);
            }
            else if (dists[1] < dists[2] && dists[1] < dists[3] && dists[1] < dists[4]
                     && dists[1] < dists[5] && dists[1] < dists[6]&& dists[1] < dists[7])
            {
                clusters[1].push_back(cur);
            }
            else if (dists[2] < dists[3] && dists[2] < dists[4]
                     && dists[2] < dists[5] && dists[2] < dists[6]&& dists[2] < dists[7])
            {
                clusters[2].push_back(cur);
            }
            else if (dists[3] < dists[4]
                     && dists[3] < dists[5] && dists[3] < dists[6]&& dists[3] < dists[7])
            {
                clusters[3].push_back(cur);
            }
            else if (dists[4] < dists[5] && dists[4] < dists[6]&& dists[4] < dists[7])
            {
                clusters[4].push_back(cur);
            }
            else if (dists[5] < dists[6]&& dists[5] < dists[7])
            {
                clusters[5].push_back(cur);
            }
            else if (dists[6] < dists[7])
            {
                clusters[6].push_back(cur);
            }
            else
            {
                clusters[7].push_back(cur);
            }
            
        }
        
        // calculate new centroids (center of mass of clusters)
        for (int k = 0; k < K; ++k)
        {
            sizes[k] = clusters[k].size();
        }
        for (int k = 0; k < K; ++k)
        {
            size_t clusterSize = sizes[k];
            float cSize = float(clusterSize);
            float bTotal = 0, gTotal = 0, rTotal = 0, xTotal = 0, yTotal = 0;
            for (size_t i = 0; i < clusterSize; ++i)
            {
                bTotal += float(clusters[k][i]->bgr[0]);
                gTotal += float(clusters[k][i]->bgr[1]);
                rTotal += float(clusters[k][i]->bgr[2]);
                xTotal += float(clusters[k][i]->x);
                yTotal += float(clusters[k][i]->y);
            }
            
            centroids[k].bgr = Vec3b(int(bTotal / cSize),
                                     int(gTotal / cSize),
                                     int(rTotal / cSize));
            centroids[k].x = int(xTotal / cSize);
            centroids[k].y = int(yTotal / cSize);
        }
        
        // repeat until clusters don't change
        bool didntChange = true;
        for (int k = 0; k < K; ++k)
        {
            if (clusters[k].size() != oldClusters[k].size())
            {
                didntChange = false;
                break;
            }
        }
        if (didntChange)
        {
            // k means finished!
            cout << "K means finished! " << clusters[0].size() << ", " << clusters[1].size() << ", " << clusters[2].size()<< ", " << clusters[3].size()<< ", " << clusters[4].size() << endl;
            cout << "K means iterations: " << iterations << endl;
            break;

        }
    }
    
    // mark pixels in each cluster with cluster color
    size_t clusterSize = sizes[0];
    for (size_t i = 0; i < clusterSize; ++i)
    {
        clusters[0][i]->bgr = Vec3b(255,0,0);
    }
    clusterSize = sizes[1];
    for (size_t i = 0; i < clusterSize; ++i)
    {
        clusters[1][i]->bgr = Vec3b(0,255,0);
    }
    clusterSize = sizes[2];
    for (size_t i = 0; i < clusterSize; ++i)
    {
        clusters[2][i]->bgr = Vec3b(0,0,255);
    }
    clusterSize = sizes[3];
    for (size_t i = 0; i < clusterSize; ++i)
    {
        clusters[3][i]->bgr = Vec3b(0,255,255);
    }
    clusterSize = sizes[4];
    for (size_t i = 0; i < clusterSize; ++i)
    {
        clusters[4][i]->bgr = Vec3b(255,0,255);
    }
    clusterSize = sizes[5];
    for (size_t i = 0; i < clusterSize; ++i)
    {
        clusters[5][i]->bgr = Vec3b(255,255,0);
    }
    clusterSize = sizes[6];
    for (size_t i = 0; i < clusterSize; ++i)
    {
        clusters[6][i]->bgr = Vec3b(100,40,255);
    }
    clusterSize = sizes[7];
    for (size_t i = 0; i < clusterSize; ++i)
    {
        clusters[7][i]->bgr = Vec3b(30,20,150);
    }
    
    cout << "PIXEL LAST\n" << cur->x << ", " << cur->y << ": (" << cur->bgr << endl;
    cur = pixels;
    cout << "PIXEL FIRST\n" << cur->x << ", " << cur->y << ": (" << cur->bgr << endl;
    
    Mat reassembledIm(downIm.rows, downIm.cols, downIm.type());
    cur = pixels;
    for (int r = 0; r < downIm.rows; ++r)
    {
        for (int c = 0; c < downImCols; ++c)
        {
            reassembledIm.at<Vec3b>(r,c) = cur->bgr;
            cur++;
        }
    }
    imshow("reassembled im", reassembledIm);
    waitKey();
    //imwrite("kmeans_stp.png", reassembledIm);
    
    
    // cleanup
    delete [] pixels;
}

// hair
void contouring2(Mat im, Mat& intrafaceMarkers)
{
    // get general shape of hair
    Mat grayIm;
    cvtColor(im, grayIm, CV_BGR2GRAY);
    Mat dst = Mat::zeros(im.rows, im.cols, CV_8UC3);
    
    Mat threshIm = Mat::zeros(grayIm.rows, grayIm.cols, CV_8UC1);
    
    threshold(grayIm, threshIm, 30, 255, THRESH_BINARY & THRESH_OTSU);
    
    //imshow("thresholded", threshIm);
    //waitKey();
    
    Mat ele = getStructuringElement(MORPH_RECT, Size(25,25));//Size(15,15));
    Mat openIm;
    morphologyEx(threshIm, openIm, MORPH_OPEN, ele);
    
    Mat dilatedIm = openIm;
    
    /*
    Mat element = getStructuringElement(MORPH_RECT, Size(2,2));
    Mat dilatedIm;
    //morphologyEx(threshIm, openIm, MORPH_CLOSE, element);
    erode (openIm, dilatedIm, element);
    //imshow("dilated", dilatedIm);
    //waitKey();
    //*/
    
    imshow("opened and dilated", openIm);
    //waitKey();
    
    // use markers to remove face region naively
    Point2d topleft, topright, botleft, botright;
    topleft.x = (int)intrafaceMarkers.at<float>(0,0);
    topleft.y = (int)intrafaceMarkers.at<float>(1,0);
    topright.x = (int)intrafaceMarkers.at<float>(0,9);
    topright.y = (int)intrafaceMarkers.at<float>(1,9);
    botleft.x = (int)intrafaceMarkers.at<float>(0,0);
    botleft.y = (int)intrafaceMarkers.at<float>(1,40);
    botright.x = (int)intrafaceMarkers.at<float>(0,9);
    botright.y = (int)intrafaceMarkers.at<float>(1,40);
    cout << topleft << ", " << topright << ", " << botleft << ", " << botright << endl;
    
    for (int r = 0; r < dilatedIm.rows; ++r)
    {
        if (r <= botleft.y && r >= topleft.y)
        {
            for (int c = 0; c < dilatedIm.cols; ++c)
            {
                if (c >= topleft.x && c <= topright.x)
                {
                    circle(dilatedIm, Point(c,r), 2, Scalar(255), -1);
                    //cout << "burn" << endl;
                    //dilatedIm.at<int>(r,c) = 50;
                }
            }
        }
        
    }
    
    Mat invertedIm(dilatedIm.rows, dilatedIm.cols, CV_8U);
    invertedIm = Scalar(255);
    invertedIm -= dilatedIm;
    
    
    Mat markers(invertedIm.rows, invertedIm.cols, CV_32SC1);
    for (int r = 0; r < invertedIm.rows; ++r)
    {
        for (int c = 0; c < invertedIm.cols; ++c)
        {
            if (invertedIm.at<uchar>(r, c) == 255)
            {
                markers.at<int>(r, c) = 2;
            }
            else
            {
                markers.at<int>(r, c) = 1;
            }
        }
    }
    //imwrite("markers_s.jpg", markers);
    
    
    
    // do watershed with specifically initialized markers
    watershed(im, markers);
    //imshow("watersheded markers", markers);
    //waitKey();
    
    Mat boundedIm = im.clone();
    for (int r = 0; r < markers.rows; ++r)
    {
        for (int c = 0; c < markers.cols; ++c)
        {
            if (markers.at<int>(r, c) == -1)
            {
                boundedIm.at<Vec3b>(r,c) = Vec3b(255,0,0);
            }
        }
    }
    imshow("bounded im", boundedIm);
    waitKey();
    
}

