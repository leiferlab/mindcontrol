# This is the new ultimate makefile for mindcontrol software
# Its all jazzed up for 64 bit windows 7
# and it works witn MSYS 1.0
# mingw-w64
# opencv  2x


##
## Human instructions
##

# make sure the path to mingw-w64 compiler is present, and not 
# the path to mingw32. 
#
# e.g. running echo $PATH should show /c/mingw64/bin NOT
# /c/mingw/bin
#
# also ensure to invoke this makefile in the msys bash shell (not windows cmd, nor in git Bash)

# You say you want to compile from source?
# Before attempting to compile you need to be 
# proficient with the following core technologies and concepts
#
# - make and makefiles
# - cmake
# - The GNU compiler, gcc and how to compile and link libraries
# - environment variables
# - the difference between statically linked and dynamically linked libraries
# - using git and git submodules
# - how to use the GNU toolchain on windows
# - specifically how to use msys and mingw and how to use mingw-w64
# - a thorough understanding of how to work with 32 bit and 64 bit binaries and libraries and how they cannot play together
# - a thorough understanding of the differences between C and C++ and when and how they can be mixed and matched
# - a working knowledge of standard windows libraries
#
# Andrew Leifer is happy to help explain the code, but cannot be responsible for 
# teaching others about these core concepts.

# A note about redistributing binaries:
# Despite adding static flags to the linker, the code seems to require the following two
# dll's for redistribution (note this is for 64bit OS):
# libgcc_s_seh-1.dll
# libstdc++-6.dll
#
# Both dll's can be found in C:\mingw64\bin


#=================================
# Location of external directories
#=================================

#DLP (ALP)
ALP_INC_DIR=3rdPartyLibs/ALPx64

# OpenCV
OPENCV2_BUILD_DIR=C:/opencv/build_mingw64
OPENCV2_SOURCE_DIR=C:/opencv/sources

#BitFlow (frame grabber SDK)
BitFlow_DIR = C:/BitFlow\ SDK\ 5.60/

#Git Version control
GIT=C:/Progra~2/Git/bin/git
#C:\Users\nji\AppData\Local\Programs\Git\bin\git.exe


#=========================
# Source Directory Layout
#=========================

#Location of directories
MyLibs=MyLibs
3rdPartyLibs=3rdPartyLibs
targetDir=bin



#=========================
# Compiler & Linker Flags
#=========================

CCC=x86_64-w64-mingw32-g++.exe
COMPFLAGS= -v  -O2 -c
#-c option means compile only (do not link)

CXX = $(CCC)
LINKFLAGS=  -v -W  -O2 -DNDEBUG 


#======================
# WinAPI Linker objects
#======================
#This WIN API linker objects must go at the END of the linker command
LinkerWinAPILibObj= -lcomctl32 -lgdi32 -lole32 -lavifil32 -lavicap32 -lwinmm -lmsvfw32 -lkernel32 -luser32 -lgdi32 -lwinspool -lshell32 -lole32 -loleaut32 -luuid -lcomdlg32 -ladvapi32 -lsetupapi -lmingw32 -lsetupapi -static-libgcc -static-libstdc++


#========
# OpenCV
#========

