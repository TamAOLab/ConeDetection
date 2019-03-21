/*
 *  radImgFunc.h
 *  
 *
 *  Created by Jianfei Liu on 3/22/12.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef RADIMGFUNC_H
#define RADIMGFUNC_H

#include "raddefinition.h"

#include <itkImageSeriesReader.h>
#include <itkImageSeriesWriter.h>
#include <itkVideoFileReader.h>
#include <itkImageFileReader.h>
#include <itkImageFileWriter.h>
#include <itkDirectory.h>
#include <itkMinimumMaximumImageCalculator.h>
#include <itkConnectedThresholdImageFilter.h>
#include <itkConnectedComponentImageFilter.h>
#include <itkExtractImageFilter.h>
#include <itkAddImageFilter.h>
#include <itkImageDuplicator.h>
#include <itkCastImageFilter.h>
#include <itkMultiplyImageFilter.h>
#include <itkThresholdImageFilter.h>
#include <itkTransformFileReader.h>
#include <itkTransformFileWriter.h>
#include <itkRescaleIntensityImageFilter.h>
#include <itkBinaryThresholdImageFilter.h>
#include <itkResampleImageFilter.h>
#include <itkIdentityTransform.h>
#include <vtkImageData.h>
#include <vtkPointData.h>
#include <itkBinaryShapeOpeningImageFilter.h>
#include <itkSliceBySliceImageFilter.h>
#include <itkNearestNeighborInterpolateImageFunction.h>
#include <itkLinearInterpolateImageFunction.h>
#include <itkBSplineInterpolateImageFunction.h>
#include <itkBinaryFillholeImageFilter.h>
#include <itkBinaryDilateImageFilter.h>
#include <itkBinaryErodeImageFilter.h>
#include <itkPointSetToImageFilter.h>
#include <itkImageMaskSpatialObject.h>
#include <vtkDataArray.h>
#include "itkImageToVTKImageFilter.h"
#include "itkVTKImageExport.h"
#include "vtkImageImport.h"
// #include <vtkImageToImageFilter.h>
#include <itkSignedMaurerDistanceMapImageFilter.h>
#include <itkRecursiveGaussianImageFilter.h>
#include <itkSubtractImageFilter.h>
#include <itkBinaryMorphologicalClosingImageFilter.h>
#include <itkBinaryMorphologicalOpeningImageFilter.h>
#include <itkAbsImageFilter.h>
#include "itkBinaryContourImageFilter.h"
#include <locale>
#include <algorithm>
#include <time.h>

//IO functions

template <class TImageType>
typename TImageType::Pointer ReadImage(const char *fileName)
{
	typedef itk::ImageFileReader< TImageType >  ReaderType;
	typename ReaderType::Pointer reader = ReaderType::New();
	reader->SetFileName(fileName);
	reader->SetReleaseDataFlag(true);
	
	try
	{
		reader->UpdateLargestPossibleRegion();
	}
	catch(itk::ExceptionObject &err)
	{
		std::cerr << err << std::endl;
	}
	return reader->GetOutput();
}

template <class TImageType>
void WriteImage(const char *fileName, typename TImageType::Pointer img)
{
	typedef itk::ImageFileWriter< TImageType > WriterType;
	typename WriterType::Pointer writer = WriterType::New();
	writer->SetFileName(fileName);
	writer->SetInput(img);
	writer->Update();
}
//=====================================================================================
//=====================================================================================
//=====================================================================================

template <typename TImageType>
void ComputeIntensityRange(typename TImageType::Pointer img, 
                           typename TImageType::PixelType & low, 
                           typename TImageType::PixelType & high)
{
	typedef itk::MinimumMaximumImageCalculator<TImageType> MinimumMaximumImageCalculatorType;
	typename MinimumMaximumImageCalculatorType::Pointer intensityRangeCalc = MinimumMaximumImageCalculatorType::New();
	intensityRangeCalc->SetImage(img);
	intensityRangeCalc->ComputeMinimum();
	low = intensityRangeCalc->GetMinimum();
	intensityRangeCalc->ComputeMaximum();
	high = intensityRangeCalc->GetMaximum();
}


template <class TInputImage, class TOutputImage>
typename TOutputImage::Pointer AllocateImage(typename TInputImage::Pointer input_img)
{
	typename TOutputImage::Pointer output_img = TOutputImage::New();
	output_img->SetRegions( input_img->GetLargestPossibleRegion() );
	output_img->SetSpacing( input_img->GetSpacing() );
	output_img->CopyInformation( input_img );
	output_img->Allocate();
    return output_img;
}

template <class TInputImage, class TOutputImage>
void AllocateImage(typename TInputImage::Pointer input_img, typename TOutputImage::Pointer output_img)
{
	output_img->SetRegions( input_img->GetLargestPossibleRegion() );
	output_img->SetSpacing( input_img->GetSpacing() );
	output_img->CopyInformation( input_img );
	output_img->Allocate();
    return output_img;
}

template <class TInputImage, class TOutputImage>
typename TOutputImage::Pointer AllocateImage(typename TInputImage::RegionType region)
{
	typename TOutputImage::Pointer output_img = TOutputImage::New();
	output_img->SetRegions( region );
	output_img->Allocate();
    return output_img;
}

template <class TImageType>
typename TImageType::Pointer AllocateImage(typename TImageType::IndexType start, 
                   typename TImageType::SizeType size)
{
	typename TImageType::Pointer output_img = TImageType::New();
	typename TImageType::RegionType region;
    
	region.SetSize(size);
    region.SetIndex(start);
    output_img->SetRegions(region);
    output_img->Allocate();
    return output_img;
}

template<class TImage>
void DeepCopy(typename TImage::Pointer input, typename TImage::Pointer output)
{
	output->SetRegions(input->GetLargestPossibleRegion());
	output->Allocate();
 
	itk::ImageRegionConstIterator<TImage> inputIterator(input, input->GetLargestPossibleRegion());
	itk::ImageRegionIterator<TImage> outputIterator(output, output->GetLargestPossibleRegion());
 
	while(!inputIterator.IsAtEnd())
    {
		outputIterator.Set(inputIterator.Get());
		++inputIterator;
		++outputIterator;
    }
}
 

template <class TImageType>
typename TImageType::Pointer ExtractImage(typename TImageType::Pointer input_img,
                  typename TImageType::IndexType &lower_index, 
                  typename TImageType::IndexType &upper_index)
{
	typename TImageType::IndexType desiredStart;
	typename TImageType::SizeType desiredSize;

	unsigned int i;
	for (i=0; i<input_img->GetImageDimension(); i++)
	{
		desiredStart[i] = lower_index[i];
		desiredSize[i] = upper_index[i]-lower_index[i];
	}
	
	typename TImageType::RegionType desiredRegion(desiredStart, desiredSize);
	typedef itk::ExtractImageFilter<TImageType, TImageType> ExtractImageFilterType;
	typename ExtractImageFilterType::Pointer extractFilter = ExtractImageFilterType::New();
	extractFilter->SetExtractionRegion(desiredRegion);
	extractFilter->SetInput(input_img);
	extractFilter->Update();
	return extractFilter->GetOutput();
}

template <class TImageType>
typename TImageType::Pointer ExtractImage(typename TImageType::Pointer input_img,
                  typename TImageType::RegionType & img_region)
{
	typedef itk::ExtractImageFilter<TImageType, TImageType> ExtractImageFilterType;
	typename ExtractImageFilterType::Pointer extractFilter = ExtractImageFilterType::New();
	extractFilter->SetExtractionRegion(img_region);
	extractFilter->SetInput(input_img);
	extractFilter->Update();
	return extractFilter->GetOutput();
}

template <typename TImageType, typename TImageType2D> //0--x, 1---y, 2--z
typename TImageType2D::Pointer Extract2DImage(typename TImageType::Pointer input_img, 
                    int slice, int direction)
{
	typename TImageType::SizeType inputSize = input_img->GetLargestPossibleRegion().GetSize();
	typename TImageType::IndexType desiredStart;
	typename TImageType::SizeType desiredSize;

	unsigned int i;
	for (i=0; i<input_img->GetImageDimension(); i++)
	{
		desiredStart[i] = input_img->GetLargestPossibleRegion().GetIndex()[i];
		desiredSize[i] = inputSize[i];
	}
	
    desiredStart[direction] = input_img->GetLargestPossibleRegion().GetIndex()[direction]+slice;
	desiredSize[direction] = 0;

	typename TImageType::RegionType desiredRegion(desiredStart, desiredSize);
	typedef itk::ExtractImageFilter<TImageType, TImageType2D> ExtractImageFilterType;
	typename ExtractImageFilterType::Pointer extractFilter = ExtractImageFilterType::New();
	extractFilter->SetExtractionRegion(desiredRegion);
    extractFilter->SetDirectionCollapseToIdentity();
	extractFilter->SetInput(input_img);
	extractFilter->Update();
	return extractFilter->GetOutput();
}

template <typename TImageType>
typename TImageType::Pointer DuplicateImage(typename TImageType::Pointer input_img)
{
	typedef itk::ImageDuplicator< TImageType > DuplicatorType;
	typename DuplicatorType::Pointer duplicator = DuplicatorType::New();
	duplicator->SetInputImage(input_img);
	duplicator->Update();
	return duplicator->GetOutput();
}

template <class TInputImage, class TOutputImage>
typename TOutputImage::Pointer CastImage(typename TInputImage::Pointer input_img)
{
	typedef itk::CastImageFilter< TInputImage, TOutputImage > CastFilterType;
	typename CastFilterType::Pointer castFilter = CastFilterType::New();
	castFilter->SetInput(input_img);
	castFilter->SetReleaseDataFlag(true);
	castFilter->Update();
	return castFilter->GetOutput();
}

template <class TInputImage, class TOutputImage>
typename TOutputImage::Pointer MultipleImageWithConstant(typename TInputImage::Pointer input_img, float val)
{
	typedef itk::MultiplyImageFilter<TInputImage, TOutputImage, TOutputImage> MultiplyImageFilterType;
	typename MultiplyImageFilterType::Pointer multiplyImageFilter = MultiplyImageFilterType::New();
    multiplyImageFilter->SetInput(input_img);
	multiplyImageFilter->SetConstant(val);
	//multiplyImageFilter->SetReleaseDataFlag(true);
	multiplyImageFilter->Update();
	return multiplyImageFilter->GetOutput();
}

template <class TInputImage1, class TInputImage2, class TOutputImage>
typename TOutputImage::Pointer MultipleImages(typename TInputImage1::Pointer input_img1, 
                    typename TInputImage2::Pointer input_img2)
{
	typedef itk::MultiplyImageFilter <TInputImage1, TInputImage2, TOutputImage > MultiplyImageFilterType;
	typename MultiplyImageFilterType::Pointer multiplyFilter = MultiplyImageFilterType::New();
	multiplyFilter->SetInput1(input_img1);
	multiplyFilter->SetInput2(input_img2);
	multiplyFilter->SetReleaseDataFlag(true);
	multiplyFilter->Update();
	return multiplyFilter->GetOutput();
}

template <class TInputImage1, class TInputImage2, class TOutputImage>
typename TOutputImage::Pointer SubtractImages(typename TInputImage1::Pointer input_img1, 
                    typename TInputImage2::Pointer input_img2)
{
	typedef itk::SubtractImageFilter <TInputImage1, TInputImage2, TOutputImage > SubtractImageFilterType;
	typename SubtractImageFilterType::Pointer subtractFilter = SubtractImageFilterType::New();
	subtractFilter->SetInput1(input_img1);
	subtractFilter->SetInput2(input_img2);
	//subtractFilter->SetReleaseDataFlag(true);
	subtractFilter->Update();
	return subtractFilter->GetOutput();
}

template <class TInputImage, class TOutputImage>
typename TOutputImage::Pointer AbsoluteImage(typename TInputImage::Pointer input_img)
{
	typedef itk::AbsImageFilter<TInputImage, TOutputImage> AbsImageFilterType;
	typename AbsImageFilterType::Pointer abs_filter = AbsImageFilterType::New();
    abs_filter->SetInput(input_img);
	abs_filter->Update();
	return abs_filter->GetOutput();
}

template <class TInputImage, class TOutputImage>
typename TOutputImage::Pointer RescaleImageIntensity(typename TInputImage::Pointer input_img,
                           typename TOutputImage::PixelType low,
                           typename TOutputImage::PixelType high)
{
	typedef itk::RescaleIntensityImageFilter< TInputImage, TOutputImage > RescaleFilterType;
	typename RescaleFilterType::Pointer rescaleFilter = RescaleFilterType::New();
	rescaleFilter->SetInput(input_img);
	rescaleFilter->SetOutputMinimum(low);
	rescaleFilter->SetOutputMaximum(high);
	rescaleFilter->Update();
	return rescaleFilter->GetOutput();
}

template <class TInputImage1, class TInputImage2, class TOutputImage, class TTransform>
typename TOutputImage::Pointer ResampleImage(typename TInputImage1::Pointer fixed_img,
				   typename TInputImage2::Pointer moving_img, 
				   typename TTransform::Pointer transform)
{
    typedef itk::ResampleImageFilter< TInputImage2, TOutputImage >    ResampleFilterType;
	typename ResampleFilterType::Pointer resample = ResampleFilterType::New();
	resample->SetTransform( transform );
	resample->SetInput( moving_img );

	resample->SetSize(    fixed_img->GetLargestPossibleRegion().GetSize() );
	resample->SetOutputOrigin(  fixed_img->GetOrigin() );
	resample->SetOutputSpacing( fixed_img->GetSpacing() );
	resample->SetOutputDirection( fixed_img->GetDirection() );
	resample->SetDefaultPixelValue( 0 );
	//resample->SetReleaseDataFlag(true);
	resample->UpdateLargestPossibleRegion();
	return resample->GetOutput();
}

template <class TInputImage, class TOutputImage, unsigned int TDimension>
typename TOutputImage::Pointer ResampleImage(typename TInputImage::Pointer input_img,
                   typename TOutputImage::SpacingType output_spacing, 
				   typename TOutputImage::SizeType original_size)
{
    typename TInputImage::SpacingType input_spacing = input_img->GetSpacing();
    typename TInputImage::SizeType input_size = input_img->GetLargestPossibleRegion().GetSize();
    typename TOutputImage::SizeType output_size;
    
    for (int i=0; i<input_img->GetImageDimension(); i++) 
    {
		if (input_spacing[i] < 0)
			input_spacing[i] = -input_spacing[i];
        output_size[i] = input_spacing[i]
        * (static_cast<double>(input_size[i])/ static_cast<double>(output_spacing[i]));

		if (original_size[i] < output_size[i])
		{
			output_size[i] = original_size[i];
			output_spacing[i] = input_img->GetSpacing()[i]/(static_cast<double>(output_size[i])/static_cast<double>(input_size[i]));
		}
    }
	
    typedef itk::IdentityTransform<double, TDimension> TransformType;
    typedef itk::ResampleImageFilter<TInputImage, TOutputImage> ResampleImageFilterType;
    typename ResampleImageFilterType::Pointer resample = ResampleImageFilterType::New();
    
    typedef itk::LinearInterpolateImageFunction<
    TInputImage, double >  InterpolatorType;
	/*typedef itk::BSplineInterpolateImageFunction<TInputImage, double, double>
               InterpolatorType;*/
    typename InterpolatorType::Pointer interpolator = InterpolatorType::New();
    
    resample->SetInterpolator( interpolator );
    resample->SetInput(input_img);
    resample->SetSize(output_size);
    resample->SetOutputSpacing(output_spacing);
    resample->SetTransform(TransformType::New());
    resample->SetOutputOrigin(input_img->GetOrigin());
	resample->SetOutputDirection ( input_img->GetDirection());
    resample->UpdateLargestPossibleRegion();
	return resample->GetOutput();
}

