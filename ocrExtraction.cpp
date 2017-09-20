/*
 *  Created by Yuxie Xiao on 8/15/17
 * _____________________________________________________________________________
 * This program contains function definitions for ocrTemplate.h
 *
 * The functions use ocr recognition on multiple image files to extract the
 * images and info of words and characters on the image.
 *
 * ____________________________________________________________________________
 */

#include "ocrExtraction.h"
#include <locale>
#include <codecvt>
#include <fstream>
#include <iostream>
#include <fcntl.h>


using namespace std;

// static counters, used for us to name the output bounding box image files
static int rectLetter = 0;
static int rectWord = 0;


/* 
 * ____________________________________________________________________________
 *  Definitions for class: OCR_LETTER
 * ____________________________________________________________________________
 */


/*
 * OCR_LETTER constructor
 * 
 * @param file: the path to the image we want to recognize
 * @param err: the error for this letter, [0 = great, 255 = terrible]
 * @param letter: the recognition result nuance returned for this letter
 * @param sqSize: the size of one side of this letter's bBox
 */
OCR_LETTER::OCR_LETTER(string file, int err, wchar_t letter, int sqSize,
                       bool insideWord)
{
    imageFile = file;       
    error = err;
    text = letter;
    squareSize = sqSize;
    inWord = insideWord;
}


/*
 * Prints the letter info to the specified output
 *
 * Results are printed in the following format:
 *
 *      1.) Path to original image
 *      2.) Name of the image for just this letter
 *      3.) This letter's confidence
 *      4.) The ocr result from nuance
 *      5.) Blank line
 *
 *
 * @param outFile: the file to write info to
 */
void OCR_LETTER::printLetterToOutput(wofstream &outFile, string letterOut)
{
    // change locale so we can print out unicode letters
    const std::locale utf8_locale = std::locale(std::locale(), new std::codecvt_utf8<wchar_t>());
    outFile.open(letterOut, ios::app);
    outFile.imbue(utf8_locale);
    
    wstring tempImageFile(imageFile.begin(), imageFile.end());
    wstring tempbBoxFile(bBoxFile.begin(), bBoxFile.end());
    
    outFile << tempImageFile << endl;
    outFile << tempbBoxFile << endl;
    if (outFile.bad())
    {
        printf(" could not save\n");
        wprintf(L"%S\n", tempbBoxFile.c_str());
        putwchar(text);
        wprintf(L"!\n");
        printf("int: %d", int(text));
        printf("\n");
    }
    outFile << error << endl;
    outFile << text << endl;
    
    outFile << endl;
    
    outFile.close();
}


/*
 * This function takes a LETTER struct, exports the bounding box of this letter
 * as it's own image, and prints out the letter's info to the specified output.
 *
 * @param hPage: the current page in the ocr process
 * @param info: stores the dimensions of the current page
 * @param outFileLetter: where to print letter information
 * @param currLetter: the letter we are exporting
 * @param letters: the vector of letters for the word that this letter is in
 * @param modeInt: the current export mode. If modeInt is 1 (-w, word only),
 *                  then this function will not export the bBox or print letter
 *                  info
 */