#OpenCV objects
openCVobjs=$(OPENCV2_BUILD_DIR)/lib/libopencv_calib3d248.dll.a  \
	$(OPENCV2_BUILD_DIR)/lib/libopencv_calib3d_pch_dephelp.a\
	$(OPENCV2_BUILD_DIR)/lib/libopencv_contrib248.dll.a\
	$(OPENCV2_BUILD_DIR)/lib/libopencv_contrib_pch_dephelp.a\
	$(OPENCV2_BUILD_DIR)/lib/libopencv_core248.dll.a\
	$(OPENCV2_BUILD_DIR)/lib/libopencv_core_pch_dephelp.a\
	$(OPENCV2_BUILD_DIR)/lib/libopencv_features2d248.dll.a\
	$(OPENCV2_BUILD_DIR)/lib/libopencv_features2d_pch_dephelp.a\
	$(OPENCV2_BUILD_DIR)/lib/libopencv_flann248.dll.a                                     \
	$(OPENCV2_BUILD_DIR)/lib/libopencv_flann_pch_dephelp.a                                \
	$(OPENCV2_BUILD_DIR)/lib/libopencv_gpu248.dll.a                                       \
	$(OPENCV2_BUILD_DIR)/lib/libopencv_gpu_pch_dephelp.a                                  \
	$(OPENCV2_BUILD_DIR)/lib/libopencv_haartraining_engine.a                              \
	$(OPENCV2_BUILD_DIR)/lib/libopencv_highgui248.dll.a                                   \
	$(OPENCV2_BUILD_DIR)/lib/libopencv_highgui_pch_dephelp.a                              \
	$(OPENCV2_BUILD_DIR)/lib/libopencv_imgproc248.dll.a                                   \
	$(OPENCV2_BUILD_DIR)/lib/libopencv_imgproc_pch_dephelp.a                              \
	$(OPENCV2_BUILD_DIR)/lib/libopencv_legacy248.dll.a                                    \
	$(OPENCV2_BUILD_DIR)/lib/libopencv_legacy_pch_dephelp.a                               \
	$(OPENCV2_BUILD_DIR)/lib/libopencv_ml248.dll.a                                        \
	$(OPENCV2_BUILD_DIR)/lib/libopencv_ml_pch_dephelp.a                                   \
	$(OPENCV2_BUILD_DIR)/lib/libopencv_nonfree248.dll.a                                   \
	$(OPENCV2_BUILD_DIR)/lib/libopencv_nonfree_pch_dephelp.a                              \
	$(OPENCV2_BUILD_DIR)/lib/libopencv_objdetect248.dll.a                                 \
	$(OPENCV2_BUILD_DIR)/lib/libopencv_objdetect_pch_dephelp.a                            \
	$(OPENCV2_BUILD_DIR)/lib/libopencv_ocl248.dll.a                                       \
	$(OPENCV2_BUILD_DIR)/lib/libopencv_ocl_pch_dephelp.a                                  \
	$(OPENCV2_BUILD_DIR)/lib/libopencv_perf_calib3d_pch_dephelp.a                         \
	$(OPENCV2_BUILD_DIR)/lib/libopencv_perf_core_pch_dephelp.a                            \
	$(OPENCV2_BUILD_DIR)/lib/libopencv_perf_features2d_pch_dephelp.a                      \
	$(OPENCV2_BUILD_DIR)/lib/libopencv_perf_gpu_pch_dephelp.a                             \
	$(OPENCV2_BUILD_DIR)/lib/libopencv_perf_highgui_pch_dephelp.a                         \
	$(OPENCV2_BUILD_DIR)/lib/libopencv_perf_imgproc_pch_dephelp.a                         \
	$(OPENCV2_BUILD_DIR)/lib/libopencv_perf_nonfree_pch_dephelp.a                         \
	$(OPENCV2_BUILD_DIR)/lib/libopencv_perf_objdetect_pch_dephelp.a                       \
	$(OPENCV2_BUILD_DIR)/lib/libopencv_perf_ocl_pch_dephelp.a                             \
	$(OPENCV2_BUILD_DIR)/lib/libopencv_perf_photo_pch_dephelp.a                           \
	$(OPENCV2_BUILD_DIR)/lib/libopencv_perf_stitching_pch_dephelp.a                       \
	$(OPENCV2_BUILD_DIR)/lib/libopencv_perf_superres_pch_dephelp.a                        \
	$(OPENCV2_BUILD_DIR)/lib/libopencv_perf_video_pch_dephelp.a                           \
	$(OPENCV2_BUILD_DIR)/lib/libopencv_photo248.dll.a                                     \
	$(OPENCV2_BUILD_DIR)/lib/libopencv_photo_pch_dephelp.a                                \
	$(OPENCV2_BUILD_DIR)/lib/libopencv_stitching248.dll.a                                 \
	$(OPENCV2_BUILD_DIR)/lib/libopencv_stitching_pch_dephelp.a                            \
	$(OPENCV2_BUILD_DIR)/lib/libopencv_superres248.dll.a                                  \
	$(OPENCV2_BUILD_DIR)/lib/libopencv_superres_pch_dephelp.a                             \
	$(OPENCV2_BUILD_DIR)/lib/libopencv_test_calib3d_pch_dephelp.a                         \
	$(OPENCV2_BUILD_DIR)/lib/libopencv_test_contrib_pch_dephelp.a                         \
	$(OPENCV2_BUILD_DIR)/lib/libopencv_test_core_pch_dephelp.a                            \
	$(OPENCV2_BUILD_DIR)/lib/libopencv_test_features2d_pch_dephelp.a                      \
	$(OPENCV2_BUILD_DIR)/lib/libopencv_test_flann_pch_dephelp.a                           \
	$(OPENCV2_BUILD_DIR)/lib/libopencv_test_gpu_pch_dephelp.a                             \
	$(OPENCV2_BUILD_DIR)/lib/libopencv_test_highgui_pch_dephelp.a                         \
	$(OPENCV2_BUILD_DIR)/lib/libopencv_test_imgproc_pch_dephelp.a                         \
	$(OPENCV2_BUILD_DIR)/lib/libopencv_test_legacy_pch_dephelp.a                          \
	$(OPENCV2_BUILD_DIR)/lib/libopencv_test_ml_pch_dephelp.a                              \
	$(OPENCV2_BUILD_DIR)/lib/libopencv_test_nonfree_pch_dephelp.a                         \
	$(OPENCV2_BUILD_DIR)/lib/libopencv_test_objdetect_pch_dephelp.a                       \
	$(OPENCV2_BUILD_DIR)/lib/libopencv_test_ocl_pch_dephelp.a                             \
	$(OPENCV2_BUILD_DIR)/lib/libopencv_test_photo_pch_dephelp.a                           \
	$(OPENCV2_BUILD_DIR)/lib/libopencv_test_superres_pch_dephelp.a                        \
	$(OPENCV2_BUILD_DIR)/lib/libopencv_test_video_pch_dephelp.a                           \
	$(OPENCV2_BUILD_DIR)/lib/libopencv_ts248.a                                            \
	$(OPENCV2_BUILD_DIR)/lib/libopencv_ts_pch_dephelp.a                                   \
	$(OPENCV2_BUILD_DIR)/lib/libopencv_video248.dll.a                                     \
	$(OPENCV2_BUILD_DIR)/lib/libopencv_video_pch_dephelp.a                                \
	$(OPENCV2_BUILD_DIR)/lib/libopencv_videostab248.dll.a                                 \
	$(OPENCV2_BUILD_DIR)/lib/libopencv_videostab_pch_dephelp.a                            \
	$(OPENCV2_BUILD_DIR)/3rdparty/lib/libIlmImf.a     \
	$(OPENCV2_BUILD_DIR)/3rdparty/lib/liblibjasper.a  \
	$(OPENCV2_BUILD_DIR)/3rdparty/lib/liblibjpeg.a    \
	$(OPENCV2_BUILD_DIR)/3rdparty/lib/liblibpng.a     \
	$(OPENCV2_BUILD_DIR)/3rdparty/lib/liblibtiff.a    \
	$(OPENCV2_BUILD_DIR)/3rdparty/lib/libzlib.a       \