template <class TInputImage, class TOutputImage>
typename TOutputImage::Pointer ResampleImage(typename TInputImage::Pointer input_img,
				   typename TOutputImage::SizeType output_size)
{
    typename TInputImage::SpacingType input_spacing = input_img->GetSpacing();
    typename TInputImage::SizeType input_size = input_img->GetLargestPossibleRegion().GetSize();
    typename TOutputImage::SpacingType output_spacing;
    
    for (int i=0; i<input_img->GetImageDimension(); i++) 
    {
		if (input_spacing[i] < 0)
			input_spacing[i] = -input_spacing[i];
        output_spacing[i] = input_spacing[i]
        * (static_cast<double>(input_size[i])/ static_cast<double>(output_size[i]));
    }
	
	typedef itk::IdentityTransform<double, TInputImage::ImageDimension> TransformType;
    typedef itk::ResampleImageFilter<TInputImage, TOutputImage> ResampleImageFilterType;
    typename ResampleImageFilterType::Pointer resample = ResampleImageFilterType::New();
    
    typedef itk::LinearInterpolateImageFunction<TInputImage, double >  InterpolatorType;
	/*typedef itk::BSplineInterpolateImageFunction<TInputImage, double, double>
               InterpolatorType;*/
    typename InterpolatorType::Pointer interpolator = InterpolatorType::New();
    
    resample->SetInterpolator( interpolator );
    resample->SetInput(input_img);
    resample->SetSize(output_size);
    resample->SetOutputSpacing(output_spacing);
    resample->SetTransform(TransformType::New());
    resample->SetOutputOrigin(input_img->GetOrigin());
	resample->SetOutputDirection ( input_img->GetDirection());
    resample->UpdateLargestPossibleRegion();
	return resample->GetOutput();
}

