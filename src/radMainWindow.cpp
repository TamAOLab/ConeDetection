#include <QtGlobal>
#if QT_VERSION >= 0x050000
#include <QMenu>
#include <QMenuBar>
#include <QToolBar>
#include <QFileDialog>
#include <QMessageBox>
#endif
#include "radmainwindow.h"
#include "radCircularVoting.h"
#include "radSeedClustering.h"
#include "QuickView.h"
#include "raddetectionaverage.h"
#include "radAboutDialog.h"
#include <time.h>

#include "version.h"

std::string ConeDetect_VERSION = __version__;

static void absFileList(QStringList &inputNames, QFileInfoList &outputFiles)
{
	QStringList filters;
	filters << "*.tif" << "*.csv";
	for (int i = 0; i < inputNames.size(); i++) {
		QFileInfo file = QFileInfo(inputNames[i]);
		if (file.isDir()) {
			QDir dir = QDir(file.absoluteFilePath());
			QStringList entries = dir.entryList(filters, QDir::Files);
			for (int j = 0; j < entries.size(); j++) {
				outputFiles.append(QFileInfo(dir, entries[j]));
			}
			continue;
		}
		if (file.exists())
			outputFiles.append(file);
	}
}

// inputNames may contain *.tif/*.csv files or directories
void decodeFileList(QStringList &inputNames, QStringList &splitFileNames, QStringList &detectionFileNames)
{
	QFileInfoList inputFiles;
	absFileList(inputNames, inputFiles);
	for (int i = 0; i < inputFiles.size(); i++) {
		QString fn = inputFiles[i].absoluteFilePath();
		if (fn.endsWith(".tif", Qt::CaseInsensitive))
			splitFileNames.append(fn);
		else if (fn.endsWith(".csv", Qt::CaseInsensitive))
			detectionFileNames.append(fn);
	}
}

radMainWindow* radMainWindow::TheApp = NULL;

// Constructor
radMainWindow::radMainWindow() 
{
	//initialize cone detection parameters
	LoadDefaultDetectionPara();

	// Handle directories
	splitHomeDir = QDir(QDir::home().filePath(".ConeDetection"));
	splitStateFile = QFileInfo(splitHomeDir, QString("state.json"));
	QDir historyDir = QDir(splitHomeDir.filePath("History"));
	// create history directory
	if (!historyDir.exists())
		historyDir.mkpath(".");
	BackupDir = historyDir.path().toStdString();
	// cout << "History dir: " << BackupDir.c_str() << std::endl;
	// cout << "State file: " << splitStateFile.filePath().toStdString().c_str() << std::endl;

	loadDir = saveDir = QDir::current();

	ConeMarkFlag = false;
	ConeEraseFlag = false;
	
	ImageView = new radImageView;

	QScreen *screen = QGuiApplication::primaryScreen();
	QRect  screenGeometry = screen->geometry();
	screen_height = screenGeometry.height();
	screen_width = screenGeometry.width();

	createActions();
    createMenus();
	createView();
	createToolBars();
	createProgressDialog();
	createHelpWindow();

	purgeHistoryDialog = new radPurgeHistoryDialog(this);
	purgeHistoryDialog->setMinimumSize(screen_width / 2, screen_height / 3);

    setWindowTitle(tr("Cone Detection ver. ") + QString(ConeDetect_VERSION.c_str()));
	setMinimumSize(screen_width / 2, screen_height * 2 / 3);
	move(screen_width / 6, screen_height / 8);
    //resize(1000, 750);

	FileIO = new radFileIO;
	DetectionPanel = new radDetectionPanel(this);
	connect(DetectionPanel, SIGNAL(launchDetectCurrent()), this, SLOT(DetectCones()));
	connect(DetectionPanel, SIGNAL(launchDetectAll()), this, SLOT(DetectConesAll()));

	setAcceptDrops(true);

	TheApp = this;

	loadState();
}

radMainWindow::~radMainWindow()
{
	delete helpWindow;
	helpWindow = NULL;

	delete purgeHistoryDialog;
	purgeHistoryDialog = NULL;

	delete progressDialog;
	progressDialog = NULL;

	delete ImageView;
	ImageView = NULL;
	delete FileIO;
	FileIO = NULL;
	delete DetectionPanel;
	DetectionPanel = NULL;

	SplitImageInfor.clear();
}

void radMainWindow::LoadDefaultDetectionPara()
{
	DetectionSettingPara.VotingRadius = 5;
	DetectionSettingPara.GradientMagnitudeThreshold = 5.0;
	DetectionSettingPara.Scale = 2.0;
	DetectionSettingPara.LOGResponse = 1.0;
	DetectionSettingPara.DimConeDetectionFlag = false;
}

void radMainWindow::closeEvent(QCloseEvent *event)
{
	helpWindow->close();
	BackupResults();
	event->accept();
}