#OpenCV library commands
openCVlibs= -Wl,--major-image-version,0,--minor-image-version,0  -lstdc++ $(openCVobjs)

#OpenCV include files (.h)
openCVinc=$(OPENCV2_SOURCE_DIR)/include \
	-I$(OPENCV2_SOURCE_DIR)/include/opencv2 \
	-I$(OPENCV2_SOURCE_DIR)/include/opencv \
	-I$(OPENCV2_SOURCE_DIR)/modules/androidcamera/include \
	-I$(OPENCV2_SOURCE_DIR)/modules/calib3d/include \
	-I$(OPENCV2_SOURCE_DIR)/modules/contrib/include \
	-I$(OPENCV2_SOURCE_DIR)/modules/core/include \
	-I$(OPENCV2_SOURCE_DIR)/modules/dynamicuda/include \
	-I$(OPENCV2_SOURCE_DIR)/modules/features2d/include \
	-I$(OPENCV2_SOURCE_DIR)/modules/flann/include \
	-I$(OPENCV2_SOURCE_DIR)/modules/gpu/include \
	-I$(OPENCV2_SOURCE_DIR)/modules/highgui/include \
	-I$(OPENCV2_SOURCE_DIR)/modules/imgproc/include \
	-I$(OPENCV2_SOURCE_DIR)/modules/java/include \
	-I$(OPENCV2_SOURCE_DIR)/modules/legacy/include \
	-I$(OPENCV2_SOURCE_DIR)/modules/ml/include \
	-I$(OPENCV2_SOURCE_DIR)/modules/nonfree/include \
	-I$(OPENCV2_SOURCE_DIR)/modules/objdetect/include \
	-I$(OPENCV2_SOURCE_DIR)/modules/ocl/include \
	-I$(OPENCV2_SOURCE_DIR)/modules/photo/include \
	-I$(OPENCV2_SOURCE_DIR)/modules/python/include \
	-I$(OPENCV2_SOURCE_DIR)/modules/stitching/include \
	-I$(OPENCV2_SOURCE_DIR)/modules/superres/include \
	-I$(OPENCV2_SOURCE_DIR)/modules/ts/include \
	-I$(OPENCV2_SOURCE_DIR)/modules/video/include \
	-I$(OPENCV2_SOURCE_DIR)/modules/videostab/include

