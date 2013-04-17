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
 * WormIllumProtocol.c
 *
 * This library is designed to have objects and functions used to
 * write, analyze and excecute illumination protocols written for worms.
 *
 * As such it relies on the high level WormAnalysis library.
 * But Other libraries like WriteOutWorm likely depend upon it.
 *
 *
 *  Created on: Nov 11, 2009
 *      Author: Andy
 */

#include <stdio.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>

//OpenCV Headers
//#include <cxcore.h>
#include "opencv2/highgui/highgui_c.h"
//#include <cv.h>

// Andy's Libraries


#include "AndysOpenCVLib.h"
#include "WormAnalysis.h"
#include "IllumWormProtocol.h"
#include "version.h"
#include "AndysComputations.h"



/*******************************************/
/*
 * Protocol Objects
 */
/*******************************************/

/*
 * Create a Protocol Object and set all of its values to zero or NULL
 * The Protocol object has some descriptions and a CvSeq object that
 * points to sequences of polygons for illumination.
 * Each step can contain an arbitrary number of polygons.
 * Each polygon can contain an arbitrary number of points.
 *
 */
Protocol* CreateProtocolObject(){
	Protocol* MyProto= (Protocol*) malloc(sizeof(Protocol));
	MyProto->GridSize.height=0;
	MyProto->GridSize.width=0;
	MyProto->Filename=NULL;
	MyProto->Description=NULL;
	MyProto->Steps=NULL;
	MyProto->memory=cvCreateMemStorage();
	return MyProto;

}



/*
 * Write everything out to its own seperate YAML file
 *
 */
void WriteProtocolToYAML(Protocol* myP){
	/** Open file for writing **/
	printf("We are about to write out a protocol. Here is some info about the protocol.:\n");
	printf("The protocol has %d steps.\n",myP->Steps->total);


	CvFileStorage* fs=cvOpenFileStorage(myP->Filename,myP->memory,CV_STORAGE_WRITE);
	if (fs==0){
		printf("fs is zero! Could you have specified the wrong directory?\n");
		return;
	}
	printf("Writing to %s\n",myP->Filename);
	/** Write out Generic comments **/
	cvWriteComment(fs, "Illumination Protocol:",0);
	cvWriteComment(fs, "Generated by the IlluminationWormProtocol Library \nmade by leifer@fas.harvard.edu",0);
	cvWriteComment(fs, "\nSoftware Version Information:",0);
	cvWriteComment(fs, build_git_sha,0);
	cvWriteComment(fs, build_git_time,0);
	cvWriteComment(fs, "\n",0);
	printf("wrote comments\n");

	WriteProtocol(myP,fs);
	cvReleaseFileStorage(&fs);
}


/**
 * Write out a Protocol to YAML file, given an initialized CvFileStorage
 */
