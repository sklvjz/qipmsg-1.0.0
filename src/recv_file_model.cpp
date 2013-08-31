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

#include "recv_file_model.h"
#include "recv_file.h"
#include "constants.h"

#include <QStringList>
#include <QObject>

void RecvFileModel::createModel()
{
    QStringList labels;
    labels << QObject::tr("fileId") << QObject::tr("Name")
        << QObject::tr("Size");

    m_model = new QStandardItemModel(0, labels.size(), 0);
    m_model->setHorizontalHeaderLabels(labels);
}

void RecvFileModel::addRow(const RecvFile &recvFile)
{
    int row = m_model->rowCount();

    m_model->insertRow(row);
    m_model->setData(m_model->index(row, RECV_FILE_VIEW_ID_POS),
                     recvFile.fileIdString());
    m_model->setData(m_model->index(row, RECV_FILE_VIEW_NAME_POS),
                     recvFile.name());
    m_model->setData(m_model->index(row, RECV_FILE_VIEW_SIZE_POS),
                     recvFile.sizeString());

    QStandardItem *item = m_model->item(row, RECV_FILE_VIEW_SIZE_POS);
    item->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
}

void RecvFileModel::removeRow(int id)
{
    for (int i = 0; i < m_model->rowCount(); ++i) {
        int fileId = m_model->data(m_model->index(i, RECV_FILE_VIEW_ID_POS))
            .toInt();
        if (id == fileId) {
            m_model->removeRow(i);
            break;
        }
    }
}

