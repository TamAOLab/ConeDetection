#include "radScaleSpace.h"
#include <cmath>

using namespace scalespace;

#define NPDE 50
#define NREMOVE 10


#define SCALESPACE_PRECISION 4
#define A_SCALE 57.2958

template <class TInput, class TOutput>
TOutput scalespace::Math_Angle(TInput dx, TInput dy)
{
	if (dx == 0 && dy == 0) return 0;  
	return  A_SCALE * atan2(dx,dy);
}

template <class TInput>	
unsigned char scalespace::Math_Direction(TInput dd)
{
	if (dd < 0) 
	{
		if (dd >= -22.5) return 0;
		else if (dd >= -67.5) return 7;     /* 7 */
		else if (dd >= -112.5) return 6;    /* 6 */
		else if (dd >= -157.5) return 5;    /* 5 */
		else if (dd >= -180) return 4;      /* 4 */
	} 
	else 
	{
		if (dd <= 22.5) return 0;
		else if (dd <= 67.5) return 1;
		else if (dd <= 112.5) return 2;
		else if (dd <= 157.5) return 3;
		else return 4;
	}
}

template <class TInputImage, class TOutputImage>
typename TOutputImage::Pointer scalespace::SmoothImage(typename TInputImage::Pointer img, float scale, const float ratio, 
	bool scale_flag)
{
	typedef itk::RecursiveGaussianImageFilter< TInputImage, TOutputImage >  GaussianFilterType;
	typedef itk::RecursiveGaussianImageFilter< TOutputImage, TOutputImage >  GaussianFilterType1;
    typename GaussianFilterType::Pointer smoothfilterX = GaussianFilterType::New();
    typename GaussianFilterType1::Pointer smoothfilterY = GaussianFilterType1::New();
		
    smoothfilterX->SetDirection(0);
    smoothfilterY->SetDirection(1);
		
    smoothfilterX->SetOrder( GaussianFilterType::ZeroOrder );
    smoothfilterY->SetOrder( GaussianFilterType1::ZeroOrder );
    
    smoothfilterX->SetNormalizeAcrossScale( scale_flag );
    smoothfilterY->SetNormalizeAcrossScale( scale_flag );
	
    smoothfilterX->SetSigma(scale);
    smoothfilterY->SetSigma(scale);
    
    smoothfilterX->SetInput( img );
    smoothfilterY->SetInput( smoothfilterX->GetOutput() );

	smoothfilterX->SetReleaseDataFlag(true);
	smoothfilterY->SetReleaseDataFlag(true);
    
    if (img->GetImageDimension() == 3)
    {
		typename GaussianFilterType1::Pointer smoothfilterZ = GaussianFilterType1::New();
		smoothfilterZ->SetDirection(2);
		smoothfilterZ->SetOrder( GaussianFilterType1::ZeroOrder );
		smoothfilterZ->SetNormalizeAcrossScale( scale_flag );
		smoothfilterZ->SetSigma(scale);
		smoothfilterZ->SetInput( smoothfilterY->GetOutput() );
		smoothfilterZ->SetReleaseDataFlag(true);
		smoothfilterZ->Update();
		
		return smoothfilterZ->GetOutput();
    }
    else
    {
		smoothfilterY->Update();
		return smoothfilterY->GetOutput();
	}
}

template <class TInputImage, class TOutputImage>
typename TOutputImage::Pointer scalespace::SmoothImage(typename TInputImage::Pointer img, float scale_x, 
		float scale_y, float scale_z, bool scale_flag)
{
	typedef itk::RecursiveGaussianImageFilter< TInputImage, TOutputImage >  GaussianFilterType;
	typedef itk::RecursiveGaussianImageFilter< TOutputImage, TOutputImage >  GaussianFilterType1;
    typename GaussianFilterType::Pointer smoothfilterX = GaussianFilterType::New();
    typename GaussianFilterType1::Pointer smoothfilterY = GaussianFilterType1::New();
    typename GaussianFilterType1::Pointer smoothfilterZ = GaussianFilterType1::New();
		
    smoothfilterX->SetDirection(0);
    smoothfilterY->SetDirection(1);
    smoothfilterZ->SetDirection(2);
		
    smoothfilterX->SetOrder( GaussianFilterType::ZeroOrder );
    smoothfilterY->SetOrder( GaussianFilterType1::ZeroOrder );
    smoothfilterZ->SetOrder( GaussianFilterType1::ZeroOrder );
    
    smoothfilterX->SetNormalizeAcrossScale( scale_flag );
    smoothfilterY->SetNormalizeAcrossScale( scale_flag );
    smoothfilterZ->SetNormalizeAcrossScale( scale_flag );
	
    smoothfilterX->SetSigma(scale_x);
    smoothfilterY->SetSigma(scale_y);
    smoothfilterZ->SetSigma(scale_z);
    
    smoothfilterX->SetInput( img );
    smoothfilterY->SetInput( smoothfilterX->GetOutput() );
    smoothfilterZ->SetInput( smoothfilterY->GetOutput() );

	smoothfilterX->SetReleaseDataFlag(true);
	smoothfilterY->SetReleaseDataFlag(true);
	smoothfilterZ->SetReleaseDataFlag(true);
	
	smoothfilterZ->Update();
	return smoothfilterZ->GetOutput();
}

template <class TInputImage, class TOutputImage>
typename TOutputImage::Pointer scalespace::DxImage(typename TInputImage::Pointer img, float scale, bool scale_flag)
{
	typedef itk::RecursiveGaussianImageFilter< TInputImage, TOutputImage > GaussianFilterType;
	typedef itk::RecursiveGaussianImageFilter< TOutputImage, TOutputImage > GaussianFilterType1;
    typename GaussianFilterType::Pointer smoothfilterX = GaussianFilterType::New();
    typename GaussianFilterType1::Pointer smoothfilterY = GaussianFilterType1::New();
	
    //compute y gradient
    smoothfilterX->SetDirection(0);
    smoothfilterY->SetDirection(1);
	
    smoothfilterX->SetOrder( GaussianFilterType::FirstOrder );
    smoothfilterY->SetOrder( GaussianFilterType1::ZeroOrder );
    
    smoothfilterX->SetNormalizeAcrossScale( scale_flag );
    smoothfilterY->SetNormalizeAcrossScale( scale_flag );
    
    smoothfilterX->SetSigma(scale);
    smoothfilterY->SetSigma(scale);
    
    smoothfilterX->SetInput( img );
    smoothfilterY->SetInput( smoothfilterX->GetOutput() );
    
	smoothfilterX->SetReleaseDataFlag(true);
	smoothfilterY->SetReleaseDataFlag(true);
    
    if (img->GetImageDimension() == 3)
    {
		typename GaussianFilterType1::Pointer smoothfilterZ = GaussianFilterType1::New();
		smoothfilterZ->SetDirection(2);
		smoothfilterZ->SetOrder( GaussianFilterType1::ZeroOrder );
		smoothfilterZ->SetNormalizeAcrossScale( scale_flag );
		smoothfilterZ->SetSigma(scale);
		smoothfilterZ->SetInput( smoothfilterY->GetOutput() );
		smoothfilterZ->SetReleaseDataFlag(true);
		smoothfilterZ->Update();
		
		return smoothfilterZ->GetOutput();
    }
    else
    {
		smoothfilterY->Update();
		return smoothfilterY->GetOutput();
	}
}