void radMainWindow::createActions()
{
    openSplitImageAct = new QAction(tr("&Open"), this);
	openSplitImageAct->setIcon(QIcon(":open.png"));
    openSplitImageAct->setShortcuts(QKeySequence::Open);
	openSplitImageAct->setToolTip(tr("Open Split Image(s) (Ctrl+O)"));
    connect(openSplitImageAct, SIGNAL(triggered()), this, SLOT(openSplitImage()));

    loadDetectionAct = new QAction(tr("&Open Detection Results"), this);
    connect(loadDetectionAct, SIGNAL(triggered()), this, SLOT(loadDetection())); 
	
    saveDetectionAct = new QAction(tr("&Save"), this);
	saveDetectionAct->setToolTip(tr("Save Current Detection Results (Ctrl+S)"));
    saveDetectionAct->setShortcuts(QKeySequence::Save);
	saveDetectionAct->setIcon(QIcon(":saveas.png"));
    connect(saveDetectionAct, SIGNAL(triggered()), this, SLOT(saveDetection()));

	saveAllDetectionsAct = new QAction(tr("Save All"), this);
	saveAllDetectionsAct->setToolTip(tr("Save All Detection Results (Ctrl+A)"));
	saveAllDetectionsAct->setIcon(QIcon(":saveall.png"));
	saveAllDetectionsAct->setShortcut(QKeySequence(tr("Ctrl+A")));
	connect(saveAllDetectionsAct, SIGNAL(triggered()), this, SLOT(saveAllDetections()));

    quitAct = new QAction(tr("&Close"), this);
    quitAct->setShortcuts(QKeySequence::Quit);
    quitAct->setToolTip(tr("Close the program (Alt+F4)"));
    connect(quitAct, SIGNAL(triggered()), this, SLOT(quit()));

	detectConesAct = new QAction(tr("&Detect"), this);
	detectConesAct->setShortcut(QKeySequence(tr("Ctrl+D")));
	detectConesAct->setIcon(QIcon(":detect.png"));
	detectConesAct->setToolTip(tr("Detect Cones (Ctrl+D)"));
    connect(detectConesAct, SIGNAL(triggered()), this, SLOT(showDetectionPanel()));

	purgeHistoryAct = new QAction(tr("Purge History"), this);
	connect(purgeHistoryAct, SIGNAL(triggered()), this, SLOT(purgeHistoryFiles()));

	QActionGroup *drawActionGroup = new QActionGroup(this);
	mouseAct = new QAction(tr("Adjust"), drawActionGroup);
    mouseAct->setIcon(QIcon(":mouse.png"));
	mouseAct->setShortcut(QKeySequence(tr("Ctrl+B")));
    mouseAct->setToolTip(tr("Mouse adjusts brightness/contrast (Ctrl+B)"));
	mouseAct->setCheckable(true);
	mouseAct->setChecked(true);
    connect(mouseAct, SIGNAL(triggered()), this, SLOT(SetMouseFlag()));

	pointAnnotationAct = new QAction(tr("Mark"), drawActionGroup);
    pointAnnotationAct->setIcon(QIcon(":draw_point.png"));
	pointAnnotationAct->setShortcut(QKeySequence(tr("Ctrl+M")));
    pointAnnotationAct->setToolTip(tr("Mouse marks cones (Ctrl+M)"));
	pointAnnotationAct->setCheckable(true);
	pointAnnotationAct->setChecked(false);
    connect(pointAnnotationAct, SIGNAL(triggered()), this, SLOT(SetPointAnnotationFlag()));

	pointEraseAct = new QAction(tr("Erase"), drawActionGroup);
	pointEraseAct->setShortcut(QKeySequence(tr("Ctrl+E")));
    pointEraseAct->setIcon(QIcon(":erase_point.png"));
    pointEraseAct->setToolTip(tr("Mouse erases cone markers (Ctrl+E)"));
	pointEraseAct->setCheckable(true);
	pointEraseAct->setChecked(false);
    connect(pointEraseAct, SIGNAL(triggered()), this, SLOT(SetPointEraseFlag()));

    undoAct = new QAction(tr("Undo"), this);
	undoAct->setShortcut(QKeySequence(tr("Ctrl+Z")));
    undoAct->setIcon(QIcon::fromTheme(":undo.png"));
    undoAct->setShortcuts(QKeySequence::Undo);
    undoAct->setToolTip(tr("Undo last operation (Ctrl+Z)"));
    connect(undoAct, SIGNAL(triggered()), this, SLOT(DoUndo()));

	aboutAct = new QAction(tr("About"), this);
	aboutAct->setIcon(QIcon(":about.png"));
	connect(aboutAct, SIGNAL(triggered()), this, SLOT(ShowAboutDialog()));

	helpAct = new QAction(tr("Help"), this);
	helpAct->setShortcut(QKeySequence(tr("F1")));
	helpAct->setToolTip(tr("Display help screen (F1)"));
	helpAct->setIcon(QIcon(":help.png"));
	connect(helpAct, SIGNAL(triggered()), this, SLOT(ShowHelpWindow()));
}

