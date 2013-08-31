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

#ifndef SEND_FILE_MODEL_H
#define SEND_FILE_MODEL_H

#include <QStringList>
#include <QStandardItemModel>
#include <QObject>

class QFileInfo;

class SendFileModel : public QObject
{
    Q_OBJECT

public:
    friend class SendFileWindow;
    SendFileModel(QObject *parent = 0): QObject(parent) { createModel(); }
    ~SendFileModel() { delete m_model; }

    void addFile(const QString &path);
    void addFile(const QStringList &pathList);
    int rowCount() const { return m_model->rowCount(); }
    QString fileNameJoin(const QString &separator) const;
    QStringList pathList() const;
    void removeRows(QList<int> rowList);

private:
    void createModel();
    QString absolutePath(int row) const;
    void addRow(const QStringList &items);
    bool contains(const QString &path) const;
    QString fileName(int row) const;
    QString sizeString(QFileInfo &fileInfo) const;

    QStandardItemModel *m_model;
};

#endif // !SEND_FILE_MODEL_H