template <class TInputImage, class TOutputImage>
typename TOutputImage::Pointer scalespace::DyImage(typename TInputImage::Pointer img, float scale, bool scale_flag)
{
	typedef itk::RecursiveGaussianImageFilter< TInputImage, TOutputImage > GaussianFilterType;
	typedef itk::RecursiveGaussianImageFilter< TOutputImage, TOutputImage > GaussianFilterType1;
    typename GaussianFilterType::Pointer smoothfilterX = GaussianFilterType::New();
    typename GaussianFilterType1::Pointer smoothfilterY = GaussianFilterType1::New();
	
    //compute y gradient
    smoothfilterX->SetDirection(0);
    smoothfilterY->SetDirection(1);
	
    smoothfilterX->SetOrder( GaussianFilterType1::ZeroOrder );
    smoothfilterY->SetOrder( GaussianFilterType::FirstOrder );
    
    smoothfilterX->SetNormalizeAcrossScale( scale_flag );
    smoothfilterY->SetNormalizeAcrossScale( scale_flag );
    
    smoothfilterX->SetSigma(scale);
    smoothfilterY->SetSigma(scale);
    
    smoothfilterX->SetInput( img );
    smoothfilterY->SetInput( smoothfilterX->GetOutput() );
    
	smoothfilterX->SetReleaseDataFlag(true);
	smoothfilterY->SetReleaseDataFlag(true);
    
    if (img->GetImageDimension() == 3)
    {
		typename GaussianFilterType1::Pointer smoothfilterZ = GaussianFilterType1::New();
		smoothfilterZ->SetDirection(2);
		smoothfilterZ->SetOrder( GaussianFilterType1::ZeroOrder );
		smoothfilterZ->SetNormalizeAcrossScale( scale_flag );
		smoothfilterZ->SetSigma(scale);
		smoothfilterZ->SetInput( smoothfilterY->GetOutput() );
		smoothfilterZ->SetReleaseDataFlag(true);
		smoothfilterZ->Update();
		
		return smoothfilterZ->GetOutput();
    }
    else
    {
		smoothfilterY->Update();
		return smoothfilterY->GetOutput();
	}
}

template <class TInputImage, class TOutputImage>
typename TOutputImage::Pointer scalespace::DzImage(typename TInputImage::Pointer img, float scale, bool scale_flag)
{
	typedef itk::RecursiveGaussianImageFilter< TInputImage, TOutputImage > GaussianFilterType;
	typedef itk::RecursiveGaussianImageFilter< TOutputImage, TOutputImage > GaussianFilterType1;
    typename GaussianFilterType::Pointer smoothfilterX = GaussianFilterType::New();
    typename GaussianFilterType1::Pointer smoothfilterY = GaussianFilterType1::New();
    typename GaussianFilterType1::Pointer smoothfilterZ = GaussianFilterType1::New();
	
    //compute y gradient
    smoothfilterX->SetDirection(0);
    smoothfilterY->SetDirection(1);
    smoothfilterZ->SetDirection(2);
	
    smoothfilterX->SetOrder( GaussianFilterType1::ZeroOrder );
    smoothfilterY->SetOrder( GaussianFilterType1::ZeroOrder );
    smoothfilterZ->SetOrder( GaussianFilterType::FirstOrder );
    
    smoothfilterX->SetNormalizeAcrossScale( scale_flag );
    smoothfilterY->SetNormalizeAcrossScale( scale_flag );
    smoothfilterZ->SetNormalizeAcrossScale( scale_flag );
    
    smoothfilterX->SetSigma(scale);
    smoothfilterY->SetSigma(scale);
    smoothfilterZ->SetSigma(scale);
    
    smoothfilterX->SetInput( img );
    smoothfilterY->SetInput( smoothfilterX->GetOutput() );
    smoothfilterZ->SetInput( smoothfilterY->GetOutput() );
    
	smoothfilterX->SetReleaseDataFlag(true);
	smoothfilterY->SetReleaseDataFlag(true);
	smoothfilterZ->SetReleaseDataFlag(true);
    
	smoothfilterZ->Update();	
	return smoothfilterZ->GetOutput();
}

template <class TInputImage, class TOutputImage>
typename TOutputImage::Pointer scalespace::DxxImage(typename TInputImage::Pointer img, float scale, bool scale_flag)
{
	typedef itk::RecursiveGaussianImageFilter< TInputImage, TOutputImage > GaussianFilterType;
	typedef itk::RecursiveGaussianImageFilter< TOutputImage, TOutputImage > GaussianFilterType1;
    typename GaussianFilterType::Pointer smoothfilterX = GaussianFilterType::New();
    typename GaussianFilterType1::Pointer smoothfilterY = GaussianFilterType1::New();
	
    //compute y gradient
    smoothfilterX->SetDirection(0);
    smoothfilterY->SetDirection(1);

    smoothfilterX->SetOrder( GaussianFilterType::SecondOrder );
    smoothfilterY->SetOrder( GaussianFilterType1::ZeroOrder );
    
    smoothfilterX->SetNormalizeAcrossScale( scale_flag );
    smoothfilterY->SetNormalizeAcrossScale( scale_flag );
	
    smoothfilterX->SetSigma(scale);
    smoothfilterY->SetSigma(scale);
    
    smoothfilterX->SetInput( img );
    smoothfilterY->SetInput( smoothfilterX->GetOutput() );

	smoothfilterX->SetReleaseDataFlag(true);
	smoothfilterY->SetReleaseDataFlag(true);
    
    if (img->GetImageDimension() == 3)
    {
		typename GaussianFilterType1::Pointer smoothfilterZ = GaussianFilterType1::New();
		smoothfilterZ->SetDirection(2);
		smoothfilterZ->SetOrder( GaussianFilterType1::ZeroOrder );
		smoothfilterZ->SetNormalizeAcrossScale( scale_flag );
		smoothfilterZ->SetSigma(scale);
		smoothfilterZ->SetInput( smoothfilterY->GetOutput() );
		smoothfilterZ->SetReleaseDataFlag(true);
		smoothfilterZ->Update();
		return smoothfilterZ->GetOutput();
    }
    else
    {
		smoothfilterY->Update(); 
		return smoothfilterY->GetOutput();
	}
}

