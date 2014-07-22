Welcome to MindControl
======================

Introduction
------------

[MindControl][1] is a software tool that allows a researcher to utilize [optogenetics][2] to manipulate neural activity in a freely moving worm for behavioral neuroscience experiments. MindControl analyzes a video stream of a swimming nematode and in real time it generates an illumination pattern that targets specific neurons or cells within the worm. The software can output these patterns to a digital micrommirror device in a closed loop. In this way, a researcher can train arbitrary pulses of laser light only on specific cells or neurons of a worm as it moves. The software records detailed information about the worm's position, orientation and the state of the system for every frame of the video stream. This data is recorded in a human- and computer- readable YAML file. The software optionally also outputs raw and annotated video streams for later analysis. The [MindControl-analysis][3] software suite generates quantitative graphs and figures of nematode behavior based on the output from MindControl.

MindControl is the software component of [CoLBeRT] [4] (**Co**ntrolling **L**ocomotion and **Be**havior in **R**eal-**T**ime) as described in Leifer, A.M., Fang-Yen, C., Gershow, M., Alkema, M., Samuel, A.D.T., "Optogenetic manipulation of neural activity in freely moving Caenorhabditis elegans," Nature Methods, in press (2010).

  [1]: http://github.com/samuellab/mindcontrol
  [2]: 	http://en.wikipedia.org/wiki/Optogenetics
  [3]: http://github.com/samuellab/mindcontrol-analysis
  [4]: http://colbert.physics.harvard.edu
  [5]: http://worms.physics.harvard.edu


Authors
-------

MindControl is written by Andrew Leifer with help from Marc Gershow. It is originally a  product of the [Samuel Lab][5] in the [Department of Physics][6] and the [Center for Brain Science][7] at [Harvard University][8]. Now it continues to be expanded and updated by Andrew Leifer at [his own lab][9] at the [Lewis-Sigler Institute][10] at [Princeton University] [11].

  [6]: http://physics.harvard.edu/
  [7]: http://cbs.fas.harvard.edu/
  [8]: http://harvard.edu
  [9]: http://leiferlab.princeton.edu
  [10]: http://genomics.princeton.edu
  [11]: http://princeton.edu


License
-------
With the exception of certain libraries in the `3rdPartyLibs/` folder, all of MindControl is released under the GNU Public License. This means you are free to copy, modify and redistribute this software. 


How to Cite
-----------

If you use any portion of this code or software in your research, please cite:

> Leifer, A.M., Fang-Yen, C., Gershow, M., Alkema, M., Samuel, A.D.T., "Optogenetic manipulation of neural activity in freely moving Caenorhabditis elegans," Nature Methods, 8, 147-142 (2011).

Compiling Your Own Binaries
---------------------------
To utilize camera or micromirror hardware functionality, you will need to compile the software from source. 

The code compiles under Windows 7 64 bit. At a minimum it requires:

*	`mingw-w64`
*	OpenCV 2
*	msys 1.0
*	`make`
*	`awk`

The calibration routines additionally requires:

*	`MATLAB`

Additionaly helper scripts require:

* Python 3x


To compile an executable that uses a camera and DLP requires

*	BitFlow framegrabber SDK
*	VIALUX ALP Basic DLP USB SDK

See the `makefile` for more details about the different compile targets. 

Contact
=======
Please contact Andrew Leifer, leifer (at) princeton.edu with questions or feedback.


An Important Note About Using This Code
=======================================

If you want to compile this source code, please note that you will need to be proficient in the following core technologies and concepts:


* C and C++
* make,  and makefiles
* The GNU compiler, gcc and how to compile and link libraries
* the difference between statically linked and dynamically linked libraries
* using git and git submodules
* how to use the GNU toolchain on windows
* specifically how to use msys and mingw and how to use mingw-w64
* a thorough understanding of how to work with 32 bit and 64 bit binaries and libraries and how they cannot play together
* a thorough understanding of the differences between C and C++ and when and how they can be mixed and matched
* a working knowledge of standard windows libraries
* an ability to compile OpenCV from source

Andrew Leifer is unable to help with questions regarding these core technologies. 
Andrew Leifer is happy, however, to answer any questions about the code itself or the algorithms used. 


Step by step install for Windows 7 64 Bit
=========================================

