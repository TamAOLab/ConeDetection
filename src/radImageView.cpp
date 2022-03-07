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
#include <QApplication>
#include <QCursor>
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

void radMouseInteractorStylePP::add_contour_pt(double picked[3])
{
	DoublePointType2D pt;
	pt[0] = picked[0];
	pt[1] = picked[1];
	contour_pts.push_back(pt);
	radMainWindow::GetPointer()->GetImageView()->SetInteractiveContours(contour_pts);
	radMainWindow::GetPointer()->GetImageView()->ResetView(false);
}

void radMouseInteractorStylePP::closest_border(double picked[3], DoublePointType2D pt, int* pidx)
{
	double dx0 = fabs(pt[0]);
	double dx1 = fabs(pt[0] - img_dims[0]);
	double dy0 = fabs(pt[1]);
	double dy1 = fabs(pt[1] - img_dims[1]);
	int idx = 0;
	if (dx0 <= dx1 && dx0 <= dy0 && dx0 <= dy1) {
		picked[0] = 0.;
		picked[1] = pt[1];
	}
	else if (dx1 <= dy0 && dx1 <= dy1) {
		picked[0] = double(img_dims[0]);
		picked[1] = pt[1];
	}
	else if (dy0 <= dy1) {
		picked[0] = pt[0];
		picked[1] = 0.;
		idx = 1;
	}
	else {
		picked[0] = pt[0];
		picked[1] = double(img_dims[1]);
		idx = 1;
	}
	if (pidx) *pidx = idx;
	// std::cout << "closest_border(): " << picked[0] << " " << picked[1] << std::endl;
}

void radMouseInteractorStylePP::closest_border_2(double picked[3], DoublePointType2D pt)
{
	int idx1;
	closest_border(picked, pt, &idx1);
	if (!contour_pts.empty()) {
		int idx0;
		double corner[3];
		closest_border(corner, contour_pts[contour_pts.size()-1], &idx0);
		if (idx0 != idx1) {
			pt[idx0] = corner[idx0];
			pt[idx1] = picked[idx1];
			// std::cout << "corner pt: " << pt[0] << " " << pt[1] << std::endl;
			contour_pts.push_back(pt);
		}
	}
}

void radMouseInteractorStylePP::OnChar()
{
	vtkRenderWindowInteractor* rwi = this->Interactor;
	char c = rwi->GetKeyCode();
	if (!strchr("wWrRxXyYzZfF", c)) {
		vtkInteractorStyleImage::OnChar();
	}
}

void radMouseInteractorStylePP::OnKeyDown()
{
	while (QApplication::overrideCursor()) QApplication::restoreOverrideCursor();
	if (!MouseIn) {
		vtkInteractorStyleImage::OnKeyDown();
		return;
	}
	vtkRenderWindowInteractor *rwi = this->Interactor;
	std::string key = rwi->GetKeySym();
	if (key == "Up") {
		radMainWindow::GetPointer()->PreviousImage();
		return;
	} else if (key == "Down") {
		radMainWindow::GetPointer()->NextImage();
		return;
	}
	if (key == "Shift_L" && MouseIn) {
		ShiftDown = true;
		if (!MouseScroll) {
			QApplication::setOverrideCursor(Qt::OpenHandCursor);
		}
	}
	vtkInteractorStyleImage::OnKeyDown();
}
void radMouseInteractorStylePP::OnKeyUp()
{
	while (QApplication::overrideCursor()) QApplication::restoreOverrideCursor();
	vtkRenderWindowInteractor *rwi = this->Interactor;
	std::string key = rwi->GetKeySym();
	if (key == "Up" || key == "Down") {
		return;
	}
	if (key == "Shift_L") {
		ShiftDown = false;
	}
	vtkInteractorStyleImage::OnKeyUp();
}
void radMouseInteractorStylePP::OnEnter()
{
	while (QApplication::overrideCursor()) QApplication::restoreOverrideCursor();
	if (!MouseIn) {
		MouseIn = true;
		if (ShiftDown) QApplication::setOverrideCursor(Qt::OpenHandCursor);
	}
	else {
		// If a second OnEnter() received without a matching OnLeave(),
		// the QVTKWidget does not have keyboard focus and it won't receive OnKeyUp() for Shift either.
		// Like the user tried to drag the mouse from another widget while holding Shift down.
		ShiftDown = false;
	}
	vtkInteractorStyleImage::OnEnter();
}

