/*
 *  Created by Yuxie Xiao on 8/10/17
 * _____________________________________________________________________________
 *
 * This program lets the user specify a list of images to extract
 * letters/words from. 
 *
 * This program takes 4 command line argumerts:
 *
 * 1. file of image paths: paths to images should be listed in
 *                         this file. The paths should be newline separated
 *
 * 2. letter output file:  name of file to write the letter info to
 *
 * 3. word output file:    name of file to write the word info to
 *
 * 4. output mode:         the mode should be one of the following:
 *                              -l : export letters only
 *                              -w : export words only
 *                              -b : export both letters and words
 *
 * All letters/words recognized by the ocr for each image will be exported
 *
 * ____________________________________________________________________________
 */
 

#include "ocrExtraction.h"

using namespace std;


/* Run the word/letter extractor for all files in the given fileList
 */
int main(int argc, char *argv[])
{
    
    RECERR rc;
    int err;
    
    string imageList;
    string outputFileLetter;
    string outputFileWord;
    string imageIn;
    string mode;
    int modeInt;
    
    HPAGE  hPage;
    
    if (argc != 5)
    {
        printf("ERROR: requires 4 arguments:"
               "\n  1.file of image paths list"
               "\n  2.output filename for letters"
               "\n  3.output filename for words"
               "\n  4. -l or -w or -b to print letters only, words only, or both"
               "\n");
        return 1;
    }
    else
    {
        imageList = argv[1];
        outputFileLetter = argv[2];
        outputFileWord = argv[3];
        mode = argv[4];
    }
    
    if (mode == "-l") { modeInt = 0; }
    else if (mode == "-w") { modeInt = 1; }
    else if (mode == "-b") { modeInt = 2; }
    else 
    { 
        printf("ERROR, modes can only be -l, -w, or -b\n");
        return 1;
    }

    string line;
    ifstream fileList(imageList); // open the file with the image paths
    while (getline(fileList, line))
    {
        line.erase(remove(line.begin(), line.end(), '\n'),
                          line.end());
        imageIn = line;
        err = setUp();
        if (err != 0)
        {
            printf("Unable to set up engine, quitting.\n");
            kRecQuit();
            return 1;
        }
        // process each image file individually
        extractAll(hPage, imageIn, outputFileLetter, outputFileWord, modeInt);
    }
    
    return 0;
}
