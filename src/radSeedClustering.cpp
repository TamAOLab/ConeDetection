/*
 *  radSeedClustering.cpp
 *  
 *
 *  Created by Jianfei Liu on 9/19/11.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */

#include "radSeedClustering.h"
#include <QDebug>
#include "QuickView.h"

#include "itkListSample.h"
#include "itkVector.h"
#include "itkMatrix.h"
#include "itkMeanSampleFilter.h"
#include "itkCovarianceSampleFilter.h"
#include "vnl/vnl_matrix_fixed.h"
#include "vnl/vnl_matrix.h"
#include "vnl/vnl_vector.h"
#include "vnl/vnl_vector_fixed.h"
#include "vnl/algo/vnl_svd.h"
#include "vnl/vnl_sample.h"
#include "vnl/algo/vnl_qr.h"
#include <math.h>

#if 0
// intermediate type used for DoG pyramids
static const int SIFT_FIXPT_SCALE = 48;
#else
// intermediate type used for DoG pyramids
static const int SIFT_FIXPT_SCALE = 1;
#endif

// maximum steps of keypoint interpolation before failure
static const int SIFT_MAX_INTERP_STEPS = 5;

// width of border in which to ignore keypoints
// static const int SIFT_IMG_BORDER = 5;

using namespace scalespace;


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

radSeedClustering::radSeedClustering()
{
	ClusteringRadius1 = 40;
	MergeRadius = 7;

	m_ScaleResponse = 1.0;
	m_ScaleSize = 0.5;
}

radSeedClustering::~radSeedClustering()
{ 
}

void radSeedClustering::ClusteringSeeds(std::pair<FloatImageType2D::Pointer, FloatImageType2D::Pointer> img_pair, 
										ShortPointArray2D & pts, vector< float > & wts)
{
	BinaryImageType2D::Pointer tmp_img = AllocateImage<FloatImageType2D, BinaryImageType2D>(img_pair.first);
	FloatImageType2D::Pointer tmp_img1 = AllocateImage<FloatImageType2D, FloatImageType2D>(img_pair.first);
	tmp_img->FillBuffer(0);
	tmp_img1->FillBuffer(0);

	BinaryImageType2D::IndexType pixelIndex;
	for (ShortPointArray2D::iterator it=pts.begin(); it != pts.end(); ++it)
	{
		pixelIndex[0] = (*it)[0];
		pixelIndex[1] = (*it)[1];
		tmp_img->SetPixel(pixelIndex, 255);
		tmp_img1->SetPixel(pixelIndex, wts[(it-pts.begin())]);
	}
	
	BinaryImageType2D::Pointer dilated_img = BinaryDilate<BinaryImageType2D>(tmp_img, ClusteringRadius);
	//dilated_img = BinaryDilate<BinaryImageType2D>(tmp_img, ClusteringRadius);

	typedef itk::BinaryImageToShapeLabelMapFilter<BinaryImageType2D> BinaryImageToShapeLabelMapFilterType;
	BinaryImageToShapeLabelMapFilterType::Pointer binaryImageToShapeLabelMapFilter = BinaryImageToShapeLabelMapFilterType::New();
	binaryImageToShapeLabelMapFilter->SetInput(dilated_img);
	binaryImageToShapeLabelMapFilter->Update();
 
	// The output of this filter is an itk::ShapeLabelMap, which contains itk::ShapeLabelObject's
	//std::cout << "There are " << binaryImageToShapeLabelMapFilter->GetOutput()->GetNumberOfLabelObjects() << " objects." << std::std::endl;
 
	// Loop over all of the blobs
	ShortPointType2D pt;
	m_Seeds.clear();
	m_SeedWeights.clear();
	float max_weight;
	for(unsigned int i = 0; i < binaryImageToShapeLabelMapFilter->GetOutput()->GetNumberOfLabelObjects(); i++)
    {
		BinaryImageToShapeLabelMapFilterType::OutputImageType::LabelObjectType* labelObject 
			= binaryImageToShapeLabelMapFilter->GetOutput()->GetNthLabelObject(i);
		// Output the bounding box (an example of one possible property) of the ith region
		//std::cout << "Object " << i << " has bounding box " << labelObject->GetBoundingBox() << std::std::endl;
		pt[0] = (labelObject->GetCentroid()[0]-img_pair.first->GetOrigin()[0])/img_pair.first->GetSpacing()[0];
		pt[1] = (labelObject->GetCentroid()[1]-img_pair.first->GetOrigin()[1])/img_pair.first->GetSpacing()[1];
		max_weight = 0;
		for (unsigned int j=0; j<labelObject->Size(); j++)
		{
			max_weight = std::max<float>(tmp_img1->GetPixel(labelObject->GetIndex(j)), max_weight);
		}
		m_Seeds.push_back(pt);
		m_SeedWeights.push_back(max_weight);

		//used for dialated image
		/*if (labelObject->Size() < 20)
		{
			for (unsigned int j=0; j<labelObject->Size(); j++)
			{
				dilated_img->SetPixel(labelObject->GetIndex(j), 0);
			}
		}*/
    }

	Peform_DBSCAN_Clustering();

	ScaleSelection(img_pair.first);
}

