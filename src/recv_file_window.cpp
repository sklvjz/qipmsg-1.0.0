// This file is part of QIpMsg.
//
// QIpMsg is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// QIpMsg is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with QIpMsg.  If not, see <http://www.gnu.org/licenses/>.
//

#include <QtCore>
#include <QtGui>

#include "recv_file_window.h"
#include "constants.h"
#include "global.h"
#include "preferences.h"
#include "sizecolumndelegate.h"
#include "recvfilesortfilterproxymodel.h"
#include "recv_file_model.h"
#include "msg_window.h"

RecvFileWindow::RecvFileWindow(QWidget *parent)
    : QWidget(parent)
{
    createRecvFileView();
    createButtonLayout();

    QVBoxLayout *mainLayout = new QVBoxLayout;

    mainLayout->addWidget(recvFileView);
    mainLayout->addSpacing(20);
    mainLayout->addLayout(buttonsLayout);

    createConnections();

    setLayout(mainLayout);

    setWindowTitle(tr("Receive File List"));
    setWindowIcon(*Global::iconSet.value("normal"));

    setWindowFlags(Qt::Dialog);
    setWindowModality(Qt::WindowModal);

    adjustSize();
}

QSize RecvFileWindow::sizeHint() const
{
    return QSize(480, 320);
}

void RecvFileWindow::createButtonLayout()
{
    saveButton = new QPushButton(tr("Save"));
    closeButton = new QPushButton(tr("Close"));
    selectAllButton = new QPushButton(tr("Select All"));
    clearSelectButton = new QPushButton(tr("Clear Select"));

    buttonsLayout = new QHBoxLayout;
    buttonsLayout->addStretch();
    buttonsLayout->addWidget(clearSelectButton);
    buttonsLayout->addSpacing(20);
    buttonsLayout->addWidget(selectAllButton);
    buttonsLayout->addSpacing(20);
    buttonsLayout->addWidget(saveButton);
    buttonsLayout->addSpacing(20);
    buttonsLayout->addWidget(closeButton);
}

void RecvFileWindow::createConnections()
{
    connect(saveButton, SIGNAL(clicked()),
            this, SLOT(save()));
    connect(closeButton, SIGNAL(clicked()),
            this, SLOT(close()));
    connect(selectAllButton, SIGNAL(clicked()),
            this, SLOT(selectAll()));
    connect(clearSelectButton, SIGNAL(clicked()),
            recvFileView, SLOT(clearSelection()));
}

void RecvFileWindow::save()
{
    QItemSelectionModel *selectionModel = recvFileView->selectionModel();
    QModelIndexList indexList
        = selectionModel->selectedRows(RECV_FILE_ID_POS);
    if (indexList.count() == 0) {
        QMessageBox::information(this, tr("Select Files First"),
                tr("Please select files you want to save first.")
                );
        return;
    }

    QString directory = selectSaveDirectory();

    // Check if file exist
    bool isOverWrite = false;
    int existType;
    while (isFileExist(directory, indexList, &existType) && !isOverWrite) {
        QMessageBox msgBox;
        msgBox.setParent(this);
        msgBox.setWindowModality(Qt::WindowModal);
        msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
        QPushButton *selectButton = msgBox.addButton(tr("Select Save Folder"),
                QMessageBox::ActionRole);
        msgBox.setWindowTitle(tr("Alert"));
        msgBox.setIcon(QMessageBox::Warning);
        QAbstractButton *okButton = msgBox.button(QMessageBox::Ok);
        QAbstractButton *cancelButton = msgBox.button(QMessageBox::Cancel);
        msgBox.setText(tr("File exists, overwrite?"));
        // not support folder overwrite
        if (existType == IPMSG_FILE_DIR) {
            okButton->setDisabled(true);
            msgBox.setText(tr("Folder exists, select other place to save."));
        }

        msgBox.exec();

        if (msgBox.clickedButton() == okButton) {
            isOverWrite = true;
        } else if (msgBox.clickedButton() == selectButton) {
            directory = selectSaveDirectory();
        } else if (msgBox.clickedButton() == cancelButton) {
            return;
        }
    }

    if (directory.isEmpty()) {  // No directory selected
        return;
    }

    // Directory selected
    Global::preferences->lastSaveFilePath = directory;

    QList<int> fileIdList;
    foreach (QModelIndex i, indexList) {
        fileIdList << recvFileView->model()->data(i).toInt();
    }

    emit saveFile(fileIdList, directory);
}

