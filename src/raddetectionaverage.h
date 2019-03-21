/*
 *  radDetectionAverage.h
 *  
 *
 *  Created by Jianfei Liu on 9/19/11.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef radDetectionAverage_H
#define radDetectionAverage_H

#include "radimgfunc.h"

// Math
#include <math.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>

class radDetectionAverage
{
private:
	static int SearchForNearestDetection(DoublePointType2D &, DoublePointArray2D &, double &);
	static double ComputeLinkAngle(DoublePointType2D &, DoublePointType2D &);
	static void AdjustFeatureLocations();
	
public:
	
	radDetectionAverage() {}
	~radDetectionAverage() {}

	static void AdjustDetections(SplitImageInformation &, bool dark_flag = false);
};

#endif // radSuperPixel_H

