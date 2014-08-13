/*
 * Copyright 2010 Andrew Leifer et al <leifer@fas.harvard.edu>
 * This file is part of MindControl.
 *
 * MindControl is free software: you can redistribute it and/or modify
 * it under the terms of the GNU  General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * MindControl s distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with MindControl. If not, see <http://www.gnu.org/licenses/>.
 *
 * For the most up to date version of this software, see:
 * https://github.com/samuellab/mindcontrol
 *
 *
 *
 * NOTE: If you use any portion of this code in your research, kindly cite:
 * Leifer, A.M., Fang-Yen, C., Gershow, M., Alkema, M., and Samuel A. D.T.,
 * 	"Optogenetic manipulation of neural activity with high spatial resolution in
 *	freely moving Caenorhabditis elegans," Nature Methods, Submitted (2010).
 */



/*
 * ~/workspace/OpticalMindControl/main.cpp
 * main.cpp
 *
 *  Created on: Jul 20, 2009
 *      Author: Andy
 */

/*
 * This is the main file for the MindControl software.
 *
 * This file starts two parallel threads. One thread is responsible for displaying
 * images, interacting with the user and manipulating the microscope stage.
 * The other thread reads in images of a moving worm and generates illumination patterns
 * corresponding to targets on that worm which are then transmitted to a digital
 * micromirror device.
 *
 *
 */

//Standard C headers
#include <unistd.h>
#include <stdio.h>
#include <ctime>
#include <time.h>
#include <conio.h>
#include <math.h>
#include <sys/time.h>

//Windows Header
#include <windows.h>

//C++ header
#include <iostream>
#include <limits>

using namespace std;


//OpenCV Headers
#include "opencv2/highgui/highgui_c.h"
#include "opencv2/imgproc/imgproc_c.h" //required for cvMoments



//Andy's Personal Headers
#include "MyLibs/AndysOpenCVLib.h"
#include "MyLibs/Talk2Camera.h"
#include "MyLibs/Talk2FrameGrabber.h"
#include "MyLibs/Talk2DLP.h"
#include "MyLibs/Talk2Matlab.h"
#include "MyLibs/AndysComputations.h"
#include "MyLibs/WormAnalysis.h"
#include "MyLibs/WriteOutWorm.h"
#include "MyLibs/IllumWormProtocol.h"
#include "MyLibs/TransformLib.h"
#include "API/mc_api_dll.h"
#include "MyLibs/experiment.h"


//3rd Party Libraries
#include "3rdPartyLibs/tictoc.h"

/** Global Variables (for multithreading) **/
UINT Thread(LPVOID lpdwParam);
IplImage* CurrentImg;
bool DispThreadHasStarted;
bool MainThreadHasStopped;
bool DispThreadHasStopped;
bool UserWantsToStop;