#==============================================
# Intel Integrated Performance Primitives (IPP)
#==============================================
	#Intel IPP
#-L"C:/Program Files (x86)/Intel/Composer XE 2013/ipp/lib/intel64"	
	
#Intell IPP libs
#-lzlib -lippvm_l -lippcc_l -lippcv_l -lippi_l -lipps_l -lippcore_l -ltbb

	
#=========================
# BitFlow Frame Graber SDK
#=========================
#BFobjects
BFObj = $(BitFlow_DIR)/Lib/BFD.lib \
	$(BitFlow_DIR)/Lib/BFDiskIO.lib \
	$(BitFlow_DIR)/Lib/BFDrv.lib \
	$(BitFlow_DIR)/Lib/BFEr.lib \
	$(BitFlow_DIR)/Lib/BFOS.lib \
	$(BitFlow_DIR)/Lib/BFS.lib \
	$(BitFlow_DIR)/Lib/Bid.lib \
	$(BitFlow_DIR)/Lib/BufferInterface.lib \
	$(BitFlow_DIR)/Lib/Cid.lib  \
	$(BitFlow_DIR)/Lib/R2Cam.lib \
	$(BitFlow_DIR)/Lib/R2D.lib \
	$(BitFlow_DIR)/Lib/R64D.lib \
	$(BitFlow_DIR)/Lib/clallserial.lib \
	$(BitFlow_DIR)/Lib/clserbit.lib \
	$(BitFlow_DIR)/Lib/DispSurf.lib

#BF include directory (.h files)
bfIncDir=$(3rdPartyLibs)/BitFlowSDK
	

#=========================
# ALP DLP (DMD) SDK
#=========================
#Note we are using 4.1 x64 binaries
#static libraries
ALP_STATIC=$(ALP_INC_DIR)/alp41basic.lib 
#dynamically linked runtime libraries
ALP_DLL=$(targetDir)/alp41basic.dll $(targetDir)/fmteos.dll

#all linker objects
ALP_OBJS=$(ALP_STATIC) $(ALP_DLL)

#Include directory (.h files)
#$(ALP_INC_DIR)



#=================================
# MindContorl API
#================================
API_DLL_dir=API/bin
API_Linker_Command= -Lbin -lmc_api

#=================================
# List of Libraries 
#=================================

#Librariers (.lib or .a)
mylibraries=  version.o AndysComputations.o $(targetDir)/mc_api.dll Talk2DLP.o Talk2Camera.o Talk2FrameGrabber.o AndysOpenCVLib.o  TransformLib.o IllumWormProtocol.o

WormSpecificLibs= WormAnalysis.o WriteOutWorm.o experiment.o 

myOpenCVlibraries=AndysComputations.o AndysOpenCVLib.o WormAnalysis.o

TimerLibrary=tictoc.o timer.o

#Hardware Independent linkable objects
hw_ind= version.o AndysComputations.o AndysOpenCVLib.o TransformLib.o IllumWormProtocol.o  $(WormSpecificLibs) $(TimerLibrary) $(openCVobjs)

#=========================
# Top-level Make Targets
#=========================

#CoLBeRT is the whole megillah: BitFlow FrameGrabber, DLP, OpenCV, Stage Control
makecolbert: $(targetDir)/colbert.exe $(targetDir)/calibrate_colbert_first.exe

makevirtual: $(targetDir)/VirtualColbert.exe

all_tests: test_DLP test_CV test_FG test_Stage