int OCR_LETTER::exportLetter(HPAGE hPage, IMG_INFO info, wofstream& outFileLetter,
                             string letterOut, LETTER currLetter, 
                             vector<OCR_LETTER *> &letters, int modeInt)
{
    int err;
    // create the rect for the letter itself
    RECT letterRect;
    left = currLetter.left;
    right = left + currLetter.width;
    top = currLetter.top;
    bottom = top + currLetter.height;
    
    // make the bBox of the letter fit the specified square
    letterRect.left = left - (squareSize - (right - left)) / 2;
    letterRect.right = right + (squareSize - (right - left)) / 2;
    letterRect.top = top - (squareSize - (bottom - top)) / 2;
    letterRect.bottom = bottom + (squareSize - (bottom - top)) / 2;

    // make rect perfectly square if there was rounding error
    letterRect.right += squareSize - 
                        (letterRect.right - letterRect.left);
    letterRect.bottom += squareSize - 
                        (letterRect.bottom - letterRect.top);

    // add some extra wiggle room to the bBox
    letterRect.left -= squareSize * 0.05;
    letterRect.top -=  squareSize * 0.05;
    letterRect.right += squareSize * 0.05;
    letterRect.bottom += squareSize * 0.05;

    left = letterRect.left;
    top = letterRect.top;
    right = letterRect.right;
    bottom = letterRect.bottom;

    // don't export letter if its square goes beyond page boundaries
    if (letterRect.left < 0 || letterRect.right > info.Size.cx ||
        letterRect.top < 0  ||  letterRect.bottom > info.Size.cy)
    {
        return 1;
        printf("couldn't export letter %d\n", rectLetter);
    } 
    

    // create the letter objext and print its info
    if (currLetter.width > 0 && currLetter.height > 0)
    {
        if (inWord)
        {
            // if the letter is part of a word, push it onto the letters vector
            letters.push_back(this);
        }
        

        if (modeInt == 0 || modeInt == 2)
        {
            // export the letter bBox 
            // our output bBox image names will be labeled with "l-" prefix
            // and an index (the static global variable rectLetter)
            bBoxFile = "l-" + to_string(rectLetter);
            string bBoxFile1 = "l-" + to_string(rectLetter) + ".tiff";
            char *pchar1 = (char *) bBoxFile1.c_str();
            err = exportRect(hPage, letterRect, (char *) pchar1);

            if (err == 0) // if exporting was successful
            {
                rectLetter += 1;  // update global counter
                // print the letter info to the file
                this->printLetterToOutput(outFileLetter, letterOut);
            }
            else 
            {
                printf("could not export letter: %d\n", rectLetter);
            }
        
        }
        return 0;   
    }
    return 1;
}

/*
 * ____________________________________________________________________________
 * End class definition for: OCR_LETTER
 * ____________________________________________________________________________
 */



/* 
 * ____________________________________________________________________________
 *  Definitions for class: OCR_WORD
 * ____________________________________________________________________________
 */


/*
 * OCR_WORD constructor
 * 
 * @param file: the path to the image we want to recognize
 * @param startIndex: starting index of this word in the pLetters array
 * @param endIndex: end index of this word's last char in pLetters array
 */
OCR_WORD::OCR_WORD(string file, int startIndex, int endIndex)
{
    imageFile = file;
    start = startIndex;
    end = endIndex;
}


/*
 * Prints the word info to the specified output
 *
 * Results are printed in the following format:
 *
 *      1.) Path to original image
 *      2.) Name of the image for just this word
 *      3.) This words's average confidence
 *      4.) The list of images for the letters in this word
 *      5.) The ocr result from nuance
 *      6.) Blank line
 *
 * @param outputFile: the file to write info to
 */
void OCR_WORD::printWordToOutput(wofstream& outFile, string wordOut)
{
    const std::locale utf8_locale = std::locale(std::locale(), new std::codecvt_utf8<wchar_t>());
    outFile.open(wordOut, ios::app);
    outFile.imbue(utf8_locale);

    wstring tempImageFile(imageFile.begin(), imageFile.end());
    wstring tempbBoxFile(bBoxFile.begin(), bBoxFile.end());

    outFile << tempImageFile << endl;
    outFile << tempbBoxFile << endl;
    outFile << averageError << endl;
    string letterFile;
    for (int i = 0; i <letters.size(); i++)
    {
        letterFile = letters[i]->getbBoxFile();
        wstring tempFile(letterFile.begin(), letterFile.end());
        outFile << tempFile << " ";
    }
    outFile << endl;
    outFile << word << endl;
    outFile << endl;
    outFile.close();
    
}


/*
 * This function exports the bBox for the given word and prints the word's info
 * into the specified file. The function also processes the letters inside the
 * word.
 *
 * @param hPage: the current page in the ocr process
 * @param pLetters: the recognition result for the current page
 * @param outFileLetter: where to print letter info
 * @param outFileWord: where to print word info
 * @param info: stores dimensions of the current page
 * @param modeInt: if modeInt is 0 (-l, letter only), this function will not
 *                  export the word bBox and will not print the word info
 */
