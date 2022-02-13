#include <iostream>
#include "radHotKeyDialog.h"

QMap<QString, QString> defaultHotKeyMap()
{
	QMap<QString, QString> m;
	m.insert("openSplitImage", "Ctrl+O");
	m.insert("saveDetection", "Ctrl+S");
	m.insert("saveAllDetections", "Ctrl+A");
	m.insert("detectCones", "Ctrl+D");
	m.insert("adjustBrightness", "Ctrl+B");
	m.insert("pointAnnotation", "Ctrl+M");
	m.insert("pointMove", "Ctrl+T");
	m.insert("pointErase", "Ctrl+E");
	m.insert("areaErase", "Ctrl+W");
	m.insert("undo", "Ctrl+Z");
	m.insert("help", "F1");
	m.insert("toggleVisibility", "F2");
	m.insert("toggleInterpolation", "Ctrl+I");
	return m;
}

static bool is_acceptable_key(QKeyEvent *event)
{
	int kc = event->key();
	Qt::KeyboardModifiers mod = event->modifiers();
	// Space to clear
	if (kc == Qt::Key_Space && mod == 0)
		return true;
	// Exclude Alt+F4 (standard shortcut for closing the program)
	if (kc == Qt::Key_F4 && mod == Qt::AltModifier)
		return false;
	// Any Meta/Alt/Ctrl/Shift + Fn combo (Meta/Alt/Ctrl/Shift optional)
	if (kc >= Qt::Key_F1 && kc <= Qt::Key_F12)
		return true;
	// Any Meta/Alt/Ctrl/Shift + [A..Z,0..9] with at least one of Meta/Alt/Ctrl modifiers
	if ((kc >= Qt::Key_0 && kc <= Qt::Key_9) || (kc >= Qt::Key_A && kc <= Qt::Key_Z)) {
		return (mod & (Qt::ControlModifier | Qt::AltModifier | Qt::MetaModifier)) != 0;
	}
	return false;
}

static QString key_to_str(QKeyEvent *event)
{
	QString res;
	int kc = event->key();
	Qt::KeyboardModifiers mod = event->modifiers();
	if (kc == Qt::Key_Space && mod == 0)
		return res;
	if (kc >= Qt::Key_F1 && kc <= Qt::Key_F12) {
		res = QString("F") + QString::number(kc - Qt::Key_F1 + 1);
	}
	else if ((kc >= Qt::Key_0 && kc <= Qt::Key_9) || (kc >= Qt::Key_A && kc <= Qt::Key_Z)) {
		res = QString(QChar(kc));
	}
	else
		return res;
	if (mod & Qt::ShiftModifier)
		res = QString("Shift+") + res;
	if (mod & Qt::ControlModifier)
		res = QString("Ctrl+") + res;
	if (mod & Qt::AltModifier)
		res = QString("Alt+") + res;
	if (mod & Qt::MetaModifier)
		res = QString("Meta+") + res;

	return res;
}

void radTableWidget::keyPressEvent(QKeyEvent *event)
{
	if (key_column >= 0 && is_acceptable_key(event)) {
		if (selectionModel()->hasSelection())
		{
			int iRow = selectedIndexes()[0].row();
			if (event->type() == QEvent::KeyPress) {
				QString kText = key_to_str(event);
				for (int j = 0; j < rowCount(); j++) {
					QTableWidgetItem *kItem = item(j, key_column);
					if (kItem->text() == kText)
						kItem->setText(tr(""));
				}
				QTableWidgetItem *kItem = item(iRow, key_column);
				kItem->setText(kText);
				kItem->setFont(bold);
			}
			return;
		}
	}
	QTableWidget::keyPressEvent(event);
}

void radTableWidget::ResetKeyFont()
{
	if (key_column < 0) return;
	for (int j = 0; j < rowCount(); j++) {
		QTableWidgetItem *kItem = item(j, key_column);
		kItem->setFont(normal);
	}
}

