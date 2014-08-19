/*
 * Copyright 2010 Andrew Leifer et al <leifer@fas.harvard.edu>
 * This file is part of MindControl.
 *
 * MindControl is free software: you can redistribute it and/or modify
 * it under the terms of the GNU  General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * MindControl is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with MindControl. If not, see <http://www.gnu.org/licenses/>.
 *
 * For the most up to date version of this software, see:
 * http://github.com/samuellab/mindcontrol
 *
 *
 *
 * NOTE: If you use any portion of this code in your research, kindly cite:
 * Leifer, A.M., Fang-Yen, C., Gershow, M., Alkema, M., and Samuel A. D.T.,
 * 	"Optogenetic manipulation of neural activity with high spatial resolution in
 *	freely moving Caenorhabditis elegans," Nature Methods, Submitted (2010).
 */


/*
 * experiment.c
 *
 *	The experiment.c/.h library is designed to be an extremely high level library.
 *	The idea here is to have all of the elements of an experiment laid out, such that
 *	a user need only to call a few high level functions to run an experiment.
 *
 *  Created on: Nov 30, 2009
 *      Author: Andy
 */

//Standard C headers
#include <unistd.h>
#include <stdio.h>
#include <time.h>
#include <conio.h>
#include <math.h>
#include <assert.h>
#include <sys/time.h>

//OpenCV Headers
#include "opencv2/highgui/highgui_c.h"
//#include "opencv/cv.h"
//#include "opencv/cxcore.h"
#include <cv.h>
#include <cxcore.h>


//Timer Libray
#include "../3rdPartyLibs/tictoc.h"

//Andy's Personal Headers
#include "AndysOpenCVLib.h"
#include "Talk2Camera.h"
#include "Talk2FrameGrabber.h"
#include "Talk2DLP.h"
#include "Talk2Matlab.h"
#include "Talk2Stage.h"
#include "AndysComputations.h"
#include "WormAnalysis.h"
#include "IllumWormProtocol.h"
#include "TransformLib.h"
#include "WriteOutWorm.h"
#include "version.h"
#include "../API/mc_api_dll.h"

#include "experiment.h"

/*
 * Creates a new experiment object and sets values to zero.
 */
Experiment* CreateExperimentStruct() {

	/** Create Experiment Object **/
	Experiment* exp;
	exp = (Experiment*) malloc(sizeof(Experiment));

	/*************************************/
	/**  Set Everything to zero or NULL **/
	/*************************************/

	/** Simulation? True/False **/
	exp->SimDLP = 0;
	exp->VidFromFile = 0;
	
	/** Fluorescence Mode **/
	exp->FluorMode = 0;

	/** GuiWindowNames **/
	exp->WinDisp = NULL;
	exp->WinDisp2 = NULL;
	exp->WinCon1 = NULL;
	exp->WinCon2 = NULL;
	exp->WinCon3 = NULL;

	/** Error information **/
	exp->e = 0;

	/** CommandLine Input **/
	exp->argv = NULL;
	exp->argc = 0;
	exp->outfname = NULL;
	exp->infname = NULL;
	exp->dirname = NULL;
	exp->protocolfname = NULL;

	/** Protocol Data **/
	exp->p = NULL;
	exp->pflag = 0;

	/** Camera Input**/
	exp->MyCamera = NULL;

	/** FrameGrabber Input **/
	exp->fg = NULL;
	exp->UseFrameGrabber = FALSE;

	/** Video input **/
	exp->capture = NULL;

	/** Last Observerd CamFrameNumber **/
	exp->lastFrameSeenOutside = 0;

	/** DLP Output **/
	exp->myDLP = 0;

	/** Calibration Data  Object**/
	exp->Calib = NULL;

	/** User-configurable Worm-related Parameters **/
	exp->Params = NULL;

	/** Information about Our Worm **/
	exp->Worm = NULL;

	/** Information about the Previous frame's Worm **/
	exp->PrevWorm = NULL;

	/** Segmented Worm in DLP Space **/
	exp->segWormDLP = NULL;

	/** internal IplImage **/
	exp->SubSampled = NULL; // Image used to subsample stuff
	exp->HUDS = NULL; //Image used to generate the Heads Up Display
	exp->CurrentSelectedImg = NULL; //The current image selected for display

	/** Internal Frame data types **/
	exp->fromCCD = NULL;
	exp->forDLP = NULL;
	exp->IlluminationFrame = NULL;

	/** Write Data To File **/
	exp->DataWriter = NULL;

	/** Write Video To File **/
	exp->Vid = NULL; //Video Writer
	exp->VidHUDS = NULL;

	/** Timing  Information **/
	exp->now = 0;
	exp->last = 0;

	/** Illumination Timing Info **/
	exp->illumStart = 0;
	exp->illumFinished = 0;

	/** Illumination Head Tail Timing Info **/
	exp->illumSweepHTtimer=0;

	/** Frame Rate Information **/
	exp->nframes = 0;
	exp->prevFrames = 0;
	exp->prevTime = 0;

	/** Stage Control **/
	exp->stageIsPresent=0;
	exp->stage=NULL;
	exp->stageVel=cvPoint(0,0);
	exp->stageCenter=cvPoint(0,0);
	exp->stageFeedbackTarget=cvPoint(512,384);
	exp->stageIsTurningOff=0;

	/** Macros **/
	exp->RECORDVID = 0;
	exp->RECORDDATA = 0;

	/** MindControl API **/
	exp->sm=NULL;

	exp->scratchMem =cvCreateMemStorage(0);

	/** Error Handling **/
	exp->e = 0;

	return exp;

}

/*
 * Load the command line arguments into the experiment object
 */
void LoadCommandLineArguments(Experiment* exp, int argc, char** argv) {
	exp->argc = argc;
	exp->argv = argv;
}

void displayHelp() {
	printf(
			"\n\nGiven a video stream, this software analyzes each frame, finds a worm and generates an illumination pattern.\n");
	printf("by Andrew Leifer, leifer@fas.harvard.edu");
	printf("\nUsage:\n\n");
	printf(
			"If run with no arguments,  uses video from ImagingSource USB camera, illuminates a worm with DLP and records no data.\n\n");
	printf("Optional arguments:\n");
	printf(
			"\t-o  baseFileName\n\t\tWrite video and data output to file using the specified base file name.\n\n");
	printf(
			"\t-d  D:/Path/To/My/Directory/\n\t\tWrite the video and data output to the specified directory. NOTE: it is important to have the trailing slash.\n\n");
	printf(
			"\t-i  InputVideo.avi\n\t\tNo camera. Use video file source instead.\n\n");
	printf(
			"\t-s\n\t\tSimulate the existence of DLP. (No physical DLP required.)\n\n");
	printf("\t-g\n\t\tUse camera attached to FrameGrabber.\n\n");
	printf("\t-t\n\t\tUse USB stage tracker.\n\n");
	printf("\t-x\n\tx 512\t Target x position  of worm for stage feedback loop. 0 is left.\n\n");
	printf("\t-y\n\ty 384\t Target y position of worm for stage feedback loop. 0 is top.\n\n");
	printf(
			"\t-p  protocol.yml\n\t\tIlluminate according to a YAML protocol file.\n\n");
	printf("\t-f\n\tOperate in fluorescence mode. Expects fluorescing blobs instead of darkfield image.. Disables worm shape tracking and disables DLP. Tracks centroid of brightest blob.\n\n");
	printf("\t-?\n\t\tDisplay this help.\n\n");
	printf("\nSee shortcutkeys.txt for a list of keyboard shortcuts.\n");
}

/*
 * Handle CommandLine Arguments
 * Parses commandline arguments.
 * Decides if user wants to record video or recorddata
 */

