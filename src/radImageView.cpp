/*
 *  radImageView.cpp
 *  
 *
 *  Created by Jianfei Liu on 9/19/11.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */

#include <QtGlobal>
#if QT_VERSION >= 0x050000
#include <QtWidgets/QMessageBox>
#endif
#include "radimageview.h"
#include "radmainwindow.h"
#include <vtkObjectFactory.h>
#include <vtkPointData.h>
#include <vtkProperty.h>
#include <vtkProperty2D.h>
#include <vtkCellData.h>
#include <vtkTextProperty.h>
#include <vtkImageProperty.h>

vtkStandardNewMacro(radMouseInteractorStylePP);

void radMouseInteractorStylePP::OnLeftButtonDown()
{
	if (radMainWindow::GetPointer()->ConeEraseFlag)
    {
		int pick_value =  this->Interactor->GetPicker()->Pick(this->Interactor->GetEventPosition()[0], 
			this->Interactor->GetEventPosition()[1], 0,  // always zero.
            this->Interactor->GetRenderWindow()->GetRenderers()->GetFirstRenderer());
		double picked[3];
		this->Interactor->GetPicker()->GetPickPosition(picked);
		
		//std::cout << "Picked value: " << picked[0] << " " << picked[1] << " " << picked[2] << ", " << pick_value << std::std::endl;
		if (pick_value != 0)
		{
			radMainWindow::GetPointer()->GetImageView()->RemoveDetectedFeatures(picked[0], picked[1], picked[2]);
			radMainWindow::GetPointer()->GetImageView()->ResetView(false);
		}
	}
	else if (radMainWindow::GetPointer()->ConeMarkFlag)
	{
		int pick_value =  this->Interactor->GetPicker()->Pick(this->Interactor->GetEventPosition()[0], 
			this->Interactor->GetEventPosition()[1], 0,  // always zero.
            this->Interactor->GetRenderWindow()->GetRenderers()->GetFirstRenderer());
		double picked[3];
		this->Interactor->GetPicker()->GetPickPosition(picked);
		
//		std::cout << "Picked value: " << picked[0] << " " << picked[1] << " " << picked[2] << ", " << pick_value << std::endl;
		if (pick_value != 0)
		{
			radMainWindow::GetPointer()->GetImageView()->AddDetectedFeatures(picked[0], picked[1], picked[2]);
			radMainWindow::GetPointer()->GetImageView()->ResetView(false);
		}
	}
	else
		vtkInteractorStyleImage::OnLeftButtonDown();
}

void radImageView::DrawInputImage()
{
	ImageAddFlag = false;
	ImageData = vtkSmartPointer<vtkImageData>::New();
	#if VTK_MAJOR_VERSION >= 6
	ImageData->AllocateScalars(VTK_UNSIGNED_CHAR, 1);
	#else
	ImageData->SetScalarTypeToUnsignedChar();
	#endif
	ImageActor = vtkSmartPointer<vtkImageActor>::New();
}

void radImageView::DrawConeDetections()
{
	ConePoints = vtkSmartPointer<vtkPoints>::New();
	ConePolydata = vtkSmartPointer<vtkPolyData>::New();
	ConePolydata->SetPoints(ConePoints);

	ConeGlyphSource = vtkSmartPointer<vtkGlyphSource2D>::New();
	ConeGlyphSource->SetGlyphTypeToCross();
	ConeGlyphSource->SetScale(6);

	ConeGlyph = vtkSmartPointer<vtkGlyph3D>::New();
	#if VTK_MAJOR_VERSION >= 6
	ConeGlyph->SetSourceConnection(ConeGlyphSource->GetOutputPort());
	ConeGlyph->SetInputData(ConePolydata);
	#else
	ConeGlyph->SetSource(ConeGlyphSource->GetOutput());
	ConeGlyph->SetInput(ConePolydata);
	#endif
	ConeGlyph->SetRange(0, 1);
	ConeGlyph->SetColorModeToColorByScalar();
	ConeGlyph->SetScaleModeToDataScalingOff();
	
	ConeMapper = vtkSmartPointer<vtkDataSetMapper>::New();
	ConeMapper->SetInputConnection(ConeGlyph->GetOutputPort());
	ConeMapper->ScalarVisibilityOff();
	ConeMapper->SetScalarRange(0, 1);
	ConeMapper->SetColorModeToMapScalars();
	ConeMapper->SetScalarModeToUsePointData();
	
	ConeActor = vtkSmartPointer<vtkActor>::New();
	ConeActor->SetMapper(ConeMapper);
	ConeActor->GetProperty()->SetColor(ResultColor[0], ResultColor[1], ResultColor[2]);
}


radImageView::radImageView()
{
	DrawInputImage();
	DrawConeDetections();


	ImageRender = vtkSmartPointer<vtkRenderer>::New();
	ImageRender->SetBackground( 0.0f, 0.0f, 0.0f );
	ImageRender->AddActor(ConeActor);

	ImageStyle = vtkSmartPointer<radMouseInteractorStylePP>::New();
	RenderWin = vtkSmartPointer<vtkRenderWindow>::New();
    RenderWin->AddRenderer(ImageRender);
	WinInteractor = vtkSmartPointer<vtkRenderWindowInteractor>::New();
    WinInteractor->SetInteractorStyle(ImageStyle);
    WinInteractor->SetRenderWindow(RenderWin);
	
	WinInteractor->Initialize();
  
	ConePoints->Initialize();
	ConePoints->SetNumberOfPoints(0);
	ConePolydata->Modified();
	ConeGlyph->Update();
}

radImageView::~radImageView()
{
}

