/*
 *  Created by Yuxie Xiao on 8/16/17
 * _____________________________________________________________________________
 *
 * This program lets the user specify a list of images to extract specific 
 * letters/words from. The user must specify the strings they
 * would like to extract from each individual image.
 * This program will find the string, and break the string into
 * its words and letters, outputting the word and letter results.
 *
 * This program takes 5 command line argumerts:
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
 * 5. file of string filepaths: each image must have its own strings-to-find
 *                              text file. Strings should be newline separated
 *                              in the textfile. The paths to these textfiles
 *                              should be listed in this file. The paths 
 *                              should be listed in the order corresponding
 *                              to the one in the image-paths file.
 *
 * ____________________________________________________________________________
 */


#include "ocrExtraction.h"

using namespace std;

int main(int argc, char *argv[])
{
    RECERR rc;
    int err;
    
    string imageList;
    string findList;
    string outputFileLetter;
    string outputFileWord;
    string imageIn;
    string findIn;

    string mode;
    int modeInt;
    
    HPAGE  hPage;
    
    if (argc != 6)
    {
        printf("ERROR: requires 5 arguments:"
               "\n  1.file of image paths list"
               "\n  2.output filename for letters"
               "\n  3.output filename for words"
               "\n  4. -l or -w or -b to print letters only, words only, or both"
               "\n  5.file of to-find-strings paths list"
               "\n");
        return 1;
    }
    else
    {
        imageList = argv[1];
        outputFileLetter = argv[2];
        outputFileWord = argv[3];
        mode = argv[4];
        findList = argv[5];
    }
    
    if (mode == "-l") { modeInt = 0; }
    else if (mode == "-w") { modeInt = 1; }
    else if (mode == "-b") { modeInt = 2; }
    else 
    { 
        printf("ERROR, modes can only be -l, -w, or -b\n");
        return 1;
    }

    string imgPathLine;
    string findPathLine;
    string findLine;
    ifstream fileList(imageList); // open the file with the image paths
    ifstream findFileList(findList);

    while (getline(fileList, imgPathLine))
    {
    	getline(findFileList, findPathLine);
        imgPathLine.erase(remove(imgPathLine.begin(), imgPathLine.end(), '\n'),
                          imgPathLine.end());
        findPathLine.erase(remove(findPathLine.begin(), findPathLine.end(), '\n'),
                          findPathLine.end());

        // set the current image and the current file of strings to find
        imageIn = imgPathLine;
        findIn = findPathLine;

        err = setUp();
        if (err != 0)
        {
            printf("Unable to set up engine, quitting.\n");
            kRecQuit();
            return 1;
        }

    
        printf("processing file: %s\n\n", imageIn.c_str());
    	extractStrings(hPage, imageIn, outputFileLetter, outputFileWord,
    			       modeInt, findIn);
        
       
    }
    
    return 0;
}