void radSeedClustering::BuildConnectionGraph(rad_graph::Graph & connection_graph, int radius)
{
	
	//first build graph
	connection_graph.GetVertices().clear();
	
	//step 1: build vertices
	for (int i=0; i<m_Seeds.size(); i++)
	{
		Vertex v(i);
		v.pt[0] = m_Seeds.at(i)[0];
		v.pt[1] = m_Seeds.at(i)[1];
		connection_graph.GetVertices().push_back(v);
	}

	double dist;
	for (int i=0; i<m_Seeds.size(); i++)
		for (int j=i+1; j<m_Seeds.size(); j++)
		{
			dist = sqrt(double((m_Seeds[i][0]-m_Seeds[j][0])*(m_Seeds[i][0]-m_Seeds[j][0])
				+(m_Seeds[i][1]-m_Seeds[j][1])*(m_Seeds[i][1]-m_Seeds[j][1])));
			if (dist <= radius)
				connection_graph.addEdge(i, j);
		}
}

void radSeedClustering::Peform_DBSCAN_Clustering()
{
	rad_graph::Graph connection_graph;
	BuildConnectionGraph(connection_graph, ClusteringRadius1);
	int c=0;
	vector<int> neighbor_set;

	for (unsigned int i=0; i<connection_graph.GetVertices().size(); i++)
	{
		if (!connection_graph.GetVertices()[i].visited)
		{
			connection_graph.GetVertices()[i].visited = true;
			std::copy(connection_graph.GetVertices()[i].neighbours.begin(), 
				connection_graph.GetVertices()[i].neighbours.end(), std::back_inserter(neighbor_set));

			if (neighbor_set.empty())
				connection_graph.GetVertices()[i].noise = true;
			else
			{
				c++;
				//expand cluster
				connection_graph.GetVertices()[i].clustering_label = c;
				//for each point P' in neighborPts
				for (int j=0; j<neighbor_set.size(); j++)
				{
					//if P' is not visited
					if(!connection_graph.GetVertices()[neighbor_set[j]].visited)
					{
						//Mark P' as visited
						connection_graph.GetVertices()[neighbor_set[j]].visited = true;
						if(!connection_graph.GetVertices()[neighbor_set[j]].neighbours.empty())
						{
							std::copy(connection_graph.GetVertices()[neighbor_set[j]].neighbours.begin(), 
								connection_graph.GetVertices()[neighbor_set[j]].neighbours.end(), std::back_inserter(neighbor_set));
						}
					}

					// if P' is not yet a member of any cluster
					// add P' to cluster c
					if(connection_graph.GetVertices()[neighbor_set[j]].clustering_label == 0)
						connection_graph.GetVertices()[neighbor_set[j]].clustering_label = c;
				}

			}
		}
	}

	m_SeedLabels.clear();
	m_SeedLabels.resize(connection_graph.GetVertices().size());
	for (unsigned int i=0; i<connection_graph.GetVertices().size(); i++)
		m_SeedLabels[i] = connection_graph.GetVertices().at(i).clustering_label;
}