void WriteProtocol(Protocol* myP, CvFileStorage* fs){
	if (fs==0){
		printf("fs is zero! Could you have specified the wrong directory?\n");
		return;
	}
	/** Write out Protocol **/
	cvStartWriteStruct(fs,"Protocol",CV_NODE_MAP,NULL);
		if (myP->Filename!=NULL) cvWriteString(fs,"Filename",myP->Filename);
		if (myP->Description!=NULL)  cvWriteString(fs,"Description",myP->Description);
		cvStartWriteStruct(fs,"GridSize",CV_NODE_MAP,NULL);
			cvWriteInt(fs,"height",myP->GridSize.height);
			cvWriteInt(fs,"width",myP->GridSize.width);
		cvEndWriteStruct(fs);
		//printf("yo\n");
		/** Write Out Steps **/
		cvStartWriteStruct(fs,"Steps",CV_NODE_SEQ,NULL);
		int j;
		int jtot=myP->Steps->total;


		CvSeqReader StepReader;
		cvStartReadSeq(myP->Steps,&StepReader,0);
		for (j = 0; j < jtot; ++j) {
			//printf("About to write step number %d\n",j);
			CvSeq** CurrentMontagePtr = (CvSeq**) StepReader.ptr;
			CvSeq* CurrentMontage=*CurrentMontagePtr;
			assert(CurrentMontage!=NULL);
		//	printf("ping\n");
		//	printf("CurrentMontage->total=%d",CurrentMontage->total);
			cvStartWriteStruct(fs,NULL,CV_NODE_SEQ,NULL);
			int k;
			int ktot=CurrentMontage->total;
		//	printf("ktot=%d\n",ktot);

			CvSeqReader MontageReader;
			cvStartReadSeq(CurrentMontage,&MontageReader);
			for (k = 0; k < ktot; ++k) {
			//	printf("About to write polygon number %d\n",k);
				WormPolygon** CurrentPolygonPtr= (WormPolygon**) MontageReader.ptr;
				WormPolygon* CurrentPolygon=*CurrentPolygonPtr;

				cvWrite(fs,NULL,CurrentPolygon->Points);

				CV_NEXT_SEQ_ELEM(CurrentMontage->elem_size,MontageReader);
			}
			CurrentMontagePtr=NULL;
			CurrentMontage=NULL;
			cvEndWriteStruct(fs);

			/** Loop to Next Step **/
			CV_NEXT_SEQ_ELEM(myP->Steps->elem_size,StepReader);

		}
		cvEndWriteStruct(fs);
	cvEndWriteStruct(fs);
}



void LoadProtocolWithFilename(const char* str, Protocol* myP){
	assert(myP!=NULL);
	myP->Filename = copyString(str);

}

void LoadProtocolWithDescription(const char* str, Protocol* myP){
	myP->Description= (char *) malloc(sizeof(char)*(strlen(str)+1));
	strcpy(myP->Description,str);
}





void DestroyProtocolObject(Protocol** MyProto){
	/** **/
	assert(MyProto!=NULL);
	if (*MyProto==NULL) return;
	if  ((*MyProto)->Filename!=NULL ) {
		free( &((*MyProto)->Filename));
		(*MyProto)->Filename=NULL;
	}

	printf("about to cycle through...\n");

	if ((*MyProto)->Steps!=NULL){
	//	printf("(*MyProto)->Steps!=NULL\n");
		int numsteps=(*MyProto)->Steps->total;
	//	printf("numsteps=%d\n",numsteps);
		for (int step= 0; step < numsteps ; ++step) {
			CvSeq** montagePtr=(CvSeq**) cvGetSeqElem((*MyProto)->Steps,step);
			CvSeq* montage=*montagePtr;
			if (montage!=NULL){
		//		printf("montage!=NULL\n");
				int numpolygons=montage->total;
			//	printf("numpolygons=%d\n",numpolygons);
				for (int k= 0; k< numpolygons; ++k) {
					WormPolygon** polygonPtr=(WormPolygon**) cvGetSeqElem(montage,k);
					WormPolygon* polygon=*polygonPtr;
		//			printf( "\tFound polygon with %d nubmer of points.\n",polygon->Points->total );
					DestroyWormPolygon(&polygon);
		//			printf("Destroying polygon...\n");

				}
			}

		}
	}


	if  ((*MyProto)->Description!=NULL ) {
		free( &((*MyProto)->Description) );
		(*MyProto)->Description=NULL;
	}

	cvReleaseMemStorage(&(*MyProto)->memory);
	free(MyProto);
	*MyProto=NULL;
}





/*******************************************/
/*
 * Steps Objects
 */
/*******************************************/


/*
 * A steps object is a cvSeq containing pointers to
 * Illumination Montages (whcih are in turn cvSeq's of pointers to Polygons).
 *
 */
CvSeq* CreateStepsObject(CvMemStorage* memory){
	if (memory==NULL) {
		printf("ERROR! memory is null in CreateStepsObject()!\n");
		return NULL;
	}
	CvSeq* mySteps=cvCreateSeq(CV_SEQ_ELTYPE_PTR,sizeof(CvSeq),sizeof(CvSeq*),memory);
	return mySteps;
}




