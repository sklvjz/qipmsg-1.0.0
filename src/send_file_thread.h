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

#ifndef SEND_FILE_THREAD_H
#define SEND_FILE_THREAD_H

#include <QThread>
#include <QTcpSocket>

class SendFileThread : public QThread
{
    Q_OBJECT

public:
    SendFileThread(int socketDescriptor, QObject *parent = 0);

    void run();

#if 0
signals:
    void error(QString errorString, QString packageId);
    void sendFileFinished(QString packageId, int fileId);

private slots:
    void handleSendFileError(QString errorString, QString packageId);
#endif

private:
    int m_socketDescriptor;
};

#endif // !SEND_FILE_THREAD_H