int OCR_WORD::processWordandLetters(HPAGE hPage, LETTER *pLetters,
                                    wofstream& outFileLetter, string outLetter,
                                    wofstream& outFileWord,   string outWord, 
                                    IMG_INFO info, int modeInt)
{
    int err;
    wchar_t currLetter;
    word = L"";
    int largestWidth, largestHeight, squareSize;
    averageError = 0;
    
    // create the rect for the word
    RECT rect;
    rect.right = pLetters[start].left + pLetters[start].width;
    rect.left = pLetters[start].left;
    rect.top = pLetters[start].top;
    rect.bottom = pLetters[start].top + pLetters[start].height;

    // find the largest height and width; this is the square bBox for the letters
    largestHeight = 0;
    largestWidth = 0;
    for (int k = start; k <= end; k++)
    {
        if ((pLetters[k].width) > largestWidth)
        {
            largestWidth = pLetters[k].width;
        }
        if ((pLetters[k].height) > largestHeight)
        {
            largestHeight = pLetters[k].height;
        }
    }
    squareSize = largestHeight;
    if (largestWidth > largestHeight) { squareSize = largestWidth; }
    
    
    // loop through all letters in the word
    for (int j = start; j <= end; j++)
    {
        // update the word's average letter error
        averageError += pLetters[j].err;

        // update the edges of the word bBox
        if (pLetters[j].left < rect.left)
        {
            rect.left = pLetters[j].left;
        }
        if (pLetters[j].left + pLetters[j].width > rect.right)
        {
            rect.right = pLetters[j].left + pLetters[j].width;
        }
        if (pLetters[j].top < rect.top)
        {
            rect.top = pLetters[j].top;
        }
        if (pLetters[j].top + pLetters[j].height > rect.bottom)
        {
            rect.bottom = pLetters[j].top + pLetters[j].height;
        }
        
        currLetter = pLetters[j].code;
        word += currLetter;

        // process each letter in the word
        OCR_LETTER *newLetter = new OCR_LETTER(imageFile, pLetters[j].err,
                                               currLetter, squareSize, TRUE);
        newLetter->exportLetter(hPage, info, outFileLetter, outLetter, 
                                pLetters[j], letters, modeInt);
    }

    // get the average letter error for the word
    averageError /= (end - start + 1);

    // add some extra wiggle room to the word bBox
    rect.left -= (largestWidth / 2);
    rect.right += (largestWidth / 2);
    rect.top -= (largestHeight / 2);
    rect.bottom += (largestHeight / 2);

    // make sure that our word's bBox does not go beyond page boundaries
    if (rect.left < 0 || rect.top)
    if ((rect.top) < 0)    rect.top = 0;
    if ((rect.right) > info.Size.cx) rect.right = info.Size.cx;
    if ((rect.bottom) > info.Size.cy) rect.bottom = info.Size.cy;

    // export bBox for the word
    if (modeInt != 0 && letters.size() > 0)
    {
        // create the name for the image we will export
        bBoxFile = "w-" + to_string(rectWord);
        string bBoxFile2 = "w-" + to_string(rectWord) + ".tiff";
        char *pchar2 = (char *) bBoxFile2.c_str();
        // export the rectangle
        err = exportRect(hPage, rect, (char *) pchar2);
        if (err == 0) // if exporting was successful
        {
            rectWord += 1;
            this->printWordToOutput(outFileWord, outWord);
            return 0;
        }
        else
        {
            printf("could not export word %d\n", rectWord);
        }
    }

    return 1;
}


/* 
 * Destructor for the OCR_WORD class
 */

OCR_WORD::~OCR_WORD()
{
    // delete all OCR_LETTERs in the letters array
    for (int i = 0; i < letters.size(); i++)
    {
        delete letters[i];
    }

}


/*
 * ____________________________________________________________________________
 * End class definition for: OCR_WORD
 * ____________________________________________________________________________
 */



/*
 * This fuction sets up the OCR Engine. It:
 * - sets the license
 * - initializes the engine,
 * - sets the output format
 * - sets the default recognition module.
 * The fuction returns 0 for success.
 */