/*******************************************/
/*
 * Illumination Objects
 */
/*******************************************/

/*
 * An illumination montage consists of a sequence of pointers to polygons
 *
 */
CvSeq* CreateIlluminationMontage(CvMemStorage* memory){
	CvSeq* myIllumMontage=cvCreateSeq(CV_SEQ_ELTYPE_PTR,sizeof(CvSeq),sizeof(WormPolygon*),memory);
	return myIllumMontage;
}



/*******************************************/
/*
 * Polygon Objects
 */
/*******************************************/

/*
 * Given a memory object, this will create a polygon object that is a CvSeq.
 *
 */
WormPolygon* CreateWormPolygon(CvMemStorage* memory,CvSize mySize){
	WormPolygon* myPoly=(WormPolygon*) malloc(sizeof(WormPolygon));
	myPoly->Points=cvCreateSeq(CV_SEQ_ELTYPE_POINT,sizeof(CvSeq),sizeof(CvPoint),memory);
	myPoly->GridSize=mySize;
	return myPoly;
}

/*
 *
 * Creates a worm polygon object from a CvSeq of Points.
 * This will clone the CvSeq and copy it into the memory storage
 * specified
 */
WormPolygon* CreateWormPolygonFromSeq(CvMemStorage* memory,CvSize GridSize,CvSeq* points){
	WormPolygon* myPoly=(WormPolygon*) malloc(sizeof(WormPolygon));
	myPoly->Points=cvCloneSeq(points,memory);
	myPoly->GridSize=GridSize;
	return myPoly;
}

//

/*
 * Destroys a polygon but doesn't free up the CvMemStorage that that polygon used
 *
 */
void DestroyWormPolygon(WormPolygon** myPoly){
	free(myPoly); 
	*myPoly=NULL;
}

/*
 * Simple function to copy a string
 */
char *copyString (const char *src) {
	assert (src != NULL);
	char *dst = (char *) malloc (strlen(src)+ 1);;
	strcpy(   dst, src);
	return dst;
}




void PrintPointsOfSeq(CvSeq* seq){
	int numpts=seq->total;
	CvSeqReader PtReader;
	cvStartReadSeq(seq,&PtReader,0);
	int i;
	for (i = 0; i < numpts; ++i) {
		CvPoint* pt= (CvPoint*) PtReader.ptr;

		printf("[%d,",pt->x);
		printf("%d] ",pt->y);
		CV_NEXT_SEQ_ELEM(seq->elem_size,PtReader);
	}
	printf("\n");

}


int VerifyProtocol(Protocol* p){
	printf("\n\n========== VERIFYING PROTOCOL============\n");
	if (p==NULL){
		printf("Protocol is NULL\n");
		return -1;
	}

	printf("Protocol description: %s\n", p->Description);
	printf("Filename= %s\n",p->Filename);
	printf("Total number of steps: p->Steps->total=%d\n",p->Steps->total);

	CvSeqReader StepReader;
	cvStartReadSeq(p->Steps, &StepReader,0);
	int numsteps=p->Steps->total;
	/** Let's loop through all of the steps **/
		for (int i= 0; i< numsteps; ++i) {
			printf("Step i=%d\n",i);

			CvSeq* CurrMontage= *( (CvSeq**) StepReader.ptr);
			printf("\tCurrMontage has %d polygons\n",CurrMontage->total);
			int numPolygons=CurrMontage->total;
			int j;


			/** Let's loop through the polygons **/

			CvSeqReader MontageReader;
			cvStartReadSeq(CurrMontage, &MontageReader);
			for (j = 0; j < numPolygons; ++j) {
				WormPolygon*  poly= *( (WormPolygon**) MontageReader.ptr );
				int numpts=poly->Points->total;
				printf(" numpts=%d\n",numpts);




				//PrintPointsOfSeq(poly->Points);




				CV_NEXT_SEQ_ELEM( CurrMontage->elem_size,MontageReader);
			}




			/** Progress to the next step **/
			CV_NEXT_SEQ_ELEM( p->Steps->elem_size, StepReader );
		}


	printf("========================================\n");
	return 0;
}