int HandleCommandLineArguments(Experiment* exp) {
	int dflag = 0;
	opterr = 0;

	int c;
	while ((c = getopt(exp->argc, exp->argv, "si:d:o:p:fgtx:y:?")) != -1) {
		switch (c) {
		case 'i': /** specify input video file **/
			exp->VidFromFile = 1;
			exp->infname = optarg;
			if (optarg == NULL) {
				printf(
						"Error. Given -i switch but no input video file was specified.\n");
				return -1;
			}
			break;

		case 'd': /** specifiy directory **/
			dflag = 1;
			if (optarg != NULL) {
				exp->dirname = optarg;
			} else {
				exp->dirname = "./"; // set to default, local directory;
			}
			break;

		case 'o': /** specify base filename of output **/
			if (optarg != NULL) {
				exp->outfname = optarg;
			} else {
				exp->outfname = "worm"; // set the base filename to the default of worm;
			}
			exp->RECORDVID = 1;
			exp->RECORDDATA = 1;
			break;

		case 's': /** Run in DLP simulation Mode **/
			exp->SimDLP = 1;
			break;

		case 'p': /** Load Protocol **/
			if (optarg != NULL) {
				exp->protocolfname = optarg;
				exp->pflag = 1;
			} else {
				fprintf(stderr,
						"Detected '-p' switch but no protocol file specified\n");
			}
			break;

		case 'g': /** Use frame grabber **/
			if (exp->VidFromFile) {
				printf(
						"Error! Cannot read video from file and use the framegrabber at the same time!\n");
				printf("Choose one or the other!");
				displayHelp();
				return -1;
			} else {
				exp->UseFrameGrabber = TRUE;
			}
			break;
		case 't': /** Use the stage tracking software **/
			exp->stageIsPresent=1;
			break;
		case 'x': /** adjust the target for stage feedback loop by these certain number of pixels **/
				if (optarg != NULL) {
					exp->stageFeedbackTarget.x = atoi(optarg);
				}
				printf("Stage feedback target x= %d pixels.\n",exp->stageFeedbackTarget.x );
		break;
		case 'y': /** adjust the target for stage feedback loop by these certain number of pixels **/
				if (optarg != NULL) {
					exp->stageFeedbackTarget.y = atoi(optarg);
				}
				printf("Stage feedback target y= %d pixels.\n",exp->stageFeedbackTarget.y );
		break;
		
		case 'f': /** fluorescence mode... expect fluorescence neurons, not darkfield image **/
				exp->FluorMode=1;
				exp->Params->FluorMode=1;

				/** For now fluorescence mode will preclude the use of the DLP **/
				exp->SimDLP=1;	
				printf("Entering fluorescence mode...\n The software will now expect fluorescence images of neurons instead of darkfield images.\n");
				printf("Also, disabling DLP functionality.\n");
		break;

		case '?':
			if (optopt == '?') {
				displayHelp();
				return -1;
			}
			/** What is this? This looks bening but wrong to me.. -andy 26 Feb 2010 **/
			if (optopt == 'i' || optopt == 'c' || optopt == 'd' || optopt
					== 's') {
				fprintf(stderr, "Option -%c requires an argument.\n", optopt);
				displayHelp();
				return -1;
			} else if (isprint(optopt)) {
				fprintf(stderr, "Unknown option `-%c'.\n", optopt);
				displayHelp();
				return -1;
			} else {
				fprintf(stderr, "Unknown option character `\\x%x'.\n", optopt);
				displayHelp();
				return -11;
			}
		default:
			displayHelp();
			return -1;
		} // end of switch

	} // end of while loop
	return 1;
}


/**
 * The illumination sweep is a feature that lets the user automatically
 * increment an on-the-fly illumination step by step across the worm
 */
int HandleIlluminationSweep(Experiment* exp){
	int debug=1;

	// Case 1: We are not doing a head-tail sweep
	if ((exp->Params->IllumSweepOn == 0) && (exp->illumSweepHTtimer ==0 ) ){
		return 0;
	}

	struct timeval curr_tv;


	// Case 2:We are initiating a head-tail sweep
	if ((exp->Params->IllumSweepOn == 1) && (exp->illumSweepHTtimer == (double) 0 ) ){
		printf("Initiating a head-to-tail illumination sweep.\n");
		/** Set the cursor to the head (tail) **/
		if (exp->Params->IllumSweepHT==1){
			exp->Params->IllumSquareOrig.y=0;
		}else{
			exp->Params->IllumSquareOrig.y=exp->Params->NumSegments-1;
		}
		/**Set the start time to now. **/
		gettimeofday(&curr_tv, NULL);
		exp->illumSweepHTtimer = curr_tv.tv_sec + (curr_tv.tv_usec / 1000000.0);

		/** Turn the DLP On **/
		exp->Params->DLPOn = 1;

		printf("Initiating head to tail illumination sweep\n");
		return 1;
	}

	double diff;
	int tenthsOfSecondsElapsed;
	/** Case 3: We are in the midst of a head-to-tail illumination sweep **/
	if ((exp->Params->IllumSweepOn == 1) && (exp->illumSweepHTtimer > (double) 0 ) ) {

		/** Is it time to increment? **/
		gettimeofday(&curr_tv, NULL);
		diff = curr_tv.tv_sec + (curr_tv.tv_usec / 1000000.0) - exp->illumSweepHTtimer;
		tenthsOfSecondsElapsed = (int) (diff * 10.0);
		/** Time to increment to the next segment ? **/
		if (tenthsOfSecondsElapsed > exp->Params->IllumDuration) {
			/** Yes.. it's time to increment to the next segment **/

			/** Would the next segment push us off of the worm? **/
			if (exp->Params->IllumSweepHT==1){
				if (exp->Params->IllumSquareOrig.y <  exp->Params->NumSegments - exp->Params->IllumSquareRad.height){
					/** Nope we are safe within the worm: INCREMENT **/
					exp->Params->IllumSquareOrig.y=exp->Params->IllumSquareOrig.y + 2*(exp->Params->IllumSquareRad.height);
				} else {
					/** We are about to walk off. We are finished **/
					exp->Params->DLPOn=0;/** Turn off DLP **/
					exp->illumSweepHTtimer = (double) 0;
					exp->Params->IllumSweepOn=0;
					return 0;
				}

			} else {
				/** We are going in the Opposite direction **/
				if (exp->Params->IllumSquareOrig.y >= exp->Params->IllumSquareRad.height ) {
					/** Nope we are safe within the worm: DECREMENT **/
					exp->Params->IllumSquareOrig.y=exp->Params->IllumSquareOrig.y - 2* (exp->Params->IllumSquareRad.height);
				} else {
					/** We are about to walk off. We are finished **/
					exp->Params->DLPOn=0;/** Turn off DLP **/
					exp->illumSweepHTtimer = (double) 0;
					exp->Params->IllumSweepOn=0;
					return 0;
				}
			}

			/** Ok.. we already incremented/decremented **/
			/**Set the start time to now. **/
			gettimeofday(&curr_tv, NULL);
			exp->illumSweepHTtimer = curr_tv.tv_sec + (curr_tv.tv_usec / 1000000.0);

			return 1;

		} else {
			/** No need to move segments... just keep on illuminating **/
			return 1;
		}
	}

	/** Case 4: user prematurely aborts head to tail illumination sweep **/
	if ((exp->Params->IllumSweepOn == 0) && (exp->illumSweepHTtimer > (double) 0 ) ) {

		/** Turn off DLP **/
		exp->Params->DLPOn=0;
		exp->illumSweepHTtimer = (double) 0;
		return 0;
	}


	printf("Error. In HandleIlluminationSweep() in experiment.c. This should never occur. :(\n");
	return -1;


}

/*
 * Calculate the Mean Curvature of the Head and Analyze the Phase of the
 * worm's sinusoidal body motions.
 *
 * Put this  in a buffer that includes prior curvatures over the last 20 frames or so.
 *
 * If we are trigging based on the phase of the worm's motion, turn the DLP on if we are
 * in the triggering region.
 *
 */