# Executables for testing different dependencies
test_DLP: $(targetDir)/testDLP.exe  

test_CV: $(targetDir)/testCV.exe 
 
# This tests the frame grabber, openCV and DLP
test_FG :  $(targetDir)/testFG.exe  

# This tests the ludl stage and also uses OpenCV
test_Stage : $(targetDir)/testStage.exe


#=========================
# Top-level Linker Targets
#=========================

$(targetDir)/VirtualColbert.exe : VirtualColbert.o \
		DontTalk2FrameGrabber.o \
		DontTalk2DLP.o \
		Talk2Stage.o \
		DontTalk2Camera.o \
		$(openCVobjs) \
		$(targetDir)/mc_api.dll \
		$(hw_ind)	
	$(CXX) $(LINKFLAGS) -o $(targetDir)/VirtualColbert.exe VirtualColbert.o $(targetDir)/mc_api.dll DontTalk2FrameGrabber.o Talk2Stage.o DontTalk2Camera.o DontTalk2DLP.o $(hw_ind) $(LinkerWinAPILibObj) 


$(targetDir)/colbert.exe : colbert.o \
		Talk2FrameGrabber.o \
		$(BFobj) \
		Talk2DLP.o \
		Talk2Stage.o \
		$(ALP_OBJS) \
		DontTalk2Camera.o \
		$(openCVobjs) \
		$(targetDir)/mc_api.dll \
		$(hw_ind)	
	$(CXX) $(LINKFLAGS) -o $(targetDir)/colbert.exe  colbert.o $(targetDir)/mc_api.dll Talk2FrameGrabber.o Talk2Stage.o DontTalk2Camera.o $(BFObj) Talk2DLP.o   $(ALP_STATIC) $(hw_ind) $(LinkerWinAPILibObj) 


$(targetDir)/calibrate_colbert_first.exe : calibrate_colbert_first.o \
		Talk2FrameGrabber.o \
		$(BFobj) \
		Talk2DLP.o \
		Talk2Stage.o \
		$(ALP_OBJS) \
		DontTalk2Camera.o \
		$(openCVobjs) \
		$(targetDir)/mc_api.dll \
		$(hw_ind)	
	$(CXX) $(LINKFLAGS) -o $(targetDir)/calibrate_colbert_first.exe \
		calibrate_colbert_first.o \
		$(targetDir)/mc_api.dll \
		Talk2FrameGrabber.o \
		Talk2Stage.o \
		DontTalk2Camera.o \
		$(BFObj) \
		Talk2DLP.o  \
		$(ALP_STATIC) \
		$(hw_ind) \
		$(LinkerWinAPILibObj) 




$(targetDir)/testDLP.exe : testDLP.o Talk2DLP.o 
		$(CXX) $(LINKFLAGS) -o $(targetDir)/testDLP.exe testDLP.o  Talk2DLP.o  $(ALP_STATIC) $(LinkerWinAPILibObj) 

$(targetDir)/testFG.exe : testFG.o Talk2FrameGrabber.o Talk2DLP.o $(BFobj)  $(ALP_OBJS) $(openCVobjs)
	$(CXX) $(LINKFLAGS) -o $(targetDir)/testFG.exe testFG.o   Talk2FrameGrabber.o $(BFObj) Talk2DLP.o   $(ALP_STATIC) $(openCVlibs) $(LinkerWinAPILibObj) 


$(targetDir)/testCV.exe : testCV.o  $(openCVobjs)
	$(CXX) $(LINKFLAGS) testCV.o -o $(targetDir)/testCV.exe $(openCVlibs) $(LinkerWinAPILibObj) 

$(targetDir)/testStage.exe : testStage.o Talk2Stage.o 
	$(CXX) $(LINKFLAGS) testStage.o -o $(targetDir)/testStage.exe Talk2Stage.o $(LinkerWinAPILibObj) 



#=========================
# Top-level Compile Source
#=========================