template <class TInputImage, class TOutputImage>
typename TOutputImage::Pointer scalespace::DyyImage(typename TInputImage::Pointer img, float scale, bool scale_flag)
{
	typedef itk::RecursiveGaussianImageFilter< TInputImage, TOutputImage > GaussianFilterType;
	typedef itk::RecursiveGaussianImageFilter< TOutputImage, TOutputImage > GaussianFilterType1;
    typename GaussianFilterType::Pointer smoothfilterX = GaussianFilterType::New();
    typename GaussianFilterType1::Pointer smoothfilterY = GaussianFilterType1::New();
	
    //compute y gradient
    smoothfilterX->SetDirection(0);
    smoothfilterY->SetDirection(1);

    smoothfilterX->SetOrder( GaussianFilterType1::ZeroOrder );
    smoothfilterY->SetOrder( GaussianFilterType::SecondOrder );
    
    smoothfilterX->SetNormalizeAcrossScale( scale_flag );
    smoothfilterY->SetNormalizeAcrossScale( scale_flag );
	
    smoothfilterX->SetSigma(scale);
    smoothfilterY->SetSigma(scale);
    
    smoothfilterX->SetInput( img );
    smoothfilterY->SetInput( smoothfilterX->GetOutput() );

	smoothfilterX->SetReleaseDataFlag(true);
	smoothfilterY->SetReleaseDataFlag(true);
    
    if (img->GetImageDimension() == 3)
    {
		typename GaussianFilterType1::Pointer smoothfilterZ = GaussianFilterType1::New();
		smoothfilterZ->SetDirection(2);
		smoothfilterZ->SetOrder( GaussianFilterType1::ZeroOrder );
		smoothfilterZ->SetNormalizeAcrossScale( scale_flag );
		smoothfilterZ->SetSigma(scale);
		smoothfilterZ->SetInput( smoothfilterY->GetOutput() );
		smoothfilterZ->SetReleaseDataFlag(true);
		smoothfilterZ->Update();
		return smoothfilterZ->GetOutput();
    }
    else
    {
		smoothfilterY->Update(); 
		return smoothfilterY->GetOutput();
	}
}

template <class TInputImage, class TOutputImage>
typename TOutputImage::Pointer scalespace::DzzImage(typename TInputImage::Pointer img, float scale, bool scale_flag)
{
	typedef itk::RecursiveGaussianImageFilter< TInputImage, TOutputImage > GaussianFilterType;
	typedef itk::RecursiveGaussianImageFilter< TOutputImage, TOutputImage > GaussianFilterType1;
    typename GaussianFilterType::Pointer smoothfilterX = GaussianFilterType::New();
    typename GaussianFilterType1::Pointer smoothfilterY = GaussianFilterType1::New();
    typename GaussianFilterType1::Pointer smoothfilterZ = GaussianFilterType1::New();
	
    //compute y gradient
    smoothfilterX->SetDirection(0);
    smoothfilterY->SetDirection(1);
    smoothfilterZ->SetDirection(2);
	
    smoothfilterX->SetOrder( GaussianFilterType1::ZeroOrder );
    smoothfilterY->SetOrder( GaussianFilterType1::ZeroOrder );
    smoothfilterZ->SetOrder( GaussianFilterType::SecondOrder );
    
    smoothfilterX->SetNormalizeAcrossScale( scale_flag );
    smoothfilterY->SetNormalizeAcrossScale( scale_flag );
    smoothfilterZ->SetNormalizeAcrossScale( scale_flag );
    
    smoothfilterX->SetSigma(scale);
    smoothfilterY->SetSigma(scale);
    smoothfilterZ->SetSigma(scale);
    
    smoothfilterX->SetInput( img );
    smoothfilterY->SetInput( smoothfilterX->GetOutput() );
    smoothfilterZ->SetInput( smoothfilterY->GetOutput() );
    
	smoothfilterX->SetReleaseDataFlag(true);
	smoothfilterY->SetReleaseDataFlag(true);
	smoothfilterZ->SetReleaseDataFlag(true);
    
	smoothfilterZ->Update();	
	return smoothfilterZ->GetOutput();
}

template <class TInputImage, class TOutputImage>
typename TOutputImage::Pointer scalespace::DxyImage(typename TInputImage::Pointer img, float scale, bool scale_flag)
{
	typedef itk::RecursiveGaussianImageFilter< TInputImage, TOutputImage > GaussianFilterType;
	typedef itk::RecursiveGaussianImageFilter< TOutputImage, TOutputImage > GaussianFilterType1;
    typename GaussianFilterType::Pointer smoothfilterX = GaussianFilterType::New();
    typename GaussianFilterType1::Pointer smoothfilterY = GaussianFilterType1::New();
	
    //compute y gradient
    smoothfilterX->SetDirection(0);
    smoothfilterY->SetDirection(1);
	
    smoothfilterX->SetOrder( GaussianFilterType1::FirstOrder );
    smoothfilterY->SetOrder( GaussianFilterType::FirstOrder );
    
    smoothfilterX->SetNormalizeAcrossScale( scale_flag );
    smoothfilterY->SetNormalizeAcrossScale( scale_flag );
    
    smoothfilterX->SetSigma(scale);
    smoothfilterY->SetSigma(scale);
    
    smoothfilterX->SetInput( img );
    smoothfilterY->SetInput( smoothfilterX->GetOutput() );
    
	smoothfilterX->SetReleaseDataFlag(true);
	smoothfilterY->SetReleaseDataFlag(true);
    
    if (img->GetImageDimension() == 3)
    {
		typename GaussianFilterType1::Pointer smoothfilterZ = GaussianFilterType1::New();
		smoothfilterZ->SetDirection(2);
		smoothfilterZ->SetOrder( GaussianFilterType1::ZeroOrder );
		smoothfilterZ->SetNormalizeAcrossScale( scale_flag );
		smoothfilterZ->SetSigma(scale);
		smoothfilterZ->SetInput( smoothfilterY->GetOutput() );
		smoothfilterZ->SetReleaseDataFlag(true);
		smoothfilterZ->Update();
		
		return smoothfilterZ->GetOutput();
    }
    else
    {
		smoothfilterY->Update();
		return smoothfilterY->GetOutput();
	}
}

template <class TInputImage, class TOutputImage>
typename TOutputImage::Pointer scalespace::DxyImage(typename TInputImage::Pointer img, float x_scale, 
	float y_scale, float z_scale, bool scale_flag)
{
	typedef itk::RecursiveGaussianImageFilter< TInputImage, TOutputImage > GaussianFilterType;
	typedef itk::RecursiveGaussianImageFilter< TOutputImage, TOutputImage > GaussianFilterType1;
    typename GaussianFilterType::Pointer smoothfilterX = GaussianFilterType::New();
    typename GaussianFilterType1::Pointer smoothfilterY = GaussianFilterType1::New();
	
    //compute y gradient
    smoothfilterX->SetDirection(0);
    smoothfilterY->SetDirection(1);
	
    smoothfilterX->SetOrder( GaussianFilterType1::FirstOrder );
    smoothfilterY->SetOrder( GaussianFilterType::FirstOrder );
    
    smoothfilterX->SetNormalizeAcrossScale( scale_flag );
    smoothfilterY->SetNormalizeAcrossScale( scale_flag );
    
    smoothfilterX->SetSigma(x_scale);
    smoothfilterY->SetSigma(y_scale);
    
    smoothfilterX->SetInput( img );
    smoothfilterY->SetInput( smoothfilterX->GetOutput() );
    
	smoothfilterX->SetReleaseDataFlag(true);
	smoothfilterY->SetReleaseDataFlag(true);
    
    if (img->GetImageDimension() == 3)
    {
		typename GaussianFilterType1::Pointer smoothfilterZ = GaussianFilterType1::New();
		smoothfilterZ->SetDirection(2);
		smoothfilterZ->SetOrder( GaussianFilterType1::ZeroOrder );
		smoothfilterZ->SetNormalizeAcrossScale( scale_flag );
		smoothfilterZ->SetSigma(z_scale);
		smoothfilterZ->SetInput( smoothfilterY->GetOutput() );
		smoothfilterZ->SetReleaseDataFlag(true);
		smoothfilterZ->Update();
		
		return smoothfilterZ->GetOutput();
    }
    else
    {
		smoothfilterY->Update();
		return smoothfilterY->GetOutput();
	}
}

