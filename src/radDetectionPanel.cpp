/*
 *  radDetectionPanel.cpp
 *  
 *
 *  Created by Jianfei Liu on 9/19/11.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */

#include <QtGui>
#include "radDetectionPanel.h"
#include "radmainwindow.h"

radDetectionPanel::radDetectionPanel(QWidget *parent)
    : QDialog(parent)
{
	//create input group
	CreateInputGroup();
    setWindowTitle(tr("Split Image Cone Detection"));
	setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
}

radDetectionPanel::~radDetectionPanel()
{ 

}

void radDetectionPanel::CreateInputGroup()
{
	DetectionSetupGroup = new QGroupBox(tr("Cone Detection Setup"));

	VotingRadiusLabel = new QLabel(tr("Voting Radius (pixel):"));
	VotingRadiusInput = new QSpinBox;
	VotingRadiusInput->setRange(0, 30);
    VotingRadiusInput->setSingleStep(1);
    VotingRadiusInput->setValue(5);
	VotingRadiusInput->setAlignment(Qt::AlignCenter);
	connect(VotingRadiusInput, SIGNAL(valueChanged(int)), this, SLOT(ChangeVotingRadius(int)));

	GradientMagnitudeLabel = new QLabel(tr("Gradient Threshold:"));
	GradientMagnitudeInput = new QDoubleSpinBox;
	GradientMagnitudeInput->setRange(0.0, 100);
    GradientMagnitudeInput->setSingleStep(1);
    GradientMagnitudeInput->setValue(5.0);
	GradientMagnitudeInput->setAlignment(Qt::AlignCenter);
	connect(GradientMagnitudeInput, SIGNAL(valueChanged(double)), this, SLOT(ChangeGradientMagnitude(double)));
	
	ScaleLabel = new QLabel(tr("Scale Value:"));
	ScaleInput = new QDoubleSpinBox;
	ScaleInput->setRange(0.0, 15);
    ScaleInput->setSingleStep(0.5);
    ScaleInput->setValue(2.0);
	ScaleInput->setAlignment(Qt::AlignCenter);
	connect(ScaleInput, SIGNAL(valueChanged(double)), this, SLOT(ChangeScale(double)));

	ScaleResponseLabel = new QLabel(tr("LoG Response:"));
	ScaleResponseInput = new QDoubleSpinBox;
	ScaleResponseInput->setRange(0.0, 3);
    ScaleResponseInput->setSingleStep(0.1);
    ScaleResponseInput->setValue(1.0);
	ScaleResponseInput->setAlignment(Qt::AlignCenter);
	connect(ScaleResponseInput, SIGNAL(valueChanged(double)), this, SLOT(ChangeLOGResponse(double)));

	DimConeBox = new QCheckBox("Dim cones?");
	DimConeBox->setChecked(false);
	connect(DimConeBox, SIGNAL(clicked(bool)), this, SLOT(ChangeDarkConeDetectionFlag(bool)));

	RestoreDefaultButton = new QPushButton("  Restore Defaults  ");
	connect(RestoreDefaultButton, SIGNAL(clicked()), this, SLOT(ClickedRestoreDefaults()));

	DetectionSetupLayout = new QGridLayout;
	DetectionSetupLayout->addWidget(VotingRadiusLabel, 0, 0);
	DetectionSetupLayout->addWidget(VotingRadiusInput, 0, 1);
	DetectionSetupLayout->addWidget(GradientMagnitudeLabel, 0, 2);
	DetectionSetupLayout->addWidget(GradientMagnitudeInput, 0, 3);
	DetectionSetupLayout->addWidget(ScaleLabel, 1, 0);
	DetectionSetupLayout->addWidget(ScaleInput, 1, 1);
	DetectionSetupLayout->addWidget(ScaleResponseLabel, 1, 2);
	DetectionSetupLayout->addWidget(ScaleResponseInput, 1, 3);
	DetectionSetupLayout->addWidget(DimConeBox, 2, 0, 1, 2, Qt::AlignLeft);
	DetectionSetupLayout->addWidget(RestoreDefaultButton, 3, 2, 1, 2, Qt::AlignRight);
	DetectionSetupGroup->setLayout(DetectionSetupLayout);
	
	DetectionLaunchGroup = new QWidget();

	DetectionLaunchLayout = new QGridLayout;
	DetectionLaunchGroup->setLayout(DetectionLaunchLayout);

	LaunchCurrentButton = new QPushButton(tr("Detect Current"));
	DetectionLaunchLayout->addWidget(LaunchCurrentButton, 0, 0);
	connect(LaunchCurrentButton, SIGNAL(clicked()), this, SLOT(ClickedDetectCurrent()));
	LaunchAllButton = new QPushButton(tr("Detect All"));
	DetectionLaunchLayout->addWidget(LaunchAllButton, 0, 1);
	connect(LaunchAllButton, SIGNAL(clicked()), this, SLOT(ClickedDetectAll()));

	RestoreDefaultButton->setAutoDefault(false);
	LaunchCurrentButton->setAutoDefault(false);
	LaunchAllButton->setAutoDefault(true);

    ViewLayout = new QVBoxLayout;
	ViewLayout->addWidget(DetectionSetupGroup);
	ViewLayout->addWidget(DetectionLaunchGroup);
	setLayout(ViewLayout);
}

void radDetectionPanel::ClickedDetectCurrent()
{
	close();
	emit launchDetectCurrent();
}

void radDetectionPanel::ClickedDetectAll()
{
	close();
	emit launchDetectAll();
}

void radDetectionPanel::ClickedRestoreDefaults()
{
	radMainWindow::GetPointer()->LoadDefaultDetectionPara();
	UpdateControlPanel(radMainWindow::GetPointer()->GetConeDetectionParameters());
}

void radDetectionPanel::ChangeVotingRadius(int radius)
{
	radMainWindow::GetPointer()->GetConeDetectionParameters().VotingRadius = radius;
}

void radDetectionPanel::ChangeGradientMagnitude(double val)
{
	radMainWindow::GetPointer()->GetConeDetectionParameters().GradientMagnitudeThreshold = val;
}

void radDetectionPanel::ChangeScale(double val)
{
	radMainWindow::GetPointer()->GetConeDetectionParameters().Scale = val;
}

void radDetectionPanel::ChangeDarkConeDetectionFlag(bool flag)
{
	radMainWindow::GetPointer()->GetConeDetectionParameters().DimConeDetectionFlag = flag;
}

void radDetectionPanel::ChangeLOGResponse(double val)
{
	radMainWindow::GetPointer()->GetConeDetectionParameters().LOGResponse = val;
}

void radDetectionPanel::UpdateControlPanel(ConeDetectionParameters & detection_para)
{
	VotingRadiusInput->setValue(detection_para.VotingRadius);
	GradientMagnitudeInput->setValue(detection_para.GradientMagnitudeThreshold);

	ScaleInput->setValue(detection_para.Scale);

	DimConeBox->setChecked(detection_para.DimConeDetectionFlag);
	ScaleResponseInput->setValue(detection_para.LOGResponse);
}

