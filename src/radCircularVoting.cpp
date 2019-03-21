/*
 *  radCircularVoting.cpp
 *  
 *
 *  Created by Jianfei Liu on 9/19/11.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */

#include "radCircularVoting.h"
#include <cmath>
#include "rounding-algorithms.h"
#include "radScaleSpace.h"
#include "radBinaryImageProcessing.h"
#include "QuickView.h"

using namespace rounding;
using namespace scalespace;
using namespace bip;

int WPoint2D::nx = 0;
int WPoint2D::ny = 0;

int Cone2D::nx = 0;
int Cone2D::ny = 0;

int ConePlane2D::nx = 0;
int ConePlane2D::ny = 0;

// 2D cone
Cone2D::Cone2D()  : dx(1), dy(0) { }
void Cone2D::setDirection(float xx, float yy)
{ dx = xx; dy = yy; }

// Static constants
const float radCircularVoting::epsilon(0.001);
const float radCircularVoting::pi = 3.1415927;

void Cone2D::vote(float* p, const VPoint2D& vp)
{
	iterator from = begin();
	iterator to = end();     
	for(iterator it = from; it != to; it++) 
		for(value_type::iterator it1=it->begin(); it1!=it->end(); it1++) 
		{    
			p[it1->d] += vp.mag * it1->w;
		}      
}

#if 0

void Cone2D::vote(float* p, float* pmask, const VPoint2D& vp)
{
	float maxvote, m;  
	iterator it, from, to;
	value_type::iterator it1, it_end;
  
	maxvote = 0;  
	from = begin();
	to = end();
    
	for(it = from; it != to; it++) 
	{	
		it1=it->begin();
		it->center_weight = pmask[it1->d];;
		maxvote = max(maxvote, it->center_weight);
	}
  
	if (maxvote<CircularVoting::epsilon)
		return;
    
	for(it = from; it != to; it++) 
	{	
		m = vp.mag * it->center_weight / maxvote;
		it_end = it->end();    
		for(it1=it->begin(); it1!=it_end; it1++) 
		{	 
			p[it1->d] += m * it1->w;	  
		}
	}
}

#endif

#if 1 // new 

void Cone2D::vote(float* p, float* pmask, const VPoint2D& vp)
{
	float sumvote, m;  
	iterator it, from, to;
	value_type::iterator it1, it_end;
  
	sumvote = 0;  
	from = begin();
	to = end();
  
	for(it = from; it != to; it++) 
	{	
		it_end = it->end();    
		for(it1=it->begin(); it1!=it_end; it1++) 
		{	 
			sumvote += pmask[it1->d];  
		}
	}
  
	if (sumvote<radCircularVoting::epsilon)
		return;
  
	for(it = from; it != to; it++) 
	{	
		m = vp.mag  / sumvote;
		it_end = it->end();    
		for(it1=it->begin(); it1!=it_end; it1++) 
		{	 
			p[it1->d] += m * pmask[it1->d];	  
		}
	}
}

#endif

radCircularVoting::radCircularVoting()
{
	m_Mingrad = 1;
  
	m_hmin = 1;
	m_hmax = 20;
	m_radius = 10;
  
	m_RegType = BrightReg;
	m_nTheta = 256;
	m_DeltaTheta = 2*pi/m_nTheta;
  
	m_zconly = false;
	m_savehistory = false; 
	m_absthresh = true;
}

radCircularVoting::~radCircularVoting()
{ 
}

void radCircularVoting::SetPrefix(const string& p)
{
	m_prefix = p;
}


void radCircularVoting::SetParams(RegionType reg_type, int hmin, int hmax, int r, float min_grad,
								  float threshold, float scale, WeightingMethod weighting_method,
								  float weighting_param, bool zc_only, bool abs_thresh)
{
	m_absthresh = abs_thresh;
	m_RegType = reg_type;
	m_hmin = max<int>(hmin, 1); 
	m_hmax = max<int>(hmax, m_hmin);
	//_radius = max<int>(r, 1);
	m_radius = max<int>(r, 0);
	m_Mingrad = min_grad;
  
	if( !m_absthresh )
		m_Threshold = max<float>(0.05, min<float>(1, threshold));
	else
		m_Threshold = threshold;

	//cout<<"Set ParamsThreshold "<<threshold<<std::endl;
	m_Scale = max<float>(0, scale);
  
	m_WeightingMethod = weighting_method;
	m_WeightingParam = weighting_param;
  
	m_zconly = zc_only;  
}