/************************************
 *
 * Illumination Routines
 *
 */


/*
 * This function makes a black image.
 *
 */
IplImage* GenerateRectangleWorm(CvSize size){
	/** Create a rectangular worm with the correct dimensions **/
	IplImage* rectWorm=cvCreateImage(size,IPL_DEPTH_8U,1);
	cvZero(rectWorm);
	return rectWorm;
}




/*
 * Returns the pointer to a montage of polygons corresponding
 * to a specific step of a protocol
 *
 * Note: This returns the sparse form of the polygon. There are only
 * as many vertices in the polygon as the author of the protocol defined.
 * e.g. a square in worm space may only be defined by four points.
 */
CvSeq* GetMontageFromProtocol(Protocol* p, int step){
	CvSeq** montagePtr=(CvSeq**) cvGetSeqElem(p->Steps,step);
	return *montagePtr;
}





/*
 * Given a Montage CvSeq of WormPolygon objects,
 * This function allocates memory for an array of
 * CvPoints and returns that.
 *
 * Returns an integer with the number of points in the polygon.
 *
 * When using this function, don't forget to release the allocated
 * memory.
 *
 */
int CreatePointArrFromMontage(CvPoint** polyArr,CvSeq* montage,int polygonNum){
	if (polygonNum >= montage->total){
		printf("ERROR! GetPointArrFromMontage() was asked to fetch the %dth polygon, but this montage only has %d polygons\n",polygonNum,montage->total);
		return -1;
	}
	WormPolygon** polygonPtr=(WormPolygon**) cvGetSeqElem(montage,polygonNum);
	WormPolygon* polygon=*polygonPtr;

	/** Allocate memory for a CvPoint array of points **/
	*polyArr= (CvPoint*) malloc( sizeof(CvPoint)*polygon->Points->total);
	cvCvtSeqToArray(polygon->Points,*polyArr,CV_WHOLE_SEQ);

	return polygon->Points->total;
}


/*
 * Takes an illumination montage containing polygons and converts it to an illumination montage
 * that has polygons with interpolated vertices, (called contour)
 *
 * Note: ContourMontage must already be created and have memstorage associated with it
 */
int CvtPolyMontage2ContourMontage(CvSeq* PolyMontage, CvSeq* ContourMontage){
	int numOfPolys=PolyMontage->total;
	CvSeqReader PolyReader;
	cvStartReadSeq(PolyMontage,&PolyReader);
		int poly;
		for (poly = 0; poly < numOfPolys; ++poly) {
			WormPolygon** polygonPtr=(WormPolygon**) PolyReader.ptr;
			WormPolygon* polygon=*polygonPtr;

			/** Create new contour  using ContourMontage's memstorage**/
			CvSeq* newcontour = cvCreateSeq(CV_SEQ_ELTYPE_POINT, sizeof(CvSeq), sizeof(CvPoint),ContourMontage->storage);

			/** Convert the polygon to the contour (interpolated vertices) **/
			CvtPolySeq2ContourSeq(polygon->Points,newcontour);

			/** package the contour as a WormPolygon object for consistency **/
			/*
			 * yes i realize this is stupid. Here is why its stupid:
			 *   1) its no longer a traditional polygon. its a contour, so i shouldn't make it a WormPolygon object
			 *   2) its completley unnecessary. There is no information on the WormPolygon object hat would be useful
			 *
			 *   But i need to do it anyway because otherwise functions that work with polygon montages will not
			 *   work with contour montages and that would be REALLY stupid.
			 *
			 */
			WormPolygon* wrappedContour= CreateWormPolygonFromSeq(ContourMontage->storage,polygon->GridSize,newcontour);


			/** Push the new contour onto the ContourMontage **/
			cvSeqPush(ContourMontage,&wrappedContour);

			/** Move to the next polygon **/
			CV_NEXT_SEQ_ELEM(PolyMontage->elem_size,PolyReader);
				
			printf("Leaking memory here IllumWormProtocol.c line 528\n");
			// DestroyWormPolygon(&wrappedContour);



		}
		return 1;
}


