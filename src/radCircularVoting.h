/*
 *  radCircularVoting.h
 *  
 *
 *  Created by Jianfei Liu on 9/19/11.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef radCircularVoting_H
#define radCircularVoting_H

#include "radimgfunc.h"

// Math
#include <math.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#include "itkPoint.h"

/** Voting point */
class VPoint2D 
{
public:
	short x, y;
    short xc, yc; //< center point 
    short angIndex; //< angle index
    int pos;
    float mag; //< magnitude
};

/** Weighted point */
class WPoint2D 
{
public:
	static int nx, ny;

    WPoint2D(int xx=0, int yy=0, float ww=0)
	: x(xx), y(yy), d(0), w(ww) {}
    
      
	static void setImageSize(int xsize, int ysize)
    {  
		nx = xsize;
		ny = ysize;
    }
      
    void setPos() 
    {  
		d = x + y*nx; 
	}
    

    int x, y; //< coordinates
    int d; //< offset position
    float w; //< weight
};

/** An intersection of cone */
//  typedef vector<WPoint2D> ConePlane2D;  
class ConePlane2D : public vector<WPoint2D> 
{
public:
	static int nx, ny;
      
    ConePlane2D(int n, const value_type &v) : vector<WPoint2D>(n, v)
    { 
	}
      
    ConePlane2D() : vector<WPoint2D>()
    { 
	}
      
      
	static void setImageSize(int xsize, int ysize)
    {
		nx = xsize;
		ny = ysize;
    }
      
    
	void setPos()
    {
		for(iterator it = begin(); it!=end(); it++)
			it->setPos();     
    }
      
      
    float center_weight;
};

/** \brief A 2D cone */
class Cone2D : public vector<ConePlane2D> 
{
public:
	static int nx, ny; //< image size

	Cone2D();
    void setDirection(float xx, float yy);
    static void setImageSize(int xsize, int ysize)
    {  
		nx = xsize;
		ny = ysize;
    }
      
      
	void init()
    {
		for(iterator it = begin(); it!=end(); it++) 
		{
			it->setPos();
		}
    }
      
    void vote(float* p, const VPoint2D& vp);
    void vote(float* p, float* pmask, const VPoint2D& vp);
      
    float dx, dy; //< direction
};

class radCircularVoting
{
private:
	
	
public:
	
	/** Region type */
    enum RegionType { BrightReg, DarkReg, AllReg };
      
    /** weighting method */
    enum WeightingMethod { PowMethod, ExpMethod };


    static const float epsilon;
    static const float pi;

	/**
	Set parameters for circular voting 
	\param reg_type region type
	\param hmin lower bound of voting range, measured in pixel
	\param hmax upper bound of voting range, measured in pixel, usually set as 3/4 of the object diameter
	\param r aperture for voting, measured in pixel
	\param min_grad gradient threshold for voting points, used for acceleration by eliminating voting points whose gradients are less than the threshold
	\param threshold saliency threshold for picking up object centers after voting; In case abs_thresh == TRUE, this value is directly used; when abs_thresh == FALSE, this value should be within (0,1), and the real threshold will be computed implicitly
	\param scale scale for gradient computation with DoG
	\param weighting_method weighting method for gradient features
	\param weighting_param weighting parameter
	\param zc_only TRUE - voting points should also be the zero-crossing points of the image; FALSE - otherwise
	\param abs_thresh TRUE - use input(threshold) directly; FALSE - use input(threshold) * max_voted_value as the threshold
       */
    void SetParams(RegionType reg_type, int hmin, int hmax, int r,   
		float min_grad, float threshold, float scale=1.5,
		WeightingMethod weighting_method=PowMethod,
		float weighting_param=1, bool zc_only=false,
		bool abs_thresh = true);

	//temporally using float type image
	void Compute(const FloatImageType2D::Pointer); 

	radCircularVoting();
	~radCircularVoting();

	/**
	\return image with voted centers and accumulated votes
    */
	inline FloatImageType2D::Pointer GetVotingImage() {return VoteImg;}
      
    /**
	\return voted object centers
    */
	const ShortPointArray2D& GetCenters() const {return m_Centers;}
      
    /**
	\return weights of voted object centers, these weights equal to accumulated votes
    */
	const vector<float>& GetCenterWeights() const {return m_CenterWeights;}
      
            
    /**
	Set the prefix of the path to store the voting landscope
    */
    void SetPrefix(const string& );
      
    /**
	Set whether to save voting landscope
    */
    void SaveHistory(bool yn=true);

private:


	/**
	Compute voting aperture in the next iteration
	\param h voting range
	\param r current voting aperture
	\return voting aperture in the next iteration
    */
    int NextConeRadius(int h, int r);
      
    /** voting */
    void Vote();

    /** 
	Compute cone for voting (2D cone is a triangle)
	\param hmin lower bound of voting range, measured in pixel
	\param hmax upper bound of voting range, measured in pixel, usually set as 3/4 of the object diameter
    */
    void ComputeCones(int hmin, int hmax, int radius);
      
    /**
	Update voting direction for a given point: vp; the new direction is from vp to the point with maximum voting response within vp's voting range.
    */
    void GetMaxDirection(VPoint2D& vp);
      
    /**
	Update voting direction for a given point: vp; the new direction is from vp to the point (x,y) within vp's voting range, in which (x,y) is the average coordinate value weighted by its votes at current iteration.
    */
    //void GetMeanDirection(VPoint2D& vp);
      
    /**
	\return The angle index in the look-up table
    */
    int ComputeAngleIndex(float dx, float dy) const;

    ShortPointArray2D m_Centers; //< detected centers of objects
    vector<float> m_CenterWeights; //< accumulated votes for each center
    
	FloatImageType2D::Pointer SumImg, MaskImg, VoteImg; //<sum of voting

    vector<VPoint2D> m_VotingPoints; //< voting candidate points
    
    vector<Cone2D> m_Cones; //< pre-computed cone structures for voting
    float m_DeltaTheta; //< step size of theta
    int m_nTheta; //< number of bins for 2*PI
    int nx, ny, npix; //< size of image
    int bw;//< padding size for border pixels during voting 


    float m_Mingrad; //< gradient threshold for voting candidate points
    int m_hmin, m_hmax, m_radius; //< lower and upper bound for voting range
    RegionType m_RegType; //< region type
    float m_Threshold; //< threshold for accumulated votes
	float m_Scale; // scale for detecting gradient information via DoG

    WeightingMethod m_WeightingMethod; // weighting method for gradient feature
    float m_WeightingParam; // weighting parameter

    bool m_zconly; //< whether only zero-crossing points can be voting points
      
    string m_prefix; //< prefix for path to record voting landscope in each iteration

    bool m_savehistory; //< whether save voting landsope in each iteration
      
    bool m_absthresh; //< use absolute value as threshold

	// double m_ClusteringRadius;
};

#endif // segFileIO_H

