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

#ifndef TRANSFER_FILE_MODEL_H
#define TRANSFER_FILE_MODEL_H

#include <QObject>

class QStandardItemModel;
class SendFileMap;

class TransferFileModel : public QObject
{
    Q_OBJECT;

public:
    friend class TransferFileWindow;
    TransferFileModel(QObject *parent = 0);
    ~TransferFileModel();

    void insertTransfer(SendFileMap*);

    void removeRow(const QString &key);
    QString packetNoString(int row) const;

public slots:
    void updateTransfer(QString packetNoString);

private:
    void createModel();
    int keyToRow(QString key) const;

    QStandardItemModel *m_model;
};

#endif // !TRANSFER_FILE_MODEL_H