int HandleCurvaturePhaseAnalysis(Experiment* exp){

	int DEBUG_FLAG=0; // print out ?


	_TICTOC_TIC_FUNC
	TICTOC::timer().tic("_CurvaturePhaseAnalysis",exp->e);

	/** If Curvature Analysis is turned off, just return **/
	if (exp->Params->CurvatureAnalyzeOn == 0){
		return EXP_SUCCESS;
	}  /** Otherwise Let's Calculate the Mean Curvature of the Head**/




	/** Smoothing parameter**/
	double sigma=5; /** made bigger **/
	int factor=exp->Params->CurvaturePhaseVisualaziationFactor; //visualization parameter

	/** Define the head in worm coordinates **/
	int HEAD_BEGIN=10;
	int HEAD_END=30;

	/** Splice the head **/
	CvSeq* headcent=cvSeqSlice(exp->Worm->Segmented->Centerline,cvSlice(HEAD_BEGIN,HEAD_END));
	int N=headcent->total - 2;
	if (DEBUG_FLAG!=0){
		printf("Whole Centerline :\n");
		printSeq(exp->Worm->Segmented->Centerline);
		printf("Just the Head:\n");
		printSeq(headcent);
	}


	/** Extract the curvature of the head **/
	double* curvature= (double*) malloc(N* (sizeof(double)));
	RefreshWormMemStorage(exp->Worm);

	/** Smooth and Extract Curvature **/
	if (extractCurvatureOfSeq( headcent,curvature,sigma,exp->Worm->MemScratchStorage)< 0) return EXP_ERROR;
	RefreshWormMemStorage(exp->Worm);
	if (DEBUG_FLAG!=0) printDoubleArr(curvature,N);

	/** Calculate Median Curvature **/
	double median_curvature=MedianOfDoubleArr(curvature,N);
	if (DEBUG_FLAG!=0) {
		printf("median_curvature*100=%f\n",median_curvature* (double) 100);
		printf("About to add the mean head curvature to the buffer.\n");
	}

	/** Store Mean head curvature in buffer that includes mean head curvatures from previous 20 frames**/
	if (AddMeanHeadCurvature(exp->Worm->TimeEvolution,median_curvature,exp->Params)!=A_OK) printf("Error adding mean curvature!!\n");
	if (DEBUG_FLAG!=0) {
		printf("exp->Worm->TimeEvolution->MeanHeadCurvatureBuffer->total=%d\n",exp->Worm->TimeEvolution->MeanHeadCurvatureBuffer->total);
		printSeqScalarDoubles(exp->Worm->TimeEvolution->MeanHeadCurvatureBuffer);
	}

	/** Calculate the derivative of the mean head curvature with respect to time **/
	double* headPhaseBuff=NULL;
	SeqDoublesToArr((const CvSeq*) exp->Worm->TimeEvolution->MeanHeadCurvatureBuffer,&headPhaseBuff);
	int N_curr=exp->Worm->TimeEvolution->MeanHeadCurvatureBuffer->total;

	if (DEBUG_FLAG!=0){
		printf("headPhaseBuff\n");
		printDoubleArr(headPhaseBuff,N_curr);
	}

	double negative_k_dot;//our buffer is backwards. 0th index is most recent.

	mean_derivative(headPhaseBuff,&negative_k_dot,N_curr);
	exp->Worm->TimeEvolution->derivativeOfHeadCurvature=-negative_k_dot;


	/** Deallocate memory for head phase buffer **/
	free(headPhaseBuff);
	if (DEBUG_FLAG!=0) {
		printf("k*%d=%f\t, kdot* %d: %f\n",factor, (double)factor *median_curvature, factor, (double)factor* (exp->Worm->TimeEvolution->derivativeOfHeadCurvature));
		printf("NumFrames: %d, kdot+/-: %d, k+/-: %d, Thresh: %d\n",exp->Params->CurvaturePhaseNumFrames, exp->Params->CurvaturePhaseDerivThresholdPositive,exp->Params->CurvaturePhaseThresholdPositive,exp->Params->CurvaturePhaseThreshold);
		cvWaitKey(1000);
	}


	/** If triggering based on phase, decide weather to turn the DLP on or off**/
	double k, kdot, trig, desiredSignk, desiredSignkdot, signOfk, signOfkdot;

	if (exp->Params->CurvaturePhaseTriggerOn != 0){
		struct timeval curr_tv;

		/*
		 * This is subtle. There is k and kdot. k is the median curvature. kdot is the derivative of the median curvature.
		 * We want to trigger based upon when |k| > trig and when sign(kdot) == desiredSign .
		 */
		kdot=exp->Worm->TimeEvolution->derivativeOfHeadCurvature;
		k=median_curvature*factor;

		/** find signOfkdot**/
		signOfkdot=-1;
		if (kdot>0) signOfkdot=1;

		/** find sign(k) **/
		signOfk=-1;
		if (k>0) signOfk=1;

		/** get desiredSign  for k and kdot**/
		desiredSignkdot= 1;
		desiredSignk =1;
		if (exp->Params->CurvaturePhaseDerivThresholdPositive ==0) desiredSignkdot=  -1;
		if (exp->Params->CurvaturePhaseThresholdPositive ==0) desiredSignk=  -1;


		trig= (double) exp->Params->CurvaturePhaseThreshold  / 10; //Divide by 10 because the slider bar is multiplied by 10

		/** trigger if the abs(k)>trig AND signOfk==desiredSign **/
		if (  (signOfk * k) > trig   && (signOfkdot==desiredSignkdot)   && (signOfk==desiredSignk))  {


			/*
			 * We want to implement a minimum DLP on-time, and a refactory
			 * period. E.g. once the DLP goes on it should stay on a minimimum of n seconds
			 * and once it goes off it should stay off a minimum of m seconds.
			 *
			 */

			/** If we are in Illumination-Stay-On-and-Refractory-Period-Mode **/
			if (exp->Params->StayOnAndRefract==1){

				/** Get timing info to Find out if Refractory Period is Over **/
				gettimeofday(&curr_tv, NULL);
				double diff = curr_tv.tv_sec + (curr_tv.tv_usec / 1000000.0) - exp->illumFinished;
				if ( diff  >  (double) exp->Params->IllumRefractoryPeriod / 10.0){

					/** Turn on the DLP for a preset amount of time **/
					exp->Params->DLPOnFlash=1;

					/** TRICKY!!  **/
					exp->Params->DLPOn=1; // let's also turn the DLP on right now! this shoudln't mess with HAndle Illumination Timing


				} else {
					// Don't actually turn on the DLP, because the refractory period isn't met.
				}

			}else{

			/** Turn on the DLP **/
			exp->Params->DLPOn = 1;

			}

		} else {
			/** The curvature is such that we are no longer triggering **/

			 if (exp->Params->StayOnAndRefract==1){
				//don't do anything because the Handle IllumiantionTiming DLP function will take care of turning things off
			 } else {
				/** Turn the DLP Off **/
				exp->Params->DLPOn = 0;
			 }
		}


	}
	TICTOC::timer().toc("_CurvaturePhaseAnalysis",exp->e);
	return A_OK;
}


/*
 * Feature to turn on DLP illumination for a specified period of time
 * and then auto shut off.
 *
 */
int HandleIlluminationTiming(Experiment* exp) {

	struct timeval curr_tv;
	double diff;

	int tenthsOfSecondsElapsed;


	/** Case 1: Nothing to do **/
	if (!exp->Params->DLPOnFlash) {
		/** DLPOnFlash is off **/
		exp->illumStart = 0;
		return 0;
	}

	/** Case 2: First time DLPOnFlash  is turned onon**/
	if ((exp->Params->DLPOnFlash) && (exp->illumStart == 0)) {
		/**Set the start time to now. **/
		gettimeofday(&curr_tv, NULL);

		/** Set illumStart Time **/
		exp->illumStart = curr_tv.tv_sec + (curr_tv.tv_usec / 1000000.0);

		/** Set illumFinished Time as now (even though we are not yet finished) **/
		exp->illumFinished =exp->illumStart;


		/** Turn the DLP On **/
		exp->Params->DLPOn = 1;
		printf("Turning on DLP transiently for %d tenths of seconds ...\n",exp->Params->IllumDuration);
		return 1;
	}

	/** Case 3: DLPOnFlash has been on **/
	if ((exp->Params->DLPOnFlash) && (exp->illumStart > (double) 0)) {
		gettimeofday(&curr_tv, NULL);
		diff = curr_tv.tv_sec + (curr_tv.tv_usec / 1000000.0) - exp->illumStart;

		/** Set IllumFinished **/
		exp->illumFinished = curr_tv.tv_sec + (curr_tv.tv_usec / 1000000.0);


		/** Determine if we should continue illuminating or stop **/
		tenthsOfSecondsElapsed = (int) (diff * 10.0);
		if (tenthsOfSecondsElapsed > exp->Params->IllumDuration) {
			/** The illumination is now finished **/
			/** Turn the DLP Off **/
			exp->Params->DLPOn = 0;
			exp->Params->DLPOnFlash = 0;

			/** Set the start time to zero.**/
			exp->illumStart = 0;
			printf("Illumination is finished.\n");
			return 0;

		} else {
			/** We should continue to illuminate **/
			exp->Params->DLPOn = 1;
			//printf("diff=%e illumstart=%d current=%d, IllumDuration=%d\n\n",diff,exp->illumStart,current,exp->Params->IllumDuration);
			return 1;
		}

	}

}

/** GUI **/

/* Assigns Default window names to the experiment object
 *
 */
void AssignWindowNames(Experiment* exp) {

	char* disp1 = (char*) malloc(strlen("Display"));
	char* disp2 = (char*) malloc(strlen("Display2"));
	char* control1 = (char*) malloc(strlen("Controls"));
	char* control2 = (char*) malloc(strlen("MoreControls"));
	char* control3 = (char*) malloc(strlen("EvenMoreControls"));

	disp1 = "Display";
	control1 = "Controls";
	control2 = "MoreControls";
	control3 = "EvenMoreControls";

	exp->WinDisp = disp1;
	exp->WinDisp2=disp2;
	exp->WinCon1 = control1;
	exp->WinCon2 = control2;
	exp->WinCon3 = control3;

}

/*
 * Release the memopry associated with window names
 * and set their pointers to Null
 */
void ReleaseWindowNames(Experiment* exp) {
	if (exp->WinDisp != NULL)
		free(exp->WinDisp);
	if (exp->WinDisp2 != NULL)
		free(exp->WinDisp2);
	if (exp->WinCon1 != NULL)
		free(exp->WinCon1);
	if (exp->WinCon2 != NULL)
		free(exp->WinCon2);
	if (exp->WinCon3 != NULL)
		free(exp->WinCon3);

	exp->WinDisp = NULL;
	exp->WinCon1 = NULL;
	exp->WinCon2 = NULL;
	exp->WinCon3 = NULL;
}


/*
 * Specifiy the stage recentering location when the user double clicks
 * a location on the image
 */
void on_mouse( int event, int x, int y, int flags, void* param){

	Experiment* exp =  (Experiment*) param;

	switch (event){
		case CV_EVENT_RBUTTONUP:{
			exp->stageFeedbackTarget.x=x;
			exp->stageFeedbackTarget.y=y;
			printf("Centering target set to: x=%d, y=%d\n",exp->stageFeedbackTarget.x,exp->stageFeedbackTarget.y);
		}

	}

}



/*
 * SetupGui
 *
 */