void radMainWindow::createMenus()
{
    fileMenu = menuBar()->addMenu(tr("&File"));
	SplitImageInputMenu = fileMenu->addMenu(tr("Split &Images"));
    SplitImageInputMenu->addAction(openSplitImageAct);

	saveMenu = fileMenu->addMenu(tr("Detection Results"));
	saveMenu->addAction(loadDetectionAct);
	saveMenu->addAction(saveDetectionAct);
	saveMenu->addAction(saveAllDetectionsAct);

    fileMenu->addAction(quitAct);

	SplitProcessingMenu = menuBar()->addMenu(tr("&Split"));
	SplitProcessingMenu->addAction(detectConesAct);
	SplitProcessingMenu->addSeparator();
	SplitProcessingMenu->addAction(purgeHistoryAct);

	helpMenu = menuBar()->addMenu(tr("&Help"));
	helpMenu->addAction(helpAct);
	helpMenu->addAction(aboutAct);
}

void radMainWindow::createView()
{
	QWidget *centralwidget = new QWidget;
	centralwidget->setAttribute(Qt::WA_DeleteOnClose, true);
    centralwidget->setObjectName(QString::fromUtf8("centralwidget"));
    setCentralWidget(centralwidget);
	ImageWidget = new QVTKWidget(centralwidget);
	// ImageWidget = new QVTKOpenGLSimpleWidget(centralwidget);
	ImageWidget->SetRenderWindow(ImageView->GetRenderWin());

	SplitFileLabel = new QLabel(tr("Split files:"));
	SplitFileListWidget = new QListWidget();
	SplitFileListWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    connect(SplitFileListWidget, SIGNAL(currentItemChanged(QListWidgetItem*, QListWidgetItem*)), this, SLOT(SwitchSplitFile(QListWidgetItem*, QListWidgetItem*)));

	FileListLayout = new QVBoxLayout;
	FileListLayout->addWidget(SplitFileLabel);
	FileListLayout->addWidget(SplitFileListWidget, 4);

	viewLayout = new QGridLayout(centralwidget);
	viewLayout->setGeometry(QRect(10, 40, 40, 300));
    viewLayout->setObjectName(QString::fromUtf8("viewLayout"));
	viewLayout->addWidget(ImageWidget, 0, 0);
	viewLayout->addLayout(FileListLayout, 0, 1);
	viewLayout->setColumnStretch(0, 5);
	viewLayout->setColumnStretch(1, 1);
}

void radMainWindow::createToolBars()
{
	QToolBar *drawToolBar = addToolBar(tr("&Draw"));
	drawToolBar->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
	drawToolBar->addAction(openSplitImageAct);
	drawToolBar->addAction(saveDetectionAct);
	drawToolBar->addAction(saveAllDetectionsAct);
	drawToolBar->addAction(detectConesAct);

	drawToolBar->addSeparator();
	drawToolBar->addAction(mouseAct);
	drawToolBar->addAction(pointAnnotationAct);
    drawToolBar->addAction(pointEraseAct);

	drawToolBar->addSeparator();
	drawToolBar->addAction(undoAct);

	lbGlSpace = new QLabel("");
	lbGlSpace->setMinimumSize(20, 20);
	drawToolBar->addWidget(lbGlSpace);

	drawToolBar->addSeparator();
	cbShowGlyphs = new QCheckBox("Show Cone Glyphs");
	cbShowGlyphs->setChecked(true);
	connect(cbShowGlyphs, SIGNAL(clicked(bool)), this, SLOT(changeConeGlyphVisibility(bool)));

	lbGlSize = new QLabel("Glyph Size");
	spGlSize = new QDoubleSpinBox();
	spGlSize->setRange(0.1, 20);
	spGlSize->setSingleStep(0.5);
	spGlSize->setValue(6.0);
	spGlSize->setAlignment(Qt::AlignCenter);
	connect(spGlSize, SIGNAL(valueChanged(double)), this, SLOT(changeConeGlyphSize(double)));

	layGlSize = new QGridLayout();
	layGlSize->addWidget(spGlSize, 0, 0);
	layGlSize->addWidget(lbGlSize, 0, 1);

	layGlSize->addWidget(cbShowGlyphs, 1, 0, 1, 2);

	wgGlSize = new QWidget();
	wgGlSize->setLayout(layGlSize);
	drawToolBar->addWidget(wgGlSize);

	drawToolBar->addSeparator();
	drawToolBar->addAction(helpAct);
}

void radMainWindow::createProgressDialog()
{
	progressDialog = new QProgressDialog("", "", 0, 100, this);
	Qt::WindowFlags flags = progressDialog->windowFlags();
	progressDialog->setWindowFlags(flags & ~(Qt::WindowCloseButtonHint |
		Qt::WindowMaximizeButtonHint | Qt::WindowMinimizeButtonHint |
		Qt::WindowSystemMenuHint | Qt::WindowContextHelpButtonHint));
	progressDialog->setCancelButton(0);
	progressDialog->reset();
	progressDialog->setWindowIcon(QIcon(":detect.png"));
	progressDialog->setWindowTitle("Cone Detection Progress");
	progressDialog->setMinimumSize(screen_width / 3, screen_height / 12);
	progressDialog->setMinimumDuration(10);
	connect(this, SIGNAL(sendFinishDetection()), this, SLOT(receiveFinishDetection()));
	connect(this, SIGNAL(updateProgress()), this, SLOT(handleUpdateProgress()));
	connect(this, &radMainWindow::updateProgressText, progressDialog, &QProgressDialog::setLabelText);
}