/*
 * Returns a pointer to a montage of illumination polygons
 * corresponding to a specific protocol step.
 *
 * NOTE: all polygons have been converted into contours so that they
 * have at least one vertex per grid point on the worm-grid
 */
CvSeq* GetMontageFromProtocolInterp(Protocol* p, int step){
	CvSeq** polymontagePtr=(CvSeq**) cvGetSeqElem(p->Steps,step);
	CvSeq* montage=CreateIlluminationMontage(p->memory);
	CvtPolyMontage2ContourMontage(*polymontagePtr,montage);
	return montage;
}


/*
 * This function generates an illumination montage containing a square in worm space
 * at a location specified.
 *
 * This function is most useful when it takes input from a slider bar.
 *
 * In that manner the user can specify a rectangular (in worm space) region of illumination
 * at run time.
 *
 */
int GenerateSimpleIllumMontage(CvSeq* montage, CvPoint origin, CvSize radius, CvSize gridSize){
	/** If the square has no length or width, then  there is nothing to do **/
	if (radius.width == 0 || radius.height ==  0) return 0;

	/** If the y value extends off the worm, we need to crop it.. (otherwise it wraps... weird!)**/


	cvClearSeq(montage);
	WormPolygon* polygon=CreateWormPolygon(montage->storage,gridSize);


	CvPoint curr;

	/** Lower Left **/
	curr=cvPoint(origin.x-radius.width, CropNumber(1,gridSize.height-1,origin.y-radius.height));
	cvSeqPush(polygon->Points,&curr);

	/** Lower Right **/
	curr=cvPoint(origin.x+radius.width, CropNumber(1,gridSize.height-1,origin.y-radius.height));
	cvSeqPush(polygon->Points,&curr);

	/** Upper Right **/
	curr=cvPoint(origin.x+radius.width, CropNumber(1,gridSize.height-1,origin.y+radius.height));
	cvSeqPush(polygon->Points,&curr);


	/** Upper Left **/
	curr=cvPoint(origin.x-radius.width, CropNumber(1,gridSize.height-1,origin.y+radius.height));
	cvSeqPush(polygon->Points,&curr);
	

	/** Push this sparse polygon onto the temp montage **/
	CvSeq* tempMontage= CreateIlluminationMontage(montage->storage);
	cvSeqPush(tempMontage,&polygon);
	/** Convert the sparse polygon into a contour-polygon **/
	CvtPolyMontage2ContourMontage(tempMontage,montage);
	cvClearSeq(tempMontage);
	return 1;

}




void DisplayPtArr(CvPoint* PtArr,int numPts){
	int k=0;
	for (k = 0; k < numPts; ++k) {
				printf(" (%d,",PtArr[k].x);
				printf(" %d)\n",PtArr[k].y);


	}
}

/*
 * Given an array of CvPoint(s), go through and add an offset
 * to all of the x or y values.
 *
 * Use XorY=0 to change the offset of the x values.
 * Use XorY=1 to change the offset of the y values.
 *
 */
void OffsetPtArray(CvPoint** Pts,int numPts,int offset,int XorY){
	if (*Pts==NULL){
		printf("Error! Pts is NULL.\n");
		return;
	}

	/** lPts is just a shortcut for *Pts **/
	CvPoint* lPts=*Pts;

	/** Loop through Array **/
	int k;
	for (k = 0; k < numPts; ++k) {
		if (XorY==0){
			lPts[k].x=lPts[k].x+offset;
		} else {
			lPts[k].y=lPts[k].y+offset;
		}
	}

}