void radMouseInteractorStylePP::OnLeave()
{
	MouseIn = false;
	while (QApplication::overrideCursor()) QApplication::restoreOverrideCursor();
	vtkInteractorStyleImage::OnLeave();
}

void radMouseInteractorStylePP::OnLeftButtonDown()
{
	while (QApplication::overrideCursor()) QApplication::restoreOverrideCursor();
	if (!MouseIn) {
		ShiftDown = false;
		vtkInteractorStyleImage::OnLeftButtonDown();
		return;
	}
	MouseScroll = false;
	if (this->Interactor->GetShiftKey()) {
		ShiftDown = MouseScroll = true;
		QApplication::setOverrideCursor(Qt::SizeAllCursor);
		vtkInteractorStyleImage::OnLeftButtonDown();
		return;
	}
	LeftMousedPressed = true;
	radMainWindow::GetPointer()->GetImageView()->GetImageDimensions(img_dims);
	ci = radMainWindow::GetPointer()->GetImageView()->GetColorInfo();
	// std::cout << "AreaEraseFlag = " << radMainWindow::GetPointer()->AreaEraseFlag << std::endl;
	if (radMainWindow::GetPointer()->mouseMode == MouseOperation::Delete_Point)
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
		return;
	}
	else if (radMainWindow::GetPointer()->mouseMode == MouseOperation::Add_Point)
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
		return;
	}
	else if (radMainWindow::GetPointer()->mouseMode == MouseOperation::Delete_Area)
	{
		// Start drawing a contour
		int pick_value = this->Interactor->GetPicker()->Pick(this->Interactor->GetEventPosition()[0],
			this->Interactor->GetEventPosition()[1], 0,  // always zero.
			this->Interactor->GetRenderWindow()->GetRenderers()->GetFirstRenderer());
		double picked[3];
		this->Interactor->GetPicker()->GetPickPosition(picked);

		// std::cout << "dims: " << img_dims[0] << " " << img_dims[1] << " " << img_dims[2] << std::endl;
		// std::cout << "<Down> Picked value: " << picked[0] << " " << picked[1] << " " << picked[2] << ", " << pick_value << std::endl;
		if (pick_value != 0)
		{
			add_contour_pt(picked);
		}
		last_pick_value = pick_value;
		return;
	}
	else if (radMainWindow::GetPointer()->mouseMode == MouseOperation::Move_Point) {
		int pick_value = this->Interactor->GetPicker()->Pick(this->Interactor->GetEventPosition()[0],
			this->Interactor->GetEventPosition()[1], 0,  // always zero.
			this->Interactor->GetRenderWindow()->GetRenderers()->GetFirstRenderer());

		//std::cout << "Picked value: " << picked[0] << " " << picked[1] << " " << picked[2] << ", " << pick_value << std::std::endl;
		if (pick_value != 0)
		{
			double picked[3];
			this->Interactor->GetPicker()->GetPickPosition(picked);

			m_xpos = picked[0];
			m_ypos = picked[1];
			m_idx = radMainWindow::GetPointer()->GetImageView()->FindMarker(&m_xpos, &m_ypos);
		}
		return;
	}
	vtkInteractorStyleImage::OnLeftButtonDown();
}