void radMainWindow::createHelpWindow()
{
	helpWindow = new QWidget();
	helpWindow->setWindowTitle(tr("Help on Cone Detection"));
	helpWindow->setWindowIcon(QIcon(":help.png"));
	helpLayout = new QVBoxLayout();
	helpBrowser = new QTextBrowser();
	helpBrowser->setOpenExternalLinks(true);
	helpLayout->addWidget(helpBrowser);
	helpWindow->setLayout(helpLayout);

	QDir appDir(QCoreApplication::applicationDirPath());
	QDir helpDir = appDir.absoluteFilePath(tr("Help"));
	QFileInfo helpFile(helpDir.path(), tr("detect.html"));
	if (!helpFile.exists()) {
		appDir.cdUp();
		helpDir = appDir.absoluteFilePath(tr("Help"));
		helpFile = QFileInfo(helpDir.path(), tr("detect.html"));
	}
	if (helpFile.exists()) {
		helpBrowser->setSource(QUrl::fromLocalFile(helpFile.filePath()));
	}
	else {
		helpBrowser->setText("Sorry, no help available at this time.");
	}

	helpWindow->setMinimumSize(screen_width * 2 / 5, screen_height * 3 / 7);
	helpWindow->move(screen_width / 5, screen_height * 2 / 7);
}

void radMainWindow::ShowAboutDialog()
{
	radAboutDialog dlg(this);
	dlg.exec();
}

void radMainWindow::ShowHelpWindow()
{
	helpWindow->showNormal();
	helpWindow->activateWindow();
}

void radMainWindow::changeConeGlyphVisibility(bool v)
{
	GetImageView()->SetConeGlyphVisibility(v);
	GetImageView()->ResetView(false);
}
void radMainWindow::changeConeGlyphSize(double sz)
{
	GetImageView()->SetGlyphScale(sz);
	GetImageView()->ResetView(false);
}

void radMainWindow::dragEnterEvent(QDragEnterEvent *event)
{
	event->acceptProposedAction();
}

void radMainWindow::dropEvent(QDropEvent *e)
{
	QStringList inputNames;
	QStringList splitFileNames;
	QStringList detectionFileNames;

	for (int i=0; i< e->mimeData()->urls().size(); i++) {
		inputNames.append(e->mimeData()->urls()[i].toLocalFile());
	}
	decodeFileList(inputNames, splitFileNames, detectionFileNames);

	openSplitImages(splitFileNames);
	loadDetections(detectionFileNames);
}

void radMainWindow::openSplitImages(QStringList & fileNames)
{
	if (fileNames.isEmpty()) return;

	ClearSplitFileList();
	SplitImageInfor.resize(fileNames.size());

	LoadDefaultDetectionPara();

	int j = 0;

	for (int i = 0; i < fileNames.size(); i++)
	{
		std::pair<FloatImageType2D::Pointer, FloatImageType2D::Pointer> res = FileIO->ReadSplitImage(fileNames[i].toStdString());
		if (res.first)
		{
			SplitImageInfor[j].split_images = res;
			SplitImageInfor[j].split_files.first = fileNames[i].toStdString();
			++j;
		}
	}
	SplitImageInfor.resize(j);

	UpdateSplitFileList(0);

	SplitFileListWidget->setCurrentRow(0);
	SplitFileListWidget->item(0)->setSelected(true);

	LoadBackupResults(0);
	LoadSplitFile(0);
}

void radMainWindow::openSplitImage()
{
	QFileDialog dialog(this);
    dialog.setFileMode(QFileDialog::ExistingFiles);
	dialog.setNameFilter(tr("TIFF (*.tif);; All Files (*)"));
	dialog.setWindowTitle("Open Split Image");
	dialog.restoreState(fileDialogState);
	dialog.setDirectory(loadDir);

	QStringList fileNames; 
	if (dialog.exec())
	{
		fileNames = dialog.selectedFiles();
	}

    if ( !fileNames.isEmpty() )
    {
		loadDir = dialog.directory();
		fileDialogState = dialog.saveState();
		saveState();

		openSplitImages(fileNames);
	}
}

void radMainWindow::saveDetection()
{
	if (SplitFileListWidget->count() == 0 || SplitFileListWidget->currentRow() < 0
		|| SplitFileListWidget->currentRow() >= SplitFileListWidget->count())
		return;

	QString preferredName;
	string tmp_str;
	// cout << SplitFileListWidget->currentItem()->text().toStdString() << std::endl;
	tmp_str.assign(SplitFileListWidget->currentItem()->text().toStdString());
	tmp_str = tmp_str + ".csv";

	preferredName = saveDir.filePath(QString::fromStdString(tmp_str));

	QFileDialog dialog(this);
	dialog.setFileMode(QFileDialog::AnyFile);
	dialog.setAcceptMode(QFileDialog::AcceptSave);
	dialog.setWindowTitle("Save Cone Detections");
	dialog.selectFile(preferredName);
	dialog.selectNameFilter("Detections (*.csv *.txt)");
	dialog.restoreState(fileDialogState);
	dialog.setDirectory(saveDir);

	if (!dialog.exec()) return;

	QString filename = dialog.selectedFiles()[0];

	if ( !filename.isNull() && !SplitImageInfor.empty() && SplitImageInfor.size() > SplitFileListWidget->currentRow())
    {
		QFileInfo saveFile = QFileInfo(filename);
		saveDir.setPath(saveFile.dir().path());
		fileDialogState = dialog.saveState();
		saveState();

		GetCurrentEditing();
		FileIO->WriteConeDetections(filename.toStdString().c_str(), 
			SplitImageInfor[SplitFileListWidget->currentRow()].split_edited_detections);
    }
}