void radCircularVoting::SaveHistory(bool yn)
{
	m_savehistory = yn;
}

int radCircularVoting::ComputeAngleIndex(float dx, float dy) const 
{
	float a = acos(dx);
	
	if (dy<0) 
	{
		a = 2*pi-a;
	}
  
	int indx_theta = (int)roundhalfeven<float>(a/m_DeltaTheta);
	return max(0, min(m_nTheta-1, indx_theta));
}

int radCircularVoting::NextConeRadius(int h, int r)
{
  //  return  r-1;
  if (h<=0 || r<0) 
  {
	  cerr << "Error in CircularVoting::nextConeRadius: null cone\n";
		return 0;
  }
  return (int)((double) h * tan(atan((double)r/(double)h) * 0.7 ));
}

void radCircularVoting::GetMaxDirection(VPoint2D& vp)
{
	Cone2D& cone = m_Cones[vp.angIndex];
	Cone2D::iterator from = cone.begin();
	Cone2D::iterator to = cone.end();
	ConePlane2D::iterator max_point;
  
	float maxvote = 0;
    
	float* p = SumImg->GetBufferPointer() + vp.pos;
	for(Cone2D::iterator it = from; it != to; it++) 
	{
		for(ConePlane2D::iterator it1=it->begin(); it1!=it->end(); it1++)
			if ( p[it1->d]>maxvote) 
			{
				maxvote = max(maxvote, p[it1->d]);
				max_point = it1;
			}    
    }

	if (maxvote<epsilon)
		return;
  
	float dx = max_point->x;
	float dy = max_point->y;
  
	float r = sqrt(dx*dx+dy*dy);
	if (r>epsilon) 
	{
		vp.xc = vp.x + max_point->x;
		vp.yc = vp.y + max_point->y;
		vp.angIndex = ComputeAngleIndex(dx/r, dy/r);
	}
}

void radCircularVoting::ComputeCones(int hmin, int hmax, int radius)
{
	m_Cones.clear();
	m_Cones = vector<Cone2D>(m_nTheta);
	
	Cone2D cone;
	cone.reserve(2*hmax);
	cone.setDirection(1, 0);
    
	for(int x=hmin; x<=hmax; x++) 
	{
		cone.push_back(ConePlane2D(1, WPoint2D(x, 0, 1)));
	}
  
	for(int x=hmin; x<=hmax; x++) 
	{
		int rmax = roundhalfeven<double>(x * radius / hmax);
		for(int y=-rmax; y<=rmax; y++) 
		{
			if (y!=0) 
			{
				float R = sqrt((double)(x*x + y*y));
				// float a = fabs((float)x)/R;
      
				WPoint2D wp;
				wp.x = x; 
				wp.y = y; 
				wp.w = 1;// a;
				int n = roundhalfeven<double>(R) - hmin;
				if (n>=0 && n<cone.size())
					cone[n].push_back(wp);
			}
		}
	}
  
	if ( m_RegType == AllReg) 
	{
		Cone2D cone1;
		cone1.setDirection(1, 0);
    
		for(int x=hmin; x<=hmax; x++) 
		{
			cone1.push_back(ConePlane2D(1, WPoint2D(-x, 0, 1)));
		}
    
		for(int x=hmin; x<=hmax; x++) 
		{
			int rmax = roundhalfeven<double>(x * radius / hmax);
			for(int y=-rmax; y<=rmax; y++)   
				if (y!=0) 
				{
					float R = sqrt((double)(x*x + y*y));
					// float a = fabs((float)x)/R;

					WPoint2D wp;
					wp.x = -x; 
					wp.y = y; 
					wp.w = 1;// a;
					int n = roundhalfeven<double>(R) - hmin;
					if (n>=0 && n<cone1.size())
						cone1[n].push_back(wp);
				}
		}
    
		for(int i=0; i<cone1.size(); i++)
			cone.push_back(cone1[i]);
	}
  
  
	cone.init();
    
	for(int i=0; i<m_nTheta; i++) 
	{
		float theta = i*m_DeltaTheta;

		Cone2D rcone = cone; // rotated cone
	
		const float cos_theta = cos(theta);
		const float sin_theta = sin(theta);
    
		for(Cone2D::iterator it1=rcone.begin(); it1!=rcone.end(); it1++)
			for(ConePlane2D::iterator it2=it1->begin(); it2!=it1->end(); it2++) 
			{
				int x = it2->x;
				int y = it2->y;
	
				it2->x = roundhalfeven<double>(cos_theta * x - sin_theta * y );
				it2->y = roundhalfeven<double>(sin_theta * x + cos_theta * y );
	
			}
    
		rcone.setDirection(cos_theta,  sin_theta);
		rcone.init();
		m_Cones[i] = rcone;        
	}
}