void radImageView::SetSplitImage(FloatImageType2D::Pointer img)
{
	ImageData->Initialize();
	ConvertITKImageToVTKImage<FloatImageType2D, unsigned char>(img, ImageData);
	ImageData->Modified();

	if (!ImageAddFlag)
	{
		#if VTK_MAJOR_VERSION >= 6
		ImageActor->SetInputData(ImageData);
		#else
		ImageActor->SetInput(ImageData);
		#endif
		ImageRender->AddActor(ImageActor);
		ImageAddFlag = true;
	}
}

void radImageView::InitializeView()
{
	ImageData->Initialize();
	ImageData->Modified();

	ConePoints->Initialize();
	ConePolydata->Modified();
	ConeGlyph->Update();

}

void radImageView::ChangeCameraOrientation(vtkSmartPointer<vtkRenderer> img_ren)
{
	img_ren->ResetCamera();
	double* fp = img_ren->GetActiveCamera()->GetFocalPoint();
	double* p = img_ren->GetActiveCamera()->GetPosition();
	double dist = sqrt( (p[0]-fp[0])*(p[0]-fp[0]) + (p[1]-fp[1])*(p[1]-fp[1]) 
		+ (p[2]-fp[2])*(p[2]-fp[2]) );
	img_ren->GetActiveCamera()->SetPosition(fp[0], fp[1], fp[2]-dist);
	img_ren->GetActiveCamera()->SetViewUp(0.0, -1.0, 0.0);
}

void radImageView::ResetView(bool camera_flag)
{
	if (camera_flag)
		ChangeCameraOrientation(ImageRender);
  RenderWin->Render();
}

void radImageView::SetConePoints(DoublePointArray2D & left_pts, vector<float> & left_scales, 
								 vector<float> & left_weights, DoublePointArray2D & right_pts, 
								 vector<float> & right_scales, vector<float> & right_weights, 
								 vector< pair<unsigned int, unsigned int> > & pt_links,
								 DoublePointArray2D & final_pts)
{
	//final detections
	ConePoints->Initialize();
	ConePoints->SetNumberOfPoints(final_pts.size());
	
	for (unsigned int i=0; i<final_pts.size(); i++)
	{
		ConePoints->SetPoint(i, final_pts[i][0], final_pts[i][1], -0.01);
	}

	undoStack.clear();

	ConePolydata->Modified();
	ConeGlyph->Update();
}

void radImageView::GetConePoints(DoublePointArray2D & edited_pts)
{
	double x[3];
	edited_pts.resize(ConePoints->GetNumberOfPoints());
	for (unsigned int i = 0; i < edited_pts.size(); i++)
	{
		ConePoints->GetPoint(i, x);
		edited_pts[i][0] = x[0];
		edited_pts[i][1] = x[1];
	}
}


void radImageView::SetConeGlyphVisibility(bool flag)
{
	ConeActor->SetVisibility(flag);
}

void radImageView::SetGlyphScale(double scale)
{
	ConeGlyphSource->SetScale(scale);
}

void radImageView::RemoveDetectedFeatures(double xpos, double ypos, double zpos)
{
	// Remove a cone marker if it's close than 3 pix to the mouse
	double radsq = 3.*3.;

	int nidx = -1;
	double mindist = 1000000.;
	double pos[3];
	vector<DoublePointType2D> pts(ConePoints->GetNumberOfPoints());
	for (int i = 0; i < ConePoints->GetNumberOfPoints(); i++) {
		ConePoints->GetPoint(i, pos);
		pts[i][0] = pos[0];
		pts[i][1] = pos[1];
		double dist = (xpos - pos[0])*(xpos - pos[0]) + (ypos - pos[1])*(ypos - pos[1]);
		if (dist < mindist) {
			mindist = dist;
			nidx = i;
		}
	}

	if (mindist < radsq)
	{
		ConePoints->GetPoint(nidx, pos);
		AddUndoEntry(true, pos[0], pos[1]);
		ConePoints->Initialize();
		for (int i = 0; i < pts.size(); i++)
		{
			if (i != nidx) {
				ConePoints->InsertNextPoint(pts[i][0], pts[i][1], -0.01);
			}
		}

		ConePoints->Modified();
		ConePolydata->Modified();
#if VTK_MAJOR_VERSION < 6
		ConePolydata->Update();
#endif
		ConeGlyph->Update();
	}
}

void radImageView::AddDetectedFeatures(double xpos, double ypos, double zpos)
{
	/*if (ConePoints->GetNumberOfPoints() == 0)
		return;*/
	ConePoints->InsertPoint(ConePoints->GetNumberOfPoints(), xpos, ypos, -0.01);
	AddUndoEntry(false, xpos, ypos);
	ConePoints->Modified();
	ConePolydata->Modified();
	#if VTK_MAJOR_VERSION < 6
	ConePolydata->Update();
	#endif
}

void radImageView::AddUndoEntry(bool del, double x, double y)
{
  undoStack.push_back(UndoEntry(del, x, y));
  if (undoStack.size() > 100) {
    undoStack.pop_front();
  }
}

void radImageView::DoUndo()
{
  if (!undoStack.size())
    return;

  UndoEntry entry = undoStack.back();
  undoStack.pop_back();

  if (!entry.m_del) {
	  vtkIdType new_len = ConePoints->GetNumberOfPoints() - 1;
	  ConePoints->Resize(new_len);
  } else {
	  ConePoints->InsertNextPoint(entry.m_x, entry.m_y, -0.01);
  }
  ConePoints->Modified();
  ConePolydata->Modified();
#if VTK_MAJOR_VERSION < 6
  ConePolydata->Update();
#endif
  ConeGlyph->Update();
  RenderWin->Render();
}

