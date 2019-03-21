/*
 *  radSeedClustering.h
 *  
 *
 *  Created by Jianfei Liu on 9/19/11.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef radSeedClustering_H
#define radSeedClustering_H

#include "radimgfunc.h"
#include "itkBinaryImageToShapeLabelMapFilter.h"
#include "itkLaplacianRecursiveGaussianImageFilter.h"
#include <itkJoinSeriesImageFilter.h>
#include "radScaleSpace.h"

// Math
#include <math.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#include "radGraph.h"

using namespace rad_graph;

class radSeedClustering
{
private:
	unsigned int ClusteringRadius;
	unsigned int ClusteringRadius1;
	unsigned int MergeRadius;
    DoublePointArray2D m_Seeds;
	vector<short> m_SeedLabels;
	vector<float> m_CenterScales;
	vector<float> m_SeedWeights;

	//BinaryImageType2D::Pointer dilated_img;

	float m_ScaleResponse;
	float m_ScaleSize;

	//Peform Density-based spatial clustering of applications with noise (DBSCAN) algorithm
	void BuildConnectionGraph(rad_graph::Graph &, int);
	void Peform_DBSCAN_Clustering();

	FloatImageType2D::Pointer ComputeLOGImage(FloatImageType2D::Pointer, double);
	void ScaleSelection(FloatImageType2D::Pointer);
	void FindNearestPoints(DoublePointType2D &, vector<bool> &, vector<unsigned int> &);
	void AdjustLocalExtrema(ShortPointType2D, double, FloatImageType::Pointer, DoublePointType2D &);
	
public:
	
	radSeedClustering();
	~radSeedClustering();
	void SetClusteringRadius(int r) {ClusteringRadius = max<int>(r, 1);}
	void SetClusteringRadius1(int r) {ClusteringRadius1 = max<int>(r, 10);}
	void SetMergeRadius(int r) {MergeRadius = r;}
	void ClusteringSeeds(std::pair<FloatImageType2D::Pointer, FloatImageType2D::Pointer>, ShortPointArray2D &, vector< float > &);

	inline DoublePointArray2D &  GetClusteringSeeds() {return m_Seeds;}
	//inline vector<short> & GetSeedLabels() {return m_SeedLabels;}

	inline vector<float> & GetCenterScales() {return m_CenterScales;}
	inline vector<float> & GetSeedWeights() {return m_SeedWeights;}

	inline void SetScaleResponse(double val) {m_ScaleResponse = val;}
	inline void SetScaleSize(double val) {m_ScaleSize = val;}

	//inline BinaryImageType2D::Pointer GetDilatedImg() {return dilated_img;}

};

#endif // segFileIO_H