void SetupGUI(Experiment* exp) {

	printf("Begining to setup GUI\n");

	//	cvNamedWindow(exp->WinDisp); // <-- This goes into the thread.
	cvNamedWindow(exp->WinDisp);
	cvNamedWindow(exp->WinDisp2);

	cvNamedWindow(exp->WinCon1);
	if (exp->FluorMode){
		cvResizeWindow(exp->WinCon1, 500, 450);
	} else {
		cvResizeWindow(exp->WinCon1, 500, 1000);

	}

	cvNamedWindow("ProtoIllum");


	/** SelectDisplay **/
	cvCreateTrackbar("SelDisplay", "Controls", &(exp->Params->Display), 7,
			(int) NULL);
	printf("Pong\n");

	/** On Off **/
	cvCreateTrackbar("On", exp->WinCon1, &(exp->Params->OnOff), 1, (int) NULL);

	/** Temporal Coding **/
	if (!(exp->FluorMode)){
		cvCreateTrackbar("TemporalIQ", exp->WinCon1, &(exp->Params->TemporalOn), 1,
			(int) NULL);
	}

	/** Segmentation Parameters**/
	cvCreateTrackbar("Threshold", exp->WinCon1, &(exp->Params->BinThresh), 255,
			(int) NULL);
	cvCreateTrackbar("Gauss=x*2+1", exp->WinCon1, &(exp->Params->GaussSize),
			15, (int) NULL);
	cvCreateTrackbar("BoundSmooth", exp->WinCon1, &(exp->Params->BoundSmoothSize),
				15, (int) NULL);
	cvCreateTrackbar("DilateErode", exp->WinCon1, &(exp->Params->DilateErode),
					1, (int) NULL);
					
	if (!(exp->FluorMode)){				
		cvCreateTrackbar("ScalePx", exp->WinCon1, &(exp->Params->LengthScale), 50,
				(int) NULL);
		cvCreateTrackbar("Proximity", exp->WinCon1,
				&(exp->Params->MaxLocationChange), 100, (int) NULL);
	}

	/**Illumination Parameters **/
	if (!(exp->FluorMode)){
		cvCreateTrackbar("x", exp->WinCon1, &(exp->Params->IllumSquareOrig.x),
				exp->Params->DefaultGridSize.width, (int) NULL);
		cvCreateTrackbar("y", exp->WinCon1, &(exp->Params->IllumSquareOrig.y),
				exp->Params->DefaultGridSize.height, (int) NULL);
		cvCreateTrackbar("xRad", exp->WinCon1,
				&(exp->Params->IllumSquareRad.width), exp->Params->DefaultGridSize.width,
				(int) NULL);
		cvCreateTrackbar("yRad", exp->WinCon1,
				&(exp->Params->IllumSquareRad.height), exp->Params->DefaultGridSize.height,
				(int) NULL);

		cvCreateTrackbar("IllumDuration", exp->WinCon1,
				&(exp->Params->IllumDuration), 70, (int) NULL);
		cvCreateTrackbar("DLPFlashOn", exp->WinCon1,
				&(exp->Params->DLPOnFlash), 1, (int) NULL);

		cvCreateTrackbar("IllumSweepHT", exp->WinCon1,
					&(exp->Params->IllumSweepHT), 1, (int) NULL);

		cvCreateTrackbar("IllumSweepOn", exp->WinCon1,
					&(exp->Params->IllumSweepOn), 1, (int) NULL);


		cvCreateTrackbar("DLPOn", exp->WinCon1, &(exp->Params->DLPOn), 1,
				(int) NULL);
	}

	/** Record Data **/
	cvCreateTrackbar("RecordOn", exp->WinCon1, &(exp->Params->Record), 1,
			(int) NULL);

	/****** Setup Debug Control Panel ******/
	cvNamedWindow(exp->WinCon2);
	
	if (exp->FluorMode){
		cvResizeWindow(exp->WinCon2, 450, 275);
	}else{
		cvResizeWindow(exp->WinCon2, 450, 800);
	}
	
		if (!(exp->FluorMode)){
		cvCreateTrackbar("FloodLight", exp->WinCon2,
				&(exp->Params->IllumFloodEverything), 1, (int) NULL);

		/** Levels **/
		cvCreateTrackbar("Min",exp->WinCon2,&(exp->Params->LevelsMin),255, (int) NULL );
		cvCreateTrackbar("Max",exp->WinCon2,&(exp->Params->LevelsMax),255, (int) NULL );

		/** Setup Information about Curvature Analysis on the extra control panel **/
		//Curvature analysis? Yes / No
		cvCreateTrackbar("KAnalyzeOn", exp->WinCon2,
				&(exp->Params->CurvatureAnalyzeOn), 1, (int) NULL);

		//Trigger based on the derivative of the mean curvature of the head? Yes/No
		cvCreateTrackbar("KTriggerOn", exp->WinCon2,
				&(exp->Params->CurvaturePhaseTriggerOn), 1, (int) NULL);

		//How many number of frames do we go back in time to calculate the derivative?
		cvCreateTrackbar("KNumFrames", exp->WinCon2,
					&(exp->Params->CurvaturePhaseNumFrames), 50, (int) NULL);

		//Abs value threshold for mean curvature, greater than which we illuminate
		cvCreateTrackbar("KThresh*10", exp->WinCon2,
					&(exp->Params->CurvaturePhaseThreshold), 100, (int) NULL);

		//Illuminate during sign of k positive or negative
		cvCreateTrackbar("KThresh+/-", exp->WinCon2,
					&(exp->Params->CurvaturePhaseThresholdPositive), 1, (int) NULL);


		//Illuminate for positive or negative derivative of curvature (kdot >? 0)?
		cvCreateTrackbar("KdotThresh+/-", exp->WinCon2,
						&(exp->Params->CurvaturePhaseDerivThresholdPositive), 1, (int) NULL);

		cvCreateTrackbar("IllumRefractPeriod", exp->WinCon2,
				&(exp->Params->IllumRefractoryPeriod), 70, (int) NULL);

		//Use the minimum DLP On and Refractory Period?
		cvCreateTrackbar("StayOn&Refract", exp->WinCon2,
						&(exp->Params->StayOnAndRefract), 1, (int) NULL);
		}
	
	/** Software Defined Circle Aperture **/
	
	/* Turn aperture on or off */
	cvCreateTrackbar("ApertureOn", exp->WinCon2,
					 &(exp->Params->ApertureOn), 1, (int) NULL);
	
	/* Set aperture X position */
	cvCreateTrackbar("ApertureX", exp->WinCon2,
					 &(exp->Params->ApertureX), exp->Worm->SizeOfImage.width , (int) NULL);
					 
 	/* Set aperture X position */
 	cvCreateTrackbar("ApertureY", exp->WinCon2,
 					 &(exp->Params->ApertureY), exp->Worm->SizeOfImage.height , (int) NULL);
					 				
  	/* Set aperture X position */
  	cvCreateTrackbar("ApertureR", exp->WinCon2,
  					 &(exp->Params->ApertureR), exp->Worm->SizeOfImage.width / 2 , (int) NULL);




	/** If we have loaded a protocol, set up protocol specific sliders **/
	if (exp->pflag) {
		cvCreateTrackbar("Protocol", exp->WinCon2, &(exp->Params->ProtocolUse),
				1, (int) NULL);

		if (exp->p->Steps->total > 1){
			cvCreateTrackbar("ProtoStep", exp->WinCon2,
					&(exp->Params->ProtocolStep), exp->p->Steps->total - 1,
					(int) NULL);

			/** Secondary Protocol Stop for Timed Switching Applications **/
			cvCreateTrackbar("Proto2", exp->WinCon2,
					&(exp->Params->ProtocolSecondaryStep), exp->p->Steps->total - 1,
					(int) NULL);
			
			/** Duration for Timed secondary protocol step illumintion **/
			cvCreateTrackbar("Proto2Dur", exp->WinCon2,
					&(exp->Params->ProtocolSecondaryDuration), 70,
					(int) NULL);
			
			cvCreateTrackbar("Proto2On", exp->WinCon2,
					&(exp->Params->ProtocolSecondaryIsOn), 1,
					(int) NULL);
					
				
		}	
		
	}

	/** Stage Related GUI elements **/
	if (exp->stageIsPresent){
		/* Slider to set Gain Factor akak StageSpeed */
		cvCreateTrackbar("StageSpeed",exp->WinCon1,&(exp->Params->stageSpeedFactor),300, (int) NULL);
		/* Within the Activezone, the gain on the feedback is linear with distance, outside it is  flat */
		cvCreateTrackbar("ActiveZone",exp->WinCon1,&(exp->Params->stageROIRadius),300, (int) NULL);
		
		if (!(exp->FluorMode)){
			cvCreateTrackbar("TargetSeg",exp->WinCon1,&(exp->Params->stageTargetSegment),99, (int) NULL);
		}

		 /** Specifiy the target for trackign by double clicking on the image **/
		 cvSetMouseCallback( "Display", on_mouse, (void*) exp);
		 printf("Right click on the image at any time to specify recentering target.\n");
	}


	printf("Created trackbars and windows\n");
	return;

}



/*
 * Update's trackbar positions for variables that can be changed by the software
 *
 */