void radMainWindow::saveAllDetections()
{
	if (SplitFileListWidget->count() == 0)
		return;

	QFileDialog dialog(this);
	dialog.setFileMode(QFileDialog::Directory);
	dialog.setWindowTitle("Select Output Directory");
	dialog.setOptions(QFileDialog::DontUseNativeDialog | QFileDialog::DontResolveSymlinks);
	// ! Non-native dialog: shows files, but save/restore state does not work
	// dialog.restoreState(fileDialogState);
	dialog.setMinimumSize(screen_width * 3 / 7, screen_height * 2 / 5);
	dialog.setDirectory(saveDir);

	if (!dialog.exec()) return;

	QString dir = dialog.selectedFiles()[0];

	if (!dir.isNull()) {
		saveDir.setPath(dir);
		// fileDialogState = dialog.saveState();
		saveState();

		pair<QFileInfo, DoublePointArray2D> *todo = new pair<QFileInfo, DoublePointArray2D>[SplitFileListWidget->count()];
		string existing = "";
		int cnt = 0;

		GetCurrentEditing();
		for (int row = 0; row < SplitFileListWidget->count(); row++) {
			string fn = SplitFileListWidget->item(row)->text().toStdString() + ".csv";
			QFileInfo saveFile = QFileInfo(saveDir, QString(fn.c_str()));
			if (saveFile.exists()) {
				++cnt;
				if (cnt <= 10) {
					if (existing.size() > 0) existing = existing + "\n";
					existing = existing + fn;
				}
			}
			todo[row] = std::make_pair(saveFile, SplitImageInfor[row].split_edited_detections);
		}

		if (existing.size() > 0) {
			QMessageBox msgBox;
			string msg = "The following file(s) already exist(s):\n";
			msg = msg + existing;
			msgBox.setText(msg.c_str());
			msgBox.setInformativeText("Do you want to write over existing files?");
			msgBox.setIcon(QMessageBox::Warning);
			msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
			msgBox.setDefaultButton(QMessageBox::Yes);
			if (msgBox.exec() == QMessageBox::Yes)
				existing.clear();
		}
		if (existing.size() == 0) {
			for (int row = 0; row < SplitFileListWidget->count(); row++) {
				QFileInfo saveFile = todo[row].first;
				DoublePointArray2D results = todo[row].second;
				FileIO->WriteConeDetections(saveFile.filePath().toStdString().c_str(), results);
			}
		}

		delete [] todo;
	}
}

void radMainWindow::loadDetections(QStringList & fileNames)
{
	if (SplitImageInfor.size() == 0) return;
	if (fileNames.isEmpty()) return;

	radBackup back_up;
	back_up.SetBackupDir(BackupDir);
	ConeDetectionParameters detection_paras;

	int firstRow = -1;
	for (int i = 0; i < fileNames.size(); i++) {
		QFileInfo fpath = QFileInfo(fileNames[i]);
		QString fn = fpath.fileName();
		if (fn.endsWith(".csv", Qt::CaseInsensitive))
			fn.resize(fn.size() - 4);
		int curRow = -1;
		// Find an open Split image using file name without extension (.csv)
		for (int row = 0; row < SplitFileListWidget->count(); row++) {
			if (fn == SplitFileListWidget->item(row)->text()) {
				curRow = row;
				break;
			}
		}
		if (curRow < 0) continue;
		if (firstRow < 0 || curRow == SplitFileListWidget->currentRow())
			firstRow = curRow;
		QString filename = fpath.absoluteFilePath();
		back_up.ReadSplitBackup(SplitImageInfor[curRow], detection_paras, 1);
		FileIO->ReadConeDetections(filename.toStdString().c_str(),
			SplitImageInfor[curRow].split_edited_detections);
		back_up.WriteBackupResults(SplitImageInfor[curRow], detection_paras, 0);
	}

	if (firstRow < 0) return;

	if (firstRow >= 0 && firstRow != SplitFileListWidget->currentRow()) {
		SplitFileListWidget->setCurrentRow(firstRow);
		SplitFileListWidget->item(firstRow)->setSelected(true);
		back_up.ReadSplitBackup(SplitImageInfor[firstRow], DetectionSettingPara, 0);
	}
	ImageView->SetConePoints(SplitImageInfor[SplitFileListWidget->currentRow()].split_left_detections,
		SplitImageInfor[SplitFileListWidget->currentRow()].split_left_detection_scales,
		SplitImageInfor[SplitFileListWidget->currentRow()].split_left_detection_weights,
		SplitImageInfor[SplitFileListWidget->currentRow()].split_right_detections,
		SplitImageInfor[SplitFileListWidget->currentRow()].split_right_detection_scales,
		SplitImageInfor[SplitFileListWidget->currentRow()].split_right_detection_weights,
		SplitImageInfor[SplitFileListWidget->currentRow()].split_detection_links,
		SplitImageInfor[SplitFileListWidget->currentRow()].split_edited_detections);

	ImageView->ResetView();
}