radHotKeyDialog::radHotKeyDialog(QWidget *parent, std::vector<ActionEntry> & _actionMap) :
	actionMap(_actionMap)
{
	setModal(true);
	setResult(QDialog::Rejected);
	setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
	setWindowIcon(QIcon(":conedetect.png"));
	setWindowTitle(tr("Keyboard Shortcuts"));

	QScreen *screen = QGuiApplication::primaryScreen();
	QRect  screenGeometry = screen->geometry();
	int screen_height = screenGeometry.height();
	int screen_width = screenGeometry.width();
	setMinimumSize(screen_width * 40 / 100, screen_height * 50 / 100);

	layout = new QVBoxLayout();

	table = new radTableWidget();

	QStringList headers;
	headers << "Action" << "Description" << "Hot Key";

	table->setColumnCount(3);
	table->SetKeyColumn(2);
	table->setRowCount(0);
	table->setHorizontalHeaderLabels(headers);
	table->verticalHeader()->setVisible(false);
	table->setEditTriggers(QAbstractItemView::NoEditTriggers);
	table->setSelectionBehavior(QAbstractItemView::SelectRows);
	table->setSelectionMode(QAbstractItemView::SingleSelection);
	table->setShowGrid(false);

	QHeaderView *verticalHeader = table->verticalHeader();
	verticalHeader->setSectionResizeMode(QHeaderView::Fixed);

	int vsize = verticalHeader->defaultSectionSize() * 6 / 10;
	verticalHeader->setDefaultSectionSize(vsize);

	table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
	table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
	table->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);

	layout->addWidget(table);

	lblInfo = new QLabel(tr("\
Select an item from the list, then press the desired control key combination or space to clear current shortcut.\n\
Acceptable key combinations are:\nCtrl/Alt/Meta + alpha (A..Z) or number (0..9), \
as well as function keys (F1..F12) with or without any modifiers.\n\
Examples: Ctrl+A, Ctrl+Shift+6, F5, Alt+F8."), this);
	layout->addWidget(lblInfo);

	buttonGroup = new QWidget();
	buttonLayout = new QGridLayout();
	buttonGroup->setLayout(buttonLayout);

	btnDefaults = new QPushButton(tr("Restore Defaults"), this);
	btnDefaults->setAutoDefault(false);
	buttonLayout->addWidget(btnDefaults, 0, 0);
	connect(btnDefaults, SIGNAL(clicked()), SLOT(handleDefaults()));

	buttonLayout->addWidget(new QWidget(this), 0, 1);

	btnSave = new QPushButton(tr("Save"), this);
	btnSave->setAutoDefault(true);
	buttonLayout->addWidget(btnSave, 0, 2);
	connect(btnSave, SIGNAL(clicked()), SLOT(accept()));

	btnClose = new QPushButton(tr("Close"), this);
	btnClose->setAutoDefault(false);
	buttonLayout->addWidget(btnClose, 0, 3);
	connect(btnClose, SIGNAL(clicked()), SLOT(close()));

	connect(table->selectionModel(), SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)), SLOT(handleSelectionChange(const QItemSelection &, const QItemSelection &)));

	layout->addWidget(buttonGroup);

	setLayout(layout);

	updateActionTable();
}

radHotKeyDialog::~radHotKeyDialog()
{

}

void radHotKeyDialog::updateActionTable()
{
	itemMap.resize(actionMap.size());
	int nActs = int(actionMap.size());
	table->setSortingEnabled(false);
	table->setRowCount(nActs);
	for (int iRow = 0; iRow < nActs; iRow++) {
		QAction *act = actionMap[iRow].act;

		QString actName = act->text();
		actName.remove("&");
		table->setItem(iRow, 0, new QTableWidgetItem(actName));
		QString actDescr = act->toolTip();
		int pidx = actDescr.indexOf("[");
		if (pidx >= 0)
			actDescr.truncate(pidx);
		actDescr = QString("  ") + actDescr + QString("  ");
		table->setItem(iRow, 1, new QTableWidgetItem(actDescr));
		QString actKey = act->shortcut().toString();
		QTableWidgetItem *itemKey = new QTableWidgetItem(actKey);
		table->setItem(iRow, 2, itemKey);

		itemMap[iRow] = itemKey;
	}
	table->sortByColumn(0, Qt::AscendingOrder);
	table->setSortingEnabled(true);
}

void radHotKeyDialog::handleDefaults()
{
	int kcol = table->GetKeyColumn();
	QMap<QString, QString> dfmap = defaultHotKeyMap();
	for (size_t j = 0; j < actionMap.size(); j++) {
		QString itemId = actionMap[j].id;
		QTableWidgetItem *itemKey = itemMap[j];
		if (dfmap.contains(itemId))
			itemKey->setText(dfmap[itemId]);
		else
			itemKey->setText(tr(""));
	}
	table->ResetKeyFont();
	table->setFocus();
}

void radHotKeyDialog::handleSelectionChange(const QItemSelection &selected, const QItemSelection &deselected)
{

}

QMap<QString, QString> radHotKeyDialog::getKeyMap()
{
	QMap<QString, QString> kmap = defaultHotKeyMap();
	for (size_t j = 0; j < actionMap.size(); j++) {
		QString itemId = actionMap[j].id;
		QTableWidgetItem *itemKey = itemMap[j];
		kmap[itemId] = itemKey->text();
	}
	return kmap;
}