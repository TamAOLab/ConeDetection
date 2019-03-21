/***********************************************************************
 **  radBinaryImageProcessing.h
 ***********************************************************************/

#if !defined(BIP_H)
#define BIP_H 1

#include "itkImageBase.h"
#include "itkImageIOBase.h"
#include "itkFlatStructuringElement.h"
#include "itkBinaryErodeImageFilter.h"
#include "radimgfunc.h"
#include "radScaleSpace.h"

#include "itkPoint.h"

#include <queue> 

#define MAXVAL 10000
// Local functions
template<class TImageType>
bool IsEdge(typename TImageType::Pointer img, itk::NeighborhoodIterator<TImageType> it) 
{
	//if (img.isNull()) return false;
    typename TImageType::PixelType v = it.GetCenterPixel();
	bool IsInBounds;
	for (unsigned int i=0; i<it.Size(); i++)
	{
		it.GetPixel(i, IsInBounds);
		if (IsInBounds && it.GetPixel(i) != v)
		{
			return true;
		}
	}

	return false;
}

namespace bip
{
	/** Number of regions */
	template <class TImageType>
	int RegionNumber(typename TImageType::Pointer img)
	{
		//if (img.isNull()) return 0;
		typename TImageType::PixelType min, max;
        ComputeIntensityRange<TImageType>(img, min, max);
		if (max < 0) return 0;
		return int(max+1);
	}

	/** Is binary image? */
    //bool IsBinaryImage(const Image &I);

	/** Reverse a BYTE image */
	template <class TImageType>
    void Reverse(typename TImageType::Pointer img)
	{
		typedef itk::ImageRegionIteratorWithIndex<TImageType> BinaryIteratorType;
		BinaryIteratorType it(img, img->GetLargestPossibleRegion());
		for (it.GoToBegin(); !it.IsAtEnd(); ++it)
			it.Set(255-it.Get());
	}

	template <class TInputImage, class TOutputImage>
	typename TOutputImage::Pointer ThresholdImage(typename TInputImage::Pointer img, double lowerThreshold, double upperThreshold)
	{
		typedef itk::BinaryThresholdImageFilter <TInputImage, TOutputImage> BinaryThresholdImageFilterType;
		typename BinaryThresholdImageFilterType::Pointer thresholdFilter = BinaryThresholdImageFilterType::New();
  
		thresholdFilter->SetInput(img);
		thresholdFilter->SetLowerThreshold(lowerThreshold);
		thresholdFilter->SetUpperThreshold(upperThreshold);
		thresholdFilter->SetInsideValue(Foreground);
		thresholdFilter->SetOutsideValue(Background);
		//thresholdFilter->SetReleaseDataFlag(true);
		thresholdFilter->Update();
		return thresholdFilter->GetOutput();
	}

	template <class TInputImage, class TOutputImage>
	typename TOutputImage::Pointer ComputeEdge(typename TInputImage::Pointer img)
	{
		//if (img.isNull()) return ByteImage();
		typename TInputImage::SizeType ndim, ndim_1, radius;
		typename TInputImage::IndexType nstart, nstart_1;
		ndim = img->GetLargestPossibleRegion().GetSize();
		for (unsigned int i=0; i<img->GetImageDimension(); i++)
		{
			nstart[i] = 0;
			nstart_1[i] = 1;
			ndim_1[i] = ndim[i]-2;
			radius[i] = 1;
		}

		typename TInputImage::RegionType region;
		region.SetSize(ndim_1);
		region.SetIndex(nstart_1);
		itk::NeighborhoodIterator<TInputImage> iterator(radius, img, region);
		typename TOutputImage::Pointer b = AllocateImage<TInputImage, TOutputImage>(img);
		b->FillBuffer(Background);
		typename TInputImage::PixelType br;
		unsigned int n;

		while(!iterator.IsAtEnd())
		{
			br = iterator.GetCenterPixel();
			n = 0;
			for (unsigned int i=0; i<iterator.Size(); i++)
			{
				if (iterator.GetPixel(i) == br) n++;
			}

			if (n<iterator.Size())
				b->SetPixel(iterator.GetIndex(iterator.Size()/2), Foreground);
			++iterator;
		}
		return b;
	}