QString RecvFileWindow::selectSaveDirectory()
{
    QFileDialog::Options options
        = QFileDialog::DontResolveSymlinks | QFileDialog::ShowDirsOnly;
    QString directory = QFileDialog::getExistingDirectory(this,
            tr("Save File"),
            Global::preferences->lastSaveFilePath,
            options);

    return directory;
}

void RecvFileWindow::createRecvFileView()
{
    proxyModel = new RecvFileSortFilterProxyModel(this);
    proxyModel
        ->setSourceModel(qobject_cast<MsgWindow *>(this->parent())
                         ->recvFileModel().m_model);

    recvFileView = new QTableView(this);
    recvFileView->setModel(proxyModel);

    recvFileView->setSortingEnabled(false);
    recvFileView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    recvFileView->horizontalHeader()->setMovable(false);
    recvFileView->verticalHeader()->hide();
    recvFileView->setSelectionBehavior(QAbstractItemView::SelectRows);
    recvFileView->setTabKeyNavigation(false);

    // no delegate needed, implement with QStandardItem->setTextAlignment
    // in recv_file_model.cpp
#if 0
    recvFileView->setItemDelegateForColumn(RECV_FILE_VIEW_SIZE_POS,
            new SizeColumnDelegate(this));
#endif

    recvFileView->hideColumn(RECV_FILE_VIEW_ID_POS);

    QItemSelectionModel *selectionModel
        = new QItemSelectionModel(recvFileView->model());
    recvFileView->setSelectionModel(selectionModel);

    QFontMetrics fm = fontMetrics();
    QHeaderView *header = recvFileView->horizontalHeader();
    header->resizeSection(RECV_FILE_VIEW_NAME_POS,
            fm.width(tr("normal-length-of-a-file-name.file")));
    header->resizeSection(RECV_FILE_VIEW_SIZE_POS,
            qMax(fm.width(tr("Size")), fm.width(" 1000000 KB ")));
}

void RecvFileWindow::selectAll()
{
    QItemSelectionModel *selectionModel = recvFileView->selectionModel();
    QAbstractItemModel *model = recvFileView->model();

    QItemSelection selection(model->index(0, 0, QModelIndex()),
            model->index(model->rowCount() - 1, model->columnCount() - 1,
                QModelIndex()));

    selectionModel->select(selection, QItemSelectionModel::Select);
}

bool RecvFileWindow::isFileExist(QString directory, QModelIndexList &indexList,
        int *existType)
{
    if (directory.isEmpty()) {
        return false;
    }

    QDir dir(directory);
    QFileInfoList fileInfoList = dir.entryInfoList();
    foreach (QFileInfo fi, fileInfoList) {
        // Skip '.' and '..' directory
        if (fi.fileName() == "." || fi.fileName() == "..") {
            continue;
        }

        if (isRecvFileExist(fi.fileName(), indexList)) {
            if (fi.isDir()) {
                *existType = IPMSG_FILE_DIR;
            } else if (fi.isFile()) {
                *existType = IPMSG_FILE_REGULAR;
            }
            return true;
        }
    }

    // File not exist
    return false;
}

bool RecvFileWindow::isRecvFileExist(QString fileName,
        QModelIndexList &indexList)
{
    QAbstractItemModel *model = recvFileView->model();
    foreach (QModelIndex index, indexList) {
        QString name = model->data(model->index(index.row(),
                    RECV_FILE_VIEW_NAME_POS)).toString();
        if (name == fileName) {
            return true;
        }
    }

    return false;
}