template <typename TImageType, typename TImageType2D>
void BinaryFillHoles2D(typename TImageType::Pointer img)
{
	typedef itk::BinaryFillholeImageFilter< TImageType2D > BinaryFillholeImageFilter2D;
	typedef itk::SliceBySliceImageFilter< TImageType, TImageType, BinaryFillholeImageFilter2D > SliceBySliceImageFilterBackgroundType;
	
    typename BinaryFillholeImageFilter2D::Pointer bkgFilter2D = BinaryFillholeImageFilter2D::New();
	//bkgFilter2D->SetBackgroundValue(0);
	bkgFilter2D->SetForegroundValue(255);
	bkgFilter2D->SetFullyConnected(false);
	
    typename SliceBySliceImageFilterBackgroundType::Pointer bkgRemover = SliceBySliceImageFilterBackgroundType::New();
	bkgRemover->SetInput( img );
	bkgRemover->SetFilter( bkgFilter2D );
	bkgRemover->SetReleaseDataFlag(true);
	bkgRemover->Update();
    img->Graft( bkgRemover->GetOutput() );
}

template <typename TImageType, unsigned int VImageDimension>
void DetermineBoundingBox(typename TImageType::Pointer input_img, typename TImageType::RegionType & output_region)	
{
	typedef itk::ImageMaskSpatialObject<VImageDimension> ImageMaskSpatialObject;
	typename ImageMaskSpatialObject::Pointer maskSO = ImageMaskSpatialObject::New();
	maskSO->SetReleaseDataFlag(true);
	maskSO->SetImage ( input_img );
	output_region = maskSO->GetAxisAlignedBoundingBoxRegion();
//	std::cout << "Bounding Box Region: " << output_region << std::endl;
}

