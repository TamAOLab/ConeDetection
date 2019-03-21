/*
 *  radImageView.h
 *  
 *
 *  Created by Jianfei Liu on 9/19/11.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef radImageView_H
#define radImageView_H

// Math
#include <math.h>
#include <fstream>
#include <iostream>
#include <vector>
#include <deque>
#include "radimgfunc.h"

#include <vtkRenderer.h>
#include <vtkProperty.h>
#include <vtkShortArray.h>
#include <vtkFloatArray.h>
#include <vtkLookupTable.h>
#include <vtkImageActor.h>
#include <vtkCamera.h>
#include <vtkInteractorStyleTrackballCamera.h>
#include <vtkInteractorStyleImage.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderWindow.h>
#include <vtkObjectFactory.h>
#include <QVTKInteractor.h>
#include <vtkWindowToImageFilter.h>
#include <vtkPolyData.h>
#include <vtkPoints.h>
#include <vtkGlyph3D.h>
#include <vtkCellArray.h>
#include <vtkActor.h>
#include <vtkGlyphSource2D.h>
#include <vtkPolyDataMapper.h>
#include <vtkDataSetMapper.h>
#include <vtkImageMapToColors.h>
#include <vtkRegularPolygonSource.h>
#include <vtkPointPicker.h>
#include <vtkObjectFactory.h>
#include <vtkRendererCollection.h>
#include <vtkSphereSource.h>
#include <vtkTubeFilter.h>
#include <vtkContourFilter.h>
#include <vtkTransform.h>

//#include "radWatershed.h"

// Define interaction style
class radMouseInteractorStylePP : public vtkInteractorStyleImage
{
private:
	DoublePointArray2D contour_pts;

  public:
    static radMouseInteractorStylePP* New();
    vtkTypeMacro(radMouseInteractorStylePP, vtkInteractorStyleImage);
 
    void OnLeftButtonDown() override;
};


//ground truth color
const double GroundTruthColor[3] = {221.0/255.0, 28.0/255.0, 119.0/255.0};
const double ResultColor[3] = {0.0/255.0, 255.0/255.0, 0.0/255.0};
const double FalsePositiveColor[3] = {253.0/255.0, 174.0/255.0, 97.0/255.0};
const double FalseNegativeColor[3] = {255.0/255.0, 255.0/255.0, 51.0/255.0};

struct UndoEntry {
UndoEntry(bool del, double x, double y) :
	m_del(del), m_x(x), m_y(y) {}
	bool m_del;
	double m_x;
	double m_y;
};

class radImageView
{
private:
	
	bool ImageAddFlag;
	
	vtkSmartPointer<vtkImageData> ImageData;
	vtkSmartPointer<vtkImageActor> ImageActor;
	void DrawInputImage();

	vtkSmartPointer<vtkPoints> ConePoints;
	vtkSmartPointer<vtkPolyData> ConePolydata;
	vtkSmartPointer<vtkGlyphSource2D> ConeGlyphSource;
	vtkSmartPointer<vtkGlyph3D> ConeGlyph;
	vtkSmartPointer<vtkDataSetMapper> ConeMapper;
	vtkSmartPointer<vtkActor> ConeActor;
	void DrawConeDetections();
	
	vtkSmartPointer<vtkRenderer> ImageRender;
	vtkSmartPointer<radMouseInteractorStylePP> ImageStyle;
	vtkSmartPointer<vtkRenderWindowInteractor> WinInteractor;
	vtkSmartPointer<vtkRenderWindow> RenderWin;

	void ChangeCameraOrientation(vtkSmartPointer<vtkRenderer>);

public:
	
	radImageView();
	~radImageView();
    
	void ResetView(bool camera_flag = true); //used for initialization

	vtkSmartPointer<vtkRenderWindow> GetRenderWin() {return RenderWin;}

	void SetSplitImage(FloatImageType2D::Pointer);

	void SetConePoints(DoublePointArray2D &, vector<float> &, vector<float> &, 
		DoublePointArray2D &, vector<float> &, vector<float> &, 
		vector< pair<unsigned int, unsigned int> > &, DoublePointArray2D &);
	void GetConePoints(DoublePointArray2D &);
	void InitializeView();
	void SetConeGlyphVisibility(bool);
	void SetGlyphScale(double);
	
	void RemoveDetectedFeatures(double, double, double);
	void AddDetectedFeatures(double, double, double);
	void AddUndoEntry(bool, double, double);
	void DoUndo();
	deque< UndoEntry > undoStack;
};

#endif // radImageView_H

