//
//  hair_shapeContexts.cpp
//  roughFace
//
//
// ============================ SHAPE CONTEXTS ============================
//
// 1. Canny edge detector to get edge contours of the hair.
// 2. Make-points method just like in hairShape.cpp, except use height or width
//      dependingon which is shorter since we can't be sure whether
//      or not the hair style is longer or shorter but we want as
//      many 'random' points as possible.
//
// Notes on invariance:
// - Translational invariance comes free with basic shape contexting.
// -  Scale invariance comes by normalizing the distance between all points
//      by the median.
// -  We don't want rotational invariance because we don't want upside-down
//      hair matching.
//
// Shape context getting:
// 1. Get histogram of vectors at each point.
// 2. Match histogram dissimilarity to determine corresponding points.
// 3. Find most overall similar library shape.
// Additional: Tangent angle dissimilarity - maybe?
//
// =========================================================================
//
//  Created by Kristen Aw on 7/26/14.
//  TODO(kristen): Clean up.
//
#include "hair_shapeContexts.h"
#include "roughFace.h"
#include <glob.h>
#include <sstream>
#include <vector>


vector<EdgePoints> getLibraryInfo(const string& globby_dir_name)
{
    vector<string> png_filenames;
    vector<Mat> library_images, edge_images;
    vector<EdgePoints> edge_points;
    glob_t glob_buffer;
    int error = glob(globby_dir_name.c_str(), 0, NULL, &glob_buffer);
    if (error == 0)
    {
        for (size_t i = 0; i < glob_buffer.gl_pathc; i++)
        {
            string filename(glob_buffer.gl_pathv[i]);
            fprintf(stderr, "%s\n", filename.c_str());
            png_filenames.push_back(filename);
            
            Mat im = imread(filename);
            library_images.push_back(im);
            Mat edges_im = getDetectedEdges(im);
            edge_images.push_back(edges_im);
            edge_points.push_back(getPointsOnEdges(edges_im, int(i)));
        }
        
        globfree(&glob_buffer);
    }
    else
    {
        fprintf(stderr, "ERROR: could not get glob");
        exit(1);
    }
    return edge_points;
}


// TODO: add argument: const string& globby_dir_name
void shapeContextsMain(struct Rough& rough, Mat original_im)
{
    // Get edge points, filename, and index of file.
    vector<EdgePoints> library_info = getLibraryInfo(
        "/Users/kristen/Documents/2_Toonchat/artTeam/Hairs_screenshot/*");
    
    vector<ShapeContext> shape_contexts;
    // Get shape contexts
    for (EdgePoints info: library_info)
    {
        shape_contexts.push_back(getShapeContext(info));
    }
    cout << "Finished getting " << shape_contexts.size() << " shape contexts." << endl;
    exit(0);
}


