/***************************************************************************
 * Nuance OmniPage Capture SDK 19.2
 * Created by Yuxie Xiao on 8/9/17
 * This is the header file for the ocrExtraction.cpp program
 ***************************************************************************/

// Please set the next macro to 0 if you want to use a non-OEM license!
#define USE_OEM_LICENSE 1

#if USE_OEM_LICENSE
// TODO: enter path to your license file below
#define LICENSE_FILE "path goes here" 
// TODO: put name of the header file defining your OEM Code as OEM_CODE below
#include "path goes here" 

#endif

// See the documentation of kRecInit about the use of company and product names
#define YOUR_COMPANY        NULL        // "Nuance"
#define YOUR_PRODUCT        NULL        // "CSDK-Samples"

/////////////////////////////////////////////////////////////////////////////

#define SID            0
#define PAGE_NUMBER_0  0



/////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <sstream>
#include <fstream>
#include <string.h>
#include "KernelApi.h"
#include <iostream>
#include <string>
#include <vector>
#include <wchar.h>


class OCR_LETTER
{
private:
    
    std::string imageFile;   // the path/name of the image this letter comes from
    std::string bBoxFile;    // the image we will create for this letter
    
    int error;              // error of the text, [0, 256], lower error means
                            // good recognition
    int squareSize;         // the size of one side of the bBox square
    bool inWord;            // True if this letter is part of a word

    
    int left;               // left   pixel coordinate of bounding box
    int top;                // top    pixel coordinate of bounding box
    int right;              // right  pixel coordinate of bounding box
    int bottom;             // bottom pixel coordinate of bounding box
    
    wchar_t text;           // the letter itself
    
    
public:
    
    // constructor
    OCR_LETTER(std::string file, int err, wchar_t letter, int sqSize,
               bool insideWord);

    std::string getbBoxFile() { return bBoxFile; }

    void printLetterToOutput(std::wofstream& outFile, std::string letterOut);

    int exportLetter(HPAGE hPage, IMG_INFO info, std::wofstream& outFileLetter,
                    std::string letterOut, LETTER currLetter, 
                    std::vector<OCR_LETTER *> &letters, int modeInt);
};


class OCR_WORD
{
private:
    
    std::string imageFile;             // the image the word comes from
    std::string bBoxFile;              // the image we will create for this word


    int averageError;                  // average error of letters in word

    int start;       // starting index in pLetter array
    int end;         // index of last letter of this word in pLetter array

    int left;        // bBox coordinates
    int top;
    int right;
    int bottom;

    std::wstring word;                 // the word itself
    std::vector<OCR_LETTER *> letters;   // the letters in this word

public:
    // constructor
    OCR_WORD(std::string file, int start, int end);

    void printWordToOutput(std::wofstream& outFile, std::string outWord);

    int processWordandLetters(HPAGE hPage, LETTER *pLetters, 
                              std::wofstream& outFileLetter, std::string outLetter,
                              std::wofstream& outFileWord, std::string outWord, 
                              IMG_INFO info, int modeInt);
    // destructor
    ~OCR_WORD();
};


/*
 * This fuction sets up the OCR Engine. It:
 * - sets the license
 * - initializes the engine,
 * - sets the output format
 * - sets the default recognition module.
 * The fuction returns 0 for success.
 * Right now, these things are all set to semi-default options for testing.
 */
extern int setUp();


/*
 * The function copies our LETTER characters into the given WCHAR buffer.
 * This function returns 0 on success.
 *
 * NOTE: the characters in a LETTER struct are stored as WCHAR datatypes
 *       (2 bytes - unsigned short).
 *       For convenience, we just insert the WCHARs into our wchar_t (4 bytes)
 *       buffer so that we can use built-in C++ search functions.
 *
 * @param pLetters: the recognition result array
 * @param nLetters: the number of LETTERS in pLetters array
 * @param buffer: wchar_t array to store characters of recognition result
 */
extern int fillCharBuffer(LETTER *pLetters, int nLetters, wchar_t *buffer);


/*
 * Crops the current page image into the rectangle given and exports the
 * rectangle into its own image.
 *
 * @param hPage: the current page
 * @param rect: the RECT structure to export
 * @param name: the filename to give to the new image file
 */
extern int exportRect(HPAGE hPage, RECT rect, char *name);


/*
 * This function processes just the letters from index prevEnd to currStart.
 * We can use this function to extract the letters that are not part of a word.
 *
 * @param hPage: the current page in the ocr process
 * @param info: stores the dimensions of the current page
 * @param pLetters: the recognition result for the page
 * @param prevEnd: the index to start letter extraction
 * @param currStart: the end index for letter extraction
 * @param outFileLetter: stream to print letter info
 * @param outLetter: name of file to print letter info to
 * @param imageFile: the current image path as a string
 *
 */
extern int processBetweenWords(HPAGE hPage, IMG_INFO info, LETTER *pLetters, 
                           int prevEnd, int currStart, wofstream& outFileLetter, 
                           string outLetter, string imageFile);


/*
 * This function takes in an image file name and processes it so that
 * we extract only specific words and letters. 
 *
 * @param hPage: the current page
 * @param imageIn: filename of the image to scan
 * @param outputLetter: filename of the text file where letter results will be
 *                      written
 * @param outputWord: filename of the text file where word results will be
 *                     written
 * @param modeInt: the mode of the program:
 *                 0 = export letters, 1 = export words, 2 = export both
 * @param toFind: the file where the strings to be found are specified.
 *                 strings in this file should be newline separated
 */
extern int extractStrings(HPAGE hPage, std::string imageIn, 
                          std::string outputLetter, std::string outputWord, 
                          int modeInt, std::string toFind);


/* 
 * This function takes in an image and exports all words and letters as 
 * their own image. It writes ocr info (error and result) about the words and
 * letters into the specified files.
 *
 * @param hPage: the current page
 * @param ImageIn: filename of the image to scane
 * @param outputLetter: filename of the text file where letter results will be
 *                      written
 * @param outputWord: filename of the text file where word results will be
 *                     written
 * @param modeInt: the mode of the program:
 *                      0 (-l) = export letters
 *                      1 (-w) = export words
 *                      2 (-b) = export both
 */
extern int extractAll(HPAGE hPage, std::string imageIn,
                       std::string outputLetter,
                       std::string outputWord,
                       int modeInt);



/* 
 *This function takes in an image file name and processes it so that
 * we extract only specific exact strings. In this case, we do not break
 * the string down into many words. The ENTIRE string will be considered
 * one word.
 *
 * @param hPage: the current page
 * @param imageIn: filename of the image to scan
 * @param outputLetter: filename of the text file where letter results will be
 *                      written
 * @param outputWord: filename of the text file where word results will be
 *                      written
 * @param modeInt: the mode of the program:
 *                 0 = export letters, 1 = export words, 2 = export both
 * @param toFind: the file where the strings to be found are specified.
 *                 strings in this file should be newline separated
 */
 
extern int extractExact(HPAGE hPage, std::string imageIn, 
                        std::string outputLetter,
                        std::string outputWord, 
                        int modeInt, std::string toFind);
