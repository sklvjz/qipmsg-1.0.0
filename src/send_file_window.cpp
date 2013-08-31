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

#include "send_file_window.h"
#include "dir_dialog.h"
#include "send_file_model.h"
#include "constants.h"
#include "sizecolumndelegate.h"
#include "global.h"
#include "preferences.h"

SendFileWindow::SendFileWindow(SendFileModel *sendFileModel, QWidget *parent)
    : QWidget(parent), m_sendFileModel(sendFileModel)
{
    createSendFileView(sendFileModel);
    createButtonLayout();

    createConnections();

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(sendFileView);
    mainLayout->addLayout(buttonLayout);

    setLayout(mainLayout);

    setWindowTitle(tr("Send File List"));

    adjustSize();
}

QSize SendFileWindow::sizeHint() const
{
    return QSize(500, 240);
}

void SendFileWindow::createSendFileView(SendFileModel *sendFileModel)
{
    proxyModel = new QSortFilterProxyModel(this);
    proxyModel->setSourceModel(sendFileModel->m_model);

    sendFileView = new QTableView(this);
    sendFileView->setModel(proxyModel);

    sendFileView->setSortingEnabled(false);
    sendFileView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    sendFileView->horizontalHeader()->setMovable(false);
    sendFileView->verticalHeader()->hide();
    sendFileView->setSelectionBehavior(QAbstractItemView::SelectRows);
    sendFileView->setTabKeyNavigation(false);

    // no delegate needed, implement with QStandardItem->setTextAlignment
    // in send_file_model.cpp
#if 0
    sendFileView->setItemDelegateForColumn(SEND_FILE_VIEW_SIZE_POS,
            new SizeColumnDelegate(this));
#endif
    sendFileView->hideColumn(SEND_FILE_VIEW_PATH_POS);

    QItemSelectionModel *selectionModel = new QItemSelectionModel(sendFileView->model());
    sendFileView->setSelectionModel(selectionModel);

    QFontMetrics fm = fontMetrics();
    QHeaderView *header = sendFileView->horizontalHeader();
    header->resizeSection(SEND_FILE_VIEW_NAME_POS,
            fm.width(tr("normal-length-of-a-file-name")));
    header->resizeSection(SEND_FILE_VIEW_SIZE_POS,
            qMax(fm.width(tr("Size")), fm.width(" 10000 KB ")));
    header->resizeSection(SEND_FILE_VIEW_LOCATION_POS,
            fm.width(tr("/normal/path/of/a/directory/")));
}

void SendFileWindow::createButtonLayout()
{
    addFileButton = new QPushButton(tr("Add File"));
    addDirButton = new QPushButton(tr("Add Directory"));
    delButton = new QPushButton(tr("Delete"));
    closeButton = new QPushButton(tr("Close"));

    buttonLayout = new QHBoxLayout;
    buttonLayout->addStretch();
    buttonLayout->addWidget(addFileButton);
    buttonLayout->addSpacing(20);
    buttonLayout->addWidget(addDirButton);
    buttonLayout->addSpacing(20);
    buttonLayout->addWidget(delButton);
    buttonLayout->addSpacing(20);
    buttonLayout->addWidget(closeButton);
}

void SendFileWindow::createConnections()
{
    connect(addFileButton, SIGNAL(clicked()),
            this, SLOT(addFile()));
    connect(addDirButton, SIGNAL(clicked()),
            this, SLOT(addDir()));
    connect(delButton, SIGNAL(clicked()),
            this, SLOT(delFile()));
    connect(closeButton, SIGNAL(clicked()),
            this, SLOT(close()));
}

void SendFileWindow::closeEvent(QCloseEvent *event)
{
    emit addFileFinished();

    event->accept();
}

void SendFileWindow::addFile()
{
    QStringList pathList;

    pathList = QFileDialog::getOpenFileNames(this, tr("Add Files"),
            Global::preferences->lastSendFilePath,
            tr("All Files (*)"));

    if (!pathList.isEmpty()) {
        Global::preferences->lastSendFilePath = pathList.at(0);
    }

    m_sendFileModel->addFile(pathList);
}

void SendFileWindow::addDir()
{
    DirDialog *dirDialog = new DirDialog(m_sendFileModel, this);

    dirDialog->show();
}

void SendFileWindow::delFile()
{
    QItemSelectionModel *selectionModel = sendFileView->selectionModel();
    QModelIndexList indexList = selectionModel->selectedRows();

    QList<int> rowList;
    foreach (QModelIndex index, indexList) {
        if (!rowList.contains(index.row())) {
            rowList.append(index.row());
        }
    }

    m_sendFileModel->removeRows(rowList);
}