FloatImageType2D::Pointer radSeedClustering::ComputeLOGImage(FloatImageType2D::Pointer img, double sigma)
{
	typedef itk::LaplacianRecursiveGaussianImageFilter<FloatImageType2D, FloatImageType2D >  filterType;
	filterType::Pointer laplacianFilter = filterType::New();
	laplacianFilter->SetInput( img ); // NOTE: input image type must be double or float
	laplacianFilter->SetSigma(sigma);
	laplacianFilter->SetNormalizeAcrossScale(true);
	laplacianFilter->Update();

	return laplacianFilter->GetOutput();
}

void radSeedClustering::FindNearestPoints(DoublePointType2D & sd, vector<bool> & marked_arr, vector<unsigned int> & ids)
{
	unsigned int dist;
	for (unsigned int i=0; i<m_Seeds.size(); i++)
	{
		dist = (sd[0]-m_Seeds[i][0])*(sd[0]-m_Seeds[i][0])+(sd[1]-m_Seeds[i][1])*(sd[1]-m_Seeds[i][1]);
		if (!marked_arr[i] && dist < MergeRadius*MergeRadius)
		{
			ids.push_back(i);
		}
	}
}

void radSeedClustering::ScaleSelection(FloatImageType2D::Pointer img)
{
	typedef itk::JoinSeriesImageFilter<FloatImageType2D, FloatImageType> JoinSeriesFilterType;
	JoinSeriesFilterType::Pointer joinFilter = JoinSeriesFilterType::New();
	joinFilter->SetOrigin(0.0);
	joinFilter->SetSpacing(1.0);

	//build LOG scale space
	//QuickView viewer;
	for (unsigned int i=0; i< Number_Of_Scale_Levels; i++)
	{
		double sigma = Initial_Scale*pow(Scale_Interval, (double)i);
		FloatImageType2D::Pointer tmp_img = ComputeLOGImage(img, sigma);
		joinFilter->SetInput(i, tmp_img);
	//	viewer.AddImage(tmp_img.GetPointer());
	}
	joinFilter->Update();
	FloatImageType::Pointer scale_space = joinFilter->GetOutput();
	//WriteImage<FloatImageType>("scale_space.hdr", scale_space);

	//viewer.Visualize();
	FloatImageType::SizeType radius;
	radius.Fill(2);
	radius[2] = 0;

	itk::NeighborhoodIterator<FloatImageType> iterator(radius, scale_space, scale_space->GetLargestPossibleRegion());
	FloatImageType::IndexType pixelIndex, maxIndex;
	float max_val, optimized_scale, val;
	bool IsInBounds;
	ShortPointArray2D seeds_backup;
	vector<float> scale_response;
	vector<float> scale_backup;
	vector<float> scale_response_backup;

	for (unsigned int j=0; j<m_Seeds.size(); j++)
	{
		maxIndex[0] = m_Seeds[j][0];
		maxIndex[1] = m_Seeds[j][1];
		maxIndex[2] = 0;
		max_val = fabs(scale_space->GetPixel(maxIndex));
		optimized_scale = Initial_Scale;

		for (unsigned int i=0; i<scale_space->GetLargestPossibleRegion().GetSize()[2]; i++)
		{
			pixelIndex[0] = m_Seeds[j][0];
			pixelIndex[1] = m_Seeds[j][1];
			pixelIndex[2] = i;
			iterator.SetLocation(pixelIndex);

			for (unsigned int k=0; k<iterator.Size(); k++)
			{
				val = fabs(iterator.GetPixel(k, IsInBounds));
				if (!IsInBounds)
					continue;

				//cout << iterator.GetIndex(k) << ": " << val << std::endl;
				if (val > max_val)
				{
					max_val = val;
					maxIndex = iterator.GetIndex(k);
					optimized_scale = Initial_Scale*pow(Scale_Interval, (double)maxIndex[2]);
				}
			}
		}

		ShortPointType2D pt;
		pt[0] = maxIndex[0];
		pt[1] = maxIndex[1];
		seeds_backup.push_back(pt);
		scale_backup.push_back(optimized_scale);
		scale_response_backup.push_back(max_val);
	}

	//threshold detection
	float scale_mean, scale_deviation;
	ComputeMeanAndVariance<float>(scale_backup, scale_mean, scale_deviation);
	float scale_response_mean, scale_response_deviation;
	ComputeMeanAndVariance<float>(scale_response_backup, scale_response_mean, scale_response_deviation);

	float scale_response_threshold = (scale_response_mean-scale_response_deviation)*m_ScaleResponse;
	scale_response_threshold = std::max<float>(0.0, scale_response_threshold);
	float scale_threshold = m_ScaleSize*(scale_mean-scale_deviation);
	scale_threshold = std::max<float>(0.0, scale_threshold);

	m_Seeds.clear();
	m_CenterScales.clear();
	for (int i=0; i<scale_backup.size(); i++)
	{
		if (scale_backup[i]>scale_threshold && scale_response_backup[i] > scale_response_threshold)
		{
			m_Seeds.push_back(seeds_backup[i]);
			m_CenterScales.push_back(scale_backup[i]);
			scale_response.push_back(scale_response_backup[i]);
		}
	}

	//group seeds again
	vector<bool> mark_array(scale_response.size(), false);
	vector<unsigned int> mark_ids;
	seeds_backup.clear();
	scale_backup.clear();
	ShortPointType2D max_pt;

	for (int i=0; i<mark_array.size(); i++)
	{
		if (!mark_array[i])
		{
			mark_ids.clear();
			mark_array[i] = true;

			FindNearestPoints(m_Seeds[i], mark_array, mark_ids);
			max_pt = m_Seeds[i];	
			max_val = scale_response[i];

			for (int j=0; j<mark_ids.size(); j++)
			{
				if (scale_response[mark_ids[j]] > max_val)
				{
					max_val = scale_response[mark_ids[j]];
					max_pt = m_Seeds[mark_ids[j]];
					optimized_scale = m_CenterScales[mark_ids[j]];
				}

				mark_array[mark_ids[j]] = true;
			}

			seeds_backup.push_back(max_pt);
			scale_backup.push_back(optimized_scale);
		}
	}

	m_CenterScales.clear();
	m_CenterScales.assign(scale_backup.begin(), scale_backup.end());
	m_Seeds.clear();
	for (unsigned int i=0; i<seeds_backup.size(); i++)
	{
		DoublePointType2D res_pt;
		res_pt[0] = seeds_backup[i][0];
		res_pt[1] = seeds_backup[i][1];
		//AdjustLocalExtrema(seeds_backup[i], m_CenterScales[i], scale_space, res_pt);
		m_Seeds.push_back(res_pt);
	}
	/*for (int i=0; i<m_Seeds.size(); i++)
		cout << m_Seeds[i] << ", " << scale_response[i] << std::endl;*/
}