template <class TInputImage, class TOutputImage>
typename TOutputImage::Pointer scalespace::DxzImage(typename TInputImage::Pointer img, float scale, bool scale_flag)
{
	typedef itk::RecursiveGaussianImageFilter< TInputImage, TOutputImage > GaussianFilterType;
	typedef itk::RecursiveGaussianImageFilter< TOutputImage, TOutputImage > GaussianFilterType1;
    typename GaussianFilterType::Pointer smoothfilterX = GaussianFilterType::New();
    typename GaussianFilterType1::Pointer smoothfilterY = GaussianFilterType1::New();
    typename GaussianFilterType1::Pointer smoothfilterZ = GaussianFilterType1::New();
	
    //compute y gradient
    smoothfilterX->SetDirection(0);
    smoothfilterY->SetDirection(1);
    smoothfilterZ->SetDirection(2);
	
    smoothfilterX->SetOrder( GaussianFilterType1::FirstOrder );
    smoothfilterY->SetOrder( GaussianFilterType1::ZeroOrder );
    smoothfilterZ->SetOrder( GaussianFilterType::FirstOrder );
    
    smoothfilterX->SetNormalizeAcrossScale( scale_flag );
    smoothfilterY->SetNormalizeAcrossScale( scale_flag );
    smoothfilterZ->SetNormalizeAcrossScale( scale_flag );
    
    smoothfilterX->SetSigma(scale);
    smoothfilterY->SetSigma(scale);
    smoothfilterZ->SetSigma(scale);
    
    smoothfilterX->SetInput( img );
    smoothfilterY->SetInput( smoothfilterX->GetOutput() );
    smoothfilterZ->SetInput( smoothfilterY->GetOutput() );
    
	smoothfilterX->SetReleaseDataFlag(true);
	smoothfilterY->SetReleaseDataFlag(true);
	smoothfilterZ->SetReleaseDataFlag(true);
    
	smoothfilterZ->Update();	
	return smoothfilterZ->GetOutput();
}

template <class TInputImage, class TOutputImage>
typename TOutputImage::Pointer scalespace::DxzImage(typename TInputImage::Pointer img, float x_scale, 
	float y_scale, float z_scale, bool scale_flag)
{
	typedef itk::RecursiveGaussianImageFilter< TInputImage, TOutputImage > GaussianFilterType;
	typedef itk::RecursiveGaussianImageFilter< TOutputImage, TOutputImage > GaussianFilterType1;
    typename GaussianFilterType::Pointer smoothfilterX = GaussianFilterType::New();
    typename GaussianFilterType1::Pointer smoothfilterY = GaussianFilterType1::New();
    typename GaussianFilterType1::Pointer smoothfilterZ = GaussianFilterType1::New();
	
    //compute y gradient
    smoothfilterX->SetDirection(0);
    smoothfilterY->SetDirection(1);
    smoothfilterZ->SetDirection(2);
	
    smoothfilterX->SetOrder( GaussianFilterType1::FirstOrder );
    smoothfilterY->SetOrder( GaussianFilterType1::ZeroOrder );
    smoothfilterZ->SetOrder( GaussianFilterType::FirstOrder );
    
    smoothfilterX->SetNormalizeAcrossScale( scale_flag );
    smoothfilterY->SetNormalizeAcrossScale( scale_flag );
    smoothfilterZ->SetNormalizeAcrossScale( scale_flag );
    
    smoothfilterX->SetSigma(x_scale);
    smoothfilterY->SetSigma(y_scale);
    smoothfilterZ->SetSigma(z_scale);
    
    smoothfilterX->SetInput( img );
    smoothfilterY->SetInput( smoothfilterX->GetOutput() );
    smoothfilterZ->SetInput( smoothfilterY->GetOutput() );
    
	smoothfilterX->SetReleaseDataFlag(true);
	smoothfilterY->SetReleaseDataFlag(true);
	smoothfilterZ->SetReleaseDataFlag(true);
    
	smoothfilterZ->Update();	
	return smoothfilterZ->GetOutput();
}

template <class TInputImage, class TOutputImage>
typename TOutputImage::Pointer scalespace::DyzImage(typename TInputImage::Pointer img, float scale, bool scale_flag)
{
	typedef itk::RecursiveGaussianImageFilter< TInputImage, TOutputImage > GaussianFilterType;
	typedef itk::RecursiveGaussianImageFilter< TOutputImage, TOutputImage > GaussianFilterType1;
    typename GaussianFilterType::Pointer smoothfilterX = GaussianFilterType::New();
    typename GaussianFilterType1::Pointer smoothfilterY = GaussianFilterType1::New();
    typename GaussianFilterType1::Pointer smoothfilterZ = GaussianFilterType1::New();
	
    //compute y gradient
    smoothfilterX->SetDirection(0);
    smoothfilterY->SetDirection(1);
    smoothfilterZ->SetDirection(2);
	
    smoothfilterX->SetOrder( GaussianFilterType1::ZeroOrder );
    smoothfilterY->SetOrder( GaussianFilterType1::FirstOrder );
    smoothfilterZ->SetOrder( GaussianFilterType::FirstOrder );
    
    smoothfilterX->SetNormalizeAcrossScale( scale_flag );
    smoothfilterY->SetNormalizeAcrossScale( scale_flag );
    smoothfilterZ->SetNormalizeAcrossScale( scale_flag );
    
    smoothfilterX->SetSigma(scale);
    smoothfilterY->SetSigma(scale);
    smoothfilterZ->SetSigma(scale);
    
    smoothfilterX->SetInput( img );
    smoothfilterY->SetInput( smoothfilterX->GetOutput() );
    smoothfilterZ->SetInput( smoothfilterY->GetOutput() );
    
	smoothfilterX->SetReleaseDataFlag(true);
	smoothfilterY->SetReleaseDataFlag(true);
	smoothfilterZ->SetReleaseDataFlag(true);
    
	smoothfilterZ->Update();	
	return smoothfilterZ->GetOutput();
}

template <class TInputImage, class TOutputImage>
typename TOutputImage::Pointer scalespace::DyzImage(typename TInputImage::Pointer img, float x_scale, 
	float y_scale, float z_scale, bool scale_flag)
{
	typedef itk::RecursiveGaussianImageFilter< TInputImage, TOutputImage > GaussianFilterType;
	typedef itk::RecursiveGaussianImageFilter< TOutputImage, TOutputImage > GaussianFilterType1;
    typename GaussianFilterType::Pointer smoothfilterX = GaussianFilterType::New();
    typename GaussianFilterType1::Pointer smoothfilterY = GaussianFilterType1::New();
    typename GaussianFilterType1::Pointer smoothfilterZ = GaussianFilterType1::New();
	
    //compute y gradient
    smoothfilterX->SetDirection(0);
    smoothfilterY->SetDirection(1);
    smoothfilterZ->SetDirection(2);
	
    smoothfilterX->SetOrder( GaussianFilterType1::ZeroOrder );
    smoothfilterY->SetOrder( GaussianFilterType1::FirstOrder );
    smoothfilterZ->SetOrder( GaussianFilterType::FirstOrder );
    
    smoothfilterX->SetNormalizeAcrossScale( scale_flag );
    smoothfilterY->SetNormalizeAcrossScale( scale_flag );
    smoothfilterZ->SetNormalizeAcrossScale( scale_flag );
    
    smoothfilterX->SetSigma(x_scale);
    smoothfilterY->SetSigma(y_scale);
    smoothfilterZ->SetSigma(z_scale);
    
    smoothfilterX->SetInput( img );
    smoothfilterY->SetInput( smoothfilterX->GetOutput() );
    smoothfilterZ->SetInput( smoothfilterY->GetOutput() );
    
	smoothfilterX->SetReleaseDataFlag(true);
	smoothfilterY->SetReleaseDataFlag(true);
	smoothfilterZ->SetReleaseDataFlag(true);
    
	smoothfilterZ->Update();	
	return smoothfilterZ->GetOutput();
}

