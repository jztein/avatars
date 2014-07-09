//
//  hairShape.cpp
//  roughFace
//
//  Created by Kristen Aw on 7/8/14.
//  Copyright (c) 2014 IntraFace. All rights reserved.
//

#include "hairShape.h"



int distKmeans(struct Pixel a, struct Pixel centroid)
{
    int b, g, r;
    b = int(a.bgr[0] - centroid.bgr[0]);
    g = int(a.bgr[1] - centroid.bgr[1]);
    r = int(a.bgr[2] - centroid.bgr[2]);
    
    int x2 = a.x - centroid.x;
    int y2 = a.y - centroid.y;
    return ((b*b + g*g + r*r) + x2*x2 + y2*y2);
    //    return ((b*b + g*g + r*r));
}

float sign(Point p1, Point p2, Point p3)
{
    return (p1.x - p3.x) * (p2.y - p3.y) - (p2.x - p3.x) * (p1.y - p3.y);
}

bool PointInTriangle(Point pt, Point v1, Point v2, Point v3)
{
    bool b1, b2, b3;
    
    b1 = sign(pt, v1, v2) < 0.0f;
    b2 = sign(pt, v2, v3) < 0.0f;
    b3 = sign(pt, v3, v1) < 0.0f;
    
    return ((b1 == b2) && (b2 == b3));
}

bool pointIsInTriangle(int r, int c, struct Rough& rough)
{
    Point v1, v2, v3;
    v1 = Point(rough.leftestEyeCorner.x, rough.highestLeftEyeCorner.y);
    v2 = Point(rough.rightestEyeCorner.x, rough.highestRightEyeCorner.y);
    v3 = Point(rough.lowerLipCoord.x, rough.lowerLipCoord.y);
    
    Point pt = Point(c, r);
    
    return PointInTriangle(pt, v1, v2, v3);
}

