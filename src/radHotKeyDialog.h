#pragma once
#include <vector>

#include <QtGlobal>
#include <QtGui>
#include <QtWidgets>
#include <QVBoxLayout>
#include <QTableWidget>
#include <QDialog>
#include <QAction>
#include <QMap>

struct ActionEntry
{
	QString id;
	QAction *act;
	ActionEntry() { act = NULL; }
	ActionEntry(const char *_id, QAction *_act) : id(_id), act(_act) {}
};

QMap<QString, QString> defaultHotKeyMap();

class radTableWidget : public QTableWidget
{
public:
	radTableWidget() : QTableWidget()
	{
		normal = QFont(this->font());
		bold = QFont(normal);
		bold.setBold(true);
	}

	int GetKeyColumn() { return key_column; }
	void SetKeyColumn(int key_column) { this->key_column = key_column; }
	void ResetKeyFont();
protected:
	virtual void keyPressEvent(QKeyEvent *event) override;
private:
	int key_column = -1;
	QFont normal;
	QFont bold;
};

class radHotKeyDialog :
	public QDialog
{
Q_OBJECT
public:
	radHotKeyDialog(QWidget *parent, std::vector<ActionEntry> & _actionMap);
	virtual ~radHotKeyDialog();

	QMap<QString, QString> getKeyMap();

private:
	std::vector<ActionEntry> &actionMap;
	std::vector<QTableWidgetItem *> itemMap;

	QVBoxLayout *layout;
	radTableWidget *table;
	QLabel *lblInfo;
	QWidget *buttonGroup;
	QGridLayout *buttonLayout;
	QPushButton *btnDefaults;
	QPushButton *btnSave;
	QPushButton *btnClose;

	void updateActionTable();

private slots:
	void handleDefaults();
	void handleSelectionChange(const QItemSelection &selected, const QItemSelection &deselected);
};
