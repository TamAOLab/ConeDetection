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
	bool LeftMousedPressed;
	bool ShiftDown, MouseScroll;
	bool MouseIn;

	ColorInfo ci;
	int img_dims[3];
	int last_pick_value;
	int m_idx;
	double m_xpos, m_ypos;

	void add_contour_pt(double picked[3]);
	void closest_border(double picked[3], DoublePointType2D pt, int* pidx = NULL);
	void closest_border_2(double picked[3], DoublePointType2D pt);

  public:
    static radMouseInteractorStylePP* New();
    vtkTypeMacro(radMouseInteractorStylePP, vtkInteractorStyleImage);
 
	radMouseInteractorStylePP()
	{
		LeftMousedPressed = false;
		ShiftDown = false;
		MouseScroll = false;
		MouseIn = false;
		m_idx = -1;
		last_pick_value = 0;
		m_xpos = m_ypos = 0;
		img_dims[0] = img_dims[1] = img_dims[2] = 0;
	}

	virtual void OnChar() override;
	virtual void OnKeyDown() override;
	virtual void OnKeyUp() override;
	virtual void OnEnter() override;
	virtual void OnLeave() override;
	virtual void OnLeftButtonDown() override;
	virtual void OnMouseMove() override;
	virtual void OnLeftButtonUp() override;
	virtual void OnMiddleButtonDown() override;
	virtual void OnMiddleButtonUp() override;
};


//ground truth color
const double GroundTruthColor[3] = {221.0/255.0, 28.0/255.0, 119.0/255.0};
const double ResultColor[3] = {0.0/255.0, 255.0/255.0, 0.0/255.0};
const double FalsePositiveColor[3] = {253.0/255.0, 174.0/255.0, 97.0/255.0};
const double FalseNegativeColor[3] = {255.0/255.0, 255.0/255.0, 51.0/255.0};

const double SmallDisplacement = -0.01;

struct UndoEntry {
	static const int ADD = 0;
	static const int DEL = 1;
	static const int IMG = 2;
	UndoEntry(int del, double x, double y, bool more=false) :
		m_del(del), m_x(x), m_y(y), m_more(more) {}
	int m_del;
	bool m_more;
	double m_x;
	double m_y;
};

class radImageView
{
private:
	
	bool ImageAddFlag;
	bool interpolationFlag;

	// Closest distance between two markers
	double closedist = 3.;
	double GetCloseSquare() {
		double _closedist = GetGlyphScale() * 0.5;
		if (_closedist > closedist) _closedist = closedist;
		return _closedist * _closedist;
	}

	vtkSmartPointer<vtkImageData> ImageData;
	vtkSmartPointer<vtkImageActor> ImageActor;
	void DrawInputImage();
	
	vtkSmartPointer<vtkPoints> InteractiveContourPoints;
	vtkSmartPointer<vtkCellArray> InteractiveContourCells;
	vtkSmartPointer<vtkPolyData> InteractiveContourPolydata;
	vtkSmartPointer<vtkPolyDataMapper> InteractiveContourMapper;
	vtkSmartPointer<vtkActor> InteractiveContourActor;
	void DrawInteractiveContours();

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

	void GetImageDimensions(int dims[3]) {
		ImageData->GetDimensions(dims);
	}
    
	void ResetView(bool camera_flag = true); //used for initialization

	vtkSmartPointer<vtkRenderWindow> GetRenderWin() {return RenderWin;}

	void SetSplitImage(FloatImageType2D::Pointer);
	void SetColorInfo(ColorInfo ci);
	void SetColorInfo(double color_level, double color_window);
	ColorInfo GetColorInfo();

	void SetConePoints(DoublePointArray2D &, vector<float> &, vector<float> &, 
		DoublePointArray2D &, vector<float> &, vector<float> &, 
		vector< pair<unsigned int, unsigned int> > &, DoublePointArray2D &);
	void GetConePoints(DoublePointArray2D &);
	void InitializeView();
	void InitializeFeatures();
	void SetConeGlyphVisibility(bool);
	double GetGlyphScale();
	void SetGlyphScale(double);
	bool GetInterpolation() { return interpolationFlag; }
	void SetInterpolation(bool flag);
	
	int GetFeatureCount() {
		return ConePoints->GetNumberOfPoints();
	}
	int FindMarker(double* pxpos, double* pypos);
	void UpdateMarkerAt(int idx, double xpos, double ypos);
	void RemoveDetectedFeatures(double, double, double);
	void EraseAreaMarkers(DoublePointArray2D &);
	void AddDetectedFeatures(double, double, double);
	void AddUndoEntry(int del, double x, double y, bool more=false);
	void AddMoveUndoEntry(int idx, double x, double y);
	void DoUndo();
	deque< UndoEntry > undoStack;

	void SetInteractiveContours(DoublePointArray2D &, bool ending_flag = false);
};

#endif // radImageView_H