void radMainWindow::loadDetection()
{
	if (SplitImageInfor.size() == 0) return;

	QFileDialog dialog(this);
	dialog.setFileMode(QFileDialog::ExistingFiles);
	dialog.setNameFilter(tr("Detections (*.csv *.txt)"));
	dialog.setWindowTitle("Open Cone Detections");
	dialog.restoreState(fileDialogState);
	dialog.setDirectory(saveDir);

	QStringList fileNames;
	if (dialog.exec()) {
		fileNames = dialog.selectedFiles();
		if (fileNames.isEmpty()) return;

		saveDir = dialog.directory();
		fileDialogState = dialog.saveState();
		saveState();
	}
	loadDetections(fileNames);
}

void radMainWindow::LoadSplitFile(int id)
{
	LoadDefaultDetectionPara();
	LoadBackupResults(id);
	ImageView->SetSplitImage(SplitImageInfor[id].split_images.first);
	ImageView->SetConePoints(SplitImageInfor[id].split_left_detections, SplitImageInfor[id].split_left_detection_scales, 
		SplitImageInfor[id].split_left_detection_weights, SplitImageInfor[id].split_right_detections, 
		SplitImageInfor[id].split_right_detection_scales, SplitImageInfor[id].split_right_detection_weights, 
		SplitImageInfor[id].split_detection_links, SplitImageInfor[id].split_edited_detections);
	ImageView->ResetView();
}

void radMainWindow::quit()
{
	helpWindow->close();
	BackupResults();
    close();
}

void radMainWindow::UpdateSplitFileList(unsigned int old_size)
{
	SplitFileListWidget->clear();
	for (int i=old_size; i<SplitImageInfor.size(); i++)
	{
		string str;
		size_t pos, pos1;

		pos = SplitImageInfor.at(i).split_files.first.rfind(".");
		pos1 = SplitImageInfor.at(i).split_files.first.rfind("/");
		if (pos != std::string::npos && pos1 != std::string::npos)
		{
			str.assign(SplitImageInfor.at(i).split_files.first.begin()+pos1+1, 
				SplitImageInfor.at(i).split_files.first.begin()+pos);
			SplitImageInfor.at(i).split_files.second = str;
			SplitFileListWidget->addItem(new QListWidgetItem(str.c_str()));
		}
	}
}


void radMainWindow::ClearSplitFileList()
{
	BackupResults();
	SplitImageInfor.clear();
	SplitFileListWidget->clear();
}

void radMainWindow::GetCurrentEditing()
{
	if (SplitFileListWidget->count() > 0 && SplitFileListWidget->currentRow() >= 0
			&& SplitFileListWidget->currentRow() < SplitImageInfor.size()) {
		ImageView->GetConePoints(SplitImageInfor[SplitFileListWidget->currentRow()].split_edited_detections);
	}
}

void radMainWindow::SwitchSplitFile(QListWidgetItem *item, QListWidgetItem*previous)
{
	int i;

	if (previous)
		for (i = 0; i < SplitImageInfor.size(); i++) {
			if (SplitImageInfor.at(i).split_files.second.compare(previous->text().toStdString()) == 0) {
				ImageView->GetConePoints(SplitImageInfor[i].split_edited_detections);
				BackupResults(i);
			}
		}
	if (item) {
		for (i = 0; i < SplitImageInfor.size(); i++)
		{
			if (SplitImageInfor.at(i).split_files.second.compare(item->text().toStdString()) == 0)
			{
				LoadSplitFile(i);
			}
		}

		qDebug() << item->text();
	}
}

void radMainWindow::showDetectionPanel()
{
	DetectionPanel->UpdateControlPanel(DetectionSettingPara);
	DetectionPanel->show();
}

void radMainWindow::purgeHistoryFiles()
{
	purgeHistoryDialog->showHistory(BackupDir);
}