	/** Connected regions(2D and 3D) */
	template <class TInputImage, class TOutputImage, class TPointType>
    typename TOutputImage::Pointer ComputeBinaryRegions(typename TInputImage::Pointer img, int &nlabel,
		       int intensity=Foreground, int rad=1, NeighborhoodType nb = Four)
	{
		//if (img.isNull()) return ShortImage();
		typename TInputImage::SizeType ndim, ndim_1, radius;
		typename TInputImage::IndexType nstart, nstart_1;
		ndim = img->GetLargestPossibleRegion().GetSize();
		for (unsigned int i=0; i<img->GetImageDimension(); i++)
		{
			nstart[i] = 0;
			nstart_1[i] = 1;
			ndim_1[i] = ndim[i]-2;
			radius[i] = rad;
		}

		typename TOutputImage::Pointer label_img = AllocateImage<TInputImage, TOutputImage>(img);
		typedef itk::ImageRegionIteratorWithIndex<TOutputImage>       OutputIteratorType;
		typedef itk::ImageRegionIteratorWithIndex<TInputImage>       InputIteratorType;
		OutputIteratorType label_it(label_img, label_img->GetLargestPossibleRegion());
		InputIteratorType img_it(img, img->GetLargestPossibleRegion());
		if (intensity < 0) 
			label_img->FillBuffer(-2);
		else 
			for(label_it.GoToBegin(), img_it.GoToBegin(); !label_it.IsAtEnd(); ++label_it, ++img_it)
				label_it.Set((img_it.Get() != intensity) ? -1 : -2);
  
		typename TOutputImage::IndexType pixelIndex;

		itk::NeighborhoodIterator<TOutputImage> label_neighbor_it(radius, label_img, label_img->GetLargestPossibleRegion());
		queue < itk::Point<TPointType, TInputImage::ImageDimension> > Q;
		int cur_int;
		itk::Point<TPointType, TInputImage::ImageDimension>  q;
  
		nlabel = 0;
		for(label_it.GoToBegin(), img_it.GoToBegin(); !img_it.IsAtEnd(); ++img_it, ++label_it)
		{
			if (label_it.Get() == -2) 
			{
				Q = queue< itk::Point<TPointType, TInputImage::ImageDimension> >();
				for (unsigned int i=0; i<label_img->GetImageDimension(); i++)
					q[i] = label_it.GetIndex()[i];

				Q.push(q);
				cur_int = img_it.Get();

				while (!Q.empty()) 
				{
					q = Q.front();
					for (unsigned int i=0; i<label_img->GetImageDimension(); i++)
						pixelIndex[i] = q[i];
					label_img->SetPixel(pixelIndex, nlabel);


					label_neighbor_it.SetLocation(pixelIndex);
					if( nb == Eight )
					{
						for (unsigned int i=0; i<label_neighbor_it.Size(); i++)
						{
							bool IsInBounds;
							label_neighbor_it.GetPixel(i, IsInBounds);

							if (IsInBounds && label_img->GetPixel(label_neighbor_it.GetIndex(i)) == -2
								&& img->GetPixel(label_neighbor_it.GetIndex(i)) == cur_int)
							{
								for (unsigned int j=0; j<label_img->GetImageDimension(); j++)
									q[j] = label_neighbor_it.GetIndex(i)[j];
								
								label_img->SetPixel(label_neighbor_it.GetIndex(i), nlabel);
							}	
						}
					}
					else if( nb == Four )
					{
						unsigned neighborhood_size = label_neighbor_it.Size();
						for (unsigned int i=0; i<neighborhood_size; i++)
						{
							bool IsInBounds;
							label_neighbor_it.GetPixel(i, IsInBounds);

							if (!IsInBounds) continue;
							unsigned int index_difference = 0;
							for (unsigned int j=0; j<label_img->GetImageDimension(); j++)
								index_difference += std::abs((int)label_neighbor_it.GetIndex(i)[j]-(int)label_neighbor_it.GetIndex(neighborhood_size/2)[j]);

							if (index_difference == 1)
							{
								for (unsigned int j=0; j<label_img->GetImageDimension(); j++)
									q[j] = label_neighbor_it.GetIndex(i)[j];
								
								label_img->SetPixel(label_neighbor_it.GetIndex(i), nlabel);
							}
						}		
					}

					Q.pop();
				}
				nlabel++;
			}
		}
		return label_img;
	}