ShapeContext getShapeContext(EdgePoints info)
{
    ShapeContext shape_context;
    
    // 1. For every point get all n-1 vectors to other points.
    // 2. Bin these into histograms.
    // 3. YOLO.
    
    vector<PointVectors> all_vector_groups;
    float sum_distance = 0.0f;
    float num_vector_pairs = 0.0f;
    
    size_t num_points = info.x_points.size();
    for (size_t i = 0; i < num_points; ++i)
    {
        float min_distance = 9999999.0f;
        float max_distance = 0.0f;
        vector<float> lengths;
        
        vector<int> x_vectors, y_vectors;
        int x_pt = info.x_points[i];
        int y_pt = info.y_points[i];
        for (size_t j = 0; j < num_points; ++j)
        {
            if (i == j)
                continue;
            
            int xij = info.x_points[j] - x_pt;
            int yij = info.y_points[j] - y_pt;
            x_vectors.push_back(xij);
            y_vectors.push_back(yij);
            float vector_length = sqrt( float(xij * xij + yij * yij) );
            lengths.push_back(vector_length);
            if (min_distance > vector_length)
                min_distance = vector_length;
            if (max_distance < vector_length)
                max_distance = vector_length;
            sum_distance += vector_length;
            num_vector_pairs += 1.0f;
        }
        
        PointVectors vector_group;
        vector_group.x_vectors = x_vectors;
        vector_group.y_vectors = y_vectors;
        vector_group.min_length = min_distance;
        vector_group.max_length = max_distance;
        vector_group.vector_lengths = lengths;
        
        all_vector_groups.push_back(vector_group);
    }
    
    // Get mean distance of all vector pairs.
    int mean_distance = int(sum_distance / num_vector_pairs);
    
    // Bin everything into histograms. Each vector group gets its own histogram.
    vector<Histogram> all_histograms;
    
    // Only need to guarantee scale and translational invariance,
    //      and translational invariance is free.
    // Scale invariance: Normalize all vectors by mean distance.
    for (size_t i = 0; i < all_vector_groups.size(); ++i)
    {
        for (size_t j = 0; j < all_vector_groups[i].x_vectors.size(); ++j)
        {
            int cur_x = all_vector_groups[i].x_vectors[j];
            int cur_y = all_vector_groups[i].y_vectors[j];
            all_vector_groups[i].x_vectors[j] = cur_x / mean_distance;
            all_vector_groups[i].y_vectors[j] = cur_y / mean_distance;
        }
        all_histograms.push_back(getHistogram(all_vector_groups[i]));
    }
    
    // Concatenate all those histograms and we get a shape context.
    // Write into a text file. Each histogram on each line.
    shape_context.im_id = info.im_id;
    
    shape_context.all_histograms = all_histograms;
    
    std::stringstream imNum_sstream;
    imNum_sstream << info.im_id;
    string imNum_str = imNum_sstream.str();
    string context_filename = "hair_contexts/shape_contexts_" + imNum_str + ".txt";
    ofstream context_file(context_filename.c_str());
    
    for (size_t line = 0; line < all_histograms.size(); ++line)
    {
        for (size_t bin = 0; bin < HISTOGRAM_BINS_PER_POINT; bin++)
        {
            context_file << all_histograms[line].bins[bin] << " ";
        }
        context_file << "\n";
    }
    
    context_file.close();
    
    return shape_context;
}


Histogram getHistogram(PointVectors vector_group)
{
    Histogram histogram;
    
    // Each bin should be a fraction
    // fraction = (num vectors in bin) / (num vectors in vector_group)
    float num_vectors = (float) vector_group.vector_lengths.size();
    
    // Define bins
    float minimum = vector_group.min_length;
    float maximum = vector_group.max_length;
    float bin_range = (maximum - minimum) / float(HISTOGRAM_BINS_PER_POINT);
    float bin_end;
    for (size_t i = 0; i < vector_group.vector_lengths.size(); ++i)
    {
        float cur_length = vector_group.vector_lengths[i];
        bool got_binned = false;
        int bin_num = 0;
        for (float bin_start = minimum; bin_start < maximum; bin_start += bin_range)
        {
            bin_end = bin_start + bin_range;
            
            if (cur_length >= (bin_start - EPSILON) && cur_length <= (bin_end + EPSILON))
            {
                histogram.bins[bin_num]++;
                got_binned = true;
                break;
            }
            ++bin_num;
        }
        if (got_binned)
        {
            break;
        }
    }
    
    // Normalize bins
    for (size_t i = 0; i < HISTOGRAM_BINS_PER_POINT; ++i)
    {
        histogram.bins[i] /= num_vectors;
    }
    
    return histogram;
}


// More or less from hairShape.cpp
Mat getDetectedEdges(Mat src)
{
    Mat src_gray;
    cvtColor( src, src_gray, CV_BGR2GRAY );
    
    int thresh = 100;
    
    Mat canny_output;
    vector<vector<Point> > contours;
    vector<Vec4i> hierarchy;
    
    Canny( src_gray, canny_output, thresh, thresh*2, 3 );
    findContours(canny_output, contours, hierarchy, CV_RETR_TREE,
                 CV_CHAIN_APPROX_SIMPLE, Point(0, 0) );
    
    /// Draw contours
    Mat drawing = Mat::zeros( canny_output.size(), CV_8UC3 );
    for( int i = 0; i< contours.size(); i++ )
    {
        Scalar color = Scalar(255,255,255);
        drawContours( drawing, contours, i, color, 2, 8, hierarchy, 0, Point() );
    }
    
    //namedWindow( "Contours", CV_WINDOW_AUTOSIZE );
    //imshow( "Contours", drawing );
    //waitKey(0);
    
    return drawing;
}