template <class TInputImage, class TOutputImage>
typename TOutputImage::Pointer scalespace::DxyzImage(typename TInputImage::Pointer img, float x_scale, 
	float y_scale, float z_scale, bool scale_flag)
{
	typedef itk::RecursiveGaussianImageFilter< TInputImage, TOutputImage > GaussianFilterType;
	typedef itk::RecursiveGaussianImageFilter< TOutputImage, TOutputImage > GaussianFilterType1;
    typename GaussianFilterType::Pointer smoothfilterX = GaussianFilterType::New();
    typename GaussianFilterType1::Pointer smoothfilterY = GaussianFilterType1::New();
    typename GaussianFilterType1::Pointer smoothfilterZ = GaussianFilterType1::New();
	
    //compute y gradient
    smoothfilterX->SetDirection(0);
    smoothfilterY->SetDirection(1);
    smoothfilterZ->SetDirection(2);
	
    smoothfilterX->SetOrder( GaussianFilterType1::FirstOrder );
    smoothfilterY->SetOrder( GaussianFilterType1::FirstOrder );
    smoothfilterZ->SetOrder( GaussianFilterType::FirstOrder );
    
    smoothfilterX->SetNormalizeAcrossScale( scale_flag );
    smoothfilterY->SetNormalizeAcrossScale( scale_flag );
    smoothfilterZ->SetNormalizeAcrossScale( scale_flag );
    
    smoothfilterX->SetSigma(x_scale);
    smoothfilterY->SetSigma(y_scale);
    smoothfilterZ->SetSigma(z_scale);
    
    smoothfilterX->SetInput( img );
    smoothfilterY->SetInput( smoothfilterX->GetOutput() );
    smoothfilterZ->SetInput( smoothfilterY->GetOutput() );
    
	smoothfilterX->SetReleaseDataFlag(true);
	smoothfilterY->SetReleaseDataFlag(true);
	smoothfilterZ->SetReleaseDataFlag(true);
    
	smoothfilterZ->Update();	
	return smoothfilterZ->GetOutput();
}
	
template <class TInputImage, class TOutputImage>
typename TOutputImage::Pointer scalespace::DxxxImage(typename TInputImage::Pointer img, float x_scale, bool scale_flag)
{
	typename TOutputImage::Pointer tmp_img = DxImage<TInputImage, TOutputImage>(img, x_scale, scale_flag);
	return DxxImage<TOutputImage, TOutputImage>(tmp_img, x_scale, scale_flag);
}

template <class TInputImage, class TOutputImage>
typename TOutputImage::Pointer scalespace::DyyyImage(typename TInputImage::Pointer img, float y_scale, bool scale_flag)
{
	typename TOutputImage::Pointer tmp_img = DyImage<TInputImage, TOutputImage>(img, y_scale, scale_flag);
	return DyyImage<TOutputImage, TOutputImage>(tmp_img, y_scale, scale_flag);
}

template <class TInputImage, class TOutputImage>
typename TOutputImage::Pointer scalespace::DzzzImage(typename TInputImage::Pointer img, float z_scale, bool scale_flag)
{
	typename TOutputImage::Pointer tmp_img = DzImage<TInputImage, TOutputImage>(img, z_scale, scale_flag);
	return DzzImage<TOutputImage, TOutputImage>(tmp_img, z_scale, scale_flag);
}

template <class TInputImage, class TOutputImage>
typename TOutputImage::Pointer scalespace::DxxyImage(typename TInputImage::Pointer img, float x_scale, 
	float y_scale, bool scale_flag)
{
	typename TOutputImage::Pointer tmp_img = DxxImage<TInputImage, TOutputImage>(img, x_scale, scale_flag);
	return DyImage<TOutputImage, TOutputImage>(tmp_img, y_scale, scale_flag);
}

template <class TInputImage, class TOutputImage>
typename TOutputImage::Pointer scalespace::DxxzImage(typename TInputImage::Pointer img, float x_scale, 
	float z_scale, bool scale_flag)
{
	typename TOutputImage::Pointer tmp_img = DxxImage<TInputImage, TOutputImage>(img, x_scale, scale_flag);
	return DzImage<TOutputImage, TOutputImage>(tmp_img, z_scale, scale_flag);
}

template <class TInputImage, class TOutputImage>
typename TOutputImage::Pointer scalespace::DxyyImage(typename TInputImage::Pointer img, float x_scale, 
	float y_scale, bool scale_flag)
{
	typename TOutputImage::Pointer tmp_img = DxImage<TInputImage, TOutputImage>(img, x_scale, scale_flag);
	return DyyImage<TOutputImage, TOutputImage>(tmp_img, y_scale, scale_flag);
}

template <class TInputImage, class TOutputImage>
typename TOutputImage::Pointer scalespace::DyyzImage(typename TInputImage::Pointer img, float y_scale, 
	float z_scale, bool scale_flag)
{
	typename TOutputImage::Pointer tmp_img = DyyImage<TInputImage, TOutputImage>(img, y_scale, scale_flag);
	return DzImage<TOutputImage, TOutputImage>(tmp_img, z_scale, scale_flag);
}

template <class TInputImage, class TOutputImage>
typename TOutputImage::Pointer scalespace::DxzzImage(typename TInputImage::Pointer img, float x_scale, 
	float z_scale, bool scale_flag)
{
	typename TOutputImage::Pointer tmp_img = DxImage<TInputImage, TOutputImage>(img, x_scale, scale_flag);
	return DzzImage<TOutputImage, TOutputImage>(tmp_img, z_scale, scale_flag);
}

template <class TInputImage, class TOutputImage>
typename TOutputImage::Pointer scalespace::DyzzImage(typename TInputImage::Pointer img, float y_scale, 
	float z_scale, bool scale_flag)
{
	typename TOutputImage::Pointer tmp_img = DyImage<TInputImage, TOutputImage>(img, y_scale, scale_flag);
	return DzzImage<TOutputImage, TOutputImage>(tmp_img, z_scale, scale_flag);
}

template <class TInputImage, class TOutputImage>
typename TOutputImage::Pointer scalespace::ComputeGradientMagnitude(typename TInputImage::Pointer img)
{
	typedef itk::GradientMagnitudeImageFilter< TInputImage, TOutputImage >  filterType;
    typename filterType::Pointer gradientFilter = filterType::New();
	gradientFilter->SetInput( img );
	gradientFilter->Update();
	return gradientFilter->GetOutput();
}

