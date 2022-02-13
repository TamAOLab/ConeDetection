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
#include <QTableWidget>
#include "raddefinition.h"

class radDetectionPanel : public QDialog
{
  Q_OBJECT
public:
	
	radDetectionPanel(QWidget *parent = 0);
	~radDetectionPanel();

	void UpdateControlPanel(ConeDetectionParameters &);
	void SetItemList(QStringList &items);
	void SetCheckedRows(QList<int> &rows);
	void SetHighlightedRow(int row);

protected:
	virtual void closeEvent(QCloseEvent *event) override;
	virtual void showEvent(QShowEvent *event) override;

private slots:

	void ChangeVotingRadius(int);
	void ChangeGradientMagnitude(double);
	void ChangeScale(double);
	void ChangeDarkConeDetectionFlag(bool);
	void ChangeLOGResponse(double);
	void ClickedDetectChecked();
	void ClickedRestoreDefaults();
	void onHeaderClicked(int);

signals:
	void launchDetectChecked(QList<int> checked);

private:
    
	void CreateInputGroup();

	QGridLayout *ViewLayout;

	QTableWidget *imageTable;

	QGroupBox *DetectionSetupGroup;
	QLabel *VotingRadiusLabel;
	QSpinBox *VotingRadiusInput;

	QLabel *GradientMagnitudeLabel;
	QDoubleSpinBox *GradientMagnitudeInput;

	QLabel *ScaleLabel;
	QDoubleSpinBox *ScaleInput;

	QCheckBox *DimConeBox;

	QPushButton *RestoreDefaultButton;

	QLabel *ScaleResponseLabel;
	QDoubleSpinBox *ScaleResponseInput;
	QGridLayout *DetectionSetupLayout;

	QGridLayout *DetectionLaunchLayout;
	QWidget *DetectionLaunchGroup;
	QPushButton *CancelButton;
	QPushButton *LaunchDetectCheckedButton;

	QRect dlgeom;
	bool dlgeomset = false;
	QFont normal, bold;
	QList<int> checkedRows;
};

#endif // radDetectionPanel

