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

	normal = QFont(this->font());
	bold = QFont(normal);
	bold.setBold(true);
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

	RestoreDefaultButton = new QPushButton("Restore Defaults");
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
	DetectionSetupLayout->addWidget(DimConeBox, 2, 2, 1, 2, Qt::AlignLeft);
	DetectionSetupLayout->addWidget(RestoreDefaultButton, 3, 0);
	DetectionSetupGroup->setLayout(DetectionSetupLayout);
	
	DetectionLaunchGroup = new QWidget();

	DetectionLaunchLayout = new QGridLayout;
	DetectionLaunchLayout->setColumnStretch(0, 10);
	DetectionLaunchLayout->setColumnStretch(1, 10);
	DetectionLaunchLayout->setColumnStretch(2, 10);
	DetectionLaunchGroup->setLayout(DetectionLaunchLayout);

	LaunchDetectCheckedButton = new QPushButton(tr("Detect Checked"));
	DetectionLaunchLayout->addWidget(LaunchDetectCheckedButton, 0, 1);
	connect(LaunchDetectCheckedButton, SIGNAL(clicked()), this, SLOT(ClickedDetectChecked()));
	CancelButton = new QPushButton(tr("Cancel"));
	DetectionLaunchLayout->addWidget(CancelButton, 0, 2);
	connect(CancelButton, SIGNAL(clicked()), this, SLOT(close()));

	RestoreDefaultButton->setAutoDefault(false);
	CancelButton->setAutoDefault(false);
	LaunchDetectCheckedButton->setAutoDefault(true);

	imageTable = new QTableWidget(0, 2);

	QStringList headers;
	headers << "\xE2\x88\x9A" << "Split File Name";

	imageTable->setHorizontalHeaderLabels(headers);
	imageTable->setColumnWidth(0, 12);
	imageTable->verticalHeader()->setVisible(false);
	imageTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
	imageTable->setSelectionBehavior(QAbstractItemView::SelectRows);
	imageTable->setSelectionMode(QAbstractItemView::SingleSelection);
	imageTable->setShowGrid(false);

	QHeaderView *hdr = imageTable->horizontalHeader();
	hdr->setSectionResizeMode(0, QHeaderView::ResizeToContents);
	hdr->setSectionResizeMode(1, QHeaderView::Stretch);
	hdr->setSectionsClickable(true);
	connect(hdr, SIGNAL(sectionClicked(int)), SLOT(onHeaderClicked(int)));

    ViewLayout = new QGridLayout;
	ViewLayout->setColumnStretch(0, 0);
	ViewLayout->setColumnStretch(1, 10);
	ViewLayout->setRowStretch(0, 10);
	ViewLayout->setRowStretch(1, 0);
	ViewLayout->setRowStretch(2, 0);
	ViewLayout->addWidget(imageTable, 0, 0, 1, 2);
	ViewLayout->addWidget(DetectionSetupGroup, 1, 0);
	ViewLayout->addWidget(DetectionLaunchGroup, 2, 0);
	setLayout(ViewLayout);
}

void radDetectionPanel::SetItemList(QStringList &items)
{
	imageTable->setRowCount(items.size());
	for (int row = 0; row < items.size(); row++) {
		QCheckBox *cb = new QCheckBox();
		cb->setContentsMargins(8, 2, 2, 0);
		imageTable->setCellWidget(row, 0, cb);
		imageTable->setItem(row, 1, new QTableWidgetItem(items[row]));
	}
	imageTable->resizeColumnsToContents();
	imageTable->resizeRowsToContents();
}
void radDetectionPanel::SetCheckedRows(QList<int> &rows)
{
	checkedRows = rows;
	for (int row=0; row<imageTable->rowCount(); row++) {
		((QCheckBox *)(imageTable->cellWidget(row, 0)))->setChecked(rows.indexOf(row) >= 0);
	}
}
void radDetectionPanel::SetHighlightedRow(int row)
{
	for (int i = 0; i < imageTable->rowCount(); i++) {
		imageTable->item(i, 1)->setFont(normal);
	}
	if (row < 0 || row >= imageTable->rowCount()) return;
	imageTable->selectRow(row);
	imageTable->item(row, 1)->setFont(bold);
	imageTable->scrollToItem(imageTable->item(row, 1));
}
void radDetectionPanel::onHeaderClicked(int hdr)
{
	if (hdr != 0 || imageTable->rowCount() == 0) return;
	int nchecked = 0;
	for (int row = 0; row < imageTable->rowCount(); row++) {
		if (((QCheckBox *)(imageTable->cellWidget(row, 0)))->isChecked()) {
			++nchecked;
			((QCheckBox *)(imageTable->cellWidget(row, 0)))->setChecked(false);
		}
	}
	if (nchecked > 0) return;
	for (int row = 0; row < imageTable->rowCount(); row++) {
		((QCheckBox *)(imageTable->cellWidget(row, 0)))->setChecked(true);
	}
}

void radDetectionPanel::closeEvent(QCloseEvent *event)
{
	dlgeom = geometry();
	dlgeomset = true;
	QDialog::closeEvent(event);
}

void radDetectionPanel::showEvent(QShowEvent *event)
{
	if (dlgeomset) setGeometry(dlgeom);
	QDialog::showEvent(event);
}

void radDetectionPanel::ClickedDetectChecked()
{
	QList<int> checked;
	for (int row = 0; row < imageTable->rowCount(); row++) {
		if (((QCheckBox *)(imageTable->cellWidget(row, 0)))->isChecked()) {
			checked << row;
		}
	}
	if (checked.size() == 0) {
		return;
	}
	close();
	emit launchDetectChecked(checked);
}

void radDetectionPanel::ClickedRestoreDefaults()
{
	radMainWindow::GetPointer()->LoadDefaultDetectionPara();
	UpdateControlPanel(radMainWindow::GetPointer()->GetConeDetectionParameters());
	SetCheckedRows(checkedRows);
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