int setUp()
{
    RECERR rc;
    
    #if (USE_OEM_LICENSE)
    //printf("Setting license information -- kRecSetLicense()\n");
    rc = kRecSetLicense(LICENSE_FILE, OEM_CODE);
    if (rc != REC_OK)
    {
        kRecQuit();
        return 1;
    }
    #endif
    
    rc = kRecInit("papaya", "extract_letters");
    if ((rc != REC_OK) && (rc != API_INIT_WARN)
        && (rc != API_LICENSEVALIDATION_WARN))
    {
        kRecQuit();
        return 1;
    }
    
    //Set default recognition module
    rc = kRecSetDefaultRecognitionModule(SID, RM_OMNIFONT_PLUS3W);
    if (rc != REC_OK)
    {
        kRecSetDefaults(SID);
        kRecQuit();
        return 1;
    }
    
    //printf("Set output format -- kRecSetDTXTFormat()\n");
    rc = kRecSetDTXTFormat(SID, DTXT_TXTS);
    if (rc != REC_OK)
    {
        kRecSetDefaults(SID);
        kRecQuit();
        return 1;
    }
    
    return 0;
}


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
int fillCharBuffer(LETTER *pLetters, int nLetters, wchar_t *buffer)
{
    memset(buffer, 0, sizeof(wchar_t) * (nLetters)); // clear the buffer
    for (int i = 0; i < nLetters; i++)
    {
        // copy each element into the buffer
        memcpy((buffer + i), &(pLetters[i].code), sizeof(pLetters[i].code));
    }
    buffer[nLetters] = L'\0'; // make the buffer null terminated
    return 0;
}


/*
 * Crops the current page image into the rectangle given and exports the
 * rectangle into its own image.
 *
 * @param hPage: the current page
 * @param rect: the RECT structure to export
 * @param name: the filename to give to the new image file
 */
int exportRect(HPAGE hPage, RECT rect, char *name)
{
    RECERR rc;
    /*
     * Common Image Formats:
     * FF_TIFNO: Uncompressed TIFF image
     * FF_TIFPB: Packbits TIFF image format
     * FF_PNG: Portable Image format for Network Graphics
     * FF_GIF: GIF image format incorporating Unisys compression
     * FF_TIFJPGNEW: New JPG Compressed TIFF image format
     */
    IMF_FORMAT  format = FF_TIFNO; // the file type for the image
    
    rc = kRecSaveImgAreaF(SID, name, format, hPage, II_CURRENT, &rect, FALSE);
    if (rc != REC_OK)
    {
        printf("Error code = %X, could not export rectangle \n", rc);
        //printf("rectT: %d, %d, %d, %d \n", rect.left, rect.top, rect.right,
        //       rect.bottom);
        return 1;
    }
    return 0;
}


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
int processBetweenWords(HPAGE hPage, IMG_INFO info, LETTER *pLetters, 
                        int prevEnd, int currStart, wofstream& outFileLetter, 
                        string outLetter, string imageFile)
{
    wchar_t currLetter;
    int modeInt = 0;    // if this method is called, we export the letter always
    int currErr;
    int left, right, top, bottom;
    int squareSize;
    vector<OCR_LETTER *> letters; // a dummy variable, we just need to pass
                                  // something to exportLetter

    for (int i = prevEnd + 1; i < currStart; i++)
    {
        currLetter = pLetters[i].code;

        // make the letter bBox a square
        squareSize = pLetters[i].height;
        if (pLetters[i].width > squareSize) { squareSize = pLetters[i].width; }

        // process and export the letter
        OCR_LETTER *newLetter = new OCR_LETTER(imageFile, pLetters[i].err,
                                               currLetter, squareSize, FALSE);
        newLetter->exportLetter(hPage, info, outFileLetter, outLetter, 
                                pLetters[i], letters, modeInt);
    }

    return 0;
}



/* 
 * This function takes in an image and exports all words and letters as 
 * their own image. It writes ocr info (error and result) about the words and
 * letters into the specified files.
 *
 * @param hPage: the current page
 * @param ImageIn: filename of the image to scan
 * @param outputLetter: filename of the text file where letter results will be
 *                      written
 * @param outputWord: filename of the text file where word results will be
 *                     written
 * @param modeInt: the mode of the program:
 *                      0 (-l) = export letters
 *                      1 (-w) = export words
 *                      2 (-b) = export both
 */
int extractAll(HPAGE hPage, string imageIn, string outputLetter,
                string outputWord, int modeInt)