/**********************************
 *   ACTUALLY ILLUMINATE
 *
 *
 */

/*
 * Illuminate a rectangle worm (worm space)
 */
void IllumRectWorm(IplImage* rectWorm,Protocol* p,int step,int FlipLR){
	CvSeq* polyMontage= GetMontageFromProtocol(p,step);
	CvSeq* montage=CreateIlluminationMontage(p->memory);
	CvtPolyMontage2ContourMontage(polyMontage,montage);

	int numOfPolys=montage->total;
	int numPtsInCurrPoly;
	CvPoint* currPolyPts=NULL;
	int poly;
	for (poly = 0; poly < numOfPolys; ++poly) {
		//printf("==poly=%d==\n",poly);
		numPtsInCurrPoly=CreatePointArrFromMontage(&currPolyPts,montage,poly);
		//DisplayPtArr(currPolyPts,numPtsInCurrPoly);

		if (FlipLR==1) {
				int k=0;
				for (k = 0; k < numPtsInCurrPoly; ++k) {
						currPolyPts[k].x=currPolyPts[k].x *-1;
				}
			}



		OffsetPtArray(&currPolyPts,numPtsInCurrPoly, (int) (p->GridSize.width / 2) ,0);

		cvFillConvexPoly(rectWorm,currPolyPts,numPtsInCurrPoly,cvScalar(255,255,255),CV_AA);


		free(currPolyPts);
	}
	cvClearSeq(montage);

}


/*
 *
 * This function takes a point defined in the grid of the worm (0,0 is the head... etc)
 * And converts that point into the coordinate system of the image.
 *
 * It takes as arguments a Segmentedworm* object which dfines the location of the boundaries
 * of a worm in an image.
 *
 * This function is used by IlilumWorm to illuminate a worm.
 *
 * Also takes an int FlipLR. When FlipLR is 1, the left/right coordinates are flipped.
 * This is useful for inverting an image in teh dorsal-ventral plane.
 */
CvPoint CvtPtWormSpaceToImageSpace(CvPoint WormPt, SegmentedWorm* worm, CvSize gridSize, int FlipLR){

	/** Find the coordinate in imspace of the pt on centerline corresponding to this y value **/
	CvPoint* PtOnCenterline=(CvPoint*) cvGetSeqElem(worm->Centerline,WormPt.y);

	/** Find the Corresponding y-value point on the boundary **/

	/** Depending on whether our pt is in the right half or the left half... **/
	if (WormPt.x==0){
			/** If the point is zero, return a point on the centerline **/
			return *PtOnCenterline;
		}

	/** If FlipLR is flagged, then flip the x-values **/
	if (FlipLR==1){
		WormPt.x=WormPt.x * -1;
	}

	CvPoint* PtOnBound;
	float sign = 1.0;
	if ( WormPt.x>0 ){
		/** We'll use the right boundary **/
		PtOnBound=(CvPoint*) cvGetSeqElem(worm->RightBound,WormPt.y);
		//sign=1;
	}else {
		/** We'll use the left boundary **/
		PtOnBound=(CvPoint*) cvGetSeqElem(worm->LeftBound,WormPt.y);
		sign = -1.0;
	}

	/**Create a vector from the centerline to the corresponding point on the boundary**/
	CvPoint vecToBound=cvPoint(PtOnBound->x - PtOnCenterline->x,PtOnBound->y - PtOnCenterline->y);

		/** (evidently important stuff happens here) **/
	float ScaleRadius = (float) (gridSize.width-1)/2;
	/** Find fractional value of x in worm space relative to the x grid dimension... **/
	float fracx=  sign * (float) WormPt.x / ScaleRadius;

	/** Pt out = pt on the centerline + scaled vector towards point on the boundary **/
	float outX= (float) (PtOnCenterline->x) + (fracx * (float) vecToBound.x);
	float outY= (float) (PtOnCenterline->y) + (fracx * (float) vecToBound.y);

	return cvPoint( (int)  (outX+.5*sign), (int) (outY+.5*sign));


}


