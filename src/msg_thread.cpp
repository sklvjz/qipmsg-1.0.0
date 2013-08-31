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

#include "msg_thread.h"
#include "constants.h"
#include "msg_server.h"
#include "global.h"
#include "send_msg.h"
#include "recv_msg.h"
#include "window_manager.h"
#include "user_manager.h"

#include <QTimer>
#include <QMessageBox>


MsgThread::~MsgThread()
{
    m_msgServer->processSendMsg();

    exit(0);
    wait();
}

void MsgThread::run()
{
    QTimer timer;
    timer.start(SEND_MSG_PROCESS_INTERVAL);

    m_msgServer = new MsgServer;

    connect(&timer, SIGNAL(timeout()),
            m_msgServer, SLOT(processSendMsg()));

    connect(m_msgServer, SIGNAL(newMsg(Msg)),
            Global::windowManager, SLOT(newMsg(Msg)));
    connect(m_msgServer, SIGNAL(newUserMsg(Msg)),
            Global::userManager, SLOT(newUserMsg(Msg)));
    connect(m_msgServer, SIGNAL(newExitMsg(Msg)),
            Global::userManager, SLOT(newExitMsg(Msg)));

    connect(m_msgServer, SIGNAL(error(QAbstractSocket::SocketError, QString)),
            this, SLOT(handleError(QAbstractSocket::SocketError, QString)));

    m_msgServer->start();

    exec();
}

void MsgThread::addSendMsg(Msg msg)
{
    QMutexLocker locker(&m_lock);

    m_sendMsgMap.insert(msg->packetNoString(), msg);
}

void MsgThread::removeSendMsg(QString packetNoString)
{
    QMutexLocker locker(&m_lock);

    m_sendMsgMap.remove(packetNoString);
}

void MsgThread::removeSendMsgNotLock(QString packetNoString)
{
    m_sendMsgMap.remove(packetNoString);
}

void MsgThread::handleError(QAbstractSocket::SocketError errorCode, QString s)
{
    qDebug() << "MsgThread::handleError";

    QString errorString(QObject::tr("udp server error"));

    QMessageBox::critical(0, QObject::tr("QIpMsg"),
                          errorString + ":\n" + s + ".");

    qDebug() << "MsgThread::handleError:" << s;

    if (errorCode == QAbstractSocket::AddressInUseError) {
        ::exit(-1);
    }
}

