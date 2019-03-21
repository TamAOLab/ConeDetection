/*
 *  radDefinition.h
 *  
 *
 *  Created by jianfei liu on 8/24/11.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef RADDEFINITION_H
#define RADDEFINITION_H

#include <itkRGBPixel.h>
#include "itkImage.h"
#include <itkImageRegionIteratorWithIndex.h>
#include <itkImageSeriesReader.h>
#include <itkImageSeriesWriter.h>
#include <itkGDCMSeriesFileNames.h>
#include <itkGDCMImageIO.h>
#include <itkPolygonSpatialObject.h>
#include <itkGroupSpatialObject.h>
#include <itkNeighborhoodIterator.h>
#include <itkPoint.h>
#include <itkPointSet.h>
#include <itkBinaryBallStructuringElement.h>
#include <itkHistogram.h>
#include <vtkSmartPointer.h>
#include <itkImageRegionConstIteratorWithIndex.h>
#include <itkVideoStream.h>

#include <vector>
#include <string>
#include <stdio.h>

using namespace std;

typedef float                               FloatPixelType;
typedef unsigned char                       BinaryPixelType;
typedef short                               ShortPixelType;

typedef itk::Image<FloatPixelType, 3>		FloatImageType;
typedef itk::Image<FloatPixelType, 2>		FloatImageType2D;
typedef itk::Image<BinaryPixelType, 2>		BinaryImageType2D;
typedef itk::Image<ShortPixelType, 2>		ShortImageType2D;

typedef itk::ImageRegionIteratorWithIndex<FloatImageType2D>     FloatIteratorType2D;
typedef itk::ImageRegionIteratorWithIndex<BinaryImageType2D>    BinaryIteratorType2D;

typedef itk::Point<short, 2>                ShortPointType2D;
typedef std::vector< ShortPointType2D >		ShortPointArray2D;

typedef itk::Point<double, 2>                DoublePointType2D;
typedef std::vector< DoublePointType2D >		DoublePointArray2D;

// ZC: zero crossing, EF: elliptic feature, LE: localized edge
enum FeatureType 
{
	BrightZC = 0x0001,
    DarkZC   = 0x0002,
    BrightEF = 0x0004,
    DarkEF   = 0x0008,
    BrightLE = 0x0010,
    DarkLE   = 0x0020
};

enum { Background = 0, Foreground = 255 };
enum NeighborhoodType { Four = 4, Eight = 8 };

struct RegionFeature 
{
	RegionFeature(){ memset(this, 0, sizeof(RegionFeature)); }
    ~RegionFeature(){}
    
    int area() const { return (xmax-xmin+1)*(ymax-ymin+1); }
    int volume() const { return (xmax-xmin+1)*(ymax-ymin+1)*(zmax-zmin+1); }
    
    int xmin, xmax, ymin, ymax, zmin, zmax; ///< range
    int xc, yc, zc, radius; ///< center of region
    int npix; ///< number of pixels(voxels)
    float mean, var; ///< statistical features
};

struct ConeDetectionParameters 
{
	int VotingRadius;
	double GradientMagnitudeThreshold;
	double Scale;
	bool DimConeDetectionFlag;
	double LOGResponse;
};

// Cone Detection Constant Parameters
const unsigned int PARA_RegionTypeBright = 0;
const unsigned int PARA_RegionTypeDark = 1;
const int PARA_VotingMinimumValue = 1;
const int PARA_VotingMaximumValue = 10;
const double PARA_VotingThreshold = 10.0;
const unsigned int PARA_WeightingMethod = 0;
const double PARA_WeightingParameter = 1.0;
const bool PARA_ZeroCrossing = false;
const bool PARA_AbsoulteThresholdFlag = true;

// Parameters used for post-processing
const unsigned int PARA_ClustingRadius = 1;
const unsigned int PARA_MergeRadius = 7;
const double PARA_LOGScale = 0.5;

struct SplitImageInformation
{
	std::pair<FloatImageType2D::Pointer, FloatImageType2D::Pointer> split_images;
	pair<string, string> split_files;
	DoublePointArray2D split_left_detections, split_right_detections;
	vector< float > split_left_detection_scales, split_right_detection_scales;
	vector< float > split_left_detection_weights, split_right_detection_weights;
	vector< pair<unsigned int, unsigned int> > split_detection_links;
	DoublePointArray2D split_final_detections;
	DoublePointArray2D split_edited_detections;

	void Initialize(bool flag)
	{
		split_left_detections.clear();
		split_right_detections.clear();

		split_left_detection_scales.clear();
		split_right_detection_scales.clear();

		split_left_detection_weights.clear();
		split_right_detection_weights.clear();

		split_detection_links.clear();
		split_final_detections.clear();
		split_edited_detections.clear();
	}
};

//used for scale selection
const unsigned int Number_Of_Scale_Levels=15;
const double  Scale_Interval=1.2;
const double  Initial_Scale=0.5;

#define INF 1E20
#define PI 3.1415926

#endif // RADDEFINITION_H