void radMouseInteractorStylePP::OnMouseMove()
{
	if (radMainWindow::GetPointer()->mouseMode == MouseOperation::Delete_Area && LeftMousedPressed)
	{
		int pick_value = this->Interactor->GetPicker()->Pick(this->Interactor->GetEventPosition()[0],
			this->Interactor->GetEventPosition()[1], 0,  // always zero.
			this->Interactor->GetRenderWindow()->GetRenderers()->GetFirstRenderer());
		double picked[3];
		this->Interactor->GetPicker()->GetPickPosition(picked);

		//std::cout << "<Move> Picked value: " << picked[0] << " " << picked[1] << " " << picked[2] << ", " << pick_value << std::endl;
		if (pick_value == 0) {
			if (last_pick_value != 0 && !contour_pts.empty()) {
				closest_border(picked, contour_pts[contour_pts.size()-1]);
				add_contour_pt(picked);
			}
		}
		else {
			if (last_pick_value == 0) {
				DoublePointType2D pt;
				pt[0] = picked[0];
				pt[1] = picked[1];
				double bord[3];
				closest_border_2(bord, pt);
				add_contour_pt(bord);
			}
			add_contour_pt(picked);
		}
		last_pick_value = pick_value;
		
		return;
	}
	else if (radMainWindow::GetPointer()->mouseMode == MouseOperation::Move_Point &&
		LeftMousedPressed && m_idx >= 0)
	{
		int pick_value = this->Interactor->GetPicker()->Pick(this->Interactor->GetEventPosition()[0],
			this->Interactor->GetEventPosition()[1], 0,  // always zero.
			this->Interactor->GetRenderWindow()->GetRenderers()->GetFirstRenderer());
		if (pick_value != 0) {
			double picked[3];
			this->Interactor->GetPicker()->GetPickPosition(picked);

			double xmax = double(img_dims[0]) - 0.2;
			double ymax = double(img_dims[1]) - 0.2;
			if (picked[0] < 0.2) picked[0] = 0.2;
			if (picked[1] < 0.2) picked[1] = 0.2;
			if (picked[0] > xmax) picked[0] = xmax;
			if (picked[1] > ymax) picked[1] = ymax;

			radMainWindow::GetPointer()->GetImageView()->UpdateMarkerAt(m_idx, picked[0], picked[1]);
		}
	}
	vtkInteractorStyleImage::OnMouseMove();
}

void radMouseInteractorStylePP::OnLeftButtonUp()
{
	while (QApplication::overrideCursor()) QApplication::restoreOverrideCursor();
	if (MouseScroll) {
		MouseScroll = false;
		if (ShiftDown) QApplication::setOverrideCursor(Qt::OpenHandCursor);
		vtkInteractorStyleImage::OnLeftButtonUp();
		LeftMousedPressed = false;
		return;
	}
	if (radMainWindow::GetPointer()->mouseMode == MouseOperation::Delete_Area)
	{
		int pick_value = this->Interactor->GetPicker()->Pick(this->Interactor->GetEventPosition()[0],
			this->Interactor->GetEventPosition()[1], 0,  // always zero.
			this->Interactor->GetRenderWindow()->GetRenderers()->GetFirstRenderer());
		double picked[3];
		this->Interactor->GetPicker()->GetPickPosition(picked);

		// std::cout << "<Up> Picked value: " << picked[0] << " " << picked[1] << " " << picked[2] << ", " << pick_value << std::endl;
		if (pick_value == 0 && !contour_pts.empty()) {
			closest_border_2(picked, contour_pts[0]);
			pick_value = 1;
		}
		if (pick_value != 0)
		{
			DoublePointType2D pt;
			pt[0] = picked[0];
			pt[1] = picked[1];
			contour_pts.push_back(pt);
		}

		radMainWindow::GetPointer()->GetImageView()->SetInteractiveContours(contour_pts, true);
		radMainWindow::GetPointer()->GetImageView()->EraseAreaMarkers(contour_pts);
		radMainWindow::GetPointer()->GetImageView()->ResetView(false);

		contour_pts.clear();
	}
	else if (radMainWindow::GetPointer()->mouseMode == MouseOperation::Move_Point &&
		LeftMousedPressed && m_idx >= 0)
	{
		radMainWindow::GetPointer()->GetImageView()->AddMoveUndoEntry(m_idx, m_xpos, m_ypos);
	}
	else {
		vtkInteractorStyleImage::OnLeftButtonUp();
		ColorInfo _ci = radMainWindow::GetPointer()->GetImageView()->GetColorInfo();
		if (fabs(_ci.color_level - ci.color_level) > 0.01 || fabs(_ci.color_window - ci.color_window) > 0.01)
			radMainWindow::GetPointer()->GetImageView()->AddUndoEntry(UndoEntry::IMG, ci.color_level, ci.color_window, false);
	}
	LeftMousedPressed = false;
}