template <typename TInputImage, typename TOutputImage>
typename TOutputImage::Pointer DistanceTransform(typename TInputImage::Pointer input_img, bool inside_flag)
{
    typedef itk::SignedMaurerDistanceMapImageFilter<TInputImage, TOutputImage> DistanceTransformType;
    typename DistanceTransformType::Pointer distance_filter = DistanceTransformType::New();
    distance_filter->SetInput(input_img);
	distance_filter->SetInsideIsPositive(inside_flag);
	distance_filter->SetUseImageSpacing( 1 );

	//distance_filter->SetReleaseDataFlag(true);
    distance_filter->Update();
    return distance_filter->GetOutput();
}

template <class TITKImage, class TType>
void ConvertITKImageToVTKImage(typename TITKImage::Pointer input_img,
                           vtkSmartPointer<vtkImageData> output_img)
{
	if (input_img->GetImageDimension() == 3)
	{
		output_img->SetDimensions(input_img->GetLargestPossibleRegion().GetSize()[0], 
			input_img->GetLargestPossibleRegion().GetSize()[1], 
			input_img->GetLargestPossibleRegion().GetSize()[2]);
		output_img->SetSpacing(input_img->GetSpacing()[0], input_img->GetSpacing()[1], input_img->GetSpacing()[2]);
		output_img->SetOrigin(input_img->GetOrigin()[0], input_img->GetOrigin()[1], input_img->GetOrigin()[2]);
	}
	else
	{
		output_img->SetDimensions(input_img->GetLargestPossibleRegion().GetSize()[0], 
			input_img->GetLargestPossibleRegion().GetSize()[1], 1);
		output_img->SetSpacing(input_img->GetSpacing()[0], input_img->GetSpacing()[1], 1.0);
		output_img->SetOrigin(input_img->GetOrigin()[0], input_img->GetOrigin()[1], 0);
	}

	#if VTK_MAJOR_VERSION >= 6
	output_img->AllocateScalars(VTK_UNSIGNED_CHAR, 1);
	#else
	output_img->SetNumberOfScalarComponents(1);
    output_img->AllocateScalars();
	#endif
	
    typedef itk::ImageRegionIteratorWithIndex<TITKImage>   TIteratorType;
    TIteratorType it(input_img, input_img->GetLargestPossibleRegion());
    //int index = 0;
    for (it.GoToBegin(); !it.IsAtEnd(); ++it) 
    {
		TType * pixel;
		if (input_img->GetImageDimension() == 3)
			pixel = static_cast<TType *>(output_img->GetScalarPointer(it.GetIndex()[0],it.GetIndex()[1],it.GetIndex()[2]));
		else
			pixel = static_cast<TType *>(output_img->GetScalarPointer(it.GetIndex()[0],it.GetIndex()[1],0));

		pixel[0] = it.Get();
    }
}

