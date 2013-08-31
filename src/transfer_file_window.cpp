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

#include "transfer_file_window.h"
#include "send_file_map.h"
#include "constants.h"
#include "global.h"
#include "send_file_manager.h"
#include "transfer_file_model.h"
#include "constants.h"

#include <QtCore>
#include <QtGui>

TransferFileWindow::TransferFileWindow(QWidget *parent)
    : QWidget(parent)
{
    createTransferFileView();
    createButtonLayout();
    createConnections();

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(transferFileView);
    mainLayout->addLayout(buttonLayout);

    setLayout(mainLayout);

    setWindowTitle(tr("Send File Monitor"));
    setWindowIcon(*Global::iconSet.value("normal"));

    adjustSize();
    move(Global::randomNearMiddlePoint());
}

QSize TransferFileWindow::sizeHint() const
{
    return QSize(640, 240);
}

void TransferFileWindow::createTransferFileView()
{
    proxyModel = new QSortFilterProxyModel(this);
    proxyModel->setSourceModel(Global::sendFileManager
            ->transferFileModel.m_model);

    transferFileView = new QTableView(this);
    transferFileView->setModel(proxyModel);

    transferFileView->setSortingEnabled(false);
    transferFileView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    transferFileView->horizontalHeader()->setMovable(false);
    transferFileView->verticalHeader()->hide();
    transferFileView->setSelectionBehavior(QAbstractItemView::SelectRows);
    transferFileView->setTabKeyNavigation(false);

    transferFileView->hideColumn(TRANSFER_FILE_VIEW_KEY_COLUMN);

    QItemSelectionModel *selectionModel = new QItemSelectionModel(transferFileView->model());
    transferFileView->setSelectionModel(selectionModel);

    QFontMetrics fm = fontMetrics();
    QHeaderView *header = transferFileView->horizontalHeader();
    header->resizeSection(TRANSFER_FILE_VIEW_NO_COLUMN,
            fm.width(tr("00000")));
    header->resizeSection(TRANSFER_FILE_VIEW_FILE_COLUMN,
            fm.width(tr("length-of-a-file-name.txt")));
    header->resizeSection(TRANSFER_FILE_VIEW_SIZE_COLUMN,
            qMax(fm.width(tr("Size")), fm.width(" 1000000 MB ")));
    header->resizeSection(TRANSFER_FILE_VIEW_STATS_COLUMN,
            fm.width(Global::sendFileManager->transferFileModel
                .m_model->headerData(TRANSFER_FILE_VIEW_STATS_COLUMN,
                    Qt::Horizontal).toString() + "  "));
    header->resizeSection(TRANSFER_FILE_VIEW_USER_COLUMN,
            fm.width(tr("default-user-name")));
}

void TransferFileWindow::createButtonLayout()
{
    buttonLayout = new QHBoxLayout;

    delButton = new QPushButton(tr("Delete"));
    closeButton = new QPushButton(tr("Close"));
    QCheckBox *checkBox = new QCheckBox(tr("Stop transfer if file changed"));
    checkBox->setEnabled(false);

    buttonLayout->addStretch();
    buttonLayout->addWidget(delButton);
    buttonLayout->addSpacing(20);
    buttonLayout->addWidget(closeButton);
    buttonLayout->addSpacing(20);
    buttonLayout->addWidget(checkBox);
}

void TransferFileWindow::createConnections()
{
    connect(closeButton, SIGNAL(clicked()),
            this, SLOT(close()));
    connect(delButton, SIGNAL(clicked()),
            this, SLOT(deleteTransfer()));
}

void TransferFileWindow::deleteTransfer()
{
    QMutexLocker locker(&Global::sendFileManager->m_lock);

    QItemSelectionModel *selectionModel = transferFileView->selectionModel();
    QModelIndexList indexList = selectionModel->selectedRows();

    QList<int> rowList;
    foreach (QModelIndex index, indexList) {
        if (!rowList.contains(index.row())) {
            rowList.append(index.row());
        }
    }

    foreach (int row, rowList) {
        QString packetNo = proxyModel->data(proxyModel->index(row,
                    TRANSFER_FILE_VIEW_KEY_COLUMN)).toString();
        if (!Global::sendFileManager
            ->transferFileMap.value(packetNo)->isTransfer()) {
            Global::sendFileManager->removeTransfer(packetNo);
        }
    }
}