* Install Bitflow SDK (Camera API) v. 5.60
* Install Karbon Bitflow Frame Grabber Board
* Hook up Camera via Cameralink
* Install Vialux & ALP Basic (DMD API) ALP Basic 4.1
* Hook up DMD via USB
* Install git http://git-scm.com/downloads  (note 1.8.5 has a bug w/ https, so use 1.8.4 for now)
* Install this git repo and the submodules it depends upon
* Install `mingw-w64` to `C:\mingw64\`, installed from Reuben's build http://sourceforge.net/projects/mingw-w64/files/Toolchains%20targetting%20Win64/Personal%20Builds/rubenvb/gcc-4.8-release/x86_64-w64-mingw32-gcc-4.8.0-win64_rubenvb.7z/download
* Add the bin directory for mingw64, `C:\mingw64\bin` to windows path environment
* Install msys via the "mingw get install" setup program http://sourceforge.net/projects/mingw/files/Installer/mingw-get-setup.exe/download  with the following packages:
    * mingw-developer-toolkit
    * msys-base
* Install openCV2 (here we use 2.4.8) https://sourceforge.net/projects/opencvlibrary/files/opencv-win/2.4.8/opencv-2.4.8.exe/download  it should extract a folder called `opencv` move this folder to `C:\opencv\`
* Download `cmake`, http://www.cmake.org/files/v2.8/cmake-2.8.12.1-win32-x86.exe
* Prepare to Compile openCV by running `cmake`
    * Point cmake to opencv's source directory: `C:/opencv/sources`
    * Create a folder for the compiled opencv libraries: `C:/opencv/build_mingw64`
    * Configure cmake as "unix makefile" or "MinGW Makefile" whichever works and point it to the toolchain file template included in this repo: `mingw-w64-toolchain.cmake`  note you may have to adjust the file paths
    * I had to manually add in some parameters for `cmake` to work, even if they were also included in the tool chain, specifically: 
        * `CMAKE_LINKER C:/mingw64/x86_64-w64-mingw32/bin/ld.exe`
        * `CMAKE_MAKE_PROGRAM C:/mingw64/bin/mingw32-make.exe` 
        * `CMAKE_EXE_LINKER_FLAGS  -static-libgcc` 
    * And I had to manually add in the location of the linker `ld.exe` and `ar.exe` by editing this file after the configuration failed: `C:\opencv\build_mingw64\CMakeFiles\2.8.12.1\CMakeCXXCompiler.cmake`  
    * Once Configure works, also run generate. 
    * Note I  ran into trouble compiling one of OpenCV's GPU modules with the "Fatal error" "File to big" and so on. So now I set `CMAKE_BUILD_TYPE` to `RELEASE` and not ``.
    * You should now be done with CMake
*Use `make` to compile Opencv
    * Open the `msys` shell
    * `cd` into `/c/opencv/build_mingw64`
    * run `/c/mingw64/bin/mingw32-make.exe install`   (if you are rerunning, be sure to `make clean` first)
    * At the end of a successful compilation, `C:\opencv\build_mingw64\lib\` should be full of `*.a` files. and `C:\opencv\build_mingw64\bin\` should be full of sample programs that actually run  when invoked from a `cmd` Windows command line environment. 
    * Add `C:\opencv\build_mingw64\bin\` to the system path environment variables
* Adjust the mindcontrol software `makefile` in preparation of compiling the mindcontrol software. Sometimes, for example opencv changes the location of different header folders or modules and this needs to be updated. Also check that the locations of `git` is the same as this gets called during compilation.

* Compile the mindcontrol software
    * Open an `MSYS` shell and `cd` into the root of this repository
    * run `echo $PATH` and confirm that `/c/mingw64/bin` is on the path. If not add this folder to the system wide environment variable in windows and start a new `MSYS` shell
    * run `make all_tests`
    * and ensure everything compiles without error. Then compile everything: `make makecolbert`

    * (interestingly it seems that the mindcontrol software require plain old `make` and not mingw64's `mingw32-make.exe` here because mingw64's make messes up my `awk` script by adding in lots of extra slashes. (the awk script gets run during making))
*Add the folder full of opencv DLL's to windows PATH, `C:\opencv\build_mingw64\bin`
*When you are ready to run the software, remember to run it under microsoft's terminal, `cmd`