VirtualColbert.o : main.cpp  \
		$(MyLibs)/Talk2DLP.h \
		$(MyLibs)/Talk2Camera.h \
		$(MyLibs)/TransformLib.h \
		$(MyLibs)/AndysOpenCVLib.h \
		$(MyLibs)/Talk2FrameGrabber.h \
		$(MyLibs)/AndysComputations.h \
		$(MyLibs)/WormAnalysis.h \
		$(MyLibs)/WriteOutWorm.h \
		$(MyLibs)/IllumWormProtocol.h \
		$(MyLibs)/TransformLib.h \
		$(MyLibs)/experiment.h
	$(CXX) $(COMPFLAGS) -o VirtualColbert.o main.cpp -I$(MyLibs) $(openCVinc)  -I$(bfIncDir)

colbert.o : main.cpp  \
		$(MyLibs)/Talk2DLP.h \
		$(MyLibs)/Talk2Camera.h \
		$(MyLibs)/TransformLib.h \
		$(MyLibs)/Talk2Camera.h \
		$(MyLibs)/AndysOpenCVLib.h \
		$(MyLibs)/Talk2FrameGrabber.h \
		$(MyLibs)/AndysComputations.h \
		$(MyLibs)/WormAnalysis.h \
		$(MyLibs)/WriteOutWorm.h \
		$(MyLibs)/IllumWormProtocol.h \
		$(MyLibs)/TransformLib.h \
		$(MyLibs)/experiment.h
	$(CXX) $(COMPFLAGS) -o colbert.o main.cpp -I$(MyLibs) $(openCVinc) -I$(bfIncDir) 

calibrate_colbert_first.o : calibrateFG.cpp \
		$(MyLibs)/Talk2DLP.h \
		$(MyLibs)/Talk2Camera.h \
		$(MyLibs)/TransformLib.h \
		$(MyLibs)/Talk2Camera.h \
		$(MyLibs)/AndysOpenCVLib.h \
		$(MyLibs)/Talk2FrameGrabber.h \
		$(MyLibs)/AndysComputations.h \
		$(MyLibs)/WormAnalysis.h \
		$(MyLibs)/WriteOutWorm.h \
		$(MyLibs)/IllumWormProtocol.h \
		$(MyLibs)/TransformLib.h \
		$(MyLibs)/experiment.h
	$(CXX) $(COMPFLAGS) calibrateFG.cpp -o calibrate_colbert_first.o -I$(MyLibs) -I$(bfIncDir) -I $(openCVinc)


testFG.o : $(MyLibs)/Talk2FrameGrabber.h testFG.cpp
	$(CCC) $(COMPFLAGS) testFG.cpp -I$(MyLibs) -I$(bfIncDir) $(openCVinc)

testDLP.o : testDLP.cpp $(MyLibs)/Talk2DLP.h 	
	$(CCC) $(COMPFLAGS) testDLP.cpp -I$(MyLibs) -I$(ALP_INC_DIR)
		
testCV.o : testCV.c
	$(CCC) $(COMPFLAGS) testCV.c $(openCVinc)

testStage.o: testStage.c
	$(CCC) $(COMPFLAGS) testStage.c $(openCVinc)
	
	
	
#=============================
# Library-level Compile Source
#=============================

experiment.o: $(MyLibs)/experiment.c $(MyLibs)/experiment.h 
	$(CCC) $(COMPFLAGS) $(MyLibs)/experiment.c $ -I$(MyLibs) $(openCVinc) -I$(bfIncDir)

#Note I am using the C++ compiler here
AndysOpenCVLib.o : $(MyLibs)/AndysOpenCVLib.c $(MyLibs)/AndysOpenCVLib.h 
	$(CXX) $(COMPFLAGS) $(MyLibs)/AndysOpenCVLib.c $(openCVinc) 
#
# Worm Related Libraries
#
	
TransformLib.o: $(MyLibs)/TransformLib.c
	$(CCC) $(COMPFLAGS) $(MyLibs)/TransformLib.c $(openCVinc) 

IllumWormProtocol.o : $(MyLibs)/IllumWormProtocol.h $(MyLibs)/IllumWormProtocol.c
	$(CXX) $(COMPFLAGS) $(MyLibs)/IllumWormProtocol.c -I$(MyLibs) $(openCVinc)	
	
WormAnalysis.o : $(MyLibs)/WormAnalysis.c $(MyLibs)/WormAnalysis.h $(myOpenCVlibraries)  
	$(CCC) $(COMPFLAGS) $(MyLibs)/WormAnalysis.c -I$(MyLibs) $(openCVinc)

