/*
 *  radDetectionAverage.cpp
 *  
 *
 *  Created by Jianfei Liu on 9/19/11.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */

#include "raddetectionaverage.h"
#include <QDebug>
#include "QuickView.h"
#include <cfloat>
#include <limits>

#include "itkListSample.h"
#include "itkMeanSampleFilter.h"
#include "itkCovarianceSampleFilter.h"
const double IntensityDevaitionRatio = 0.5;

template <typename TVal>
void ComputeMeanAndVariance(vector<TVal> & vecs, TVal & mean, TVal & deviation)
{
	const unsigned int MeasurementVectorLength = 1;
	typedef itk::Vector< TVal, MeasurementVectorLength > MeasurementVectorType;
	typedef itk::Statistics::ListSample< MeasurementVectorType > SampleType;
    typename SampleType::Pointer sample = SampleType::New();
	sample->SetMeasurementVectorSize( MeasurementVectorLength );

    for (typename vector<TVal>::iterator it = vecs.begin(); it != vecs.end(); ++it)
	{
		MeasurementVectorType mv;
		mv[0] = *it;
		sample->PushBack( mv );
	}

	typedef itk::Statistics::MeanSampleFilter< SampleType > MeanAlgorithmType;
    typename MeanAlgorithmType::Pointer meanAlgorithm = MeanAlgorithmType::New();
	meanAlgorithm->SetInput( sample );
	meanAlgorithm->Update();
	mean = meanAlgorithm->GetMean()[0];

	typedef itk::Statistics::CovarianceSampleFilter< SampleType > CovarianceAlgorithmType;
    typename CovarianceAlgorithmType::Pointer covarianceAlgorithm = CovarianceAlgorithmType::New();
	covarianceAlgorithm->SetInput( sample );
	covarianceAlgorithm->Update();
	deviation = sqrt(covarianceAlgorithm->GetCovarianceMatrix()(0, 0));
}

int radDetectionAverage::SearchForNearestDetection(DoublePointType2D & bpt, DoublePointArray2D &dpt_array, double &min_dist)
{
	double threshold = 400;
	DoublePointArray2D::iterator d_it;
	double dist;

	min_dist = std::numeric_limits<double>::max();
	int result_id = -1;
	int id = 0;
	for (d_it=dpt_array.begin(); d_it != dpt_array.end(); ++d_it, ++id)
	{
		if (bpt[0] <= (*d_it)[0]) continue;

		dist = (bpt[0]-(*d_it)[0])*(bpt[0]-(*d_it)[0]) + (bpt[1]-(*d_it)[1])*(bpt[1]-(*d_it)[1]);
		if (dist < min_dist)
		{
			min_dist = dist;
			result_id = id;
		}
	}

	if (min_dist > threshold)
		return -1;
	else
		return result_id;
}

double radDetectionAverage::ComputeLinkAngle(DoublePointType2D &pt1, DoublePointType2D &pt2)
{
	if (pt1[0] == pt2[0])
	{
		return 90.;
	}
	else
	{
		return fabs(atan((pt1[1]-pt2[1])/(pt1[0]-pt2[0])) * 180 / PI);
	}
}

