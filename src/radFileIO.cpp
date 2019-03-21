/*
 *  radFileIO.cpp
 *  
 *
 *  Created by Jianfei Liu on 9/19/11.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */

#include "radFileIO.h"
#include <vtkPointData.h>
#include <QDebug>
#include <iostream>

#include "QuickView.h"
#include <QMessageBox>
#include <itkRGBToLuminanceImageFilter.h>

radFileIO::radFileIO()
{
}

radFileIO::~radFileIO()
{ 
}

std::pair<FloatImageType2D::Pointer, FloatImageType2D::Pointer> radFileIO::ReadSplitImage(string fileName)
{
	std::pair<FloatImageType2D::Pointer, FloatImageType2D::Pointer> res;
	res.first = NULL;
	res.second = NULL;
	
	QImageReader reader(fileName.c_str());
	QImage img = reader.read();
	if( !img.isNull() )	 //Load succceded ?
	{		
		FloatImageType2D::IndexType img_start;
		FloatImageType2D::SizeType img_size;
		img_start[0] = img_start[1] = 0;
		img_size[0] = img.size().width();
		img_size[1] = img.size().height();
		res.first = AllocateImage<FloatImageType2D>(img_start, img_size);
		res.second = AllocateImage<FloatImageType2D>(img_start, img_size);

		FloatIteratorType2D first_it(res.first, res.first->GetLargestPossibleRegion());
		FloatIteratorType2D second_it(res.second, res.second->GetLargestPossibleRegion());
		for (first_it.GoToBegin(), second_it.GoToBegin(); !first_it.IsAtEnd(); ++first_it, ++second_it)
		{
			QRgb value = img.pixel(first_it.GetIndex()[0], first_it.GetIndex()[1]);
			QColor color(value);

			float img_val;
			img_val = 0.3*color.red()+0.6*color.green()+0.11*color.blue();
			if (img_val < 0) img_val = 0;
			else if (img_val > 255) img_val = 255;

			first_it.Set((unsigned char)img_val);
			second_it.Set(255-(unsigned char)img_val);

		}
	}

	return res;
}

void radFileIO::ExtractNumsFromString(string & input_str, vector<double> & pts, int file_index)
{
	std::stringstream ss(input_str);
	std::string temp;
	int val[2];

	std::stringstream stream;
	if (input_str.find(",") == string::npos)
	{
		while(getline(ss, temp, ' ')) // delimiter as space
		{
			stream.str(temp);
			stream >> val[0];
		}

		stream >> val[1];
		pts.push_back(val[1]);
		pts.push_back(val[0]);
	}
	else
	{
		size_t pos = input_str.find(",");
		size_t pos1 = 0;
		string tmp_str;
		double val;

		while (pos != string::npos)
		{
			tmp_str.assign(input_str.begin()+pos1, input_str.begin()+pos);
			val = atof(tmp_str.c_str());
			pts.push_back(val);

			pos1 = pos+1;
			pos = input_str.find(",", pos1+1);
		}
		tmp_str.assign(input_str.begin()+pos1, input_str.end());
		val = atof(tmp_str.c_str());
		pts.push_back(val);
	}
}

void radFileIO::WriteConeDetections(const char *filename, DoublePointArray2D & cone_pts)
{
	ofstream myfile(filename);

	if (myfile.is_open())
	{
		//output lesion pts
		for (unsigned int i=0; i<cone_pts.size(); i++)
			myfile << cone_pts[i][0] << "," << cone_pts[i][1] << std::endl;

		myfile.close();
	}
	else 
		cout << "Unable to save file";
}

void radFileIO::ReadConeDetections(const char *filename, DoublePointArray2D & cone_pts)
{
  ifstream myfile(filename);
  if (myfile.is_open())
  {
    string buff;
    DoublePointType2D point;
    cone_pts.clear();
    while (myfile) { 
      if (!std::getline(myfile, buff, ',')) break;
      point[0] = std::stod(buff);
      if (!std::getline(myfile, buff)) break;
      point[1] = std::stod(buff);
      cone_pts.push_back(point);
    }
    myfile.close();
  }
}
