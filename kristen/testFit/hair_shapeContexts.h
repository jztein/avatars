//
//  hair_shapeContexts.h
//  roughFace
//
//  Created by Kristen Aw on 7/26/14.
//

#ifndef __roughFace__hair_shapeContexts__
#define __roughFace__hair_shapeContexts__

#include <iostream>
#include "roughFace.h"

#define SCAN_FREQUENCY 20.0f
#define HISTOGRAM_BINS_PER_POINT 10
#define EPSILON 1e-6

struct EdgePoints {
    vector<int> x_points;
    vector<int> y_points;
    
    string filename;
    int im_id;
};

struct Histogram {
    float bins[HISTOGRAM_BINS_PER_POINT];
};

struct ShapeContext{
    int im_id;
    
    // vector of PointVector histograms
    vector<Histogram> all_histograms;
    
    // filename of recorded shape context
    // One histogram per line, each histogram
    //      has HISTOGRAM_BINS_PER_POINT bins.
    string filename;
};

struct PointVectors {
    vector<int> x_vectors;
    vector<int> y_vectors;
    
    vector<float> vector_lengths;
    float min_length;  // minimum vector length
    float max_length;  // maximum vector length
};

Histogram getHistogram(PointVectors vector_group);

ShapeContext getShapeContext(EdgePoints info);

Mat getDetectedEdges(Mat src);

// Get library of png files from a dir
// Make shape context of each png png file
// Save all shape contexts.
vector<EdgePoints> getLibraryInfo(const string& globby_dir_name);

EdgePoints getPointsOnEdges(Mat& contourIm, int imNum);

void shapeContextsMain(struct Rough& rough, Mat original_im);

#endif /* defined(__roughFace__hair_shapeContexts__) */