// More or less from hairShape.cpp
EdgePoints getPointsOnEdges(Mat& contourIm, int imNum)
{
    // SVG init.
    std::stringstream imNum_sstream;
    imNum_sstream << imNum;
    string imNumString = imNum_sstream.str();
    string points_filename = "hair_points/context_points_" + imNumString + ".txt";
    string svg_filename = "hair_svgs/SVG_HAIR_" + imNumString + ".html";
    ofstream svgFile(svg_filename.c_str());
    ofstream points_file(points_filename.c_str());
    
    int cSvgWidth = contourIm.cols - 1;
    int cSvgHeight = contourIm.rows - 1;
    
    // SVG init
    svgFile << "<!DOCTYPE html>\n<html>\n<body>\n<svg width='" << cSvgWidth << "' height='"
        << cSvgHeight << "' xmlns='http://www.w3.org/2000/svg' xmlns:xlink= 'http://www.w3.org/1999/xlink' viewBox='0 0 "
        << cSvgWidth << " " << cSvgHeight << "'> \n <defs>\n</defs>\n";
    
    // contour line = white
    // rest = black
    
    int startPtX = 0;
    int startPtY = 0;
    
    // used to estimate radius of circle
    int lowestY = 0, highestY = contourIm.rows - 1;
    int leftestX = contourIm.cols - 1, rightestX = 0;
    
    Vec3b whiteOutColor(255,255,255);
    Vec3b blackOutColor(0,0,0);
    Vec3b curColor;
    for (int r = 0; r < contourIm.rows; ++r)
    {
        for (int c = 0; c < contourIm.cols; ++c)
        {
            curColor = contourIm.at<Vec3b>(r,c);
            if (curColor == whiteOutColor)
            {
                if (lowestY < r)
                    lowestY = r;
                if (highestY > r)
                    highestY = r;
                if (leftestX > c)
                    leftestX = c;
                if (rightestX < c)
                    rightestX = c;
                
                if (startPtX == 0 && startPtY == 0) {
                    startPtX = c;
                    startPtY = r;
                }
            }
        }
    }
    
    svgFile << "<circle style='fill:#0000ff' cx='" << startPtX << "' cy='" << startPtY
        << "' r='3' />\n";
    
    vector<int> xPoints, yPoints;
    xPoints.push_back(startPtX);
    yPoints.push_back(startPtY);
    
    // Do a scan to get points
    
    int scanFrequency = int(float(lowestY - highestY) / SCAN_FREQUENCY);
    for (int r = 0; r < contourIm.rows; r+=scanFrequency)
    {
        for (int c = 0; c < contourIm.cols; ++c)
        {
            curColor = contourIm.at<Vec3b>(r,c);
            if (curColor == whiteOutColor)
            {
                // save found point
                xPoints.push_back(c);
                yPoints.push_back(r);
                
                // go to next row
                c += scanFrequency;
            }
        }
    }
    
    cout << "Points: " << xPoints.size() << endl;
    
    for (size_t i = 0; i < xPoints.size(); ++i)
    {
        // Draw red points.
        svgFile << "<circle style='fill:#0000ff' cx='" << xPoints[i] << "' cy='"
            << yPoints[i] << "' r='3' />\n";
        points_file << xPoints[i] << "," << yPoints[i] << "\n";
    }
    
    // SVG clean up
    svgFile << "</svg>\n</body>\n</html>\n";
    
    svgFile.close();
    points_file.close();
    
    struct EdgePoints im_edge_points;
    im_edge_points.x_points = xPoints;
    im_edge_points.y_points = yPoints;
    im_edge_points.filename = points_filename;
    im_edge_points.im_id = imNum;
    return im_edge_points;
}