{
    RECERR rc;
    int err;
    IMG_INFO info;
    
    LETTER *pLetters;
    int nLetters;
    
    // Loading the image to scan
    rc = kRecLoadImgF(SID, imageIn.c_str(), &hPage, PAGE_NUMBER_0);
    if (rc != REC_OK)
    {
        printf("Error code = %X\n", rc);
        printf("LoadError! %s\n", imageIn.c_str());
        kRecQuit();
        return 1;
    }
    
    // Preprocessing page with default settings
    rc = kRecPreprocessImg(SID, hPage);
    if (rc != REC_OK)
    {
        printf("Error code = %X\n", rc);
        kRecFreeImg(hPage);
        kRecQuit();
        return 1;
    }
    
    // get the page info (page size, resolution, etc..)
    rc = kRecGetImgInfo(SID, hPage, II_CURRENT, &info);
    
    // automatically locate zones
    rc = kRecLocateZones(SID, hPage);   

    // Recognizing page
    rc = kRecRecognize(SID, hPage, NULL);
    if (rc != REC_OK)
    {
        printf("Error code = %X\n", rc);
        kRecFreeImg(hPage);
        kRecQuit();
        return (rc==NO_TXT_WARN?2:1);
    }
    
    // Get recognition result
    rc = kRecGetLetters(hPage, II_CURRENT, &pLetters, &nLetters);


    wofstream outFileWord;   // ofstream for printing word output
    wofstream outFileLetter; // ofstream for printing letter output
    int start = 0;           // index of the first letter in the current word
    int end = -1;            // index of the last letter in the current word
    int prevEnd = -1;        // index of the last letter in the previous word
    bool foundEnd = FALSE;
    bool foundStart = TRUE;

    // go through all letters to look for words and characters
    for (int i = 0; i < nLetters; i++)
    {
        // ignore the character if it has no width (newline, etc...)
        if (pLetters[i].width == 0 || pLetters[i].height == 0)
        {
            foundStart = FALSE;
            foundEnd = TRUE;
            continue;
        }

        // the end of a word is marked by the ENDOFWORD flag
        if (foundStart && pLetters[i].makeup == R_ENDOFWORD)
        {
            // we found a word
            end = i;
            foundStart = FALSE;
            foundEnd = TRUE;

            // create the word object and process it
            OCR_WORD newWord = OCR_WORD(imageIn, start, end);
            newWord.processWordandLetters(hPage, pLetters, outFileLetter, 
                                          outputLetter, outFileWord, outputWord,
                                          info, modeInt);

            // process letters between the current word and the previous word
            if (prevEnd > -1 && prevEnd < nLetters && modeInt != 1)
            {
                processBetweenWords(hPage, info, pLetters, prevEnd, start,
                                    outFileLetter, outputLetter, imageIn);
            }

            prevEnd = end;
        }

        // the start of a word is marked by the first non-space letter after
        // the end of a word
        else if (foundEnd && (pLetters[i].spcInfo).spcCount < 1 &&
                 pLetters[i].width > 0)
        {
            // we found the start of a word
            start = i;
            foundStart = TRUE;
            foundEnd = FALSE;
        }
    }

    // process the remaining letters if there are any left
    if (end < nLetters && (modeInt != 1))
    {
        processBetweenWords(hPage, info, pLetters, end, nLetters, outFileLetter,
                            outputLetter, imageIn);
    }

    // clean stuff up
    outFileLetter.flush();
    outFileWord.flush();
    outFileWord.close();
    outFileLetter.close();

    kRecFreeImg(hPage);
    rc = kRecFree(pLetters);
    kRecQuit();
    return 0;
}


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
int extractStrings(HPAGE hPage, string imageIn, string outputLetter,
                   string outputWord, int modeInt, string toFind)
{
    RECERR rc;
    int err;
    IMG_INFO info;
    
    LETTER *pLetters;
    int nLetters;
    
    // Loading the image to scan
    rc = kRecLoadImgF(SID, imageIn.c_str(), &hPage, PAGE_NUMBER_0);
    if (rc != REC_OK)
    {
        printf("Error code = %X\n", rc);
        printf("LoadError! %s\n", imageIn.c_str());
        kRecQuit();
        return 1;
    }
    
    // Preprocessing page with default settings
    rc = kRecPreprocessImg(SID, hPage);
    if (rc != REC_OK)
    {
        printf("Error code = %X\n", rc);
        kRecFreeImg(hPage);
        kRecQuit();
        return 1;
    }
    
    // get the page info (page size, resolution, etc..)
    rc = kRecGetImgInfo(SID, hPage, II_CURRENT, &info);
    
    // automatically locate zones
    rc = kRecLocateZones(SID, hPage);  

    // Recognizing page
    rc = kRecRecognize(SID, hPage, NULL);
    if (rc != REC_OK)
    {
        printf("Error code = %X\n", rc);
        kRecFreeImg(hPage);
        kRecQuit();
        return (rc==NO_TXT_WARN?2:1);
    }
    
    // Get recognition result
    rc = kRecGetLetters(hPage, II_CURRENT, &pLetters, &nLetters);

    wofstream outFileWord;      // ofstream for printing word output
    wofstream outFileLetter;    // ofstream for printing letter output

    wchar_t *allText;           // stores the entire array of recog. letters
    wchar_t *allTextPointer;    // a ptr to mark the current position in allText
    wchar_t *found;             // pointer to where we find a match of toFind
    wchar_t *findThis;          // the toFind string as a wchar_t array

    bool foundEnd = FALSE;
    bool foundStart = TRUE;
    int start;          // index of first letter in current word
    int end;            // index of last letter in current word
    int prevEnd;        // index of last letter in previous word
    ptrdiff_t index;    // index to mark position in allText
    string findLine;

    // malloc a buffer to store recognition result
    allText = (wchar_t *) malloc(sizeof(wchar_t) * (nLetters + 1));

    //put recognition result into wchar buffer
    fillCharBuffer(pLetters, nLetters, allText);

    ifstream currFindFile(toFind);

    while (getline(currFindFile, findLine))
    {
        allTextPointer = allText;

        // create the wchar_t array for toFind
        findThis = (wchar_t *) malloc(sizeof(wchar_t) *
                                      (findLine.length() + 1));

        mbstowcs(findThis, findLine.c_str(), findLine.length() + 1);
        findThis[findLine.length()] = L'\0';  

        // look for the string in the recognition result
        found = wcsstr(allTextPointer, findThis); // will be NULL if no match found
        while (found != NULL)
        {

            index = found - allText;
        
            start = index;
            end = index - 1;
            // now start is the index of a match in allText
            foundEnd = FALSE;
            foundStart = TRUE;

            // go through all the letters in the matching string
            for (int j = index; j < index + findLine.length(); j++)
            {  
                prevEnd = -1;
                // pull out the words from the matching string and process them
                if (foundStart && pLetters[j].makeup == R_ENDOFWORD)
                {      
                    // we found a word
                    end = j;   
                    foundStart = FALSE;
                    foundEnd = TRUE;

                    // create the word object and process it
                    OCR_WORD newWord = OCR_WORD(imageIn, start, end);
                    newWord.processWordandLetters(hPage, pLetters, outFileLetter, 
                                                  outputLetter, outFileWord, 
                                                  outputWord, info, modeInt);

                    // process letters between the current word and the previous word
                    if (prevEnd > -1 && prevEnd < index + findLine.length() && 
                        modeInt != 1)
                    {       
                        processBetweenWords(hPage, info, pLetters, prevEnd, start,
                                            outFileLetter, outputLetter, imageIn);
                    }
                    prevEnd = end;
                }

                // the start of a word is marked by the first non-space letter after
                // the end of a word
                else if (foundEnd && (pLetters[j].spcInfo).spcCount < 1 &&
                         pLetters[j].width > 0)
                {
                    // we found the start of a word
                    start = j;
                    foundStart = TRUE;
                    foundEnd = FALSE;
                }
            }
          
             // process the remaining letters if there are any left
            if (end < (index + findLine.length()) && (modeInt != 1))
            {
                
                processBetweenWords(hPage, info, pLetters, end, 
                                    (index + findLine.length()), outFileLetter,
                                     outputLetter, imageIn);
            }
            
            // move the current pointer up to search the rest of the text
            allTextPointer = (found + findLine.length());

            if ((allTextPointer - allText) > nLetters)
            {
                // break if we've searched the whole text
                break; 
            }
            found = wcsstr(allTextPointer, findThis);
        }

        // clean stuff up
        outFileLetter.flush();
        outFileWord.close();
        outFileWord.flush();
        outFileLetter.close();
        free(findThis);

    }
    
    

    free(allText);
    kRecFreeImg(hPage);
    rc = kRecFree(pLetters);
    kRecQuit();
    return 0;
}


