/*
 *  radFileIO.h
 *  
 *
 *  Created by Jianfei Liu on 9/19/11.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef radFileIO_H
#define radFileIO_H

#include "radimgfunc.h"
#include <itkSpatialObjectToImageFilter.h>
#include <itkPasteImageFilter.h>
#include <itkJoinSeriesImageFilter.h>
#include <QDir>
#include <QDirIterator>
#include <QFileInfo>
#include <QProgressBar>
#include <QApplication>
#include <QImageReader>
#include <QImage>

// Math
#include <math.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>

class radFileIO
{
private:
	void ExtractNumsFromString(string &, vector<double> &, int);
public:
	
	radFileIO();
	~radFileIO();
    
	std::pair<FloatImageType2D::Pointer, FloatImageType2D::Pointer> ReadSplitImage(string);
	
	void WriteConeDetections(const char *, DoublePointArray2D &);
	void ReadConeDetections(const char *, DoublePointArray2D &);
};

#endif // segFileIO_H

