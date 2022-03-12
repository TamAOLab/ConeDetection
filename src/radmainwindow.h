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
#include <QMessageBox>
#if QT_VERSION >= 0x050000
#include <QMenu>
#endif
#include "radimageview.h"
#include "radFileIO.h"
#include "QActionGroup.h"
#include "radDetectionPanel.h"
#include "radbackup.h"
#include "radPurgeHistoryDialog.h"
#include "radHotKeyDialog.h"

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

	MouseOperation mouseMode = MouseOperation::Normal;

	void openSplitImages(QStringList & fileNames, bool save_state=false);
	void loadDetections(QStringList & fileNames);
	void NextImage() { OnNextImage(); }
	void PreviousImage() { OnPreviousImage(); }

	void checkWhatsNew();

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

	void DetectConesChecked(QList<int> checked);
	void BackupResults(unsigned int img_id);
	void showDetectionPanel();
	void ToggleVisibility();
	void ToggleInterpolation();
	void ToggleVoronoi();
	void purgeHistoryFiles();
	void selectHotkeys();
	void SwitchSplitFile(QListWidgetItem*, QListWidgetItem*);
	
	void SetMouseFlag();
	void SetPointAnnotationFlag();
	void SetPointMoveFlag();
	void SetPointEraseFlag();
	void SetAreaEraseFlag();
	void DoUndo();
	void DeleteAllConeMarkers();

	void onConeGlyphVisibility(bool v);
	void changeConeGlyphSize(double sz);

	void receiveFinishDetection();
	void handleUpdateProgress();

	void ShowAboutDialog();
	void ShowHelpWindow();
	void ShowWhatsNewWindow();

	void OnNextImage();
	void OnPreviousImage();

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

	void saveShortcuts(QMap<QString, QString> kmap);
	QMap<QString, QString> loadShortcuts();
	void applyShortcuts(QMap<QString, QString> kmap);

	QAction *openSplitImageAct;

	QAction *saveDetectionAct;
	QAction *saveAllDetectionsAct;
	QAction *loadDetectionAct;
	QAction *quitAct;

	QAction *nextImageAct;
	QAction *prevImageAct;

	QAction *detectConesAct;
	QAction *deleteAllConesAct;
	QAction *toggleVisibilityAct;
	QAction *toggleInterpolationAct;
	QAction *purgeHistoryAct;
	QAction *setHotkeysAct;

	QAction *mouseAct;
	QAction* pointAnnotationAct;
	QAction* pointMoveAct;
	QAction *pointEraseAct;
	QAction *areaEraseAct;
	QAction *undoAct;
	QAction* voronoiAct;

	QAction* aboutAct;
	QAction* whatsNewAct;
	QAction* helpAct;

	std::vector<ActionEntry> actionMap;
	
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
	QFileInfo shortcutFile;
	QByteArray fileDialogState;
	QDir saveDir;
	QDir loadDir;
	std::string BackupDir;
	
	QDir helpDir;
	QFileInfo helpFile;
	QString lastVersion;

	void UpdateSplitFileList(bool newlist=true);
	void ClearSplitFileList();
	void GetCurrentEditing();
	void LoadSplitFile(int);
	
	void DetectSplitImage(unsigned int img_id);
	void changeConeGlyphVisibility(bool v);
	void RestoreVisibility();

	// void DetectSplitImage();
	QList<int> checkedItems;
	void DetectSplitImageChecked();

	void BackupResults();
	void LoadBackupResults(int id);
};

#endif