template <class TITKImage1, class TITKImage2, class TType>
void ConvertITKImageToVTKImage1(typename TITKImage1::Pointer input_img1, typename TITKImage2::Pointer input_img2,
                           vtkSmartPointer<vtkImageData> output_img)
{
    output_img->SetDimensions(input_img1->GetLargestPossibleRegion().GetSize()[0], 
		input_img1->GetLargestPossibleRegion().GetSize()[1], 
		input_img1->GetLargestPossibleRegion().GetSize()[2]);
	/*output_img->SetExtent(0, input_img1->GetLargestPossibleRegion().GetSize()[0]-1, 
		0, input_img1->GetLargestPossibleRegion().GetSize()[1]-1, 
		0, input_img1->GetLargestPossibleRegion().GetSize()[2]-1);*/
    output_img->SetSpacing(input_img1->GetSpacing()[0], input_img1->GetSpacing()[1], input_img1->GetSpacing()[2]);
    output_img->SetOrigin(input_img1->GetOrigin()[0], input_img1->GetOrigin()[1], input_img1->GetOrigin()[2]);
	#if VTK_MAJOR_VERSION >= 6
	output_img->AllocateScalars(VTK_UNSIGNED_CHAR, 1);
	#else
	output_img->SetNumberOfScalarComponents(1);
    output_img->AllocateScalars();
	#endif
	
	//initialize output_img
	typedef itk::ImageRegionConstIteratorWithIndex<TITKImage1>   TIteratorType1;
    TIteratorType1 it1(input_img1, input_img1->GetLargestPossibleRegion());
    //int index = 0;
    for (it1.GoToBegin(); !it1.IsAtEnd(); ++it1) 
    {
		TType * pixel = static_cast<TType *>(output_img->GetScalarPointer(it1.GetIndex()[0],it1.GetIndex()[1],it1.GetIndex()[2]));
		pixel[0] = 0;
    }

	//assign dose image and first determine the starting index
    typename TITKImage2::IndexType dose_start, used_dose_start;
    typename TITKImage2::SizeType dose_size = input_img2->GetLargestPossibleRegion().GetSize();
	typename TITKImage2::SizeType used_dose_size;
	typename TITKImage1::SizeType original_size = input_img1->GetLargestPossibleRegion().GetSize();
    
    for (int i=0; i<input_img1->GetImageDimension(); i++) 
    {
		used_dose_start[i] = 0;
        dose_start[i] = ((input_img2->GetOrigin()[i]-input_img1->GetOrigin()[i])/input_img1->GetSpacing()[i]+0.5);

		if (dose_size[i] + dose_start[i] > original_size[i])
		{
			used_dose_size[i] = original_size[i]-dose_start[i];
		}
		else
			used_dose_size[i] = dose_size[i];

		if (dose_start[i] < 0)
			dose_start[i] = 0;
    }

	typename TITKImage2::RegionType used_dose_region(used_dose_start, used_dose_size);
    typedef itk::ImageRegionConstIteratorWithIndex<TITKImage2>   TIteratorType2;
    TIteratorType2 it2(input_img2, used_dose_region);
    //int index = 0;
    for (it2.GoToBegin(); !it2.IsAtEnd(); ++it2) 
    {
		TType * pixel = static_cast<TType *>(output_img->GetScalarPointer(it2.GetIndex()[0]+dose_start[0], it2.GetIndex()[1]+dose_start[1],
			it2.GetIndex()[2]+dose_start[2]));
		pixel[0] = it2.Get();
    }
}

