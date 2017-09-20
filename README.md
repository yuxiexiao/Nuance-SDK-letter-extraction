# Nuance-SDK-letter-extraction

Additional tools to be used with the Nuance SDK to extract sub-images of (specified) strings and characters from a picture.


Note: Main function definitions are given in ocrExtraction.cpp

This program contains 2 different image extraction modes:

  1.) Extract Letters only.
      Running the program with this mode will run the nuance OCR engine on the given images. After recognition,
      all found characters will be exported as its own image in the same direcory the program was run from. This program will
      pull out the subimage of each specific letter (the subimage will be square and maintains the same scale as the original image)
      and save it in the directory. The program will write to an output textfile the names of each new sub-image, the OCR recognition
      result for each sub-image, and the confidence level of the recognition result.
      
  2.) Extract Words and Letters.
      Running the program with this mode will run the nuance OCR engine on the given images. This mode performs all of the 
      letter extraction steps in mode 1. Additionally, in this mode, the program will attempt to pull out subimages for
      recognized words (determined by page whitespace and character proximity/location). The subimage containing each word is
      exported and saved as a new image in the working directory. The program writes to two output textfiles: one to display
      letter information (as in mode 1) and one to display word information. The word-info textfile contains the names of
      each new word sub-image, the corresponding letters (and letter subimages) that make up that word, and the 
      OCR recognition confidence. 
     
The user can also specify specific strings for the program to extract. String extraction also has two modes:
  
   1.) Extract exact string: the program run the nuance OCR engine to find all occurances of the specified string
                             on an image. The program will export the subimage that contains that entire string, and
                             give additional recognition result information in an output textfile.
                             
   2.) Extract words from string: After all occurances of the specified string are found, the program will search those
                                  strings and extract the words inside the string as their own subimages.
