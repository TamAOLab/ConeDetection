/*
 *  radDetectionPanel.h
 *  
 *
 *  Created by Jianfei Liu on 9/19/11.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef radDetectionPanel_H
#define radDetectionPanel_H

#include <QDialog>
#include <QComboBox>
#include <QGroupBox>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QListWidget>
#include <QGridLayout>
#include <QDoubleValidator>
#include <QDoubleSpinBox>
#include <QCheckBox>
#include <QRadioButton>
#include <QSlider>
#include <QColorDialog>
#include "raddefinition.h"

class radDetectionPanel : public QDialog
{
  Q_OBJECT
public:
	
	radDetectionPanel(QWidget *parent = 0);
	~radDetectionPanel();

	void UpdateControlPanel(ConeDetectionParameters &);

private slots:

	void ChangeVotingRadius(int);
	void ChangeGradientMagnitude(double);
	void ChangeScale(double);
	void ChangeDarkConeDetectionFlag(bool);
	void ChangeLOGResponse(double);
	void ClickedDetectCurrent();
	void ClickedDetectAll();
	void ClickedRestoreDefaults();

signals:
	void launchDetectCurrent();
	void launchDetectAll();

private:
    
	void CreateInputGroup();

	QVBoxLayout *ViewLayout;

	QGroupBox *DetectionSetupGroup;
	QLabel *VotingRadiusLabel;
	QSpinBox *VotingRadiusInput;

	QLabel *GradientMagnitudeLabel;
	QDoubleSpinBox *GradientMagnitudeInput;
	// QLabel *VotingThresholdLabel;
	// QDoubleSpinBox *VotingThresholdInput;

	QLabel *ScaleLabel;
	QDoubleSpinBox *ScaleInput;

	// QLabel *MergeRadiusLabel;
	// QSpinBox *MergeRadiusInput;

	QCheckBox *DimConeBox;

	QPushButton *RestoreDefaultButton;

	QLabel *ScaleResponseLabel; // , *ScaleSizeLabel;
	QDoubleSpinBox *ScaleResponseInput; // , *ScaleSizeInput;
	QGridLayout *DetectionSetupLayout;

	QGridLayout *DetectionLaunchLayout;
	QWidget *DetectionLaunchGroup;
	QPushButton *LaunchCurrentButton;
	QPushButton *LaunchAllButton;
};

#endif // radDetectionPanel