void radDetectionAverage::AdjustDetections(SplitImageInformation & split_infor, bool dark_flag)
{
	//Remove some false positives due to low contrast
	//get the mean and deviation of the current point set
	vector<float> intensity_array;
	float intensity_mean, intensity_deviation;
	FloatImageType2D::IndexType pixelIndex;
	for (unsigned int i=0; i<split_infor.split_left_detections.size(); i++)
	{
		//use four links and current pixel
		pixelIndex[0] = split_infor.split_left_detections[i][0];
		pixelIndex[1] = split_infor.split_left_detections[i][1];
		intensity_array.push_back(split_infor.split_images.first->GetPixel(pixelIndex));

		if (split_infor.split_left_detections[i][0]-1 >= 0)
			pixelIndex[0] = split_infor.split_left_detections[i][0]-1;
		intensity_array.push_back(split_infor.split_images.first->GetPixel(pixelIndex));

		if (split_infor.split_left_detections[i][0]+1 < split_infor.split_images.first->GetLargestPossibleRegion().GetSize()[0])
			pixelIndex[0] = split_infor.split_left_detections[i][0]+1;
		intensity_array.push_back(split_infor.split_images.first->GetPixel(pixelIndex));

		pixelIndex[0] = split_infor.split_left_detections[i][0];
		if (split_infor.split_left_detections[i][1]-1 >= 0)
			pixelIndex[1] = split_infor.split_left_detections[i][1]-1;
		intensity_array.push_back(split_infor.split_images.first->GetPixel(pixelIndex));

		if (split_infor.split_left_detections[i][1]+1 < split_infor.split_images.first->GetLargestPossibleRegion().GetSize()[1])
			pixelIndex[1] = split_infor.split_left_detections[i][1]+1;
		intensity_array.push_back(split_infor.split_images.first->GetPixel(pixelIndex));
	}
	ComputeMeanAndVariance(intensity_array, intensity_mean, intensity_deviation);

	//determine the cone detection link
	split_infor.split_detection_links.clear();
	vector< pair<double, int> >  bpt_links(split_infor.split_left_detections.size(), 
		pair<double, int>(1000000, -1));

	for (unsigned int i=0; i<split_infor.split_right_detections.size(); i++)
	{
		pixelIndex[0] = split_infor.split_right_detections[i][0];
		pixelIndex[1] = split_infor.split_right_detections[i][1];
		if (split_infor.split_images.first->GetPixel(pixelIndex) < intensity_mean+IntensityDevaitionRatio*intensity_deviation)
			continue;

		//search for the left part
		double min_dist;
		int result_id = SearchForNearestDetection(split_infor.split_right_detections[i], 
			split_infor.split_left_detections, min_dist);
		if (result_id != -1 && bpt_links[result_id].first > min_dist
			&& ComputeLinkAngle(split_infor.split_right_detections[i], split_infor.split_left_detections[result_id]) < 60)
		{
			bpt_links[result_id].first = min_dist;
			bpt_links[result_id].second = i;
		}
	}

	for (unsigned int i=0; i<bpt_links.size(); i++)
	{
		std::pair<unsigned int, unsigned int> tmp_pair;
		if (bpt_links[i].second != -1)
		{
			tmp_pair.first = bpt_links[i].second;
			tmp_pair.second = i;
			split_infor.split_detection_links.push_back(tmp_pair);
		}
	}

	//Adjust feature points
	split_infor.split_final_detections.clear();

	unsigned int bpt_id, dpt_id;
	DoublePointType2D bpt, dpt;
	double bpt_scale, dpt_scale;
	DoublePointType2D tmp_pt;
	vector<bool> AdjustedFeatures(split_infor.split_right_detections.size(), false);
	vector<bool> AdjustedFeatures1(split_infor.split_left_detections.size(), false);
	for (unsigned int i=0; i<split_infor.split_detection_links.size(); i++)
	{
		bpt_id = split_infor.split_detection_links[i].first;
		dpt_id = split_infor.split_detection_links[i].second;
		AdjustedFeatures[bpt_id] = true;
		AdjustedFeatures1[dpt_id] = true;

		bpt_scale = split_infor.split_right_detection_scales[bpt_id];
		dpt_scale = split_infor.split_left_detection_scales[dpt_id];

		bpt = split_infor.split_right_detections[bpt_id];
		dpt = split_infor.split_left_detections[dpt_id];

		tmp_pt[0] = (bpt_scale*bpt[0]+dpt_scale*dpt[0])/(bpt_scale+dpt_scale);
		tmp_pt[1] = (bpt_scale*bpt[1]+dpt_scale*dpt[1])/(bpt_scale+dpt_scale);

		split_infor.split_final_detections.push_back(tmp_pt);
	}

	for (unsigned int i=0; i<split_infor.split_right_detections.size(); i++)
	{
		if (!AdjustedFeatures[i])
		{
			pixelIndex[0] = split_infor.split_right_detections[i][0];
			pixelIndex[1] = split_infor.split_right_detections[i][1];

			if (split_infor.split_images.first->GetPixel(pixelIndex) > intensity_mean+IntensityDevaitionRatio*intensity_deviation)
			{
				split_infor.split_final_detections.push_back(split_infor.split_right_detections[i]);
			}
		}
	}

	if (dark_flag)
	{
		for (unsigned int i=0; i<split_infor.split_left_detections.size(); i++)
		{
			if (!AdjustedFeatures1[i])
			{
				pixelIndex[0] = split_infor.split_left_detections[i][0];
				pixelIndex[1] = split_infor.split_left_detections[i][1];

				if (split_infor.split_images.second->GetPixel(pixelIndex) > intensity_mean+IntensityDevaitionRatio*intensity_deviation)
				{
					split_infor.split_final_detections.push_back(split_infor.split_left_detections[i]);
				}
			}
		}
	}
}