	template <class TInputImage, class TOutputImage, class TPointType>
    typename TOutputImage::Pointer ComputeShortRegions(typename TInputImage::Pointer img, int &nlabel)
	{
		//if (img.isNull()) return ShortImage();
		typename TInputImage::SizeType ndim, ndim_1, radius;
		typename TInputImage::IndexType nstart, nstart_1;
		ndim = img->GetLargestPossibleRegion().GetSize();
		for (unsigned int i=0; i<img->GetImageDimension(); i++)
		{
			nstart[i] = 0;
			nstart_1[i] = 1;
			ndim_1[i] = ndim[i]-2;
			radius[i] = 1;
		}

		typename TOutputImage::Pointer label_img = AllocateImage<TInputImage, TOutputImage>(img);
		label_img->FillBuffer(-2);

		int cur_int;
		queue < itk::Point<TPointType, TInputImage::ImageDimension> > Q;
		itk::Point<TPointType, TInputImage::ImageDimension> q;

		nlabel = 0;
		typedef itk::ImageRegionIteratorWithIndex<TOutputImage>       OutputIteratorType;
		typedef itk::ImageRegionIteratorWithIndex<TInputImage>       InputIteratorType;
		OutputIteratorType label_it(label_img, label_img->GetLargestPossibleRegion());
		InputIteratorType img_it(img, img->GetLargestPossibleRegion());
		typename TOutputImage::IndexType pixelIndex;

		itk::NeighborhoodIterator<TOutputImage> label_neighbor_it(radius, label_img, 
			label_img->GetLargestPossibleRegion());

		for(label_it.GoToBegin(), img_it.GoToBegin(); !img_it.IsAtEnd(); ++img_it, ++label_it)
		{
			if (label_it.Get() == -2) 
			{
				Q = queue< itk::Point<TPointType, TInputImage::ImageDimension> >();
				for (unsigned int i=0; i<label_img->GetImageDimension(); i++)
					q[i] = label_it.GetIndex()[i];

				Q.push(q);
				cur_int = img_it.Get();

				while (!Q.empty()) 
				{
					q = Q.front();
					for (unsigned int i=0; i<label_img->GetImageDimension(); i++)
						pixelIndex[i] = q[i];
					label_img->SetPixel(pixelIndex, nlabel);

					label_neighbor_it.SetLocation(pixelIndex);

					for (unsigned int i=0; i<label_neighbor_it.Size(); i++)
					{
						bool IsInBounds;
						label_neighbor_it.GetPixel(i, IsInBounds);

						if (IsInBounds && label_img->GetPixel(label_neighbor_it.GetIndex(i)) == -2
							&& img->GetPixel(label_neighbor_it.GetIndex(i)) == cur_int)
						{
							for (unsigned int j=0; j<label_img->GetImageDimension(); j++)
								q[j] = label_neighbor_it.GetIndex(i)[j];
								
							label_img->SetPixel(label_neighbor_it.GetIndex(i), nlabel);
						}	
					}

					Q.pop();
				}
				nlabel++;
			}
		}

		return label_img;
	}

