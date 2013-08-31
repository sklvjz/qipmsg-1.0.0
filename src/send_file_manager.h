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

#ifndef SEND_FILE_MANAGER_H
#define SEND_FILE_MANAGER_H

#include "transfer_file_model.h"
#include "send_file_map.h"

#include <QObject>
#include <QMap>
#include <QMutex>

class SendFileMap;

class SendFileManager : public QObject
{
    Q_OBJECT

public:
    friend class ServeSocket;
    friend class TransferFileWindow;

    SendFileManager() {}
    ~SendFileManager();

    void addTransfer(QString, SendFileMap *);
    void addTransferLocked(QString, SendFileMap *);

    TransferFileModel transferFileModel;
    QMap<QString, SendFileMap *> transferFileMap;

signals:
    void transferCountChanged(int);

public slots:
    void removeTransfer(QString key);
    void removeTransferLocked(QString key);

private:
    QMutex m_lock;
};

#endif // !SEND_FILE_MANAGER_H