/*
 * Creates an illumination
 * according to an illumination montage and the location of a segmented worm.
 *
 * To use with protocol, use GetMontageFromProtocolInterp() first
 *
 * FlipLR is a bool. When set to 1, the illumination pattern is reflected across the worm's centerline.
 */
void IllumWorm(SegmentedWorm* segworm, CvSeq* IllumMontage, IplImage* img,CvSize gridSize, int FlipLR){
	int DEBUG=0;
	if (DEBUG) printf("In IllumWorm()\n");
	CvPoint* polyArr=NULL;
	int k;
	int numpts=0;
	for (k = 0; k < IllumMontage->total; ++k) {

		numpts=CreatePointArrFromMontage(&polyArr,IllumMontage,k);
		//ReturnHere
		int j;
		//DisplayPtArr(polyArr,numpts);
		CvPoint* ptPtr=polyArr;
		for (j = 0; j < numpts; ++j) {
			/** make a local copy of the current pt in worm space **/
			CvPoint wormPt=*(ptPtr);
			/** replace that point with the new pt in image space **/
			*(ptPtr)=CvtPtWormSpaceToImageSpace(wormPt,segworm, gridSize,FlipLR);
			/** move to the next pointer **/
			ptPtr++;
		}



		if (DEBUG) {
				int i;
			printf("new polygon\n");
			for (i = 0; i < numpts; i++) {
				printf(" (%d, %d)\n",polyArr[i].x,polyArr[i].y);
				cvCircle(img, polyArr[i], 1, cvScalar(255, 255, 255), 1);
				cvShowImage("Debug",img);
				cvWaitKey(10);
			}

		}


		/** Actually draw the polygon **/
		cvFillPoly(img,&polyArr,&numpts,1,cvScalar(255,255,255),8);

		free(polyArr);
		polyArr=NULL;
	}

	if (DEBUG)	{
		IplImage* TempImage=cvCreateImage(cvGetSize(img),IPL_DEPTH_8U,1);
		DrawSequence(&TempImage,segworm->LeftBound);
		DrawSequence(&TempImage, segworm->RightBound);
		double weighting=0.4;
		cvAddWeighted(img,weighting,TempImage,1,0,TempImage);
		cvShowImage("Debug",TempImage);
	}

}




/************************************************
 *
 *
 * VERY HIGH LEVEL
 *
 */


/*
 * Illuminate the Segmented worm using the protocol in p
 * with step specified in Params->ProtocolStep
 *
 * and writing to dest
 */
int IlluminateFromProtocol(SegmentedWorm* SegWorm,Frame* dest, Protocol* p,WormAnalysisParam* Params){

	/** Check to See if the Worm->Segmented has any NULL values**/
	if (SegWorm->Centerline==NULL || SegWorm->LeftBound==NULL || SegWorm->RightBound ==NULL ){
		printf("Error! The Worm->Segmented had NULL children. in SimpleIlluminateWorm()\n");
		return -1;
	}



	/** Check to See that the Segmented Values are Not Zero **/
	if (SegWorm->Centerline->total==0 || SegWorm->LeftBound->total==0 || SegWorm->RightBound->total ==0 ){
		printf("Error! At least one of the following: Centerline or Right and Left Boundaries in Worm->Segmented has zero points in SimpleIlluminateWorm()\n");
		return -1;
	}

	/** Create a Temp Image **/
	IplImage* TempImage=cvCreateImage(cvGetSize(dest->iplimg), IPL_DEPTH_8U, 1);

	/** Grab a montage for the selected step **/
	CvSeq* montage=GetMontageFromProtocolInterp(p,Params->ProtocolStep);
	IllumWorm(SegWorm,montage,TempImage,p->GridSize,Params->IllumFlipLR);
	LoadFrameWithImage(TempImage,dest);

//	cvClearSeq(montage);
	cvReleaseImage(&TempImage);
	return 0;
}


