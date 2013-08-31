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

#include "transfer_file_model.h"
#include "send_file_map.h"
#include "constants.h"
#include "global.h"
#include "send_file_manager.h"

#include <QtCore>
#include <QStandardItemModel>


TransferFileModel::TransferFileModel(QObject *parent)
    : QObject(parent)
{
    createModel();
}

TransferFileModel::~TransferFileModel()
{
    delete m_model;
}

void TransferFileModel::createModel()
{
    QStringList labels;
    labels << tr("No.") << tr("file") << tr("size")
        << tr("Total/Sended/Send") << tr("User") << tr("Packet ID");

    m_model = new QStandardItemModel(0, labels.size(), this);
    m_model->setHorizontalHeaderLabels(labels);
}

void TransferFileModel::insertTransfer(SendFileMap *sendFileMap)
{
    int row = m_model->rowCount();
    m_model->insertRow(row);

    m_model->setData(m_model->index(row, TRANSFER_FILE_VIEW_NO_COLUMN),
                     sendFileMap->packetNoString());
    m_model->setData(m_model->index(row, TRANSFER_FILE_VIEW_FILE_COLUMN),
                     sendFileMap->fileNames());
    m_model->setData(m_model->index(row, TRANSFER_FILE_VIEW_SIZE_COLUMN),
                     sendFileMap->sizeInfo());
    m_model->setData(m_model->index(row, TRANSFER_FILE_VIEW_STATS_COLUMN),
                     sendFileMap->sendStats());
    m_model->setData(m_model->index(row, TRANSFER_FILE_VIEW_USER_COLUMN),
                     sendFileMap->recvUserInfo());
    m_model->setData(m_model->index(row, TRANSFER_FILE_VIEW_KEY_COLUMN),
                     sendFileMap->packetNoString());
}

// XXX NOTE: caller must hold sendFileManager lock when calling this.
void TransferFileModel::updateTransfer(QString packetNoString)
{
    int row = keyToRow(packetNoString);

    SendFileMap *map
        = Global::sendFileManager->transferFileMap.value(packetNoString);
#if 0
    if (!map) {
        return;
    }
#endif

    // Qt 4.3.5 need this statement, Qt 4.4.0 not need.
    qRegisterMetaType<QModelIndex>("QModelIndex");

    // XXX TODO: set row to No. is not right
    m_model->setData(m_model->index(row, TRANSFER_FILE_VIEW_NO_COLUMN),
                     row);
    m_model->setData(m_model->index(row, TRANSFER_FILE_VIEW_FILE_COLUMN),
                     map->fileNames());
    m_model->setData(m_model->index(row, TRANSFER_FILE_VIEW_SIZE_COLUMN),
                     map->sizeInfo());
    m_model->setData(m_model->index(row, TRANSFER_FILE_VIEW_STATS_COLUMN),
                     map->sendStats());
    m_model->setData(m_model->index(row, TRANSFER_FILE_VIEW_USER_COLUMN),
                     map->recvUserInfo());
    m_model->setData(m_model->index(row, TRANSFER_FILE_VIEW_KEY_COLUMN),
                     map->packetNoString());
}

int TransferFileModel::keyToRow(QString key) const
{
    for (int row = 0; row < m_model->rowCount(); ++row) {
        if (packetNoString(row) == key) {
            return row;
        }
    }
}

QString TransferFileModel::packetNoString(int row) const
{
    return m_model->data(m_model->index(row, TRANSFER_FILE_VIEW_KEY_COLUMN))
        .toString();
}

void TransferFileModel::removeRow(const QString &key)
{
    QList<int> rowList;
    for (int row = 0; row < m_model->rowCount(); ++row) {
        if (packetNoString(row) == key) {
            rowList << row;
        }
    }

    qSort(rowList.begin(), rowList.end(), qGreater<int>());
    foreach (int row, rowList) {
        m_model->removeRow(row);
    }
}

