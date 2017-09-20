all : extractAll extractExact extractStrings
# Path for OCR dylibs:
OCRLIBPATH = ../Frameworks/Nuance-OmniPage-CSDK-RunTime.framework/Versions/Current/Libraries

# Path for OCR include files:
OCRINCPATH = ../Frameworks/Nuance-OmniPage-CSDK.framework/Versions/Current/Headers

# Path to substitute @rpath for:
OCRRUNPATH = @executable_path/../Frameworks

# Linker options:
OCRLIBS = -L$(OCRLIBPATH) -lkernelapi -lrecapiplus -lrecpdf -Wl,-rpath,$(OCRRUNPATH)

# Compiler options:
CXXFLAGS = -O3 -arch i386 -arch x86_64 -mmacosx-version-min=10.7 -I $(OCRINCPATH)

extractAll: extractLetters.cpp ocrExtraction.cpp ocrExtraction.h
	clang++ -std=c++11 -stdlib=libc++ $(CXXFLAGS) ocrExtraction.cpp extractLetters.cpp -o	$@ $(OCRLIBS)

extractExact: extractExact.cpp ocrExtraction.cpp ocrExtraction.h
	clang++ -std=c++11 -stdlib=libc++ $(CXXFLAGS) ocrExtraction.cpp extractExact.cpp -o	$@ $(OCRLIBS)

extractStrings: extractStrings.cpp ocrExtraction.cpp ocrExtraction.h
	clang++ -std=c++11 -stdlib=libc++ $(CXXFLAGS) ocrExtraction.cpp extractStrings.cpp -o 	$@ $(OCRLIBS)

.Phony : clean

deleteL:
	rm -rf l-*

deleteW:
	rm -rf w-*

clean: 
	rm -f *.o extractAll extractStrings extractExact