template <class TInputImage, class TOutputImage>
typename TOutputImage::Pointer scalespace::ComputeGradientMagnitude(typename TInputImage::Pointer Ix, 
	typename TInputImage::Pointer Iy)
{
	typename TOutputImage::Pointer res_img = AllocateImage<TInputImage, TOutputImage>(Ix);
	typedef itk::ImageRegionIteratorWithIndex<TOutputImage>  OutputIteratorType;
	typedef itk::ImageRegionIteratorWithIndex<TInputImage>  InputIteratorType;
	
	InputIteratorType Ix_it(Ix, Ix->GetLargestPossibleRegion());
	InputIteratorType Iy_it(Iy, Iy->GetLargestPossibleRegion());
	OutputIteratorType res_it(res_img, res_img->GetLargestPossibleRegion());
	
	for (Ix_it.GoToBegin(), Iy_it.GoToBegin(), res_it.GoToBegin(); !res_it.IsAtEnd(); 
		++res_it, ++Ix_it, ++Iy_it)
	{
		res_it.Set(sqrt(Ix_it.Get()*Ix_it.Get()+Iy_it.Get()*Iy_it.Get()));
	}
	
	return res_img;
}
 
template <class TInputImage, class TOutputImage>
typename TOutputImage::Pointer scalespace::ComputeGradientMagnitude(typename TInputImage::Pointer Ix, 
	typename TInputImage::Pointer Iy, typename TInputImage::Pointer Iz)
{
	typename TOutputImage::Pointer res_img = AllocateImage<TInputImage, TOutputImage>(Ix);
	typedef itk::ImageRegionIteratorWithIndex<TOutputImage>  OutputIteratorType;
	typedef itk::ImageRegionIteratorWithIndex<TInputImage>  InputIteratorType;
	
	InputIteratorType Ix_it(Ix, Ix->GetLargestPossibleRegion());
	InputIteratorType Iy_it(Iy, Iy->GetLargestPossibleRegion());
	InputIteratorType Iz_it(Iz, Iz->GetLargestPossibleRegion());
	OutputIteratorType res_it(res_img, res_img->GetLargestPossibleRegion());
	
	for (Ix_it.GoToBegin(), Iy_it.GoToBegin(), Iz_it.GoToBegin(), res_it.GoToBegin(); !res_it.IsAtEnd(); 
		++res_it, ++Ix_it, ++Iy_it, ++Iz_it)
	{
		res_it.Set(sqrt(Ix_it.Get()*Ix_it.Get()+Iy_it.Get()*Iy_it.Get()+Iz_it.Get()*Iz_it.Get()));
	}
	
	return res_img;
}   

template <class TInputImage, class TOutputImage>
typename TOutputImage::Pointer scalespace::ComputeAngleInRadian(typename TInputImage::Pointer Ix, 
	typename TInputImage::Pointer Iy)
{
	typename TOutputImage::Pointer res_img = AllocateImage<TInputImage, TOutputImage>(Ix);
	
	typedef itk::ImageRegionIteratorWithIndex<TOutputImage>  OutputIteratorType;
	typedef itk::ImageRegionIteratorWithIndex<TInputImage>  InputIteratorType;
	
	InputIteratorType Ix_it(Ix, Ix->GetLargestPossibleRegion());
	InputIteratorType Iy_it(Iy, Iy->GetLargestPossibleRegion());
	OutputIteratorType res_it(res_img, res_img->GetLargestPossibleRegion());
	
	for (Ix_it.GoToBegin(), Iy_it.GoToBegin(), res_it.GoToBegin(); !res_it.IsAtEnd(); 
		++res_it, ++Ix_it, ++Iy_it)
	{
		res_it.Set(atan2(Ix_it.Get(), Iy_it.Get()));
	}
	
	return res_img;
}

template <class TInputImage, class TOutputImage>
typename TOutputImage::Pointer scalespace::ComputeAngle(typename TInputImage::Pointer Ix, 
	typename TInputImage::Pointer Iy)
{
	typename TOutputImage::Pointer res_img = AllocateImage<TInputImage, TOutputImage>(Ix);
	
	typedef itk::ImageRegionIteratorWithIndex<TOutputImage>  OutputIteratorType;
	typedef itk::ImageRegionIteratorWithIndex<TInputImage>  InputIteratorType;
	
	InputIteratorType Ix_it(Ix, Ix->GetLargestPossibleRegion());
	InputIteratorType Iy_it(Iy, Iy->GetLargestPossibleRegion());
	OutputIteratorType res_it(res_img, res_img->GetLargestPossibleRegion());
	
	for (Ix_it.GoToBegin(), Iy_it.GoToBegin(), res_it.GoToBegin(); !res_it.IsAtEnd(); 
		++res_it, ++Ix_it, ++Iy_it)
	{
		res_it.Set(Math_Angle<TOutputImage::PixelType, TOutputImage::PixelType>(Ix_it.Get(), Iy_it.Get()));
	}
	
	return res_img;
}

template <class TInputImage, class TOutputImage>
typename TOutputImage::Pointer scalespace::ComputeDirection(typename TInputImage::Pointer Ix, 
	typename TInputImage::Pointer Iy)
{
	typename TOutputImage::Pointer res_img = AllocateImage<TInputImage, TOutputImage>(Ix);
	
	typedef itk::ImageRegionIteratorWithIndex<TOutputImage>  OutputIteratorType;
	typedef itk::ImageRegionIteratorWithIndex<TInputImage>  InputIteratorType;
	
	InputIteratorType Ix_it(Ix, Ix->GetLargestPossibleRegion());
	InputIteratorType Iy_it(Iy, Iy->GetLargestPossibleRegion());
	OutputIteratorType res_it(res_img, res_img->GetLargestPossibleRegion());
	
	for (Ix_it.GoToBegin(), Iy_it.GoToBegin(), res_it.GoToBegin(); !res_it.IsAtEnd(); 
		++res_it, ++Ix_it, ++Iy_it)
	{
		res_it.Set(Math_Direction<TInputImage::PixelType>(Ix_it.Get(), Iy_it.Get()));
	}
	
	return res_img;
}