void radCircularVoting::Vote()
{
	FloatImageType2D::IndexType img_start, img_end;
	FloatImageType2D::SizeType img_size;
	img_start[0] = img_start[1] = 0;
	img_size[0] = nx;
	img_size[1] = ny;

	SumImg = NULL;
	SumImg = AllocateImage<FloatImageType2D>(img_start, img_size);
	SumImg->FillBuffer(0);
	MaskImg = NULL;
	MaskImg = AllocateImage<FloatImageType2D>(img_start, img_size);
	MaskImg->FillBuffer(0);
  
	float* pmask = MaskImg->GetBufferPointer();
	float* psum = SumImg->GetBufferPointer();
  
	vector<VPoint2D>::iterator voting_points_begin = m_VotingPoints.begin();
	vector<VPoint2D>::iterator voting_points_end = m_VotingPoints.end();
  
	for(vector<VPoint2D>::iterator it=voting_points_begin; it!=voting_points_end; it++) 
	{	
		it->pos = it->x+it->y*nx;
	}
  
  
	ComputeCones(m_hmin, m_hmax, m_radius);
  
	for(vector<VPoint2D>::iterator it=voting_points_begin; it!=voting_points_end; it++) 
	{
		m_Cones[it->angIndex].vote(psum+it->pos, *it);

		/*QuickView viewer;
		viewer.AddImage(SumImg.GetPointer());
		viewer.AddImage(SumImg.GetPointer());
		viewer.Visualize();*/
	}

	FloatImageType2D::Pointer roi;
	// int loop = 0;
	int r = m_radius;
  
	while ( r>= 0 ) 
	{
		//    cout << "Radius = " << r<< std::endl;
		ComputeCones(m_hmin, m_hmax, r);
		// m_RegType = BrightReg;
    
		if (m_savehistory) 
		{
			img_start[0] = img_start[1] = bw;
			img_end[0] = nx-bw;
			img_end[1] = ny-bw;
			roi = ExtractImage<FloatImageType2D>(SumImg, img_start, img_end); 
			std::stringstream out;
			out << r;
			string vote_image_number = out.str();
			out.str().clear();
			
			if (r < 10) 
				vote_image_number = "0"+ vote_image_number+".hdr";
			else
				vote_image_number = vote_image_number+".hdr";
			/* comment for saving 16bit ics file instead
			writePGM(_prefix+"vote_"+vote_image_number+".pgm",
			convertToByteImage(*roi, true));
			*/
			WriteImage<FloatImageType2D>(string(m_prefix+"vote_"+vote_image_number).c_str(), roi);
			roi = NULL;
		}
    
		for(vector<VPoint2D>::iterator it=voting_points_begin; it!=voting_points_end; it++) 
		{	
			GetMaxDirection(*it);    
		}    
    
		memcpy(pmask, psum, npix*sizeof(float));
		memset(psum, 0,  npix*sizeof(float));
    

		for(vector<VPoint2D>::iterator it=voting_points_begin; it!=voting_points_end; it++) 
		{
			m_Cones[it->angIndex].vote(psum+it->pos, pmask+it->pos, *it);
		}
    
		if (r<=0) break;
    
		r = NextConeRadius(m_hmax, r);
	}
  
	ComputeCones(m_hmin, m_hmax, 0);
}

