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

#include "send_file_model.h"
#include "constants.h"
#include "helper.h"

#include <QFileInfo>

void SendFileModel::createModel()
{
    QStringList labels;
    labels << tr("Name") << tr("Size")
        << tr("Location") << tr("Absolute Path");

    m_model = new QStandardItemModel(0, labels.size(), this);
    m_model->setHorizontalHeaderLabels(labels);

    QStandardItem *item
        = m_model->horizontalHeaderItem(SEND_FILE_VIEW_LOCATION_POS);
    item->setTextAlignment(Qt::AlignLeft);
}

void SendFileModel::addFile(const QString &path)
{
    if (path.isEmpty() || contains(path)) {
        return;
    }

    QFileInfo fileInfo(path);

    QStringList items;
    items << fileInfo.fileName() << sizeString(fileInfo)
        << fileInfo.absolutePath() << fileInfo.absoluteFilePath();

    addRow(items);
}

void SendFileModel::addFile(const QStringList &pathList)
{
    foreach (QString path, pathList) {
        addFile(path);
    }
}

void SendFileModel::addRow(const QStringList &items)
{
    int row = m_model->rowCount();
    int column = 0;
    m_model->insertRow(row);
    foreach (QString s, items) {
        m_model->setData(m_model->index(row, column), s);
        ++column;
    }

    QStandardItem *item = m_model->item(row, SEND_FILE_VIEW_SIZE_POS);
    item->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
}

bool SendFileModel::contains(const QString &path) const
{
    for (int row = 0; row < m_model->rowCount(); ++row) {
        if (absolutePath(row) == path) {
            return true;
        }
    }

    return false;
}

QString SendFileModel::absolutePath(int row) const
{
    return m_model->data(m_model->index(row, SEND_FILE_VIEW_PATH_POS))
        .toString();
}

QString SendFileModel::fileNameJoin(const QString &separator) const
{
    QStringList sl;
    for (int row = 0; row < m_model->rowCount(); ++row) {
        sl << fileName(row);
    }

    return sl.join(separator);
}

QStringList SendFileModel::pathList() const
{
    QStringList sl;
    for (int row = 0; row < m_model->rowCount(); ++row) {
        sl << absolutePath(row);
    }

    return sl;
}

QString SendFileModel::fileName(int row) const
{
    return m_model->data(m_model->index(row, SEND_FILE_VIEW_NAME_POS))
        .toString();
}

void SendFileModel::removeRows(QList<int> rowList)
{
    qSort(rowList.begin(), rowList.end(), qGreater<int>());
    foreach (int row, rowList) {
        m_model->removeRow(row);
    }
}

QString SendFileModel::sizeString(QFileInfo &fileInfo) const
{
    QString s;
    if (fileInfo.isDir()) {
        s = "Folder";
    } else {
        s = QString("%1").arg(Helper::sizeStringUnit(fileInfo.size(), " "));
    }

    return s;
}