template <class TInputImage, class TOutputImage>
typename TOutputImage::Pointer scalespace::ComputeFeatures3D(typename TInputImage::Pointer img, float scale, 
	FeatureType ft, const float ratio)   
{ 
	//if (img.isNull()) return ByteImage();
	FloatImageType::Pointer pimg = scalespace::SmoothImage<TInputImage, FloatImageType>(img, scale, ratio);
	//if (pimg.isNull()) return ByteImage();
  
	FloatImageType::SizeType ndim, ndim_1, radius;
	FloatImageType::IndexType nstart, nstart_1;
	ndim = pimg->GetLargestPossibleRegion().GetSize();
	for (unsigned int i=0; i<3; i++)
	{
		nstart[i] = 0;
		nstart_1[i] = 1;
		ndim_1[i] = ndim[i]-2;
		radius[i] = 1;
	}
  
	// int z, x, y;  
	float Ix, Iy, Iz, Ixx, Iyy, Izz, Ixy, Ixz, Iyz, det, t, le;
	bool ef2;
	typename TOutputImage::Pointer feat = AllocateImage<TInputImage, TOutputImage>(img);
	feat->FillBuffer(Background);
	typename TOutputImage::IndexType centerIndex;
	
	FloatImageType::RegionType region;
	region.SetSize(ndim_1);
	region.SetIndex(nstart_1);
	itk::NeighborhoodIterator<FloatImageType> iterator(radius, pimg, region);
	
	switch (ft) 
	{
	case BrightZC:
	
		while(!iterator.IsAtEnd())
		{
			// Set the current pixel to white
			t = 2.0*iterator.GetCenterPixel();
			Ixx = iterator.GetPixel(12) + iterator.GetPixel(14) - t;
			Iyy = iterator.GetPixel(10) + iterator.GetPixel(16) - t;
			Izz = iterator.GetPixel(4) + iterator.GetPixel(22) - t;
			
			if ((Ixx + Iyy + Izz) < 0) 
			{
				centerIndex = iterator.GetIndex(13);
				feat->SetPixel(centerIndex, Foreground);
			}
			++iterator;
		}
		break;
		
	case DarkZC:
		
		while(!iterator.IsAtEnd())
		{
			// Set the current pixel to white
			t = 2.0*iterator.GetCenterPixel();
			Ixx = iterator.GetPixel(12) + iterator.GetPixel(14) - t;
			Iyy = iterator.GetPixel(10) + iterator.GetPixel(16) - t;
			Izz = iterator.GetPixel(4) + iterator.GetPixel(22) - t;
			
			if ((Ixx + Iyy + Izz) > 0) 
				feat->SetPixel(iterator.GetIndex(13), Foreground);
			++iterator;
		}
		break;
		
  case BrightLE:
  
		while(!iterator.IsAtEnd())
		{
			// Set the current pixel to white
			t = 2.0*iterator.GetCenterPixel();
			Ixx = iterator.GetPixel(12) + iterator.GetPixel(14) - t;
			Iyy = iterator.GetPixel(10) + iterator.GetPixel(16) - t;
			Izz = iterator.GetPixel(4) + iterator.GetPixel(22) - t;
			
			Ixy = 0.25 * (iterator.GetPixel(17) + iterator.GetPixel(9) 
				- iterator.GetPixel(11) - iterator.GetPixel(15));
			Ixz = 0.25 * (iterator.GetPixel(23) + iterator.GetPixel(3) 
				- iterator.GetPixel(21) - iterator.GetPixel(5));
			Iyz = 0.25 * (iterator.GetPixel(25) + iterator.GetPixel(1) 
				- iterator.GetPixel(19) - iterator.GetPixel(7));
				
			Ix =  0.5 *(iterator.GetPixel(14) - iterator.GetPixel(12)); 
			Iy =  0.5 *(iterator.GetPixel(16) - iterator.GetPixel(10)); 
			Iz =  0.5 *(iterator.GetPixel(22) - iterator.GetPixel(4)); 
	  
			le = Ix*Ix*Ixx + Iy*Iy*Iyy + Iz*Iz*Izz + 2*(Ix*Iy*Ixy + Ix*Iz*Ixz + Iy*Iz*Iyz);
			if (le < 0) 
				feat->SetPixel(iterator.GetIndex(13), Foreground);
			++iterator;
		}
		break;
		
	case DarkLE:
		while(!iterator.IsAtEnd())
		{
			// Set the current pixel to white
			t = 2.0*iterator.GetCenterPixel();
			Ixx = iterator.GetPixel(12) + iterator.GetPixel(14) - t;
			Iyy = iterator.GetPixel(10) + iterator.GetPixel(16) - t;
			Izz = iterator.GetPixel(4) + iterator.GetPixel(22) - t;
			
			Ixy = 0.25 * (iterator.GetPixel(17) + iterator.GetPixel(9) 
				- iterator.GetPixel(11) - iterator.GetPixel(15));
			Ixz = 0.25 * (iterator.GetPixel(23) + iterator.GetPixel(3) 
				- iterator.GetPixel(21) - iterator.GetPixel(5));
			Iyz = 0.25 * (iterator.GetPixel(25) + iterator.GetPixel(1) 
				- iterator.GetPixel(19) - iterator.GetPixel(7));
				
			Ix =  0.5 *(iterator.GetPixel(14) - iterator.GetPixel(12)); 
			Iy =  0.5 *(iterator.GetPixel(16) - iterator.GetPixel(10)); 
			Iz =  0.5 *(iterator.GetPixel(22) - iterator.GetPixel(4)); 
	  
			le = Ix*Ix*Ixx + Iy*Iy*Iyy + Iz*Iz*Izz + 2*(Ix*Iy*Ixy + Ix*Iz*Ixz + Iy*Iz*Iyz);
			if (le > 0) 
				feat->SetPixel(iterator.GetIndex(13), Foreground);
			++iterator;
		}
		break;   
    
	case BrightEF:
	
		while(!iterator.IsAtEnd())
		{
			// Set the current pixel to white
			t = 2.0*iterator.GetCenterPixel();
			Ixx = iterator.GetPixel(12) + iterator.GetPixel(14) - t;
			Iyy = iterator.GetPixel(10) + iterator.GetPixel(16) - t;
			Izz = iterator.GetPixel(4) + iterator.GetPixel(22) - t;
			
			Ixy = 0.25 * (iterator.GetPixel(17) + iterator.GetPixel(9) 
				- iterator.GetPixel(11) - iterator.GetPixel(15));
			Ixz = 0.25 * (iterator.GetPixel(23) + iterator.GetPixel(3) 
				- iterator.GetPixel(21) - iterator.GetPixel(5));
			Iyz = 0.25 * (iterator.GetPixel(25) + iterator.GetPixel(1) 
				- iterator.GetPixel(19) - iterator.GetPixel(7));
				
			det = Ixx*(Iyy*Izz-Iyz*Iyz) - Ixy*(Ixy*Izz-Ixz*Iyz) + Ixz*(Ixy*Iyz-Iyy*Ixz);
			ef2 = Ixx*Iyy>Ixy*Ixy  && Ixx*Izz>Ixz*Ixz && Iyy*Izz>Iyz*Iyz;		 
			if ( ef2 && Ixx < 0  && det < 0 )  
				feat->SetPixel(iterator.GetIndex(13), Foreground);
			++iterator;
		}
		break;
		
	case DarkEF:
	
		while(!iterator.IsAtEnd())
		{
			// Set the current pixel to white
			t = 2.0*iterator.GetCenterPixel();
			Ixx = iterator.GetPixel(12) + iterator.GetPixel(14) - t;
			Iyy = iterator.GetPixel(10) + iterator.GetPixel(16) - t;
			Izz = iterator.GetPixel(4) + iterator.GetPixel(22) - t;
			
			Ixy = 0.25 * (iterator.GetPixel(17) + iterator.GetPixel(9) 
				- iterator.GetPixel(11) - iterator.GetPixel(15));
			Ixz = 0.25 * (iterator.GetPixel(23) + iterator.GetPixel(3) 
				- iterator.GetPixel(21) - iterator.GetPixel(5));
			Iyz = 0.25 * (iterator.GetPixel(25) + iterator.GetPixel(1) 
				- iterator.GetPixel(19) - iterator.GetPixel(7));
				
			det = Ixx*(Iyy*Izz-Iyz*Iyz) - Ixy*(Ixy*Izz-Ixz*Iyz) + Ixz*(Ixy*Iyz-Iyy*Ixz);
			ef2 = Ixx*Iyy>Ixy*Ixy  && Ixx*Izz>Ixz*Ixz && Iyy*Izz>Iyz*Iyz;		 
			if ( ef2 && Ixx > 0  && det > 0)  
				feat->SetPixel(iterator.GetIndex(13), Foreground);
			++iterator;
		}
		break;
    
	default: ; // Nothing to do
	}
  
	return feat;
}

