#ifndef RADMAINWINDOW_H
#define RADMAINWINDOW_H

#include <QtGlobal>
#include <QMainWindow>
#include <QtGui>
#include "QVTKWidget.h"
#include <QPushButton.h>
#include <QSlider>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTextBrowser>
#include <QCheckBox>
#include <QLineEdit>
#include <QProgressBar>
#include <QProgressDialog>
#include <QDoubleValidator>
#include <QTreeWidget>
#if QT_VERSION >= 0x050000
#include <QMenu>
#endif
#include "radimageview.h"
#include "radFileIO.h"
#include "QActionGroup.h"
#include "radDetectionPanel.h"
#include "radbackup.h"
#include "radPurgeHistoryDialog.h"

class QAction;
class QActionGroup;
class QLabel;
class QGridLayout;
class QVTKWidget;

void decodeFileList(QStringList &inputNames, QStringList &splitFileNames, QStringList &detectionFileNames);

class radMainWindow : public QMainWindow
{
  Q_OBJECT
public:

	// Constructor/Destructor
	radMainWindow(); 
	~radMainWindow();
    
	static radMainWindow* GetPointer() { return TheApp; }
	radImageView * GetImageView() {return ImageView;}
	radFileIO * GetFileIO() {return FileIO;}
	inline ConeDetectionParameters & GetConeDetectionParameters() {return DetectionSettingPara;}

	void LoadDefaultDetectionPara();
	void saveState();
	void loadState();

	bool ConeMarkFlag;
	bool ConeEraseFlag;

	void openSplitImages(QStringList & fileNames);
	void loadDetections(QStringList & fileNames);

protected:
	virtual void closeEvent(QCloseEvent *event);
	void dragEnterEvent(QDragEnterEvent * event);
	void dropEvent(QDropEvent *e);
	
private slots:
	void openSplitImage();
	void saveDetection();
	void saveAllDetections();
	void loadDetection();
	void quit();

	void DetectCones();
	void DetectConesAll();
	void BackupResults(unsigned int img_id);
	void showDetectionPanel();
	void purgeHistoryFiles();
	void SwitchSplitFile(QListWidgetItem*, QListWidgetItem*);
	
	void SetMouseFlag();
	void SetPointAnnotationFlag();
	void SetPointEraseFlag();
	void DoUndo();

	void changeConeGlyphVisibility(bool v);
	void changeConeGlyphSize(double sz);

	void receiveFinishDetection();
	void handleUpdateProgress();

	void ShowAboutDialog();
	void ShowHelpWindow();

signals:
	void sendFinishDetection();
	void updateProgress();
	void updateProgressText(QString text);
	
private:
    
	static radMainWindow* TheApp;
	int screen_width, screen_height;
	
	void createActions();
	void createMenus();
	void createView();
	void createToolBars();
	void createProgressDialog();
	void createHelpWindow();
    
	QAction *openSplitImageAct;

	QAction *saveDetectionAct;
	QAction *saveAllDetectionsAct;
	QAction *loadDetectionAct;
	QAction *quitAct;

	QAction *detectConesAct;
	QAction *purgeHistoryAct;

	QAction *mouseAct;
	QAction *pointAnnotationAct;
	QAction *pointEraseAct;
	QAction *undoAct;

	QAction *aboutAct;
	QAction *helpAct;
	
	QMenu *fileMenu, *saveMenu;
	QMenu *SplitImageInputMenu;
	QMenu *SplitProcessingMenu;
	QMenu *helpMenu;
	QMenu *detectionMenu;

	QLabel *lbGlSpace;
	QCheckBox *cbShowGlyphs;
	QLabel *lbGlSize;
	QDoubleSpinBox *spGlSize;
	QGridLayout *layGlSize;
	QWidget *wgGlSize;
	
	QWidget *centralwidget;
	QGridLayout *viewLayout;
	QVTKWidget *ImageWidget;
	QLabel *SplitFileLabel;
	QListWidget *SplitFileListWidget;
	QVBoxLayout *FileListLayout;

	QProgressDialog *progressDialog;

	QWidget *helpWindow;
	QVBoxLayout *helpLayout;
	QTextBrowser *helpBrowser;
	
	radImageView *ImageView;
	radFileIO *FileIO;
	radDetectionPanel *DetectionPanel;
	radPurgeHistoryDialog *purgeHistoryDialog;
	
	vector< SplitImageInformation > SplitImageInfor;
	ConeDetectionParameters DetectionSettingPara;

	QDir splitHomeDir;
	QFileInfo splitStateFile;
	QByteArray fileDialogState;
	QDir saveDir;
	QDir loadDir;
	string BackupDir;

	void UpdateSplitFileList(unsigned int old_size = 0);
	void ClearSplitFileList();
	void GetCurrentEditing();
	void LoadSplitFile(int);
	
	void DetectSplitImage(unsigned int img_id);

	void DetectSplitImage();
	void DetectSplitImageAll();

	void BackupResults();
	void LoadBackupResults(int id);
};

#endif