void radMouseInteractorStylePP::OnMiddleButtonDown()
{
	while (QApplication::overrideCursor()) QApplication::restoreOverrideCursor();
	if (MouseIn) {
		MouseScroll = true;
		QApplication::setOverrideCursor(Qt::SizeAllCursor);
	}
	vtkInteractorStyleImage::OnMiddleButtonDown();
}
void radMouseInteractorStylePP::OnMiddleButtonUp()
{
	while (QApplication::overrideCursor()) QApplication::restoreOverrideCursor();
	MouseScroll = false;
	if (ShiftDown) QApplication::setOverrideCursor(Qt::OpenHandCursor);
	vtkInteractorStyleImage::OnMiddleButtonUp();
}

void radImageView::DrawInputImage()
{
	ImageAddFlag = false;
	interpolationFlag = true;
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

void radImageView::DrawInteractiveContours()
{
	//==========================used for contour selection=========================================
	InteractiveContourPoints = vtkSmartPointer<vtkPoints>::New();
	InteractiveContourCells = vtkSmartPointer<vtkCellArray>::New();
	InteractiveContourPolydata = vtkSmartPointer<vtkPolyData>::New();
	InteractiveContourPolydata->SetPoints(InteractiveContourPoints);
	InteractiveContourPolydata->SetLines(InteractiveContourCells);
	InteractiveContourMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
	InteractiveContourMapper->SetInputData(InteractiveContourPolydata);
	InteractiveContourMapper->ScalarVisibilityOff();
	InteractiveContourActor = vtkSmartPointer<vtkActor>::New();
	InteractiveContourActor->SetMapper(InteractiveContourMapper);
	InteractiveContourActor->GetProperty()->SetColor(240 / 255.0, 180 / 255.0, 14 / 255.0);
	InteractiveContourActor->GetProperty()->SetLineWidth(5.);
	//=============================================================================================
}


radImageView::radImageView()
{
	DrawInputImage();
	DrawConeDetections();
	DrawInteractiveContours();

	ImageRender = vtkSmartPointer<vtkRenderer>::New();
	ImageRender->SetBackground( 0.0f, 0.0f, 0.0f );
	ImageRender->AddActor(ConeActor);
	ImageRender->AddActor(InteractiveContourActor);

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
		if (!interpolationFlag)
			ImageActor->InterpolateOff();
		ImageRender->AddActor(ImageActor);
		ImageAddFlag = true;
	}

}

void radImageView::SetColorInfo(ColorInfo ci)
{
	ImageActor->GetProperty()->SetColorLevel(ci.color_level);
	ImageActor->GetProperty()->SetColorWindow(ci.color_window);
}
void radImageView::SetColorInfo(double color_level, double color_window)
{
	ImageActor->GetProperty()->SetColorLevel(color_level);
	ImageActor->GetProperty()->SetColorWindow(color_window);
}
ColorInfo radImageView::GetColorInfo()
{
	ColorInfo ci;
	ci.color_level = ImageActor->GetProperty()->GetColorLevel();
	ci.color_window = ImageActor->GetProperty()->GetColorWindow();
	return ci;
}

void radImageView::InitializeView()
{
	ImageData->Initialize();
	ImageData->Modified();

	InitializeFeatures();
}

