//
//  utils.cpp
//  IntraFace
//
//  Created by Kristen Aw on 4/3/14.
//  Copyright (c) 2014 IntraFace. All rights reserved.
//
#include "utils.h"

using namespace std;

bool parseArgs(int argc, char** argv, struct Rough& rough)
{
    if (argc < 2) // arguments come in pairs
    {
        // usage
        cout << "Usage:\nTesting SVG face making\n.\
                 roughFace -t <test_image>\n\
                 test_image should best be a screenshot of the subject's face as seen through a web cam."
             << endl;
        return false;
    }
    
    // still have arguments to parse
    // option key
    char* curArg = argv[0];
    
    // test image
    if (strcmp(curArg, "-t") == 0)
    {
        rough.testImage = imread(argv[1]);
        if (!rough.testImage.data)
        {
            cout << "Cannot read test image" << endl;
            return false;
        }
        rough.testImageName = string(argv[1]);
        parseArgs(argc-2, argv+2, rough);
    }
    // face align
    else if (strcmp(curArg, "--align") == 0)
    {
        cout << "moany" << endl;
        rough.align = true;
        parseArgs(argc-1, argv+1, rough);
    }
    else if (strcmp(curArg, "--html") == 0)
    {
        cout << "svg in html file: " << curArg << endl;
        rough.htmlFile = string(argv[1]);
        parseArgs(argc-2, argv+2, rough);
    }
    else if (strcmp(curArg, "--jpg") == 0)
    {
        cout << "intraface markers in jpg: " << curArg << endl;
        rough.markersJpg = string(argv[1]);
        parseArgs(argc-2, argv+2, rough);
    }
    else if (strcmp(curArg, "--kmeansJpg") == 0)
    {
        cout << "kmeans in jpg: " << curArg << endl;
        rough.kmeansJpg = string(argv[1]);
        parseArgs(argc-2, argv+2, rough);
    }
    else if (strcmp(curArg, "--kmeans") == 0)
    {
        cout << "do kmeans clustering for hair!" << endl;
        rough.doKmeans = true;
        parseArgs(argc-1, argv+1, rough);
    }
    
    return false;
}