template <class TInputImage, class TOutputImage>
typename TOutputImage::Pointer scalespace::ComputeFeatures2D(typename TInputImage::Pointer img, float scale, 
	FeatureType ft) 
{

	//if (img.isNull()) return ByteImage();

	if (ft == BrightZC || ft == DarkZC) 
	{
		FloatImageType2D::Pointer Ixx = DxxImage<TInputImage, FloatImageType2D>(img, scale);
		FloatImageType2D::Pointer Iyy = DyyImage<TInputImage, FloatImageType2D>(img, scale);
		typename TOutputImage::Pointer e = AllocateImage<TInputImage, TOutputImage>(img);
		
		FloatIteratorType2D Ixx_it(Ixx, Ixx->GetLargestPossibleRegion());
		FloatIteratorType2D Iyy_it(Iyy, Iyy->GetLargestPossibleRegion());
    
		if (ft==BrightZC)
		{
			for (Ixx_it.GoToBegin(), Iyy_it.GoToBegin(); !Ixx_it.IsAtEnd(); ++Ixx_it, ++Iyy_it)
				e->SetPixel(Ixx_it.GetIndex(), (Ixx_it.Get()+Iyy_it.Get() < 0 ? Foreground : Background));
		}
		else 
		{
			for (Ixx_it.GoToBegin(), Iyy_it.GoToBegin(); !Ixx_it.IsAtEnd(); ++Ixx_it, ++Iyy_it)
				e->SetPixel(Ixx_it.GetIndex(), (Ixx_it.Get()+Iyy_it.Get() > 0 ? Foreground : Background));
		}
		return e;
	}


	FloatImageType2D::Pointer pimg = scalespace::SmoothImage<TInputImage, FloatImageType2D>(img, scale);
	FloatImageType2D::SizeType ndim, ndim_1, radius;
	FloatImageType2D::IndexType nstart, nstart_1;
	ndim = pimg->GetLargestPossibleRegion().GetSize();
	for (unsigned int i=0; i<2; i++)
	{
		nstart[i] = 0;
		nstart_1[i] = 1;
		ndim_1[i] = ndim[i]-2;
		radius[i] = 1;
	}
	 
	float Ix, Iy, Ixx, Iyy, Ixy, t;
	typename TOutputImage::Pointer feat = AllocateImage<TInputImage, TOutputImage>(img);
	feat->FillBuffer(Background);
	
	FloatImageType2D::RegionType region;
	region.SetSize(ndim_1);
	region.SetIndex(nstart_1);
	itk::NeighborhoodIterator<FloatImageType2D> iterator(radius, pimg, region);

	switch (ft) 
	{
	case BrightZC:
	
		while(!iterator.IsAtEnd())
		{
			// Set the current pixel to white
			t = 2.0*iterator.GetCenterPixel();
			Ixx = iterator.GetPixel(5) + iterator.GetPixel(3) - t;
			Iyy = iterator.GetPixel(7) + iterator.GetPixel(1) - t;
			
			if ((Ixx + Iyy) < 0) 
				feat->SetPixel(iterator.GetIndex(4), Foreground);
			++iterator;
		}
		break;
		
	case DarkZC:
	
		while(!iterator.IsAtEnd())
		{
			// Set the current pixel to white
			t = 2.0*iterator.GetCenterPixel();
			Ixx = iterator.GetPixel(5) + iterator.GetPixel(3) - t;
			Iyy = iterator.GetPixel(7) + iterator.GetPixel(1) - t;
			
			if ((Ixx + Iyy) > 0) 
				feat->SetPixel(iterator.GetIndex(4), Foreground);
			++iterator;
		}
		break;
	
	case BrightLE:
	
		while(!iterator.IsAtEnd())
		{
			// Set the current pixel to white
			t = 2.0*iterator.GetCenterPixel();
			Ixx = iterator.GetPixel(5) + iterator.GetPixel(3) - t;
			Iyy = iterator.GetPixel(7) + iterator.GetPixel(1) - t;
			Ixy = 0.25 * (iterator.GetPixel(8) + iterator.GetPixel(0) 
				- iterator.GetPixel(2) - iterator.GetPixel(6));
	
			Ix =  0.5 *(iterator.GetPixel(5) - iterator.GetPixel(3)); 
			Iy =  0.5 *(iterator.GetPixel(7) - iterator.GetPixel(1)); 
			
			if (Ix*Ix*Ixx+Iy*Iy*Iyy+2*Ix*Iy*Ixy < 0) 
				feat->SetPixel(iterator.GetIndex(4), Foreground);
			++iterator;
		}
		break;
		
	case DarkLE:
  
		while(!iterator.IsAtEnd())
		{
			// Set the current pixel to white
			t = 2.0*iterator.GetCenterPixel();
			Ixx = iterator.GetPixel(5) + iterator.GetPixel(3) - t;
			Iyy = iterator.GetPixel(7) + iterator.GetPixel(1) - t;
			Ixy = 0.25 * (iterator.GetPixel(8) + iterator.GetPixel(0) 
				- iterator.GetPixel(2) - iterator.GetPixel(6));
	
			Ix =  0.5 *(iterator.GetPixel(5) - iterator.GetPixel(3)); 
			Iy =  0.5 *(iterator.GetPixel(7) - iterator.GetPixel(1)); 
			
			if (Ix*Ix*Ixx+Iy*Iy*Iyy+2*Ix*Iy*Ixy > 0)  
				feat->SetPixel(iterator.GetIndex(4), Foreground);
			++iterator;
		}
		break;
    
	case BrightEF:
  
		while(!iterator.IsAtEnd())
		{
			// Set the current pixel to white
			t = 2.0*iterator.GetCenterPixel();
			Ixx = iterator.GetPixel(5) + iterator.GetPixel(3) - t;
			Iyy = iterator.GetPixel(7) + iterator.GetPixel(1) - t;
			Ixy = 0.25 * (iterator.GetPixel(8) + iterator.GetPixel(0) 
				- iterator.GetPixel(2) - iterator.GetPixel(6));
	
			if ( Ixx*Iyy>Ixy*Ixy && Ixx < 0)  
				feat->SetPixel(iterator.GetIndex(4), Foreground);
			++iterator;
		}
		break;
		
	case DarkEF:
	
		while(!iterator.IsAtEnd())
		{
			// Set the current pixel to white
			t = 2.0*iterator.GetCenterPixel();
			Ixx = iterator.GetPixel(5) + iterator.GetPixel(3) - t;
			Iyy = iterator.GetPixel(7) + iterator.GetPixel(1) - t;
			Ixy = 0.25 * (iterator.GetPixel(8) + iterator.GetPixel(0) 
				- iterator.GetPixel(2) - iterator.GetPixel(6));
	
			if ( Ixx*Iyy>Ixy*Ixy && Ixx > 0)   
				feat->SetPixel(iterator.GetIndex(4), Foreground);
			++iterator;
		}
		break;
    
	default: ; // Nothing to do
    
	}

	return feat;
}	