template <class TITKImage>
void ConvertITKImageToVTKImage(const TITKImage * input_img, vtkSmartPointer<vtkImageData> output_img)
{
	typedef itk::ImageToVTKImageFilter<TITKImage>       ConnectorType;
	typename ConnectorType::Pointer connector = ConnectorType::New();
	connector->SetInput(input_img);
	connector->SetReleaseDataFlag(true);
	connector->Update();
	output_img->ShallowCopy( connector->GetOutput() );
}

template <class TITKImage>
vtkSmartPointer<vtkImageData> ConvertITKImageToVTKImage(typename TITKImage::Pointer input_img)
{
	typedef itk::ImageToVTKImageFilter<TITKImage>       ConnectorType;
	typename ConnectorType::Pointer connector = ConnectorType::New();

	connector->GetExporter()->SetInput(input_img);
	connector->GetImporter()->Update();

	return connector->GetImporter()->GetOutput();
}

template <class TITKImage, class TType>
typename TITKImage::Pointer convertVtkToItk(vtkImageData* vtkInput)
{
	typename TITKImage::Pointer itkoutput = TITKImage::New();
	typename TITKImage::RegionType region;
	typename TITKImage::SizeType size;
	typename TITKImage::IndexType start;
	typename TITKImage::SpacingType spacing;
	typename TITKImage::PointType origin;

	for (int i=0; i<3; i++)
	{
		size[i] = vtkInput->GetDimensions()[i];
		start[i] = 0;
		spacing[i] = vtkInput->GetSpacing()[i];
		origin[i] = vtkInput->GetOrigin()[i];
	}
    
	region.SetSize(size);
    region.SetIndex(start);
    itkoutput->SetRegions(region);
	itkoutput->SetSpacing( spacing );
	itkoutput->SetOrigin(origin);
    itkoutput->Allocate();
    
    typedef itk::ImageRegionIteratorWithIndex<TITKImage>   TIteratorType;
    TIteratorType it(itkoutput, itkoutput->GetLargestPossibleRegion());
    //int index = 0;
    for (it.GoToBegin(); !it.IsAtEnd(); ++it) 
    {
		TType * pixel = static_cast<TType *>(vtkInput->GetScalarPointer(it.GetIndex()[0],it.GetIndex()[1],it.GetIndex()[2]));
		it.Set(pixel[0]);
    }

	return itkoutput;
}

//here, used for string operation
// templated version of my_equal so it could work with both char and wchar_t
template<typename charT>
struct my_equal {
    my_equal( const std::locale& loc ) : loc_(loc) {}
    bool operator()(charT ch1, charT ch2) {
        return std::toupper(ch1, loc_) == std::toupper(ch2, loc_);
    }
private:
    const std::locale& loc_;
};

