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
//  TODO(kristen): Matching, Clean up.
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
            //fprintf(stderr, "%s\n", filename.c_str());
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
    /* PREPARE LIBRARY
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
    
    //*/
    
    // GET SHAPE CONTEXT OF INPUT IMAGE
    Mat contourIm = getDetectedEdges(original_im);
    EdgePoints input_edge_points = getPointsOnEdges(contourIm, 0);
    ShapeContext input_shape_context = getShapeContext(input_edge_points);
}


ShapeContext getShapeContext(EdgePoints info, string name)
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
        
        vector<float> x_vectors, y_vectors;
        vector<float> angles;   // angle compared to right X-axis.
        float x_pt = info.x_points[i];
        float y_pt = info.y_points[i];
        
        float xy_dist = sqrt(x_pt * x_pt + y_pt * y_pt);
        
        for (size_t j = 0; j < num_points; ++j)
        {
            if (i == j)
                continue;

            float x_j = info.x_points[j];
            float y_j = info.y_points[j];
            
            float xij = float(x_j - x_pt);
            float yij = float(y_j- y_pt);
            
            float xy_j_dist = sqrt(x_j * x_j + y_j * y_j);
            float angle = acos((x_pt * x_j + y_pt * y_pt)/ (xy_dist * xy_j_dist));
            angles.push_back(angle);
            
            // angle 1
            // angle 2
            // angle = angle 1 - angle 2
            
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
        vector_group.angles = angles;
        
        all_vector_groups.push_back(vector_group);
    }
    
    // Get mean distance of all vector pairs.
    float mean_distance = sum_distance / num_vector_pairs;
    
    // Bin everything into histograms. Each vector group gets its own histogram.
    vector<Histogram> all_histograms;
    
    // Only need to guarantee scale and translational invariance,
    //      and translational invariance is free.
    // Scale invariance: Normalize all vectors by mean distance.
    for (size_t i = 0; i < all_vector_groups.size(); ++i)
    {
        for (size_t j = 0; j < all_vector_groups[i].x_vectors.size(); ++j)
        {
            float new_x = all_vector_groups[i].x_vectors[j] / mean_distance;
            float new_y = all_vector_groups[i].y_vectors[j] / mean_distance;
            all_vector_groups[i].x_vectors[j] = new_x;
            all_vector_groups[i].y_vectors[j] = new_y;
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
    string context_filename;
    if (name.size() > 0)
    {
        context_filename = name;
    }
    else
    {
        context_filename = "hair_contexts/shape_contexts_" + imNum_str;
    }
    shape_context.filename = context_filename;
    context_filename += ".txt";
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
    
    drawShapeContext(shape_context);
    
    return shape_context;
}


Histogram getHistogram(PointVectors vector_group)
{
    Histogram histogram;
    
    // initialize histogram bins
    for (size_t i = 0; i < HISTOGRAM_BINS_PER_POINT; ++i)
    {
        histogram.bins[i] = 0;
    }
    
    // Do radial binning!
    float min_angle = 0.0f;
    float max_angle = 2 * 3.15;
    float bin_range = (max_angle - min_angle) / float(HISTOGRAM_BINS_PER_POINT);
    float bin_end;
    for (size_t i = 0; i < vector_group.vector_lengths.size(); ++i)
    {
        float cur_angle = vector_group.angles[i];
        int bin_num = 0;
        for (float bin_start = min_angle; bin_start < max_angle; bin_start += bin_range)
        {
            bin_end = bin_start + bin_range;
            
            if (cur_angle >= (bin_start - EPSILON) && cur_angle <= (bin_end + EPSILON))
            {
                histogram.bins[bin_num]++;
                break;
            }
            ++bin_num;
        }
    }
    
    // Each bin should be a fraction
    // fraction = (num vectors in bin) / (num vectors in vector_group)
    float num_vectors = (float) vector_group.angles.size();
    
    // Normalize bins
    for (size_t i = 0; i < HISTOGRAM_BINS_PER_POINT; ++i)
    {
        histogram.bins[i] /= num_vectors;
    }
    drawHistogram(histogram);

    return histogram;
}

void drawHistogram(Histogram histogram)
{
    int rows = 4;
    int cols = 4;//HISTOGRAM_BINS_PER_POINT;
    
    //cout << "rows: " << rows << ", cols: " << cols << endl;
    
    Mat hist_im(rows, cols, CV_8UC3);
    for (int r = 0; r < rows; ++r)
    {
        for (int c = 0; c < cols; ++c)
        {
            int color = (int) (255.0 * float(histogram.bins[r*c+c]));
            hist_im.at<Vec3b>(r,c) = Vec3b(color, color, color);
        }
    }
    /*
    Mat tmp;
    pyrUp(hist_im, tmp, Size(hist_im.cols*2, hist_im.rows*2) );
    pyrUp(tmp, tmp, Size(tmp.cols*2, tmp.rows*2) );
    pyrUp(tmp, tmp, Size(tmp.cols*2, tmp.rows*2) );
    pyrUp(tmp, tmp, Size(tmp.cols*2, tmp.rows*2) );
    pyrUp(tmp, hist_im, Size(tmp.cols*2, tmp.rows*2) );
     */
    //imshow("histogram image", hist_im);
    //waitKey(0);
    //exit(0);
    //imwrite("histogram image", hist_im);
}


void drawShapeContext(ShapeContext shape_context, string name)
{
    std::stringstream imNum_sstream;
    imNum_sstream << shape_context.im_id;
    string imNumString = imNum_sstream.str();
    string context_im_filename;
    if (name.size() > 0)
    {
        context_im_filename = name;
    }
    else
    {
        context_im_filename =
        "hair_contexts/drawn/context_points_" + imNumString + ".jpg";
    }
    
    int rows = int(shape_context.all_histograms.size());
    int cols = HISTOGRAM_BINS_PER_POINT;
    
    cout << "rows: " << rows << ", cols: " << cols << endl;
    
    Mat context_im(rows, cols, CV_32FC1);
    for (int r = 0; r < rows; ++r)
    {
        for (int c = 0; c < cols; ++c)
        {
            int color = 255 * shape_context.all_histograms[r].bins[c];
            context_im.at<Vec3b>(r,c) = Vec3b(color);
        }
    }
    
    Mat big_context_im, tmp;
    pyrUp(context_im, tmp, Size(context_im.cols*2, context_im.rows*2) );
    pyrUp(tmp, tmp, Size(tmp.cols*2, tmp.rows*2) );
    pyrUp(tmp, big_context_im, Size(tmp.cols*2, tmp.rows*2) );
    //imshow(context_im_filename, big_context_im);
    //waitKey(0);
    imwrite(context_im_filename, big_context_im);
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
EdgePoints getPointsOnEdges(Mat& contourIm, int imNum, string points_name, string svg_name)
{
    // SVG init.
    std::stringstream imNum_sstream;
    imNum_sstream << imNum;
    string imNumString = imNum_sstream.str();
    string points_filename;
    if (points_name.size() > 0)
    {
        points_filename = points_name;
    }
    else
    {
        points_filename = "hair_points/context_points_" + imNumString + ".txt";
    }
    
    string svg_filename;
    if (svg_name.size() > 0)
    {
        svg_filename = svg_name;
    }
    else
    {
        svg_filename = "hair_svgs/SVG_HAIR_" + imNumString + ".html";
    }
    
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
    if (scanFrequency < 0)
    {
        scanFrequency = -scanFrequency;
    }
    
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