/*
 * This function takes in an image file name and processes it so that
 * we extract only specific exact strings. In this case, we do not break
 * the string down into many words. The ENTIRE string will be considered
 * one word.
 *
 * @param hPage: the current page
 * @param imageIn: filename of the image to scan
 * @param outputLetter: filename of the text file where letter results will be
 *                      written
 * @param outputString: filename of the text file where word results will be
 *                     written
 * @param modeInt: the mode of the program:
 *                 0 = export letters, 1 = export words, 2 = export both
 * @param toFind: the file where the strings to be found are specified.
 *                 strings in this file should be newline separated
 */
int extractExact(HPAGE hPage, string imageIn, string outputLetter,
                   string outputWord, int modeInt, string toFind)
{
    RECERR rc;
    int err;
    IMG_INFO info;
    
    LETTER *pLetters;
    int nLetters;
    
    // Loading the image to scan
    rc = kRecLoadImgF(SID, imageIn.c_str(), &hPage, PAGE_NUMBER_0);
    if (rc != REC_OK)
    {
        printf("Error code = %X\n", rc);
        printf("LoadError! %s\n", imageIn.c_str());
        kRecQuit();
        return 1;
    }
    
    // Preprocessing page with default settings
    rc = kRecPreprocessImg(SID, hPage);
    if (rc != REC_OK)
    {
        printf("Error code = %X\n", rc);
        kRecFreeImg(hPage);
        kRecQuit();
        return 1;
    }
    
    // get the page info (page size, resolution, etc..)
    rc = kRecGetImgInfo(SID, hPage, II_CURRENT, &info);
    
    // automatically locate zones
    rc = kRecLocateZones(SID, hPage);  

    // Recognizing page
    rc = kRecRecognize(SID, hPage, NULL);
    if (rc != REC_OK)
    {
        printf("Error code = %X\n", rc);
        kRecFreeImg(hPage);
        kRecQuit();
        return (rc==NO_TXT_WARN?2:1);
    }
    
    // Get recognition result
    rc = kRecGetLetters(hPage, II_CURRENT, &pLetters, &nLetters);

    wofstream outFileWord;      // ofstream for printing word output
    wofstream outFileLetter;    // ofstream for printing letter output

    wchar_t *allText;           // stores the entire array of recog. letters
    wchar_t *allTextPointer;    // a ptr to mark the current position in allText
    wchar_t *found;             // pointer to where we find a match of toFind
    wchar_t *findThis;          // the toFind string as a wchar_t array

    ptrdiff_t index;    // index to mark position in allText
    string findLine;

    // malloc a buffer to store recognition result
    allText = (wchar_t *) malloc(sizeof(wchar_t) * (nLetters + 1));

    //put recognition result into wchar buffer
    fillCharBuffer(pLetters, nLetters, allText);

    ifstream currFindFile(toFind);

    while (getline(currFindFile, findLine))
    {
        allTextPointer = allText;

        // create the wchar_t array for toFind
        findThis = (wchar_t *) malloc(sizeof(wchar_t) *
                                      (findLine.length() + 1));

        mbstowcs(findThis, findLine.c_str(), findLine.length() + 1);
        findThis[findLine.length()] = L'\0';  

        // look for the string in the recognition result
        found = wcsstr(allTextPointer, findThis); // will be NULL if no match found
        while (found != NULL)
        {

            index = found - allText;

            // create the word(string) object and process it
            OCR_WORD newWord = OCR_WORD(imageIn, index, index - 1 + (findLine.length()));
            newWord.processWordandLetters(hPage, pLetters, outFileLetter, 
                                          outputLetter, outFileWord, 
                                          outputWord, info, modeInt);

            
            // move the current pointer up to search the rest of the text
            allTextPointer = (found + findLine.length());

            if ((allTextPointer - allText) > nLetters)
            {
                // break if we've searched the whole text
                break; 
            }
            found = wcsstr(allTextPointer, findThis);
        }

        // clean stuff up
        outFileLetter.flush();
        outFileWord.close();
        outFileWord.flush();
        outFileLetter.close();
        free(findThis);
    }
     
    free(allText);
    kRecFreeImg(hPage);
    rc = kRecFree(pLetters);
    kRecQuit();
    return 0;
}



