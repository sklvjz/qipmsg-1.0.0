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

#ifndef RECV_FILE_THREAD_H
#define RECV_FILE_THREAD_H

#include "recv_file_transfer.h"

#include <QThread>
#include <QString>

class RecvFileMap;

class RecvFileThread : public QThread
{
    Q_OBJECT

public:
    RecvFileThread(RecvFileMap *recvFileMap, QObject *parent = 0)
        : QThread(parent), m_recvFileMap(recvFileMap) {}

    ~RecvFileThread()
    {
        delete m_recvFileTransfer;
        // we do not need to delete m_recvFileMap, it's managed by MsgWindow
    }

    void run();

    void resumeTransfer();

signals:
    void recvFileError(QString errorString);
    void recvFileFinished();
    void stopTransfer();
    void abortTransfer();

private slots:

private:
    RecvFileMap *m_recvFileMap;
    RecvFileTransfer *m_recvFileTransfer;
};

#endif // !RECV_FILE_THREAD_H