void UpdateGUI(Experiment* exp) {

		cvSetTrackbarPos("DLPFlashOn", exp->WinCon1, (exp->Params->DLPOnFlash));
		cvSetTrackbarPos("DLPOn", exp->WinCon1, (exp->Params->DLPOn));

		/** Illumination Controls **/
		cvSetTrackbarPos("x", exp->WinCon1, (exp->Params->IllumSquareOrig.x));
		cvSetTrackbarPos("y", exp->WinCon1, (exp->Params->IllumSquareOrig.y));
		cvSetTrackbarPos("xRad", exp->WinCon1, (exp->Params->IllumSquareRad.width));
		cvSetTrackbarPos("yRad", exp->WinCon1, (exp->Params->IllumSquareRad.height));

		cvSetTrackbarPos("IllumSweepHT", exp->WinCon1, (exp->Params->IllumSweepHT));
		cvSetTrackbarPos("IllumSweepOn", exp->WinCon1, (exp->Params->IllumSweepOn));


		/** Threshold **/
		cvSetTrackbarPos("Threshold", exp->WinCon1, (exp->Params->BinThresh));
		cvSetTrackbarPos("Gauss=x*2+1",exp->WinCon1, exp->Params->GaussSize);

		/** Updated Temporal IQ **/
		/** Temporal Coding **/
		cvSetTrackbarPos("TemporalIQ", exp->WinCon1, (exp->Params->TemporalOn));


		cvSetTrackbarPos("IllumDuration", exp->WinCon1,
				(exp->Params->IllumDuration));


		/** Protocol Stuff **/
		/** If we have loaded a protocol, update protocol specific sliders **/
		if (exp->pflag) {
			cvSetTrackbarPos("Protocol", exp->WinCon2, exp->Params->ProtocolUse);

			if (exp->p->Steps->total > 1){
				cvSetTrackbarPos("ProtoStep", exp->WinCon2,
						(exp->Params->ProtocolStep));
						
			/** Secondary Protocol Stop for Timed Switching Applications **/
			cvSetTrackbarPos("Proto2", exp->WinCon2,
					(exp->Params->ProtocolSecondaryStep));
			
			/** Duration for Timed secondary protocol step illumintion **/
			cvSetTrackbarPos("Proto2Dur", exp->WinCon2,
					(exp->Params->ProtocolSecondaryDuration));
			
			cvSetTrackbarPos("Proto2On", exp->WinCon2,
					(exp->Params->ProtocolSecondaryIsOn));						
			}
		}

		/** Floodlight **/
		cvSetTrackbarPos("FloodLight", exp->WinCon2,exp->Params->IllumFloodEverything);


		/** Record **/
		cvSetTrackbarPos("RecordOn", exp->WinCon1, (exp->Params->Record));

		/** Record **/
		cvSetTrackbarPos("On", exp->WinCon1, (exp->Params->OnOff));

		/**Stage Speed **/
		cvSetTrackbarPos("StageSpeed",exp->WinCon1,(exp->Params->stageSpeedFactor));

	return;

}

/*** Start Video Camera ***/

/*
 * Initialize camera library
 * Allocate Camera Data
 * Select Camera and Show Properties dialog box
 * Start Grabbing Frames as quickly as possible
 * *
 * OR open up the video file for reading.
 */
void RollVideoInput(Experiment* exp) {
	if (exp->VidFromFile) { /** Use source from file **/
		/** Define the File catpure **/
		exp->capture = cvCreateFileCapture(exp->infname);

	} else {
		/** Use source from camera **/
		if (exp->UseFrameGrabber) {
			exp->fg = TurnOnFrameGrabber();

			printf("Checking frame size of frame grabber..\n");
			/** Check to see that our image sizes are all the same. **/
			if ((int) exp->fg->xsize != exp->fromCCD->size.width
					|| (int) exp->fg->ysize != exp->fromCCD->size.height) {
				printf("Error in RollVideoInput!\n");
				printf(
						"Size from framegrabber does not match size in IplImage fromCCD!\n");
				printf(" exp->fg->xsize=%d\n", (int) exp->fg->xsize);
				printf(" exp->fromCCD->size.width=%d\n",
						exp->fromCCD->size.width);
				printf(" exp->fg->ysize=%d\n", (int) exp->fg->ysize);
				printf(" exp->fromCCD->size.height=%d\n",
						exp->fromCCD->size.height);
				return;
			}

			printf("Frame size checks out..");

			/**Use Frame Grabber **/
		} else {
			/** Use ImagingSource USB Camera **/

			/** Turn on Camera **/
			T2Cam_InitializeLib();
			T2Cam_AllocateCamData(&(exp->MyCamera));
			T2Cam_ShowDeviceSelectionDialog(&(exp->MyCamera));
			/** Start Grabbing Frames and Update the Internal Frame Number iFrameNumber **/
			T2Cam_GrabFramesAsFastAsYouCan(&(exp->MyCamera));
		}

	}
}

/**** Read in Calibration Data ***/
/*
 * Create calibration Data structure
 * Load calibration data from file
 * return -1 if the calibration from file doesn't exist.
 */
int HandleCalibrationData(Experiment* exp) {
	exp->Calib
			= CreateCalibData(cvSize(NSIZEX, NSIZEY), cvSize(NSIZEX, NSIZEY));
	int ret = LoadCalibFromFile(exp->Calib, "calib.dat");
	if (ret != 0) {
		printf(
				"Error reading in calibrationfile!!\nPlease run CalibrateApparatus to generate calibration file calib.dat\nThank you.\nGoodbye.\n");
		return -1;
	}
	return 0;

}

/*
 * This function allocates images and frames
 * And a Worm Object
 *
 * And a Parameter Object
 * For internal manipulation
 *
 *
 */
void InitializeExperiment(Experiment* exp) {

	/*** Create IplImage **/
	IplImage* SubSampled = cvCreateImage(cvSize(NSIZEX / 2, NSIZEY / 2),
			IPL_DEPTH_8U, 1);
	IplImage* HUDS = cvCreateImage(cvSize(NSIZEX, NSIZEY), IPL_DEPTH_8U, 1);


	exp->CurrentSelectedImg= cvCreateImage(cvSize(NSIZEX,NSIZEY), IPL_DEPTH_8U,1);

	exp->SubSampled = SubSampled;
	exp->HUDS = HUDS;

	/*** Create Frames **/
	Frame* fromCCD = CreateFrame(cvSize(NSIZEX, NSIZEY));
	Frame* forDLP = CreateFrame(cvSize(NSIZEX, NSIZEY));
	Frame* IlluminationFrame = CreateFrame(cvSize(NSIZEX, NSIZEY));

	exp->fromCCD = fromCCD;
	exp->forDLP = forDLP;
	exp->IlluminationFrame = IlluminationFrame;

	/** Create Worm Data Struct and Worm Parameter Struct **/
	WormAnalysisData* Worm = CreateWormAnalysisDataStruct();
	WormAnalysisParam* Params = CreateWormAnalysisParam();
	InitializeEmptyWormImages(Worm, cvSize(NSIZEX, NSIZEY));
	InitializeWormMemStorage(Worm);

	/** Create SegWormDLP object using memory from the worm object **/
	exp->segWormDLP = CreateSegmentedWormStruct();

	exp->Worm = Worm;
	exp->Params = Params;
	if (exp->FluorMode)
		exp->Params->FluorMode=1;


	/** Setup Previous Worm **/
	WormGeom* PrevWorm = CreateWormGeom();
	exp->PrevWorm = PrevWorm;

	/** Create MindControl API Shared Memory **/
	exp->sm=MC_API_StartServer();

}

/*
 * Free up all of the different allocated memory for the
 * experiment.
 *
 */
void ReleaseExperiment(Experiment* exp) {
	/** Free up Frames **/
	if (exp->fromCCD != NULL)
		DestroyFrame(&(exp->fromCCD));
	if (exp->forDLP != NULL)
		DestroyFrame(&(exp->forDLP));
	if (exp->IlluminationFrame != NULL)
		DestroyFrame(&(exp->IlluminationFrame));

	/** Stop MindControl API Shared Memory Server **/
	if (exp->sm!=NULL){
		MC_API_StopServer(exp->sm);
		exp->sm=NULL;
	}


	/** Free up Strings **/
	exp->dirname = NULL;
	exp->infname = NULL;
	exp->outfname = NULL;

	/** The segmented worm DLP structure **/
	// Note that the memorystorage for the Cvseq's are in exp->worm->Memorystorage
	free(exp->segWormDLP);

	/** Free up Worm Objects **/
	if (exp->Worm != NULL) {
		DestroyWormAnalysisDataStruct((exp->Worm));
		exp->Worm = NULL;
	}

	if (exp->Params != NULL) {
		DestroyWormAnalysisParam((exp->Params));
		exp->Params = NULL;
	}
	if (exp->PrevWorm != NULL) {
		DestroyWormGeom(&(exp->PrevWorm));
		exp->PrevWorm = NULL;
	}

	/** Free up internal iplImages **/
	if (exp->SubSampled != NULL)
		cvReleaseImage(&(exp->SubSampled));
	if (exp->HUDS != NULL)
		cvReleaseImage(&(exp->HUDS));

	/** Free Up Calib Data **/
	if (exp->Calib != NULL)
		DestroyCalibData(exp->Calib);

	/** Release Window Names **/
	ReleaseWindowNames(exp);

}

/* Destroy the experiment object.
 * To be run after ReleaseExperiment()
 */
void DestroyExperiment(Experiment** exp) {
	cvReleaseMemStorage( &((*exp)->scratchMem));
	free(*exp);
	*exp = NULL;
}

/*********************************************
 *
 * Image Acquisition
 *
 */

/** Grab a Frame from either camera or video source
 *
 */