// find substring (case insensitive)
template<typename T>
int ci_find_substr( const T& str1, const T& str2, const std::locale& loc = std::locale() )
{
    typename T::const_iterator it = std::search( str1.begin(), str1.end(),
        str2.begin(), str2.end(), my_equal<typename T::value_type>(loc) );
    if ( it != str1.end() ) return it - str1.begin();
    else return -1; // not found
}

template <typename ITK_Exporter, typename VTK_Importer>
static void ConnectPipelines(ITK_Exporter Exporter, vtkSmartPointer<VTK_Importer> Importer)
{
	Importer->SetUpdateInformationCallback(Exporter->GetUpdateInformationCallback());
	Importer->SetPipelineModifiedCallback(Exporter->GetPipelineModifiedCallback());
	Importer->SetWholeExtentCallback(Exporter->GetWholeExtentCallback());
	Importer->SetSpacingCallback(Exporter->GetSpacingCallback());
	Importer->SetOriginCallback(Exporter->GetOriginCallback());
	Importer->SetScalarTypeCallback(Exporter->GetScalarTypeCallback());
	Importer->SetNumberOfComponentsCallback(Exporter->GetNumberOfComponentsCallback());
	Importer->SetPropagateUpdateExtentCallback(Exporter->GetPropagateUpdateExtentCallback());
	Importer->SetUpdateDataCallback(Exporter->GetUpdateDataCallback());
	Importer->SetDataExtentCallback(Exporter->GetDataExtentCallback());
	Importer->SetBufferPointerCallback(Exporter->GetBufferPointerCallback());
	Importer->SetCallbackUserData(Exporter->GetCallbackUserData());
}

template <typename VTK_Exporter, typename ITK_Importer>
static void ConnectPipelines(vtkSmartPointer<VTK_Exporter> Exporter, ITK_Importer Importer)
{
	Importer->SetUpdateInformationCallback(Exporter->GetUpdateInformationCallback());
	Importer->SetPipelineModifiedCallback(Exporter->GetPipelineModifiedCallback());
	Importer->SetWholeExtentCallback(Exporter->GetWholeExtentCallback());
	Importer->SetSpacingCallback(Exporter->GetSpacingCallback());
	Importer->SetOriginCallback(Exporter->GetOriginCallback());
	Importer->SetScalarTypeCallback(Exporter->GetScalarTypeCallback());
	Importer->SetNumberOfComponentsCallback(Exporter->GetNumberOfComponentsCallback());
	Importer->SetPropagateUpdateExtentCallback(Exporter->GetPropagateUpdateExtentCallback());
	Importer->SetUpdateDataCallback(Exporter->GetUpdateDataCallback());
	Importer->SetDataExtentCallback(Exporter->GetDataExtentCallback());
	Importer->SetBufferPointerCallback(Exporter->GetBufferPointerCallback());
	Importer->SetCallbackUserData(Exporter->GetCallbackUserData());
} 

template <typename TImageType>
typename TImageType::Pointer BinaryDilate(typename TImageType::Pointer input_img, unsigned int radius)
{
	typedef itk::BinaryBallStructuringElement<typename TImageType::PixelType, TImageType::ImageDimension>  StructuringElementType;
	StructuringElementType se;
	
	se.SetRadius( radius );
	se.CreateStructuringElement();
	
	typedef itk::BinaryDilateImageFilter<TImageType, TImageType, StructuringElementType> BinaryDilateImageFilterType;
	typename BinaryDilateImageFilterType::Pointer dilater = BinaryDilateImageFilterType::New();
	dilater->SetInput( input_img );
	dilater->SetKernel( se );
	dilater->SetForegroundValue(255);
	dilater->SetBackgroundValue(0);
	dilater->Update();
	
    return dilater->GetOutput();
}


template <typename TImageType>
typename TImageType::Pointer BinaryErode(typename TImageType::Pointer input_img, unsigned int radius)
{
    typedef itk::BinaryBallStructuringElement<typename TImageType::PixelType, TImageType::ImageDimension>  StructuringElementType;
	StructuringElementType se;
	
	se.SetRadius( radius );
	se.CreateStructuringElement();
	
	typedef itk::BinaryErodeImageFilter<TImageType, TImageType, StructuringElementType> BinaryErodeImageFilterType;
	typename BinaryErodeImageFilterType::Pointer erodeFilter = BinaryErodeImageFilterType::New();
	erodeFilter->SetInput( input_img );
	erodeFilter->SetKernel( se );
	erodeFilter->SetForegroundValue(255);
	erodeFilter->SetBackgroundValue(0);
	erodeFilter->Update();
	
    return erodeFilter->GetOutput();
}