template <class T>
T round(T number)
{
    return number < 0.0 ? ceil(number - 0.5) : floor(number + 0.5);
}

void radSeedClustering::AdjustLocalExtrema(ShortPointType2D pt, double optimal_scale, 
										   FloatImageType::Pointer scale_space, 
										   DoublePointType2D &res_pt)
{
	res_pt[0] = pt[0];
	res_pt[1] = pt[1];

	if (pt[0] == 0 || pt[0] == scale_space->GetLargestPossibleRegion().GetSize()[0]-1
		|| pt[1] == 0 || pt[1] == scale_space->GetLargestPossibleRegion().GetSize()[1]-1)
		return;

	int idx = (log(optimal_scale/Initial_Scale)/log(Scale_Interval)+0.5);
	if (idx == 0 || idx == Number_Of_Scale_Levels-1)
		return;

	const float img_scale = 1.f/(255*SIFT_FIXPT_SCALE);
    const float deriv_scale = img_scale*0.5f;
    const float second_deriv_scale = img_scale;
    const float cross_deriv_scale = img_scale*0.25f;
	DoublePointType2D backup_res_pt;
	backup_res_pt[0] = pt[0];
	backup_res_pt[1] = pt[1];

	float xi=0, xr=0, xc=0;
    int i = 0;

	const FloatImageType2D::Pointer img = Extract2DImage<FloatImageType, FloatImageType2D>(scale_space, idx, 2);
    const FloatImageType2D::Pointer prev = Extract2DImage<FloatImageType, FloatImageType2D>(scale_space, idx-1, 2);
    const FloatImageType2D::Pointer next = Extract2DImage<FloatImageType, FloatImageType2D>(scale_space, idx+1, 2);

	typedef itk::Vector<double, 3> VectorType;
	typedef itk::Matrix<double, 3, 3> MatrixType;
	VectorType dD, X;
	FloatImageType2D::IndexType pixelIndex1, pixelIndex2, pixelIndex3, pixelIndex4;
	for( ; i < SIFT_MAX_INTERP_STEPS; i++ )
    {
		if (pt[0] == 0 || pt[0] == scale_space->GetLargestPossibleRegion().GetSize()[0]-1
			|| pt[1] == 0 || pt[1] == scale_space->GetLargestPossibleRegion().GetSize()[1]-1)
		{
			res_pt[0] = backup_res_pt[0];
			res_pt[1] = backup_res_pt[1];
            return;
		}
		pixelIndex1[0] = pt[0]-1;
		pixelIndex1[1] = pt[1];
		pixelIndex2[0] = pt[0]+1;
		pixelIndex2[1] = pt[1];
		dD[0] = (img->GetPixel(pixelIndex2) - img->GetPixel(pixelIndex1))*deriv_scale;

		pixelIndex1[0] = pt[0];
		pixelIndex1[1] = pt[1]-1;
		pixelIndex2[0] = pt[0];
		pixelIndex2[1] = pt[1]+1;
		dD[1] = (img->GetPixel(pixelIndex2) - img->GetPixel(pixelIndex1))*deriv_scale;

		pixelIndex1[0] = pt[0];
		pixelIndex1[1] = pt[1];
		dD[2] = (next->GetPixel(pixelIndex1) - prev->GetPixel(pixelIndex1))*deriv_scale;

		float v2 = (float)img->GetPixel(pixelIndex1)*2;

		pixelIndex1[0] = pt[0]-1;
		pixelIndex1[1] = pt[1];
		pixelIndex2[0] = pt[0]+1;
		pixelIndex2[1] = pt[1];
        float dxx = (img->GetPixel(pixelIndex2) + img->GetPixel(pixelIndex1) - v2)*second_deriv_scale;

		pixelIndex1[0] = pt[0];
		pixelIndex1[1] = pt[1]-1;
		pixelIndex2[0] = pt[0];
		pixelIndex2[1] = pt[1]+1;
        float dyy = (img->GetPixel(pixelIndex2) + img->GetPixel(pixelIndex1) - v2)*second_deriv_scale;

		pixelIndex1[0] = pt[0];
		pixelIndex1[1] = pt[1];
        float dss = (next->GetPixel(pixelIndex1) + prev->GetPixel(pixelIndex1) - v2)*second_deriv_scale;

		pixelIndex1[0] = pt[0]+1;
		pixelIndex1[1] = pt[1]+1;
		pixelIndex2[0] = pt[0]-1;
		pixelIndex2[1] = pt[1]+1;
		pixelIndex3[0] = pt[0]+1;
		pixelIndex3[1] = pt[1]-1;
		pixelIndex4[0] = pt[0]-1;
		pixelIndex4[1] = pt[1]-1;
        float dxy = (img->GetPixel(pixelIndex1) - img->GetPixel(pixelIndex2) -
                     img->GetPixel(pixelIndex3) + img->GetPixel(pixelIndex4))*cross_deriv_scale;

		pixelIndex1[0] = pt[0]-1;
		pixelIndex1[1] = pt[1];
		pixelIndex2[0] = pt[0]+1;
		pixelIndex2[1] = pt[1];
        float dxs = (next->GetPixel(pixelIndex2) - next->GetPixel(pixelIndex1) -
                     prev->GetPixel(pixelIndex2) + prev->GetPixel(pixelIndex1))*cross_deriv_scale;

		pixelIndex1[0] = pt[0];
		pixelIndex1[1] = pt[1]-1;
		pixelIndex2[0] = pt[0];
		pixelIndex2[1] = pt[1]+1;
        float dys = (next->GetPixel(pixelIndex2) - next->GetPixel(pixelIndex1) -
                     prev->GetPixel(pixelIndex2) + prev->GetPixel(pixelIndex1))*cross_deriv_scale;

		MatrixType H;
		H(0,0) = dxx;
		H(0,1) = dxy;
		H(0,2) = dxs;
		H(1,0) = dxy;
		H(1,1) = dyy;
		H(1,2) = dys;
		H(2,0) = dxs;
		H(2,1) = dys;
		H(2,2) = dss;

		vnl_vector<double> vnl_X = vnl_qr< double >( H.GetVnlMatrix() ).solve( dD.GetVnlVector() );
		X.SetVnlVector(vnl_X);

        xi = -X[2];
        xr = -X[1];
        xc = -X[0];

		if( std::abs(xi) < 0.5f && std::abs(xr) < 0.5f && std::abs(xc) < 0.5f )
		{
			res_pt[0] = pt[0] + xc;
			res_pt[1] = pt[1] + xr;
            break;
		}

        if( std::abs(xi) > (float)(std::numeric_limits<int>::max()/3) || std::abs(xr) > (float)(std::numeric_limits<int>::max()/3) ||
            std::abs(xc) > (float)(std::numeric_limits<int>::max()/3) )
            return;

		pt[0] += (int)round<float>(xc);
        pt[1] += (int)round<float>(xr);
		res_pt[0] = pt[0] + xc;
		res_pt[1] = pt[1] + xr;
        
        if( res_pt[0] < 0 || res_pt[0] >= img->GetLargestPossibleRegion().GetSize()[0] - 1  ||
            res_pt[1] < 0 || res_pt[1] >= img->GetLargestPossibleRegion().GetSize()[1] - 1 )
		{
			res_pt[0] = backup_res_pt[0];
			res_pt[1] = backup_res_pt[1];
            return;
		}
	}

	// ensure convergence of interpolation
    if( i >= SIFT_MAX_INTERP_STEPS )
        return;

	//{
 //       Matx31f dD((img.at<sift_wt>(r, c+1) - img.at<sift_wt>(r, c-1))*deriv_scale,
 //                  (img.at<sift_wt>(r+1, c) - img.at<sift_wt>(r-1, c))*deriv_scale,
 //                  (next.at<sift_wt>(r, c) - prev.at<sift_wt>(r, c))*deriv_scale);
 //       float t = dD.dot(Matx31f(xc, xr, xi));

 //       contr = img.at<sift_wt>(r, c)*img_scale + t * 0.5f;
 //       if( std::abs( contr ) * nOctaveLayers < contrastThreshold )
 //           return false;

 //       // principal curvatures are computed using the trace and det of Hessian
 //       float v2 = img.at<sift_wt>(r, c)*2.f;
 //       float dxx = (img.at<sift_wt>(r, c+1) + img.at<sift_wt>(r, c-1) - v2)*second_deriv_scale;
 //       float dyy = (img.at<sift_wt>(r+1, c) + img.at<sift_wt>(r-1, c) - v2)*second_deriv_scale;
 //       float dxy = (img.at<sift_wt>(r+1, c+1) - img.at<sift_wt>(r+1, c-1) -
 //                    img.at<sift_wt>(r-1, c+1) + img.at<sift_wt>(r-1, c-1)) * cross_deriv_scale;
 //       float tr = dxx + dyy;
 //       float det = dxx * dyy - dxy * dxy;

 //       if( det <= 0 || tr*tr*edgeThreshold >= (edgeThreshold + 1)*(edgeThreshold + 1)*det )
 //           return false;
 //   }
}