int GrabFrame(Experiment* exp) {

	if (!(exp->VidFromFile)) {
		/** Acquire from Physical Camera **/
		if (exp->UseFrameGrabber) {
			/** Use BitFlow SDK to acquire from Frame Grabber **/
			if (AcquireFrame(exp->fg)==T2FG_ERROR){
				return EXP_ERROR;
			}

			/** Check to see if file sizes match **/

			LoadFrameWithBin(exp->fg->HostBuf, exp->fromCCD);

		} else {

			/** Acqure from ImagingSource USB Cam **/

			exp->lastFrameSeenOutside = exp->MyCamera->iFrameNumber;
			/*** Create a local copy of the image***/
			LoadFrameWithBin(exp->MyCamera->iImageData, exp->fromCCD);

		}

	} else {

		/** Acquire  from file **/

		IplImage* tempImg;
		/** Grab the frame from the video **/
		tempImg = cvQueryFrame(exp->capture);

		/** Stall for a little bit **/
		//Sleep(50);


		if (tempImg == NULL) {
			printf("There was an error querying the frame from video!\n");
			return EXP_VIDEO_RAN_OUT;
		}

		/** Create a new temp image that is grayscale and of the same size **/
		IplImage* tempImgGray = cvCreateImage(cvGetSize(tempImg), IPL_DEPTH_8U,
				1);

		/** Convert Color to GrayScale **/
		cvCvtColor(tempImg, tempImgGray, CV_RGB2GRAY);

		/** Load the frame into the fromCCD frame object **/
		/*** ANDY! THIS WILL FAIL BECAUSE THE SIZING ISN'T RIGHT **/
		LoadFrameWithImage(tempImgGray, exp->fromCCD);
		cvReleaseImage(&tempImgGray);
		/*
		 * Note: for some reason thingt crash when you go cvReleaseImage(&tempImg)
		 * And there don't seem to be memory leaks if you leave it. So I'm going to leave it in place.
		 *
		 */
	}

	exp->Worm->frameNum++;
	return EXP_SUCCESS;
}

/*
 * Is a frame ready from the camera?
 *
 */
int isFrameReady(Experiment* exp) {
	if (!(exp->VidFromFile) && !(exp->UseFrameGrabber)) {
		/** If This isn't a simulation.. **/
		/** And if we arent using the frame grabber **/
		return (exp->MyCamera->iFrameNumber > exp->lastFrameSeenOutside);
	} else {
		/** Otherwise just keep chugging... **/

		/** Unless we're reading from video, in which case we should fake like we're waiting for something **/
		if (exp->VidFromFile)
			cvWaitKey(100);
		return 1;
	}
}

/*********************** RECORDING *******************/

/*
 * Sets up data recording and video recording
 * Will record video if exp->RECORDVID is 1
 * and record data if exp->RECORDDATA is 1
 *
 */
int SetupRecording(Experiment* exp) {

	printf("About to setup recording\n");
	char* DataFileName;
	if (exp->RECORDDATA) {
		if (exp->dirname == NULL || exp->outfname == NULL)
			printf("exp->dirname or exp->outfname is NULL!\n");

		/** Setup Writing and Write Out Comments **/
		exp->DataWriter = SetUpWriteToDisk(exp->dirname,exp->outfname, exp->Worm->MemStorage);

		/** We should Quit Now if any of the data Writing is not working **/
		if (exp->DataWriter->error < 0 ) return -1;

		/** Write the Command Line argument Out for reference **/
		WriteOutCommandLineArguments(exp->DataWriter, exp->argc, exp->argv);

		/**  Write out the default grid size for non-protocol based illumination **/
		WriteOutDefaultGridSize(exp->DataWriter, exp->Params);

		/** Write the Protocol Out for reference **/
		if (exp->pflag) {
			WriteProtocol(exp->p, exp->DataWriter->fs);
		}

		BeginToWriteOutFrames(exp->DataWriter);

		printf("Initialized data recording\n");
		DestroyFilename(&DataFileName);
	}

	/** Set Up Video Recording **/
	char* MovieFileName;
	char* HUDSFileName;

	if (exp->RECORDVID) {
		if (exp->dirname == NULL || exp->outfname == NULL)
			printf("exp->dirname or exp->outfname is NULL!\n");

		MovieFileName = CreateFileName(exp->dirname, exp->outfname, ".avi");
		HUDSFileName = CreateFileName(exp->dirname, exp->outfname, "_HUDS.avi");

		exp->Vid = cvCreateVideoWriter(MovieFileName,
				CV_FOURCC('M','J','P','G'), 30, cvSize(NSIZEX , NSIZEY),
				0);
		exp->VidHUDS = cvCreateVideoWriter(HUDSFileName,
				CV_FOURCC('M','J','P','G'), 30, cvSize(NSIZEX / 2, NSIZEY / 2),
				0);
		if (exp->Vid ==NULL ) printf("\tERROR in SetupRecording! exp->Vid is NULL\nYou probably are missing the default codec.\n");
		if (exp->VidHUDS ==NULL ) printf("\tERROR in SetupRecording! exp->VidHUDS is NULL\n You probably are missing the default codec.\n");
		DestroyFilename(&MovieFileName);
		DestroyFilename(&HUDSFileName);
		printf("Initialized video recording\n");
	}
	return 0;

}

/*
 * Finish writing video and  and data
 * and release
 *
 */
void FinishRecording(Experiment* exp) {
	/** Finish Writing Video to File and Release Writer **/
	if (exp->Vid != NULL)
		cvReleaseVideoWriter(&(exp->Vid));
	if (exp->VidHUDS != NULL)
		cvReleaseVideoWriter(&(exp->VidHUDS));

	/** Finish Writing to Disk **/
	if (exp->RECORDDATA)
		FinishWriteToDisk(&(exp->DataWriter));

}

/************************************************/
/*   Frame Rate Routines
 *
 */
/************************************************/

/*
 *This is the frame rate timer.
 */
void StartFrameRateTimer(Experiment* exp) {
	exp->prevTime = clock();
	exp->prevFrames = 0;

}

/*
 * If more than a second has elapsed
 * Calculate the frame rate and print i tout
 *
 */
void CalculateAndPrintFrameRateAndInfo(Experiment* exp) {
	/*** Print out Frame Rate ***/
	int fps,factor;
	if ((exp->Worm->timestamp - exp->prevTime) > CLOCKS_PER_SEC) {

		/** Simply count the frames given in the last second **/
		fps=exp->Worm->frameNum - exp->prevFrames;

		/** If we are doing real time analysis of head curvature **/
		if (exp->Params->CurvatureAnalyzeOn) {
			factor=exp->Params->CurvaturePhaseVisualaziationFactor;
			/** display the head curvature and derivative along with the frame rate **/
			printf("%d fps \tk*%d=%Lf \tkdot*%d=%Lf \n", fps, factor, (double)factor*exp->Worm->TimeEvolution->currMeanHeadCurvature,factor, (double)factor* exp->Worm->TimeEvolution->derivativeOfHeadCurvature);
		}else{
			/** Print only frames **/
			printf("%d fps\n", fps);
		}

		/** In all cases, reset the timer **/
		exp->prevFrames = exp->Worm->frameNum;
		exp->prevTime = exp->Worm->timestamp;
	}
}

/************************************************/
/*   Action Chunks
 *
 */
/************************************************/

/*
 * If the DLP is on, don't do anything.
 * If the DLP is off, clear the IlluminationFrame
 * and send that to the DLP so that none of hte DLP mirrors
 * are exposed
 */
void ClearDLPifNotDisplayingNow(Experiment* exp) {
	/** If the DLP is not displaying **/
	if (exp->Params->DLPOn == 0) {
		/** Clear the DLP **/
		RefreshFrame(exp->IlluminationFrame);
		if (!(exp->SimDLP))
			T2DLP_SendFrame((unsigned char *) exp->IlluminationFrame->binary,
					exp->myDLP);
	}
}

/*
 * Given an image in teh worm object, segment the worm
 *
 */
void DoSegmentation(Experiment* exp) {
	_TICTOC_TIC_FUNC
	/*** <segmentworm> ***/

	/*** Find Worm Boundary ***/
	/*
	 *  There is a lot in this one function, FindWormBoudnary(), including:
	 *  Gaussian Blurring
	 *  Thresholding
	 *  Blob Detection
	 *  etc
	 */
		
		

		
	TICTOC::timer().tic("_FindWormBoundary",exp->e);
	if (!(exp->e))
		FindWormBoundary(exp->Worm, exp->Params);
	TICTOC::timer().toc("_FindWormBoundary",exp->e);

	/** If we are in fluorescence mode  **/
	if (exp->FluorMode){
		// We are done.. Nothing More to do..  
		return; 
	}	

	/*** Find Worm Head and Tail ***/
	if (!(exp->e) )
		exp->e = GivenBoundaryFindWormHeadTail(exp->Worm, exp->Params);

	/** If we are doing temporal analysis, improve the WormHeadTail estimate based on prev frame **/
	if (exp->Params->TemporalOn && !(exp->e) ){
		PrevFrameImproveWormHeadTail(exp->Worm, exp->Params, exp->PrevWorm);
	}

	/** if the user is manually inducing a head/tail flip **/
	if (exp->Params->InduceHeadTailFlip){
		ReverseWormHeadTail(exp->Worm);
		/** Turn the flag off **/
		exp->Params->InduceHeadTailFlip=0;
		/*
		 * Note, of course, this function only makes sense if the user is also doing temporal intelligence.
		 * (Otherwise the flipped head tail would immediately reverse itself in the next frame.)
		 * But for completeness we allow the use to do the flip here.
		 */
	}

	/*** Segment the Worm ***/
	if (!(exp->e))
		exp->e = SegmentWorm(exp->Worm, exp->Params);

	/** Update PrevWorm Info **/
	if (!(exp->e))
		LoadWormGeom(exp->PrevWorm, exp->Worm);

	/*** </segmentworm> ***/
_TICTOC_TOC_FUNC
}


/*
 * Add a rectangle to the image to denote the target for stage recentering.
 */