void radMainWindow::DetectSplitImage(unsigned int img_id)
{
	radCircularVoting rcv;
	radSeedClustering cluster;

	SplitImageInfor[img_id].Initialize(false);

	clock_t t_start, t_end;

	t_start = clock();
	//step 1: detection bright regions
	rcv.SetParams(radCircularVoting::RegionType(PARA_RegionTypeBright), 
		PARA_VotingMinimumValue, PARA_VotingMaximumValue,
		DetectionSettingPara.VotingRadius, DetectionSettingPara.GradientMagnitudeThreshold, 
		PARA_VotingThreshold, DetectionSettingPara.Scale,
		radCircularVoting::WeightingMethod(PARA_WeightingMethod),
		PARA_WeightingParameter, PARA_ZeroCrossing,
		PARA_AbsoulteThresholdFlag);
	rcv.SetPrefix(BackupDir);
	rcv.SaveHistory(false);
	rcv.Compute(SplitImageInfor[img_id].split_images.first);

	ShortPointArray2D seeds = rcv.GetCenters();
	vector<float> weights = rcv.GetCenterWeights();

	//clustering neighbored points
	cluster.SetClusteringRadius(PARA_ClustingRadius);
	cluster.SetScaleResponse(DetectionSettingPara.LOGResponse);
	cluster.SetMergeRadius(PARA_MergeRadius);
	cluster.SetScaleSize(PARA_LOGScale);
	cluster.ClusteringSeeds(SplitImageInfor[img_id].split_images, seeds, weights);
	
	SplitImageInfor[img_id].split_right_detections.resize(cluster.GetClusteringSeeds().size());
	SplitImageInfor[img_id].split_right_detection_scales.resize(cluster.GetCenterScales().size());
	SplitImageInfor[img_id].split_right_detection_weights.resize(cluster.GetSeedWeights().size());
	
	for (unsigned int i=0; i<cluster.GetClusteringSeeds().size(); i++)
	{
		SplitImageInfor[img_id].split_right_detections[i][0] = cluster.GetClusteringSeeds()[i][0];
		SplitImageInfor[img_id].split_right_detections[i][1] = cluster.GetClusteringSeeds()[i][1];
		SplitImageInfor[img_id].split_right_detection_scales[i] = cluster.GetCenterScales()[i];
		SplitImageInfor[img_id].split_right_detection_weights[i] = cluster.GetSeedWeights()[i];
	}

	emit updateProgress();

	//step 2: detection dark regions
	rcv.SetParams(radCircularVoting::RegionType(PARA_RegionTypeDark), 
		PARA_VotingMinimumValue, PARA_VotingMaximumValue,
		DetectionSettingPara.VotingRadius, DetectionSettingPara.GradientMagnitudeThreshold, 
		PARA_VotingThreshold, DetectionSettingPara.Scale,
		radCircularVoting::WeightingMethod(PARA_WeightingMethod),
		PARA_WeightingParameter, PARA_ZeroCrossing,
		PARA_AbsoulteThresholdFlag);
	rcv.SetPrefix(BackupDir);
	rcv.SaveHistory(false);
	rcv.Compute(SplitImageInfor[img_id].split_images.first);

	seeds = rcv.GetCenters();
	weights = rcv.GetCenterWeights();

	//clustering neighbored points
	cluster.SetClusteringRadius(PARA_ClustingRadius);
	cluster.SetScaleResponse(DetectionSettingPara.LOGResponse);
	cluster.SetMergeRadius(PARA_MergeRadius);
	cluster.SetScaleSize(PARA_LOGScale);
	cluster.ClusteringSeeds(SplitImageInfor[img_id].split_images, seeds, weights);
	
	SplitImageInfor[img_id].split_left_detections.resize(cluster.GetClusteringSeeds().size());
	SplitImageInfor[img_id].split_left_detection_scales.resize(cluster.GetCenterScales().size());
	SplitImageInfor[img_id].split_left_detection_weights.resize(cluster.GetSeedWeights().size());
	
	for (unsigned int i=0; i<cluster.GetClusteringSeeds().size(); i++)
	{
		SplitImageInfor[img_id].split_left_detections[i][0] = cluster.GetClusteringSeeds()[i][0];
		SplitImageInfor[img_id].split_left_detections[i][1] = cluster.GetClusteringSeeds()[i][1];
		SplitImageInfor[img_id].split_left_detection_scales[i] = cluster.GetCenterScales()[i];
		SplitImageInfor[img_id].split_left_detection_weights[i] = cluster.GetSeedWeights()[i];
	}

	emit updateProgress();

	//step 3: determine the pair
	radDetectionAverage::AdjustDetections(SplitImageInfor[img_id], DetectionSettingPara.DimConeDetectionFlag);

	emit updateProgress();

	t_end = clock();
	// cout << "Detection time: " << (double)(t_end - t_start)/CLOCKS_PER_SEC << std::endl;

	// Make a copy of final detections for editing, then save backup
	SplitImageInfor[img_id].split_edited_detections = SplitImageInfor[img_id].split_final_detections;
	BackupResults(img_id);
}

void radMainWindow::receiveFinishDetection() {
	progressDialog->reset();

	if (SplitFileListWidget->count() == 0 || SplitFileListWidget->currentRow() < 0
		|| SplitFileListWidget->currentRow() >= SplitImageInfor.size())
		return;

	ImageView->SetConePoints(SplitImageInfor[SplitFileListWidget->currentRow()].split_left_detections,
		SplitImageInfor[SplitFileListWidget->currentRow()].split_left_detection_scales,
		SplitImageInfor[SplitFileListWidget->currentRow()].split_left_detection_weights,
		SplitImageInfor[SplitFileListWidget->currentRow()].split_right_detections,
		SplitImageInfor[SplitFileListWidget->currentRow()].split_right_detection_scales,
		SplitImageInfor[SplitFileListWidget->currentRow()].split_right_detection_weights,
		SplitImageInfor[SplitFileListWidget->currentRow()].split_detection_links,
		SplitImageInfor[SplitFileListWidget->currentRow()].split_edited_detections);

	ImageView->ResetView();
}

void radMainWindow::handleUpdateProgress() {
	progressDialog->setValue(progressDialog->value() + 1);
}