void radCircularVoting::Compute(const FloatImageType2D::Pointer img)
{
	nx = img->GetLargestPossibleRegion().GetSize()[0];
	ny = img->GetLargestPossibleRegion().GetSize()[1];
	npix = nx*ny;

	m_VotingPoints = vector<VPoint2D>();
	m_VotingPoints.reserve(npix/2);
    
	const float reg_sign = m_RegType==DarkReg ? -1 : 1;
	FloatImageType2D::Pointer Dx = DxImage<FloatImageType2D, FloatImageType2D>(img, m_Scale);
	FloatImageType2D::Pointer Dy = DyImage<FloatImageType2D, FloatImageType2D>(img, m_Scale);
	FloatImageType2D::Pointer G = AllocateImage<FloatImageType2D, FloatImageType2D>(img);

	FloatIteratorType2D G_it(G, G->GetLargestPossibleRegion());
	FloatIteratorType2D Dx_it(Dx, Dx->GetLargestPossibleRegion());
	FloatIteratorType2D Dy_it(Dy, Dy->GetLargestPossibleRegion());

	for (G_it.GoToBegin(), Dx_it.GoToBegin(), Dy_it.GoToBegin(); !G_it.IsAtEnd(); ++G_it, ++Dx_it, ++Dy_it)
	{
		Dx_it.Set(Dx_it.Get()*reg_sign);
		Dy_it.Set(Dy_it.Get()*reg_sign);
		float g = sqrt(Dx_it.Get()*Dx_it.Get()+Dy_it.Get()*Dy_it.Get());
		G_it.Set(g);

		if (g > epsilon)
		{
			Dx_it.Set(Dx_it.Get()/g);
			Dy_it.Set(Dy_it.Get()/g);
		}
	}

	float mingrad, maxgrad;
	ComputeIntensityRange<FloatImageType2D>(G, mingrad, maxgrad);

	/*QuickView viewer;
	viewer.AddImage(img.GetPointer());
	viewer.AddImage(G.GetPointer());
	viewer.Visualize();*/
	/*WriteImage<FloatImageType2D>("G.hdr", G);
	WriteImage<FloatImageType2D>("img.hdr", img);*/

	if (maxgrad > epsilon)
	{
		for (G_it.GoToBegin(); !G_it.IsAtEnd(); ++G_it)
		{
			if (G_it.Get() < m_Mingrad)
				G_it.Set(0);
			else if (!m_absthresh)
				G_it.Set(G_it.Get()/maxgrad);
		}
	}

	/*viewer.AddImage(G.GetPointer());
	viewer.Visualize();*/
 
	if (m_zconly) 
	{
		//change here to 3D feature detection
		BinaryImageType2D::Pointer feature_img = ComputeFeatures2D<FloatImageType2D, BinaryImageType2D>(img, m_Scale, BrightZC);
		BinaryImageType2D::Pointer ZC = ComputeEdge<BinaryImageType2D, BinaryImageType2D>(feature_img); 
		BinaryIteratorType2D ZC_it(ZC, ZC->GetLargestPossibleRegion());
		for(ZC_it.GoToBegin(), G_it.GoToBegin(); !ZC_it.IsAtEnd(); ++ZC_it, ++G_it) 
		{
			if (!ZC_it.Get())	G_it.Set(0);
		}
	}
  
	// Weighting   
	if (m_WeightingMethod == PowMethod && fabs(m_WeightingParam-1) > epsilon) 
	{
		for (G_it.GoToBegin(); !G_it.IsAtEnd(); ++G_it) 
		{
			if (G_it.Get() > epsilon)
				G_it.Set(pow(G_it.Get(), m_WeightingParam));
		}
	}
	else if (m_WeightingMethod == ExpMethod) 
	{
		for (G_it.GoToBegin(); !G_it.IsAtEnd(); ++G_it) 
		{
			if (G_it.Get() > epsilon)
				G_it.Set(exp( G_it.Get() *  m_WeightingParam ));
		}
	}
	/*viewer.AddImage(G.GetPointer());
	viewer.Visualize();*/
  
	bw = sqrt((double)(m_radius*m_radius + m_hmax*m_hmax)) + 3;
	const int bw2 = 2*bw;
  
	int indx;
	VPoint2D vp;
	for (G_it.GoToBegin(), Dx_it.GoToBegin(), Dy_it.GoToBegin(); !G_it.IsAtEnd(); 
		++G_it, ++Dx_it, ++Dy_it)
	{
		if (G_it.Get() > epsilon)
		{
			if ((indx=ComputeAngleIndex(Dx_it.Get(), Dy_it.Get())) >= 0 ) 
			{
				vp.x = G_it.GetIndex()[0] + bw;
				vp.y = G_it.GetIndex()[1] + bw;
				vp.mag = G_it.Get(); 
				vp.angIndex = indx;
				m_VotingPoints.push_back(vp);
			}	
		}
	}
  
	//cout << "#Voting points = " << _voting_points.size() << std::endl;
	nx += bw2;
	ny += bw2;
	npix = nx*ny;
    
	WPoint2D::setImageSize(nx, ny);
	ConePlane2D::setImageSize(nx, ny);
	Cone2D::setImageSize(nx, ny);
  
	Vote();
  
	// Switch back to the original size
	nx -= bw2;
	ny -= bw2;
	npix = nx*ny;
	VoteImg = AllocateImage<FloatImageType2D, FloatImageType2D>(img);
	VoteImg->FillBuffer(0);
  
	float val, maxval=0;
	FloatIteratorType2D Vote_it(VoteImg, VoteImg->GetLargestPossibleRegion());
	FloatImageType2D::IndexType pixelIndex;
	for (Vote_it.GoToBegin(); !Vote_it.IsAtEnd(); ++Vote_it)
	{
		pixelIndex[0] = Vote_it.GetIndex()[0]+bw;
		pixelIndex[1] = Vote_it.GetIndex()[1]+bw;
		val = SumImg->GetPixel(pixelIndex);
		Vote_it.Set(val);
		maxval = max<float>(maxval, val);
	}
	//  return;

	int nc;
	//cout<<"Voting Threshold "<<_threshold<<" max Value "<<maxval<<" thresh * maxval:" <<_threshold * maxval<<std::endl;
	// ByteImage bIm = threshold(_vote,_threshold*maxval) ;
	// ByteImage bIm = threshold(_vote,_threshold) ;
	BinaryImageType2D::Pointer bIm = AllocateImage<FloatImageType2D, BinaryImageType2D>(VoteImg);
	BinaryIteratorType2D bIm_it(bIm, bIm->GetLargestPossibleRegion());
	for (bIm_it.GoToBegin(), Vote_it.GoToBegin(); !bIm_it.IsAtEnd(); ++bIm_it, ++Vote_it)
	{
		if (m_absthresh)
			bIm_it.Set(Vote_it.Get()> m_Threshold ? 255:0);
		else
			bIm_it.Set(Vote_it.Get()> m_Threshold*maxval ? 255:0);
	}
   
	ShortImageType2D::Pointer label = ComputeBinaryRegions<BinaryImageType2D, ShortImageType2D, double>(bIm, nc);
	vector<RegionFeature> rf = ComputeRegionFeatures<ShortImageType2D, FloatImageType2D>(label, nc, VoteImg);
	VoteImg->FillBuffer(0);
	m_CenterWeights.clear();
	m_Centers.clear();

	for(vector<RegionFeature>::iterator it=rf.begin(); it!=rf.end(); it++) 
	{
		pixelIndex[0] = it->xc;
		pixelIndex[1] = it->yc;
		if (VoteImg->GetLargestPossibleRegion().IsInside(pixelIndex)) 
		{
			itk::Point<short, 2> pt;
			VoteImg->SetPixel(pixelIndex, it->mean);
			pt[0] = it->xc;
			pt[1] = it->yc;

			if (it->mean != 0)
			{
				m_Centers.push_back(pt);
				m_CenterWeights.push_back(it->mean);
			}
		}   
	}
  
//	WriteImage<FloatImageType2D>("VoteImg.hdr", VoteImg);
	for(vector<VPoint2D>::iterator it=m_VotingPoints.begin(); it!=m_VotingPoints.end(); it++) 
	{
		it->x -= bw;
		it->y -= bw;
		it->xc -= bw;
		it->yc -= bw;
	}
}