void radImageView::InitializeFeatures()
{
	ConePoints->Initialize();
	ConePolydata->Modified();
	ConeGlyph->Update();

	InteractiveContourPoints->Initialize();
	InteractiveContourCells->Initialize();
	InteractiveContourPoints->Modified();
	InteractiveContourCells->Modified();
	InteractiveContourPolydata->Modified();

	undoStack.clear();
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

double radImageView::GetGlyphScale()
{
	return ConeGlyphSource->GetScale();
}

void radImageView::SetGlyphScale(double scale)
{
	ConeGlyphSource->SetScale(scale);
}

void radImageView::SetInterpolation(bool flag)
{
	interpolationFlag = flag;
	if (ImageAddFlag) {
		if (interpolationFlag)
			ImageActor->InterpolateOn();
		else
			ImageActor->InterpolateOff();
	}
}

int radImageView::FindMarker(double *pxpos, double *pypos)
{
	double radsq = closedist * closedist;
	int idx = -1;
	double mindist = 1000000.;
	double xpos = *pxpos;
	double ypos = *pypos;
	double pos[3];

	for (int i = 0; i < ConePoints->GetNumberOfPoints(); i++) {
		ConePoints->GetPoint(i, pos);
		double dist = (xpos - pos[0]) * (xpos - pos[0]) + (ypos - pos[1]) * (ypos - pos[1]);
		if (dist < mindist) {
			mindist = dist;
			idx = i;
		}
	}

	if (mindist < radsq) {
		ConePoints->GetPoint(idx, pos);
		*pxpos = pos[0];
		*pypos = pos[1];
		return idx;
	}

	return -1;
}
void radImageView::UpdateMarkerAt(int idx, double xpos, double ypos)
{
	ConePoints->SetPoint(idx, xpos, ypos, -0.01);
	ConePoints->Modified();
	ConePolydata->Modified();
	RenderWin->Render();
}

void radImageView::RemoveDetectedFeatures(double xpos, double ypos, double zpos)
{
	// Remove a cone marker if it's close than XXX pix to the mouse
	double radsq = GetCloseSquare();

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
		AddUndoEntry(UndoEntry::DEL, pos[0], pos[1]);
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

//-- "Winding Number" algorithm for the inclusion of a point in polygon, adapted from:
// http://geomalgorithms.com/a03-inclusion.html

// tests if a point is Left|On|Right of an infinite line.
//    Input:  three points P0, P1, and P2
//    Return: >0 for P2 left of the line through P0 and P1
//            =0 for P2  on the line
//            <0 for P2  right of the line
static inline int isLeft(DoublePointType2D P0, DoublePointType2D P1, DoublePointType2D P2)
{
	return ((P1[0] - P0[0]) * (P2[1] - P0[1])
		- (P2[0] - P0[0]) * (P1[1] - P0[1]));
}

static bool IsPointInsidePolygon(DoublePointType2D P, DoublePointArray2D& vertices)
{
	int n = (int)vertices.size();
	if (n == 0) return false;

	// V[] = vertex points of a polygon V[n+1] with V[n]=V[0]
	std::vector<DoublePointType2D> V(vertices);
	V.push_back(vertices[0]);

	int wn = 0;    // the  winding number counter (=0 only when P is outside)

	// loop through all edges of the polygon
	for (int i = 0; i < n; i++) {		// edge from V[i] to  V[i+1]
		if (V[i][1] <= P[1]) {			// start y <= P.y
			if (V[i + 1][1] > P[1])		// an upward crossing
				if (isLeft(V[i], V[i + 1], P) > 0)	// P left of  edge
					++wn;				// have  a valid up intersect
		}
		else {							// start y > P.y (no test needed)
			if (V[i + 1][1] <= P[1])		// a downward crossing
				if (isLeft(V[i], V[i + 1], P) < 0)	// P right of  edge
					--wn;				// have  a valid down intersect
		}
	}
	return wn != 0;
}

//-- End of "Winding Number" algorithm

void radImageView::EraseAreaMarkers(DoublePointArray2D &contour)
{
	double pos[3];
	DoublePointArray2D del_pts;
	DoublePointArray2D final_pts;
	for (int i = 0; i < ConePoints->GetNumberOfPoints(); i++) {
		ConePoints->GetPoint(i, pos);
		DoublePointType2D pt(pos);
		if (!IsPointInsidePolygon(pt, contour))
			final_pts.push_back(pt);
		else
			del_pts.push_back(pt);
	}
	if (del_pts.size() == 0) return;

	size_t last_del = del_pts.size() - 1;
	for (size_t i = 0; i < del_pts.size(); i++) {
		AddUndoEntry(UndoEntry::DEL, del_pts[i][0], del_pts[i][1], i > 0);
	}

	ConePoints->Initialize();
	ConePoints->SetNumberOfPoints(final_pts.size());

	for (unsigned int i = 0; i < final_pts.size(); i++)
	{
		ConePoints->SetPoint(i, final_pts[i][0], final_pts[i][1], -0.01);
	}

	ConePolydata->Modified();
	ConeGlyph->Update();
}

void radImageView::AddDetectedFeatures(double xpos, double ypos, double zpos)
{
	// Don't add a cone marker if it's close than XXX pix to existing one
	double radsq = GetCloseSquare();
	double pos[3];
	for (int i = 0; i < ConePoints->GetNumberOfPoints(); i++) {
		ConePoints->GetPoint(i, pos);
		double distsq = (xpos - pos[0])*(xpos - pos[0]) + (ypos - pos[1])*(ypos - pos[1]);
		if (distsq < radsq) return;
	}

	ConePoints->InsertPoint(ConePoints->GetNumberOfPoints(), xpos, ypos, -0.01);
	AddUndoEntry(UndoEntry::ADD, xpos, ypos);
	ConePoints->Modified();
	ConePolydata->Modified();
	#if VTK_MAJOR_VERSION < 6
	ConePolydata->Update();
	#endif
}

void radImageView::AddUndoEntry(int del, double x, double y, bool more)
{
  undoStack.push_back(UndoEntry(del, x, y, more));
  if (undoStack.size() > 500) {
    undoStack.pop_front();
  }
}

void radImageView::AddMoveUndoEntry(int idx, double x, double y)
{
	double pos[3];
	ConePoints->GetPoint(idx, pos);
	if (fabs(pos[0] - x) < 0.0001 && fabs(pos[1] - y) < 0.0001) return;
	AddUndoEntry(UndoEntry::DEL, x, y);
	AddUndoEntry(UndoEntry::ADD, pos[0], pos[1], true);
}

void radImageView::DoUndo()
{
  if (!undoStack.size())
    return;

  bool has_more = true;
  do
  {
	  UndoEntry entry = undoStack.back();
	  undoStack.pop_back();

	  if (entry.m_del == UndoEntry::ADD) {
		  double pos[3];
		  int del_idx = -1;
		  DoublePointArray2D pts(ConePoints->GetNumberOfPoints());
		  for (int i = 0; i < ConePoints->GetNumberOfPoints(); i++) {
			  ConePoints->GetPoint(i, pos);
			  pts[i][0] = pos[0];
			  pts[i][1] = pos[1];
			  if (del_idx < 0 && fabs(pos[0] - entry.m_x) < 0.0001 && fabs(pos[1] - entry.m_y) < 0.0001) {
				  del_idx = i;
			  }
		  }
		  if (del_idx >= 0) {
			  ConePoints->Initialize();
			  for (size_t i = 0; i < pts.size(); i++) {
				  if (i != (size_t)del_idx)
					  ConePoints->InsertNextPoint(pts[i][0], pts[i][1], -0.01);
			  }
		  }
	  }
	  else if (entry.m_del == UndoEntry::DEL) {
		  ConePoints->InsertNextPoint(entry.m_x, entry.m_y, -0.01);
	  }
	  else if (entry.m_del == UndoEntry::IMG) {
		  SetColorInfo(entry.m_x, entry.m_y);
	  }
	  has_more = entry.m_more;
  } while ((undoStack.size() > 0) && has_more);

  ConePoints->Modified();
  ConePolydata->Modified();
#if VTK_MAJOR_VERSION < 6
  ConePolydata->Update();
#endif
  ConeGlyph->Update();
  RenderWin->Render();
}

void radImageView::SetInteractiveContours(DoublePointArray2D &pts, bool ending_flag)
{
	InteractiveContourPoints->Initialize();
	InteractiveContourPoints->SetNumberOfPoints(pts.size());
	InteractiveContourCells->Initialize();
	InteractiveContourCells->InsertNextCell(pts.size());

	for (unsigned int i = 0; i < pts.size(); i++)
	{
		InteractiveContourPoints->SetPoint(i, pts[i][0], pts[i][1], SmallDisplacement);
		InteractiveContourCells->InsertCellPoint(i);
	}
	InteractiveContourPoints->Modified();
	InteractiveContourCells->Modified();
	InteractiveContourPolydata->Modified();

	if (ending_flag)
	{
		InteractiveContourPoints->Initialize();
		InteractiveContourCells->Initialize();
		InteractiveContourPoints->Modified();
		InteractiveContourCells->Modified();
		InteractiveContourPolydata->Modified();
	}
}