int main (int argc, char** argv){
	int DEBUG=0;
	if (DEBUG){
		cvNamedWindow("Debug");
	//	cvNamedWindow("Debug2");
	}


	/** Display output about the OpenCV setup currently installed **/
	DisplayOpenCVInstall();

	/** Create a new experiment object **/
	Experiment* exp=CreateExperimentStruct();


	/** Create memory and objects **/
	InitializeExperiment(exp);
	exp->e=0; //set errors to zero.


	/** Deal with CommandLineArguments **/
	LoadCommandLineArguments(exp,argc,argv);
	if (HandleCommandLineArguments(exp)==-1) return -1;
	
	printf("After loading command line arguments exp->Params->FluorMode=%d\n",exp->Params->FluorMode);

	/** Read In Calibration Data ***/
	if (HandleCalibrationData(exp)<0) return -1;

	/** Load protocol YAML file **/
	if (exp->pflag) LoadProtocol(exp);

	VerifyProtocol(exp->p);

	/** Start Camera or Vid Input **/
	RollVideoInput(exp);

	/** Prepare DLP ***/
	if (!(exp->SimDLP)){
		exp->myDLP= T2DLP_on();
	}

	/** Setup Segmentation Gui **/
	AssignWindowNames(exp);


	/** Start New Thread **/
	DWORD dwThreadId;
	HANDLE hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE) Thread, (void*) exp,
				0, &dwThreadId);
	if (hThread == NULL) {
		printf("Cannot create thread.\n");
		return -1;
	}



	// wait for thread
	DispThreadHasStarted=FALSE;
	DispThreadHasStopped=FALSE;
	MainThreadHasStopped=FALSE;
	while (!DispThreadHasStarted)
		Sleep(10);

	/** SetUp Data Recording **/
	exp->e = SetupRecording(exp);

	/*Start the frame rate timer */
	StartFrameRateTimer(exp);

	/** Quit now if we have some errors **/
	if(exp->e != 0) return -1;


	/** Giant While Loop Where Everything Happens **/
	TICTOC::timer().tic("WholeLoop");
	int VideoRanOut=0;
	UserWantsToStop=0;
	while (UserWantsToStop!=1) {
		_TICTOC_TIC_FUNC
		TICTOC::timer().tic("OneLoop");
		if (isFrameReady(exp)) {

			/** Set error to zero **/
			exp->e=0;
			TICTOC::timer().tic("GrabFrame()");
			/** Grab a frame **/
			int ret=0;
			ret=GrabFrame(exp);
			TICTOC::timer().toc("GrabFrame()");

			if (ret==EXP_VIDEO_RAN_OUT){
				VideoRanOut=1;
				printf("Video ran out!\n");
				break;
			}

			if (ret==EXP_ERROR){
				/** Loop again to try to get another frame **/
				printf("Trying again to grab a frame...\n");
				if (UserWantsToStop) break;
				continue;
			}


			/** Calculate the frame rate and every second print the result **/
			CalculateAndPrintFrameRateAndInfo(exp);


			/** Do we even bother doing analysis?**/
			if (exp->Params->OnOff==0){
				/**Don't perform any analysis**/;
				continue;
			}
			
			
			/**** Functions to decide if Illumination Should be on Or Off ***/
			/** Handle Transient Illumination Timing **/
			HandleIlluminationTiming(exp);

			/** Handle head-tail illumination sweep **/
			HandleIlluminationSweep(exp);


			
			/** Load Image into Our Worm Objects **/

			if (exp->e == 0) exp->e=RefreshWormMemStorage(exp->Worm);
			if (exp->e == 0) exp->e=LoadWormImg(exp->Worm,exp->fromCCD->iplimg);

			/** Apply Levels**/  //Note this is slightly redundant with LoadWormImg
			if (exp->e == 0) exp->e=simpleAdjustLevels(exp->fromCCD->iplimg, exp->Worm->ImgOrig, exp->Params->LevelsMin, exp->Params->LevelsMax);


			TICTOC::timer().tic("EntireSegmentation");
			/** Do Segmentation **/
			DoSegmentation(exp);
			TICTOC::timer().toc("EntireSegmentation");
						
			
			/** If we are not in fluorescence mode **/
			/** Do targeting, and DLP stuff and worm boundary stuff **/
			if (!(exp->FluorMode)){
				/** Real-Time Curvature Phase Analysis, and phase induced illumination **/
				HandleCurvaturePhaseAnalysis(exp);

				/** If the DLP is not displaying right now, than turn off the mirrors */
				ClearDLPifNotDisplayingNow(exp);


				/* Transform the segmented worm coordinates into DLP space */
				/* Note that this is much more computationally efficient than to transform the original image 
				or to transform the resulting illumination pattern                                           */ 
				
				TICTOC::timer().tic("TransformSegWormCam2DLP");
				if ((exp->e == 0)&& !(exp->FluorMode)){
					TransformSegWormCam2DLP(exp->Worm->Segmented, exp->segWormDLP,exp->Calib);
				}
				TICTOC::timer().toc("TransformSegWormCam2DLP");

				/** Handle the Choise of Illumination Protocol Here**/
				/** ANDY: write this here **/
				if (exp->e == 0){
					HandleTimedSecondaryProtocolStep(exp->p,exp->Params);
				}
			


				/*** Do Some Illumination ***/
				if (exp->e == 0)  {
					/** Clear the illumination pattern **/
					SetFrame(exp->forDLP,0);
					SetFrame(exp->IlluminationFrame,0);

					if (exp->Params->IllumFloodEverything) {
						SetFrame(exp->IlluminationFrame,128); // Turn all of the pixels on
						SetFrame(exp->forDLP,128); // Turn all of the pixels o

					} else {

						if (!(exp->Params->ProtocolUse)) /** if not running the protocol **/{
							/** Otherwise Actually illuminate the  region of the worm your interested in **/

							DoOnTheFlyIllumination(exp);

							/** Repeat but for the DLP space for sending to DLP **/
						} else{
							TICTOC::timer().tic("IlluminateFromProtocol()");
							
							/** Illuminate the worm in DLP space **/
							IlluminateFromProtocol(exp->segWormDLP,exp->forDLP,exp->p,exp->Params);
							
							/** Illuminate The worm in Camera Space **/						
							IlluminateFromProtocol(exp->Worm->Segmented,exp->IlluminationFrame,exp->p,exp->Params);

							TICTOC::timer().toc("IlluminateFromProtocol()");

						}

					}
					/** If InvertIllumination is on, then do that now in both cam space and DLP space**/
					if (exp->Params->IllumInvert) InvertIllumination(exp);
				} else {
					printf("Error in exp->e in the mainloop! code line 295\n");
				}


				TICTOC::timer().tic("SendFrameToDLP");
				if (exp->e == 0 && exp->Params->DLPOn && !(exp->SimDLP)) T2DLP_SendFrame((unsigned char *) exp->forDLP->binary, exp->myDLP); // Send image to DLP
				TICTOC::timer().toc("SendFrameToDLP");
			} else {
					/** Clear the illumination pattern **/
					SetFrame(exp->forDLP,0);
					SetFrame(exp->IlluminationFrame,0);
			}

			
		

			/*** DIsplay Some Monitoring Output ***/
			if (exp->e == 0) CreateWormHUDS(exp->HUDS,exp->Worm,exp->Params,exp->IlluminationFrame);
			if (exp->e==0 && exp->stageIsPresent==1) MarkRecenteringTarget(exp);


			if (exp->e == 0 &&  EverySoOften(exp->Worm->frameNum,exp->Params->DispRate) ){
				TICTOC::timer().tic("DisplayOnScreen");
				/** Setup Display but don't actually send to screen **/
				PrepareSelectedDisplay(exp);
				TICTOC::timer().toc("DisplayOnScreen");
			}
			


			if (exp->e == 0) {

				/** Send and Receive Values from API / Shared Memory **/
				TICTOC::timer().tic("SyncAPI");
				SyncAPI(exp);
				TICTOC::timer().tic("SyncAPI");

				/** Write Values to Disk **/
				TICTOC::timer().tic("DoWriteToDisk()");
				DoWriteToDisk(exp);
				TICTOC::timer().toc("DoWriteToDisk()");

			}
			

			if (exp->e != 0) {
				printf("\nError in main loop. :(\n");
				//where emergency stage shutoff used to go

			}

		}
		if (UserWantsToStop) break;
			TICTOC::timer().toc("OneLoop");

	}
	/** Shut down the main thread **/


	TICTOC::timer().toc("WholeLoop");
	/** Tell the display thread that the main thread is shutting down**/
	MainThreadHasStopped=TRUE;

	TICTOC::timer().tic("FinishRecording()");
	FinishRecording(exp);
	TICTOC::timer().toc("FinishRecording()");


	if (!(exp->SimDLP)) 	{
		T2DLP_clear(exp->myDLP);
		T2DLP_off(exp->myDLP);
	}

	if (!(exp->VidFromFile) && !(exp->UseFrameGrabber)){
		/***** Turn off Camera & DLP ****/
		T2Cam_TurnOff(&(exp->MyCamera));
		T2Cam_CloseLib();
	}

	if (!(exp->VidFromFile) && (exp->UseFrameGrabber)){
		CloseFrameGrabber(exp->fg);
	}



	printf("%s",TICTOC::timer().generateReportCstr());
    if (!DispThreadHasStopped){
	   printf("Waiting for DisplayThread to Stop...");

    }
	while (!DispThreadHasStopped){
		printf(".");
		Sleep(500);
		cvWaitKey(10);
	}



	if (exp->stageIsPresent) {
		ShutOffStage(exp);
		printf("\nLast used stage centering coordinates x=%d, y=%d\n",exp->stageFeedbackTarget.x,exp->stageFeedbackTarget.y);
	}
	VerifyProtocol(exp->p);
	ReleaseExperiment(exp);
	DestroyExperiment(&exp);

	printf("\nMain Thread: Good bye.\n");
	return 0;
}