void MarkRecenteringTarget(Experiment* exp){

	CvPoint a=cvPoint( exp->stageFeedbackTarget.x +2, exp->stageFeedbackTarget.y +2);
	CvPoint b=cvPoint(exp->stageFeedbackTarget.x -2, exp->stageFeedbackTarget.y -2);
	cvRectangle(exp->HUDS,a,b, cvScalar(255,255,255),1);

}


/*
 * Prepare the Selected Display
 *
 */
void PrepareSelectedDisplay(Experiment* exp) {
	/** There are no errors and we are displaying a frame **/
	switch (exp->Params->Display) {
	case 0:
		//			 cvShowImage(exp->WinDisp, exp->Worm->ImgOrig);
		exp->CurrentSelectedImg = exp->Worm->ImgOrig;

		break;
	case 1:
		//			cvShowImage(exp->WinDisp,exp->HUDS);
		exp->CurrentSelectedImg = exp->HUDS;
		break;
	case 2:
		//			 cvShowImage(exp->WinDisp,exp->Worm->ImgThresh);
		exp->CurrentSelectedImg = exp->Worm->ImgThresh;
		break;
	case 3:
		/** Implement this!! **/
		//			 DisplayWormHeadTail(exp->Worm,exp->WinDisp);
		exp->CurrentSelectedImg = exp->HUDS; // For now...
		break;
	case 4:
		/** Implement this!! **/
		DisplayWormSegmentation(exp->Worm,exp->CurrentSelectedImg);

		break;
	case 5:
		//			cvShowImage(exp->WinDisp,exp->IlluminationFrame->iplimg);
		exp->CurrentSelectedImg = exp->IlluminationFrame->iplimg;
		break;
	case 6:
		//			cvShowImage(exp->WinDisp, exp->forDLP->iplimg);
		exp->CurrentSelectedImg = exp->forDLP->iplimg;
		break;
	default:
		break;
	}
	//cvWaitKey(1); // Pause one millisecond for things to display onscreen.

}

/*
 *
 * Handle KeyStroke
 *
 * Returns 1 when the user is trying to exit
 *
 */
int HandleKeyStroke(int c, Experiment* exp) {
	switch (c) {
	case 27:
		printf("User has pressed escape!\n");
		printf("Setting Stagge Tracking Variables to off");
		exp->Params->stageTrackingOn=0;
		exp->stageIsTurningOff=1;
		return 1;
		break;
	case ' ': /** Turn on off dlp **/
		Toggle(&(exp->Params->DLPOn));
		break;
	case 'r': /** record **/
		Toggle(&(exp->Params->Record));
		break;
	case 'f': /** turn on off flood light **/
		Toggle(&(exp->Params->IllumFloodEverything));
		break;

	/** on off **/
	case 'o':
		Toggle(&(exp->Params->OnOff));
		break;
	/** On-The Fly Illumination Origin **/
	case 'j':
		Decrement(&(exp->Params->IllumSquareOrig.x),0);
		break;
	case 'l':
		Increment(&(exp->Params->IllumSquareOrig.x),exp->Params->DefaultGridSize.width-1);
		break;
	case 'k':
		Decrement(&(exp->Params->IllumSquareOrig.y),0);
		break;
	case 'i':
		Increment(&(exp->Params->IllumSquareOrig.y),exp->Params->DefaultGridSize.height-1);
		break;

	/** On-The Fly Illumination Radius **/
	case 'a':
		Decrement(&(exp->Params->IllumSquareRad.width),0);
		printf("Illumination Square Width Radius: %d",exp->Params->IllumSquareRad.height);
		break;
	case 'd':
		Increment(&(exp->Params->IllumSquareRad.width),exp->Params->DefaultGridSize.width-1);
		printf("Illumination Square Width Radius: %d",exp->Params->IllumSquareRad.height);
		break;
	case 's':
		Decrement(&(exp->Params->IllumSquareRad.height),0);
		printf("Illumination Square Height Radius: %d",exp->Params->IllumSquareRad.height);
		break;
	case 'w':
		Increment(&(exp->Params->IllumSquareRad.height),exp->Params->DefaultGridSize.height-1);
		printf("Illumination Square Height Radius: %d",exp->Params->IllumSquareRad.height);
		break;

	/** Head-Tail Illumination Sweep **/
	case 'u': //initiate head-to-tail illumination sweep
		Toggle(&(exp->Params->IllumSweepOn));
		printf("u key pressed!\n");
		break;

	case 'U'://switch direction of head sweep
		Toggle(&(exp->Params->IllumSweepHT));
		break;
	/** Protocol **/
	case 'p':
		if (exp->pflag) Toggle(&(exp->Params->ProtocolUse));
		break;
	case '.':
		if (exp->pflag) Increment(&(exp->Params->ProtocolStep),CropNumber(0,exp->Params->ProtocolTotalSteps,exp->Params->ProtocolTotalSteps-1));
		break;
	case ',':
		if (exp->pflag) Decrement(&(exp->Params->ProtocolStep),0);
		break;

	/** Threshold **/
	case ']':
		Increment(&(exp->Params->BinThresh),200);
		break;
	case '[':
		Decrement(&(exp->Params->BinThresh),0);
		break;

	/** Gaussian Blur **/
	case 'G':
		Increment(&(exp->Params->GaussSize),10);
		break;
	case 'g':
		Decrement(&(exp->Params->GaussSize),0);
		break;

	/** Timed DLP on **/
	case '/':
		Toggle(&(exp->Params->DLPOnFlash));
		break;

	case '<':
		Decrement(&(exp->Params->IllumDuration),0);
		break;

	case '>':
		Increment(&(exp->Params->IllumDuration),100);
		break;

	/** Timed Secondary Protocol Illumination **/
    case 'q':
		Toggle(&(exp->Params->ProtocolSecondaryIsOn));
		break;
		
	/** Temporal **/
	case 't':
		Toggle(&(exp->Params->TemporalOn));
		break;
	case 'F':
		Toggle(&(exp->Params->InduceHeadTailFlip));
		break;



	/** Invert Selection **/
	case 'v':
		Toggle(&(exp->Params->IllumInvert));
		break;

	/** Reflect Illumination over centerline **/
	/** (flip dorsal-ventral) **/
	case 'V':
		Toggle(&(exp->Params->IllumFlipLR));
		break;

	/** Tracker **/
	case '\t':
		Toggle(&(exp->Params->stageTrackingOn));
		if (exp->Params->stageTrackingOn==0) {
			/** If we are turning the stage off, let the rest of the code know **/
			printf("Turning tracking off!\n");
			exp->stageIsTurningOff=1;
		} else {
			printf("Turning trackign on!\n");
		}
		break;
	case 'X':
		Increment(&(exp->Params->stageSpeedFactor),50);
		printf("stageSpeedFactor=%d\n",exp->Params->stageSpeedFactor);
		break;
	case 'Z':
		Decrement(&(exp->Params->stageSpeedFactor),0);
		printf("stageSpeedFactor=%d\n",exp->Params->stageSpeedFactor);
		break;

	case 127: /** Delete key **/
	case 8: /** Backspace key **/
		exp->Params->stageTrackingOn=0;
		exp->stageIsTurningOff=1;
		printf("Instructing stage to turn off..");
		break;

	default:
		return 0;
		break;
	}
	return 0;
}


/*
 * Write out current values to MindControl API and read in
 * values set by external processes.
 *
 * At the moment, MindControl writes out the current frame and the
 * status of the DLP. It reads in the laser power values.
 */
void SyncAPI(Experiment* exp){

	/** Write out to the MindControl API **/
	MC_API_SetCurrentFrame(exp->sm, exp->Worm->frameNum);
	MC_API_SetDLPOnOff(exp->sm,exp->Params->DLPOn);

	/** Load in Info From Laser Controller **/
	if (MC_API_isLaserControllerPresent(exp->sm)) {
		exp->Params->GreenLaser=MC_API_GetGreenLaserPower(exp->sm);
		exp->Params->BlueLaser=MC_API_GetBlueLaserPower(exp->sm);
	} else {
		exp->Params->GreenLaser=-1;
		exp->Params->BlueLaser=-1;
	}

	return;

}

/*
 * Write video and data to Disk
 *
 */
void DoWriteToDisk(Experiment* exp) {


	/** Throw error if the user has asked to record, but the system is not in record mode **/
	if (exp->Params->Record && (exp->RECORDVID!=1)  ){
		printf("ERROR!! THE SYSTEM IS NOT IN RECORD MODE!\n");
		printf("restart the system to record.\n");
	}

	/** Record VideoFrame to Disk**/
	if (exp->RECORDVID && exp->Params->Record) {
	
		TICTOC::timer().tic("cvWriteFrame");
		cvWriteFrame(exp->Vid, exp->Worm->ImgOrig);
		if (exp->Vid==NULL ) printf("\tERROR in DoWriteToDisk!\n\texp->Vid is NULL\n");
		if (exp->SubSampled ==NULL ) printf("\tERROR in DoWriteToDisk!\n\texp->exp->Subsampled==NULL\n");

		TICTOC::timer().toc("cvWriteFrame");

		cvResize(exp->HUDS, exp->SubSampled, CV_INTER_LINEAR);
		if (exp->VidHUDS==NULL ) printf("\tERROR in DoWriteToDisk!\n\texp->VidHUDS is NULL\n");
		if (exp->SubSampled ==NULL ) printf("\tERROR in DoWriteToDisk!\n\texp->exp->Subsampled==NULL\n");

		cvWriteFrame(exp->VidHUDS, exp->SubSampled);
	}

	/** Record data frame to diskl **/

	if (exp->RECORDDATA && exp->Params->Record) {
		TICTOC::timer().tic("AppendWormFrameToDisk");
		AppendWormFrameToDisk(exp->Worm, exp->Params, exp->DataWriter);
		TICTOC	::timer().toc("AppendWormFrameToDisk");
	}

}