void clustering(Mat im, Mat& intrafaceMarkers, struct Rough& rough)
{
    // Estimated boxing
    Point guessedLeft, guessedRight, guessedTop;
    guessedLeft = rough.leftestEyeCorner - (rough.centerCoord - rough.leftestEyeCorner);
    guessedRight = rough.rightestEyeCorner + (rough.rightestEyeCorner - rough.centerCoord);
    guessedTop = rough.centerCoord - 2*(rough.lowerLipCoord - rough.centerCoord);
    
    // Block out estimated face so that eyes/ specs won't get in the way
    // Assumption: color of center of nose should be neutral face color
    Vec3b neutralColor = im.at<Vec3b>(rough.centerCoord.y, rough.centerCoord.x);
    neutralColor *= 0.9;
    Vec3b whiteOutColor(255, 255,255);
    //neutralColor = Vec3b(255, 255, 255);
    
    for (int r = 0; r < im.rows; ++r)
    {
        for (int c = 0; c < im.cols; ++c)
        {
            if (c <= guessedLeft.x || c >= guessedRight.x || r <= guessedTop.y) {
                im.at<Vec3b>(r,c) = whiteOutColor;
                continue;
            }
            if (pointIsInTriangle(r,c, rough)) {
                im.at<Vec3b>(r,c) = neutralColor;
            }
        }
    }
    
    //imshow("neutralized face", im);
    //waitKey();
    //exit(0);
    
    // Do k-means
    
    // 1) background, hair, facae
    //    int k = 3;
    
    // downsample image for k-means clustering speed up.
    Mat downIm, tmp;
    pyrDown(im, tmp, Size(im.cols/2, im.rows/2) );
    pyrDown(tmp, tmp, Size(tmp.cols/2, tmp.rows/2) );
    pyrDown(tmp, downIm, Size(tmp.cols/2, tmp.rows/2) );
    
    //imshow("downsampled im", downIm);
    //waitKey();
    
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
    
    // mark pixels in each cluster with cluster color for display
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
    Mat upIm;
    pyrUp(reassembledIm, tmp, Size(reassembledIm.cols*2, reassembledIm.rows*2) );
    pyrUp(tmp, upIm, Size(tmp.cols*2, tmp.rows*2) );
    pyrUp(tmp, upIm, Size(tmp.cols*2, tmp.rows*2) );
    /*
    imshow("reassembled upsampled im", upIm);
    waitKey();
    //*/
    /*
    imshow("ori im", im);
    waitKey();
    //*/
    
    
    // IDEA:
    // 1. guess dominant (largest) hair cluster by polling estimated hair positions
    // relative to intraface markers, and take mode cluster as dominant-hair-cluster
    // 2. take (dist with pixel position) clusters to figure out whole hair piece
    // in conjunction with 1.'s cluster (i.e. if there are 2 clusters that make up whole hair) OR use hair outline with Canny edge detection
    // 3. use intraface markers to kill off eyebrows and mouth noise.
    // 4. get contour around this combined hair cluster
    
    // Guess hair cluster color.
    // We can poll for more points and estimate multiple OR dominate hair cluster
    // but for now, keep it simple.
    guessedTop = 0.5 * (rough.centerCoord - 1.3*(rough.lowerLipCoord - rough.centerCoord));
    guessedLeft = 0.5 * (rough.leftestEyeCorner - 0.8*(rough.centerCoord - rough.leftestEyeCorner));
    guessedRight = 0.5 * (rough.rightestEyeCorner + 0.8*(rough.rightestEyeCorner - rough.centerCoord));
    Vec3b hairClusterColor1 = upIm.at<Vec3b>(guessedTop.y, guessedTop.x);
    Vec3b hairClusterColor1b = upIm.at<Vec3b>(guessedTop.y-10, guessedTop.x+10);
    Vec3b hairClusterColor1c = upIm.at<Vec3b>(guessedTop.y-10, guessedTop.x+10);
    cout << hairClusterColor1 << hairClusterColor1b << hairClusterColor1c << endl;
    Vec3b hairClusterColor2 = upIm.at<Vec3b>(rough.centerCoord.y, guessedLeft.x);
    Vec3b hairClusterColor3 = upIm.at<Vec3b>(rough.centerCoord.y, guessedRight.x);
    
    Vec3b dominantHair;
    int numLikeH1 = 0, numLikeH2 = 0, numLikeH3 = 0;
    if (hairClusterColor2 == hairClusterColor1)
    {
        numLikeH1++;
        numLikeH2++;
    }
    if (hairClusterColor3 == hairClusterColor1)
    {
        numLikeH1++;
        numLikeH3++;
    }
    if (hairClusterColor2 == hairClusterColor3)
    {
        numLikeH2++;
        numLikeH3++;
    }
    if (hairClusterColor1 == hairClusterColor1b)
    {
        numLikeH1++;
    }
    if (hairClusterColor2 == hairClusterColor1b)
    {
        numLikeH2++;
    }
    if (hairClusterColor3 == hairClusterColor1b)
    {
        numLikeH3++;
    }
    if (hairClusterColor1 == hairClusterColor1c)
    {
        numLikeH1++;
    }
    if (hairClusterColor2 == hairClusterColor1c)
    {
        numLikeH2++;
    }
    if (hairClusterColor3 == hairClusterColor1c)
    {
        numLikeH3++;
    }
    
    if (numLikeH1 >= numLikeH2 && numLikeH1 >= numLikeH3)
    {
        dominantHair = hairClusterColor1;
    }
    else if (numLikeH2 >= numLikeH3)
    {
        dominantHair = hairClusterColor2;
    }
    else
    {
        dominantHair = hairClusterColor3;
    }
    
    
    Vec3b curColor;
    Vec3b blackOutColor(0,0,0);
    for (int r = 0; r < upIm.rows; ++r)
    {
        for (int c = 0; c < upIm.cols; ++c)
        {
            curColor = upIm.at<Vec3b>(r,c);
            if (curColor != dominantHair)
            {
                upIm.at<Vec3b>(r,c) = whiteOutColor;
            }
            else
            {
                upIm.at<Vec3b>(r,c) = blackOutColor;
            }
        }
    }
    imwrite(rough.kmeansJpg, upIm);
    
    // draw estimated hair blob shapes
    
    Mat contourIm = getContours(upIm);
    
    
    
    // cleanup
    delete [] pixels;
    
}

Mat getContours(Mat src)
{
    Mat src_gray;
    cvtColor( src, src_gray, CV_BGR2GRAY );
    
    int thresh = 100;
    
    Mat canny_output;
    vector<vector<Point> > contours;
    vector<Vec4i> hierarchy;
    
    /// Detect edges using canny
    Canny( src_gray, canny_output, thresh, thresh*2, 3 );
    /// Find contours
    findContours( canny_output, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0) );
    
    /// Draw contours
    Mat drawing = Mat::zeros( canny_output.size(), CV_8UC3 );
    for( int i = 0; i< contours.size(); i++ )
    {
        Scalar color = Scalar(255,255,255);
        drawContours( drawing, contours, i, color, 2, 8, hierarchy, 0, Point() );
    }
    
    /*// Show in a window
    namedWindow( "Contours", CV_WINDOW_AUTOSIZE );
    imshow( "Contours", drawing );
    waitKey();
    //*/
    
    return drawing;
}


void drawSvg(Mat& contourIm)
{
    ;
}
















































///////// ************************* /////////
/////////                           /////////
/////////        DEPRECATED         /////////
/////////                           /////////
///////// ************************* /////////































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
