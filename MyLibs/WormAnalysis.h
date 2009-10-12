/*
 * WormAnalysis.h
 *
 *  Created on: Oct 12, 2009
 *      Author: andy
 *
 *      This library contains functions that are specific to analyzing worms.
 *
 *      Functions in this library depend on:
 *      	AndysOpenCVLib.h
 *      	AndysComputations.h
 */

#ifndef WORMANALYSIS_H_
#define WORMANALYSIS_H_

typedef struct WormAnalysisParamStruct{
	int LengthScale;
	int LengthOffset;
	int BinThresh;
	int GaussSize;
} WormAnalysisParam;

typedef struct WormImageAnalysisStruct{
	CvSize SizeOfImage;
	IplImage* ImgOrig;
	IplImage* ImgSmooth;
	IplImage* ImgThresh;
	CvMemStorage* ImageStorage;
	CvMemStorage* ScratchStorage;
	CvSeq* InitialBoundary;
	CvPoint* Head;
	CvPoint* Tail;
	int TailIndex;
	int HeadIndex;
	CvSeq* Centerline;
	CvSeq* SegmentCenterline;
	CvSeq* SegmentLeft;
	CvSeq* SegmentRight;
}WormAnalysisData;


typedef struct WormIlluminationStcut{
	IplImage* IlluminationPattern;
	CvSeq* SegmentCenterline;
	CvSeq* SegmentLeft;
	CvSeq* SegmentRight;
	int* IlluminationArr;
}WormIlluminationData;




/*
 * Create Blank Images for WormAnalysisData given the image size.
 *
 */
void InitializeEmptyImages(WormAnalysisData* Worm, CvSize ImageSize);

/*
 * This function is run after IntializeEmptyImages.
 * And it loads a color original into the WoirmAnalysisData strucutre.
 * The color image is converted to an 8 bit black and white.
 *
 *
 */
void LoadColorOriginal(WormAnalysisData* Worm, IplImage* ImgColorOrig);

/*
 * De-Allocates images
 *
 * Will Also De-Allocate Memory
 *
 */
void DeAllocateWormAnalysisData(WormAnalysisData* Worm);

/*
 * Allocate MemoryStorage
 *
 */
int CreateWormMemStorage(WormAnalysisData* Worm);


/*
 * Clear MemoryStorage
 *
 */
int ClearWormMemStorage(WormAnalysisData* WormData);


/*
 * Find's the Worm Head and Tail
 *
 */
int WormFindHeadTail(WormAnalysisData* WormData, WormAnalysisParam* WormParams);


/*
 * Illuminate a segment of the worm. Either the right, or left side.
 * Takes a sequence of the centerline. Uses the preceeding point to find a vector
 * along the centerline, than draws a rectangle perpendicular to this vector, a radius rsquared pixels
 * away from the centerline
 */
void IlluminateWormSegment(IplImage** image, CvSeq* centerline, CvSeq* Boundary, int segment);









#endif /* WORMANALYSIS_H_ */