void radMainWindow::DetectSplitImage()
{
	//cout << SplitFileListWidget->count() << ", " << SplitFileListWidget->currentRow() << std::endl;
	if (SplitFileListWidget->count() == 0 || SplitFileListWidget->currentRow() < 0
		|| SplitFileListWidget->currentRow() >= SplitImageInfor.size())
		return;

	emit updateProgressText(SplitFileListWidget->item(SplitFileListWidget->currentRow())->text());
	DetectSplitImage(SplitFileListWidget->currentRow());

}

void radMainWindow::DetectSplitImageAll()
{
	if (SplitFileListWidget->count() == 0 || SplitFileListWidget->currentRow() < 0
		|| SplitFileListWidget->currentRow() >= SplitImageInfor.size())
		return;
	
	clock_t starttime = clock();
	for (int i=0; i<SplitImageInfor.size(); i++)
	{
		QString txt = SplitFileListWidget->item(i)->text() + "\n";
		if (i > 0) {
			clock_t elapsed = clock() - starttime;
			long e_seconds = long(double(elapsed)*(SplitImageInfor.size() - i) / i / CLOCKS_PER_SEC + 0.5);
			long e_minutes = e_seconds / 60; e_seconds %= 60;
			long e_hours = e_minutes / 60; e_minutes %= 60;
			txt.append("Estimated Remaining Time: ");
			if (e_hours > 0) {
				txt.append(QString::number(e_hours) + "h ");
				txt.append(QString::number(e_minutes) + "m ");
			} else if (e_minutes > 0)
				txt.append(QString::number(e_minutes) + "m ");
			txt.append(QString::number(e_seconds) + "s");
		}
		emit updateProgressText(txt);
		DetectSplitImage(i);
	}

}

void radMainWindow::DetectCones()
{
	progressDialog->reset();
	progressDialog->setMaximum(3);
	QThread *thread = QThread::create([this] {
		DetectSplitImage();
		emit sendFinishDetection();
	});
	thread->start();
	progressDialog->exec();
}

void radMainWindow::DetectConesAll()
{
	progressDialog->reset();
	progressDialog->setMaximum(SplitImageInfor.size() * 3);
	QThread *thread = QThread::create([this] {
		DetectSplitImageAll();
		emit sendFinishDetection();
	});
	thread->start();
	progressDialog->exec();
}

void radMainWindow::BackupResults(unsigned int img_id)
{
	radBackup back_up;
	back_up.SetBackupDir(BackupDir);
	
	back_up.WriteBackupResults(SplitImageInfor[img_id], DetectionSettingPara);
}

void radMainWindow::BackupResults()
{
	if (0 == SplitFileListWidget->count() || 0 == SplitImageInfor.size()) return;
	radBackup back_up;
	back_up.SetBackupDir(BackupDir);

	GetCurrentEditing();
	for (int img_id = 0; img_id < SplitImageInfor.size(); img_id++) {
		back_up.WriteBackupResults(SplitImageInfor[img_id], DetectionSettingPara);
	}
}

void radMainWindow::LoadBackupResults(int id)
{
	radBackup back_up;
	back_up.SetBackupDir(BackupDir);
	back_up.ReadSplitBackup(SplitImageInfor[id], DetectionSettingPara);

	//update rad detection panel
	DetectionPanel->UpdateControlPanel(DetectionSettingPara);
}


void radMainWindow::SetPointAnnotationFlag()
{
	ConeMarkFlag = true;
	ConeEraseFlag = false;
}


void radMainWindow::SetPointEraseFlag()
{
	ConeMarkFlag = false;
	ConeEraseFlag = true;
}


void radMainWindow::SetMouseFlag()
{
	ConeMarkFlag = false;
	ConeEraseFlag = false;
}

void radMainWindow::DoUndo() { 
  GetImageView()->DoUndo();
}

void radMainWindow::saveState() {
	QJsonObject jobj;
	jobj["loadDir"] = loadDir.path();
	jobj["saveDir"] = saveDir.path();
	jobj["fileDialogState"] = QString(fileDialogState.toBase64());
	QJsonDocument json = QJsonDocument(jobj);

	// cout << json.toJson(QJsonDocument::Indented).toStdString().c_str() << std::endl;
	QFile fo(splitStateFile.filePath());
	if (fo.open(QIODevice::WriteOnly)) {
		fo.write(json.toJson(QJsonDocument::Indented));
		fo.close();
	}
}

void radMainWindow::loadState() {
	QFile fi(splitStateFile.filePath());
	if (fi.open(QIODevice::ReadOnly)) {
		QJsonDocument json = QJsonDocument::fromJson(fi.readAll());
		fi.close();

		QJsonObject jobj = json.object();
		// cout << json.toJson(QJsonDocument::Indented).toStdString().c_str() << std::endl;
		if (jobj["loadDir"].isString())
			loadDir.setPath(jobj["loadDir"].toString());
		if (jobj["saveDir"].isString())
			saveDir.setPath(jobj["saveDir"].toString());
		if (jobj["fileDialogState"].isString())
			fileDialogState = QByteArray::fromBase64(QByteArray(jobj["fileDialogState"].toString().toStdString().c_str()));
	}
}