	/** Region statistics and features */
	template <class TInputImage1, class TInputImage2>
    vector<RegionFeature> ComputeRegionFeatures(typename TInputImage1::Pointer label, int nlabel,
		typename TInputImage2::Pointer br)
	{  
		if (!label) 
			return  vector<RegionFeature>();

		vector<RegionFeature> rf(nlabel, RegionFeature());
		int k;
		for(k=0; k<nlabel; k++) 
		{
			rf[k].xc = 0; rf[k].yc = 0; rf[k].zc = 0;  rf[k].radius = 0;
			rf[k].npix = 0; rf[k].mean = 0; rf[k].var = 0;
		}

		typedef itk::ImageRegionIteratorWithIndex<TInputImage1>   Input1IteratorType;
		Input1IteratorType label_it(label, label->GetLargestPossibleRegion());

		for (label_it.GoToBegin(); !label_it.IsAtEnd(); ++label_it)
		{
			if ( (k=label_it.Get()) >= 0 && k < nlabel) 
			{
				if (rf[k].npix == 0) 
				{
					rf[k].xmin = rf[k].xmax = label_it.GetIndex()[0];
					rf[k].ymin = rf[k].ymax = label_it.GetIndex()[1];

					if (label->GetImageDimension() == 3)
						rf[k].zmin = rf[k].zmax = label_it.GetIndex()[2];	    
					else
						rf[k].zmin = rf[k].zmax = 0;
				} 
				else 
				{
					rf[k].xmin = std::min<int>(rf[k].xmin, (int)label_it.GetIndex()[0]);
					rf[k].xmax = std::max<int>(rf[k].xmax, (int)label_it.GetIndex()[0]);
					rf[k].ymin = std::min<int>(rf[k].ymin, (int)label_it.GetIndex()[1]);
					rf[k].ymax = std::max<int>(rf[k].ymax, (int)label_it.GetIndex()[1]);

					if (label->GetImageDimension() == 3)
					{
						rf[k].zmin = std::min<int>(rf[k].zmin, (int)label_it.GetIndex()[2]);
						rf[k].zmax = std::max<int>(rf[k].zmax, (int)label_it.GetIndex()[2]);
					}
				}
				rf[k].xc += (int)label_it.GetIndex()[0]; 
				rf[k].yc += (int)label_it.GetIndex()[1]; 
				if (label->GetImageDimension() == 3)
					rf[k].zc += (int)label_it.GetIndex()[2];
				rf[k].npix++; 
			}
		}
	
		float radius;
		for(k=0; k<nlabel; k++) 
		{
			if (rf[k].npix > 0) 
			{
				rf[k].xc /= rf[k].npix; 
				rf[k].yc /= rf[k].npix; 
				rf[k].zc /= rf[k].npix;

				if (label->GetImageDimension() == 2) // 2D
					radius = sqrt((float)rf[k].npix/3.14159);
				else // 3D
					radius = std::pow( rf[k].npix*3.0/(4.0*3.14159), 1./3.);

				rf[k].radius = std::max<int>((int)(radius), 1);
			}
		}

		// If intensity(brightness) image is given, 
		// then compute the following statistical features 
		if (!br) 
			return rf; // If no intensity image is given
  
		for (unsigned int i=0; i<br->GetImageDimension(); i++)
		{
			if (label->GetLargestPossibleRegion().GetSize()[i] != br->GetLargestPossibleRegion().GetSize()[i]) 
			{
				cerr << "In bip::regionFeatures" << endl;
				cerr << "   Warning: label and brightness must have the same size" << endl;
				cerr << "   Brightness image ignored" << endl;
				return rf;
			}
		}

		for (label_it.GoToBegin(); !label_it.IsAtEnd(); ++label_it)
		{
			if (label_it.Get() >= 0)  
				rf[label_it.Get()].mean += br->GetPixel(label_it.GetIndex());   
		}
  
		for(k = 0; k < nlabel; k++)
		{
			if (rf[k].npix > 0)
				rf[k].mean /= (float) rf[k].npix;
		}
  
		float d;
		for (label_it.GoToBegin(); !label_it.IsAtEnd(); ++label_it)
		{
			if ((k = label_it.Get()) >= 0 && k < nlabel) 
			{
				d = br->GetPixel(label_it.GetIndex()) -  rf[k].mean;
				rf[k].var += d*d;
			}
		}
  
		for(k = 0; k < nlabel; k++)
		{
			if (rf[k].npix > 1)
				rf[k].var =  rf[k].var/ (float) (rf[k].npix-1);
			else 
				rf[k].var = 0;
		}

		return rf;
	}

	template <class TInputImage1, class TInputImage2>
    vector<RegionFeature> ComputeRegionFeatures(typename TInputImage1::Pointer img, int nlabel)
	{
		typename TInputImage2::Pointer tmp_img;
		return ComputeRegionFeatures<TInputImage1, TInputImage2>(img, nlabel, tmp_img);
	}
    
} // End of namespace bip

#endif // radBinaryImageProcessing_H