/**
 * Thread to display image & control stage feedback
 */
UINT Thread(LPVOID lpdwParam) {
	Experiment* exp= (Experiment*) lpdwParam;
	printf("DisplayThread: Hello!\n");
	MSG Msg;

	SetupGUI(exp);
	cvWaitKey(30);
//	SetPriorityClass(GetCurrentProcess(), BELOW_NORMAL_PRIORITY_CLASS);

	printf("Beginning ProtocolStep Display\n");
	DispThreadHasStarted = TRUE;
	cvWaitKey(30);

	/** Protocol WormSpace Display **/
	int prevProtocolStep;
	int prevIllumFlipLR=exp->Params->IllumFlipLR;
	IplImage* rectWorm;
	if (exp->pflag){ /** If a protocol was loaded **/
		rectWorm= GenerateRectangleWorm(exp->p->GridSize);
		cvZero(rectWorm);
		IllumRectWorm(rectWorm,exp->p,exp->Params->ProtocolStep,exp->Params->IllumFlipLR);
		prevProtocolStep=exp->Params->ProtocolStep;
		cvShowImage("ProtoIllum",rectWorm);
	}

	printf("DispThread: invoking stage...\n ");
	InvokeStage(exp);

	printf("DispThread: Starting loop\n");

	printf("Waiting a few ms to start the loop.\n");
	cvWaitKey(50);
	printf("DispThread: Entering display loop.\n");
	int key;
	int k=0;
	while (!MainThreadHasStopped) {



		//needed for display window
			if (PeekMessage(&Msg, NULL, 0, 0, PM_REMOVE))
				DispatchMessage(&Msg);


			TICTOC::timer().tic("DisplayThreadGuts");
			TICTOC::timer().tic("cvShowImage");
			if (exp->Params->OnOff){
				cvShowImage("Display",exp->CurrentSelectedImg);
			}else{
				cvShowImage(exp->WinDisp, exp->fromCCD->iplimg);
			}
			TICTOC::timer().toc("cvShowImage");

			if (MainThreadHasStopped==1) continue;



			/** If we are using protocols and we havec chosen a new protocol step **/
			if (exp->Params->ProtocolUse &&  ( (prevProtocolStep!= exp->Params->ProtocolStep) || prevIllumFlipLR != exp->Params->IllumFlipLR  ) )  {
				cvZero(rectWorm);
				IllumRectWorm(rectWorm,exp->p,exp->Params->ProtocolStep,exp->Params->IllumFlipLR);
				prevProtocolStep=exp->Params->ProtocolStep;
				prevIllumFlipLR=exp->Params->IllumFlipLR;
				/** Update the Protocol **/
				cvShowImage("ProtoIllum",rectWorm);

			}

			TICTOC::timer().toc("DisplayThreadGuts");
			UpdateGUI(exp);

			key=cvWaitKey(20); //This controls how often the stage and GUI get updated


			if (MainThreadHasStopped==1) continue;

			if (HandleKeyStroke(key,exp)) {
				printf("\n\nEscape key pressed!\n\n");

				/** Let the Other thread know that the user wants to stop **/
				UserWantsToStop=1;

				/** Emergency Shut off the Stage **/
				printf("Emergency stage shut off.");
				if (exp->stageIsPresent) ShutOffStage(exp);

				/** Exit the display thread immediately **/
				DispThreadHasStopped=TRUE;
				printf("\nDisplayThread: Goodbye!\n");
				return 0;


			}

			UpdateGUI(exp);

			if(EverySoOften(k,1)){ //This determines how often the stage is updated
				
				
				if (exp->e != 0 && exp->stageIsPresent) {
					printf("\tAuto-safety STAGE SHUTOFF from dispThread!\n");
					ShutOffStage(exp);
				} else {
				
					/** Do the Stage Tracking **/
					TICTOC::timer().tic("HandleStageTracker()");
					HandleStageTracker(exp);
					 TICTOC::timer().toc("TimeBetStageUpdate()");
					 TICTOC::timer().tic("TimeBetStageUpdate()");
					TICTOC::timer().toc("HandleStageTracker()");
				
				}

				/** Write the Recent Frame Number to File to be accessed by the Annotation System **/
				TICTOC::timer().tic("WriteRecentFrameNumberToFile()");
				WriteRecentFrameNumberToFile(exp);

				TICTOC::timer().toc("WriteRecentFrameNumberToFile()");
			}

			k++;


	}

	if (exp->stageIsPresent) ShutOffStage(exp);

	//if (exp->pflag) cvReleaseImage(&rectWorm);

	//	printf("%s",TICTOC::timer().generateReportCstr());
		printf("\nDisplayThread: Goodbye!\n");
		DispThreadHasStopped=TRUE;
	return 0;
}