template <typename TImageType>
typename TImageType::Pointer BinaryClosing(typename TImageType::Pointer input_img, unsigned int radius)
{
    typedef itk::BinaryBallStructuringElement<typename TImageType::PixelType, TImageType::ImageDimension>  StructuringElementType;
	StructuringElementType se;
	
	se.SetRadius( radius );
	se.CreateStructuringElement();
	
	typedef itk::BinaryMorphologicalClosingImageFilter<TImageType, TImageType, StructuringElementType> BinaryClosingImageFilterType;
	typename BinaryClosingImageFilterType::Pointer closing = BinaryClosingImageFilterType::New();
	closing->SetInput( input_img );
	closing->SetKernel( se );
	closing->SetForegroundValue(255);
	closing->Update();
	
    return closing->GetOutput();
}

template <typename TImageType>
typename TImageType::Pointer BinaryOpening(typename TImageType::Pointer input_img, unsigned int radius)
{
    typedef itk::BinaryBallStructuringElement<typename TImageType::PixelType, TImageType::ImageDimension>  StructuringElementType;
	StructuringElementType se;
	
	se.SetRadius( radius );
	se.CreateStructuringElement();
	
	typedef itk::BinaryMorphologicalOpeningImageFilter<TImageType, TImageType, StructuringElementType> BinaryOpeningImageFilterType;
	typename BinaryOpeningImageFilterType::Pointer opening = BinaryOpeningImageFilterType::New();
	opening->SetInput( input_img );
	opening->SetKernel( se );
	opening->SetForegroundValue(255);
	opening->Update();
	
    return opening->GetOutput();
}

template <typename TImageType>
typename TImageType::Pointer BinaryContouring(typename TImageType::Pointer input_img)
{
	typedef itk::BinaryContourImageFilter<TImageType, TImageType> binaryContourImageFilterType;
	typename binaryContourImageFilterType::Pointer binaryContourFilter = binaryContourImageFilterType::New();
	binaryContourFilter->SetInput( input_img );
	binaryContourFilter->Update();
	
    return binaryContourFilter->GetOutput();
}

template <class TInputImage, class TOutputImage>
typename TOutputImage::Pointer BinarizeImage(typename TInputImage::Pointer img, double lowerThreshold, double upperThreshold)
{
	typedef itk::BinaryThresholdImageFilter <TInputImage, TOutputImage> BinaryThresholdImageFilterType;
	typename BinaryThresholdImageFilterType::Pointer thresholdFilter = BinaryThresholdImageFilterType::New();
  
	thresholdFilter->SetInput(img);
	thresholdFilter->SetLowerThreshold(lowerThreshold);
	thresholdFilter->SetUpperThreshold(upperThreshold);
	thresholdFilter->SetInsideValue(255);
	thresholdFilter->SetOutsideValue(0);
	//thresholdFilter->SetReleaseDataFlag(true);
	thresholdFilter->Update();
	return thresholdFilter->GetOutput();
}

template <class TInputImage, class TOutputImage>
typename TOutputImage::Pointer SubtractImages(typename TInputImage::Pointer input_img1,
                                              typename TInputImage::Pointer input_img2)
{
    typedef itk::SubtractImageFilter <TInputImage, TInputImage, TOutputImage > SubtractImageFilterType;
    typename SubtractImageFilterType::Pointer subtractFilter = SubtractImageFilterType::New();
    subtractFilter->SetInput1(input_img1);
    subtractFilter->SetInput2(input_img2);
    subtractFilter->Update();
    return subtractFilter->GetOutput();
}

template <class TInputImage, class TOutputImage>
typename TOutputImage::Pointer AddImageWithConstant(typename TInputImage::Pointer input_img1,
                                                    typename TInputImage::PixelType val)
{
    typedef itk::AddImageFilter <TInputImage, TInputImage, TOutputImage > AddImageFilterType;
    typename AddImageFilterType::Pointer addFilter = AddImageFilterType::New();
    addFilter->SetInput1(input_img1);
    addFilter->SetConstant2(val);
    addFilter->Update();
    return addFilter->GetOutput();
}

template <class TInputImage, class TOutputImage>
typename TOutputImage::Pointer SubtractImageWithConstant(typename TInputImage::Pointer input_img1,
                                                         typename TInputImage::PixelType val)
{
    typedef itk::SubtractImageFilter <TInputImage, TInputImage, TOutputImage > SubtractImageFilterType;
    typename SubtractImageFilterType::Pointer subtractFilter = SubtractImageFilterType::New();
    subtractFilter->SetInput1(input_img1);
    subtractFilter->SetConstant2(val);
    subtractFilter->Update();
    return subtractFilter->GetOutput();
}


#endif // RADIMGFUNC_H
