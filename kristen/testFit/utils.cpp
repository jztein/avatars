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
    
    return false;
}