/*
 * Use the slider bar to generate a rectangle in an arbitrary location and illuminate with it on the fly
 *
 */
int DoOnTheFlyIllumination(Experiment* exp) {
	CvSeq* montage = CreateIlluminationMontage(exp->Worm->MemScratchStorage);
	/** Note, out of laziness I am hardcoding the grid dimensions to be Numsegments by number of segments **/
	
	CvPoint origin = ConvertSlidlerToWormSpace(exp->Params->IllumSquareOrig,exp->Params->DefaultGridSize);
	int tmp;
	tmp=GenerateSimpleIllumMontage(montage, origin, exp->Params->IllumSquareRad, exp->Params->DefaultGridSize);
	/** Illuminate the worm **/
	/** ...in camera space **/
	IllumWorm(exp->Worm->Segmented, montage, exp->IlluminationFrame->iplimg,
			exp->Params->DefaultGridSize,exp->Params->IllumFlipLR);
			
	LoadFrameWithImage(exp->IlluminationFrame->iplimg, exp->IlluminationFrame);
	/** ... in DLP space **/
	IllumWorm(exp->segWormDLP, montage, exp->forDLP->iplimg,
			exp->Params->DefaultGridSize,exp->Params->IllumFlipLR);
	LoadFrameWithImage(exp->forDLP->iplimg, exp->forDLP);
	cvClearSeq(montage);
	return 0;

}

/**
 * Invert the illumination, so white becomes black and vice-versa.
 */
void InvertIllumination(Experiment* exp){
	IplImage* temp= cvCreateImage( cvSize(exp->IlluminationFrame->iplimg->width,exp->IlluminationFrame->iplimg->height),
			IPL_DEPTH_8U, 1);

	/** Invert Illumination Frame **/

	cvXorS(exp->IlluminationFrame->iplimg,cvScalar(255,255,255),temp);
	LoadFrameWithImage(temp,exp->IlluminationFrame);



	/** Invert DLP Frame **/
	cvXorS(exp->forDLP->iplimg,cvScalar(255,255,255),temp);
	LoadFrameWithImage(temp,exp->forDLP);


}


/*********************
 *
 *  Protocol related functions
 *
 */

/*
 * Load a protocol from a YAML file into the
 * experiment structure.
 *
 * Protocol filename must be specified in exp->protocolfname
 */
void LoadProtocol(Experiment* exp) {
	exp->p = LoadProtocolFromFile(exp->protocolfname);
	/** Set the protocol to be enabled by default. **/
	exp->Params->ProtocolUse = 1;
	exp->Params->ProtocolTotalSteps=exp->p->Steps->total;
}

/*
 *  Releases a protocol from the experiment object.
 */
void ReleaseProtocolFromExperiment(Experiment* exp) {
	if (exp->p == NULL)
		return;
	DestroyProtocolObject(&(exp->p));
	return;
}


/*
 * Writes a recent frame number to file
 */
int WriteRecentFrameNumberToFile(Experiment* exp){
	if (exp->RECORDDATA == 0) return 0;
	if (exp->Worm->frameNum < 0 || exp->DataWriter->filename == NULL  || build_git_sha == NULL) return -1;
	FILE* pFile;
	pFile = fopen("recentFrameNum.txt","w");
	fprintf(pFile,"%d\n%s\n%s\n%u",exp->Worm->frameNum,exp->DataWriter->filename,build_git_sha,exp->Params->DLPOn);
	fclose(pFile);
	return 0;
}




/**************************************************
 * Stage Tracking and FEedback System
 *
 * This should really probably go in a special library called Stage Tracking
 * that depends on both OpenCV AND Talk2STage.c, but its a huge pain to modify the makefile
 * to create a new library that has only one function in it.
 *
 * Alternatively this could conceivably go in Talk2Stage.c, but then I find it weird
 * that Talk2Stage.c should depend on OpenCV, because ultimatley it should be more general.
 *
 * It doesn't really belong in experiment.c either because it is not a method of experiment.c
 * But for now that is where it will sit.
 *
 * Update for DualMag:
 *  The dual magnification microscope presents a unique case. There I am trying to keep a spot on the
 *  worm centered in a very small box (the field of view of the high magnification objective, which is just
 *  a subset of the field of view of the low magnification objective). I want to work very hard to keep the object
 *  at the target location, so I want to have very high gain (speed). This gain varies linearly with the distance
 *  that the object is away from the target. But at some point, if the object is way off target, I don't actually want
 *  to go so fast. At that point I just give up and go at some set max speed.
 *
 *  In other words, the velocity of the stage is linear with the objects distance from the target up until a point.
 *  When the object is out side the activeZoneRadius, then the velocity is flat.
 *
 *  The user can set both the active zone radius and the gain (speed).
 */



CvPoint AdjustStageToKeepObjectAtTarget(HANDLE stage, CvPoint* obj,CvPoint target, int speed,int activeZoneRadius){
	if (obj==NULL){
		printf("Error! obj is NULL in AdjustStageToKeepObjectAtTarget()\n");
		return cvPoint(0,0);
	}

	if (obj==NULL){
		printf("Error! target is NULL in AdjustStageToKeepObjectAtTarget()\n");
		return cvPoint(0,0);
	}
//	printf("About to adjust stage.\n");

	CvPoint diff;
	CvPoint vel;
	/** (stage-obj)*speed **/
//	printf("obj= (%d, %d), target =(%d, %d)\n",obj->x, obj->y, target->x, target->y);

	diff.x=target.x-obj->x;
	diff.y=target.y-obj->y;

	//printf("About to Multiply!\n");
	vel.x= CropNumber(-activeZoneRadius,activeZoneRadius, diff.x)*speed;
	vel.y= CropNumber(-activeZoneRadius,activeZoneRadius, diff.y)*speed;

	//printf("SpinStage: vel.x=%d, vel.y=%d\n",vel.x,vel.y);
	spinStage(stage,vel.y,vel.x); //swapped values here to accomodate weird camera angle relative to stage

	return vel;

}


/*
 * Scan for the USB device.
 */
int InvokeStage(Experiment* exp){
	exp->stageCenter=cvPoint(NSIZEX/2 , NSIZEY/2 );

	exp->stage=InitializeUsbStage();
	if (exp->stage==NULL){
		printf("ERROR! Invoking the stage failed.\nTurning tracking off.\n");
		exp->Params->stageTrackingOn=0;
		return 0;
	} else {
		printf("Telling stage to HALT.\n");
		haltStage(exp->stage);
	}

}


int ShutOffStage(Experiment* exp){
	haltStage(exp->stage);
}

/*
 * Update the Stage Tracker.
 * If the Stage tracker is not initialized, don't do anything.
 * If the stage tracker is initialized then either do the tracking,
 * or if we are in the process of turning off tracking off, then tell
 * the stage to halt and update flags.
 */
int HandleStageTracker(Experiment* exp){
	if (exp->stageIsPresent==1){ /** If the Stage is Present **/
		if (exp->stage==NULL) return 0;

		if (exp->Params->stageTrackingOn==1){
			if (exp->Params->OnOff==0){ /** if the analysis system is off **/
				/** Turn the stage off **/
				exp->stageIsTurningOff=1;
				exp->Params->stageTrackingOn=0;
				printf("Setting flags to turn stage off in HandleStageTracker()\n");
			} else {
			/** Move the stage to keep the worm centered in the field of view **/
			
			
			/** Find The Point on the Worm To Center **/
			CvPoint* PtOnWorm;
			
				if (exp->Params->FluorMode == 0){
						
					/** Get the Point on the worm some distance along the centerline **/
					PtOnWorm = (CvPoint*) cvGetSeqElem(exp->Worm->Segmented->Centerline, exp->Params->stageTargetSegment);
				
				}else{
					/** Track based on the centroid of the binary image **/
					PtOnWorm = (CvPoint*) exp->Worm->FluorFeatures->centroid;
				}

				/** Adjust the stage velocity to keep that point centered in the field of view **/
				exp->Worm->stageVelocity=AdjustStageToKeepObjectAtTarget(exp->stage,PtOnWorm,exp->stageFeedbackTarget,exp->Params->stageSpeedFactor, exp->Params->stageROIRadius);

				printf("."); // ANDY: Consider removing this if it takes time.. 
			}
		}
		if (exp->Params->stageTrackingOn==0){/** Tracking Should be off **/
		//	printf("Tracking is off in HandleStageTracker()\n");
			/** If we are in the process of turning tacking off **/
			if (exp->stageIsTurningOff==1){
				/** Tell the stage to Halt **/
				printf("Tracking Stopped!");
				printf("Telling stage to HALT.\n");
				haltStage(exp->stage);
				exp->stageIsTurningOff=0;
			}
			/** The stage is already halted, so there is nothing to do. **/
			//printf("The stage is already halted in HandleStageTracker()\n.");
		}

	}
	return 0;
}



