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

#ifndef MSG_SERVER_H
#define MSG_SERVER_H

#include "recv_msg.h"
#include "send_msg.h"
#include "msg.h"

#include <QObject>
#include <QUdpSocket>

class SendMsg;

class MsgServer : public QObject
{
    Q_OBJECT

public:
    MsgServer(QObject *parent = 0);

    void start();

signals:
    void newUserMsg(Msg msg);
    void newExitMsg(Msg msg);
    void newMsg(Msg msg);
    void error(QAbstractSocket::SocketError, QString errorString);
    void releaseFile(QString additionalInfo);
    void msgReaded(QString name);

public slots:
    void processSendMsg();
    void socketEerror(QAbstractSocket::SocketError);

private slots:
    void readPacket();

private:
    void handleMsg(Msg &msg);
    bool isSupportedCommand(QByteArray &datagram) const;
    void broadcastMsg(Msg &msg);
    void broadcastUserMsg(Msg &msg);
    bool isResendNeeded(Msg &msg) const;
    void updateAddresses();

    void processRecvMsg(Msg msg);
    void processRecvReleaseFilesMsg(Msg msg);
    void processRecvReadMsg(Msg msg);
    void processRecvSendMsg(Msg msg);
    void processEntryMsg(Msg msg);

    QList<QHostAddress> m_broadcastAddresses;
    QList<QHostAddress> m_ipAddresses;
    QUdpSocket m_udpSocket;
};

#endif // !MSG_SERVER_H