/**********************
 *
 * File Input/Output
 *
 */

/*
 * Load a  Protocol From yaml File
 *
 */
Protocol* LoadProtocolFromFile(const char* filename){
		Protocol* myP=CreateProtocolObject();
		LoadProtocolWithFilename(filename,myP);
		CvFileStorage* fs=cvOpenFileStorage(myP->Filename,0,CV_STORAGE_READ);
		printf("Opened File: %s for reading\n", myP->Filename);
		/** Point to Protocol Object **/
		CvFileNode* protonode=cvGetFileNodeByName(fs,NULL,"Protocol");

		/** Load in Description **/
		CvFileNode* node=cvGetFileNodeByName(fs,protonode,"Description");
		myP->Description=copyString(cvReadString(node,NULL));
		printf("Loading in Protocol, Description:\n %s\n",myP->Description);

		/** Load in Grid Size **/
		node=cvGetFileNodeByName(fs,protonode,"GridSize");
		int height=cvReadIntByName(fs,node,"height",-1);
		int width=cvReadIntByName(fs,node,"width",-1);
				printf(" width =%d\n",width );
								printf(" height=%d\n",height);
		if (height>0 && width>0){
			myP->GridSize=cvSize(width,height);
		}

		/** Create the Steps Object and Load it into the Protocol **/
		myP->Steps=CreateStepsObject(myP->memory);

		/** Point to the Steps node  in the YAML file **/
		node=cvGetFileNodeByName(fs,protonode,"Steps");



		/** Create a local object that contains the information of the steps **/
		CvSeq* stepSeq=node->data.seq;
		int numsteps=stepSeq->total;
		printf("numsteps=%d\n",numsteps);

		CvSeqReader StepReader;
		cvStartReadSeq( stepSeq, &StepReader, 0 );

		/** Let's loop through all of the steps **/
		for (int i= 0; i< numsteps; ++i) {

			/**Create Illumination Montage Object **/
			CvSeq* montage=CreateIlluminationMontage(myP->memory);

			/** Find the node of the current image montage (step) **/
			CvFileNode* montageNode = (CvFileNode*)StepReader.ptr;

			CvSeq* montageSeq=montageNode->data.seq;
			int numPolygonsInMontage=montageSeq->total;
//			printf("Step %d: %d polygon(s) found\n",i,numPolygonsInMontage);

			CvSeqReader MontageReader;
			cvStartReadSeq( montageSeq, &MontageReader, 0 );

			/** Loop through all of the polygons **/
			for (int k = 0; k < numPolygonsInMontage; ++k) {
				/** Load the CvSeq Polygon Objects and push them onto the montage **/
				CvFileNode* polygonNode = (CvFileNode*)MontageReader.ptr;
				CvSeq* polygonPts =(CvSeq*) cvRead(fs,polygonNode); // <---- Andy come back here.
				printf("\tStep %d, Polygon %d: %d points found.\n",i,k,polygonPts->total);

				/**
				 * Now we have the points for our polygon so we need to load
				 * those points into a polygon object
				 */
				WormPolygon* polygon= CreateWormPolygonFromSeq(myP->memory,myP->GridSize,polygonPts);
				//printf("\t\t %d points copied\n",polygon->Points->total);

				/** Add the polygon to the montage **/
				cvSeqPush(montage,&polygon);
				//printf("\t\t Current montage now has %d polygons\n",montage->total);

				/** Move to the next polygon **/
				CV_NEXT_SEQ_ELEM( montageSeq->elem_size, MontageReader );
			}
			cvClearSeq(montageSeq);
			numPolygonsInMontage=0;

			//printf("Loading a montage with %d polygons on the protocol\n.",montage->total);
			/** Load the montage onto the step object**/
			cvSeqPush(myP->Steps,&montage);

			/** Progress to the next step **/
			CV_NEXT_SEQ_ELEM( stepSeq->elem_size, StepReader );

		}

		return myP;

}