WriteOutWorm.o : $(MyLibs)/WormAnalysis.c $(MyLibs)/WormAnalysis.h $(MyLibs)/WriteOutWorm.c $(MyLibs)/WriteOutWorm.h $(myOpenCVlibraries) 
	$(CCC) $(COMPFLAGS) $(MyLibs)/WriteOutWorm.c -I$(MyLibs) $(openCVinc)

$(MyLibs)/WriteOutWorm.c :  $(MyLibs)/version.h 
	

#
# Talk2 Libraries
#
# if talk to stage causes trouble try compiling without -mwindows flag.
Talk2Stage.o: $(MyLibs)/Talk2Stage.c $(MyLibs)/Talk2Stage.h
	$(CCC) $(COMPFLAGS) $(MyLibs)/Talk2Stage.c -I$(MyLibs)

	
Talk2FrameGrabber.o: $(MyLibs)/Talk2FrameGrabber.cpp $(MyLibs)/Talk2FrameGrabber.h
	$(CCC) $(COMPFLAGS) $(MyLibs)/Talk2FrameGrabber.cpp -I$(bfIncDir)

Talk2DLP.o: $(MyLibs)/Talk2DLP.cpp $(MyLibs)/Talk2DLP.h
	$(CCC) $(COMPFLAGS) $(MyLibs)/Talk2DLP.cpp -I$(MyLibs) -I$(ALP_INC_DIR)


	
	
#
# Totally Seperate Libraries
#	
	
AndysComputations.o : $(MyLibs)/AndysComputations.c $(MyLibs)/AndysComputations.h
	$(CCC) $(COMPFLAGS) $(MyLibs)/AndysComputations.c 

	
tictoc.o: $(3rdPartyLibs)/tictoc.cpp $(3rdPartyLibs)/tictoc.h 
	$(CXX) $(COMPFLAGS) $(3rdPartyLibs)/tictoc.cpp $ -I$(3rdPartyLibs) 
	
timer.o: $(3rdPartyLibs)/Timer.cpp $(3rdPartyLibs)/Timer.h 
	$(CXX) $(COMPFLAGS) $(3rdPartyLibs)/Timer.cpp $ -I$(3rdPartyLibs) 
	

#
# Hardware independent hacks
#

DontTalk2Camera.o : $(MyLibs)/DontTalk2Camera.c $(MyLibs)/Talk2Camera.h
	$(CCC) $(COMPFLAGS) $(MyLibs)/DontTalk2Camera.c -I$(MyLibs)  $(TailOpts)

DontTalk2FrameGrabber.o: $(MyLibs)/DontTalk2FrameGrabber.cpp $(MyLibs)/Talk2FrameGrabber.h
	$(CCC) $(COMPFLAGS) $(MyLibs)/DontTalk2FrameGrabber.cpp  -I$(bfIncDir)

DontTalk2DLP.o: $(MyLibs)/DontTalk2DLP.c $(MyLibs)/Talk2DLP.h
	$(CCC) $(COMPFLAGS) $(MyLibs)/DontTalk2DLP.c -I$(MyLibs)

#=============================
# Dependency Applications
#=============================
	
## Mind Control API
$(targetDir)/mc_api.dll: $(API_DLL_dir)/mc_api.dll
	cp API/bin/mc_api.dll $(targetDir)/mc_api.dll

$(API_DLL_dir)/mc_api.dll: API/makefile
	cd API && make clean && make all && cd ..

	

#==================
# Versioning System
#==================
	
# note that version.c is generated at the very top. under "timestamp"
version.o : $(MyLibs)/version.c $(MyLibs)/version.h 
	$(CCC) $(COMPFLAGS) $(MyLibs)/version.c  -I$(MyLibs)  

#Trick so that git generates a version.c file
$(MyLibs)/version.c: FORCE 
	$(GIT) rev-parse HEAD | awk ' BEGIN {print "#include \"version.h\""} {print "extern const char * build_git_sha = \"" $$0"\";"} END {}' > $(MyLibs)/version.c
	date | awk 'BEGIN {} {print "extern const char * build_git_time = \""$$0"\";"} END {} ' >> $(MyLibs)/version.c	

FORCE:


#==============
# Dummy targets
#==============

.PHONY: clean run
clean:
	rm -rfv *.o
	rm -fv 	$(API_DLL_dir)/mc_api.dll


