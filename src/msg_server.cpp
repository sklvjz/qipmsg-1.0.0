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

#include "msg_server.h"
#include "send_msg.h"
#include "global.h"
#include "constants.h"
#include "transfer_codec.h"
#include "msg_thread.h"
#include "preferences.h"
#include "user_manager.h"
#include "send_file_manager.h"

#include <QMutexLocker>
#include <QTextCodec>
#include <QNetworkInterface>
#include <QAbstractSocket>
#include <QSet>

MsgServer::MsgServer(QObject *parent)
    : QObject(parent)
{
    updateAddresses();

    connect(&m_udpSocket, SIGNAL(readyRead()),
            this, SLOT(readPacket()));
    connect(this, SIGNAL(releaseFile(QString)),
            Global::sendFileManager, SLOT(removeTransferLocked(QString)));
    connect(&m_udpSocket, SIGNAL(error(QAbstractSocket::SocketError)),
            this, SLOT(socketEerror(QAbstractSocket::SocketError)));
}

void MsgServer::start()
{
    m_udpSocket.bind(QHostAddress::Any, IPMSG_DEFAULT_PORT);
}

void MsgServer::readPacket()
{
    qDebug() << "MsgServer::readPacket";

    while (m_udpSocket.hasPendingDatagrams()) {
        QHostAddress senderIp;
        quint16 senderPort;
        QByteArray datagram;
        datagram.resize(m_udpSocket.pendingDatagramSize());
        if (m_udpSocket.readDatagram(datagram.data(), datagram.size(),
                                         &senderIp, &senderPort) == -1) {
            continue;
        }

        if (!isSupportedCommand(datagram)) {
            continue;
        }

        QString packet
            = Global::transferCodec->codec()->toUnicode(datagram);

        RecvMsg recvMsg(packet, senderIp, senderPort);

        processRecvMsg(Msg(recvMsg));
    }
}

void MsgServer::processRecvMsg(Msg msg)
{
    qDebug() << "MsgServer::processRecvMsg";

    switch (GET_MODE(msg->flags())) {
    case IPMSG_BR_ENTRY:
        processEntryMsg(msg);
        break;

    case IPMSG_BR_EXIT:
        emit newExitMsg(msg);
        break;

    case IPMSG_ANSENTRY:
        emit newUserMsg(msg);
        break;

    case IPMSG_BR_ABSENCE:
        // XXX TODO: support it
        break;

    case IPMSG_SENDMSG:
        processRecvSendMsg(msg);
        break;

    case IPMSG_READMSG:
        processRecvReadMsg(msg);
        break;

    case IPMSG_ANSREADMSG:
    case IPMSG_RECVMSG:
    case IPMSG_DELMSG:
        // do nothing
        // XXX TODO: do something?????????
        break;

    case IPMSG_RELEASEFILES:
        processRecvReleaseFilesMsg(msg);
        break;

    default:
        break;
    }
}

void MsgServer::processEntryMsg(Msg msg)
{
    emit newUserMsg(msg);

    SendMsg sendMsg(msg->ipAddress(), msg->port(),
                    Global::userManager->entryMessage(),
                    ""/* extendedInfo */, IPMSG_ANSENTRY);

    Global::msgThread->addSendMsg(Msg(sendMsg));
}

void MsgServer::processRecvReleaseFilesMsg(Msg msg)
{
    emit releaseFile(msg->additionalInfo());
}

void MsgServer::processRecvReadMsg(Msg msg)
{
    if (Global::preferences->isReadCheck) {
        emit newMsg(msg);
    }

    SendMsg sendMsg(msg->ipAddress(), msg->port(), msg->packetNoString(),
                    ""/* extendedInfo */, IPMSG_ANSREADMSG);

    Global::msgThread->addSendMsg(Msg(sendMsg));
}

void MsgServer::processRecvSendMsg(Msg msg)
{
    if (GET_OPT(msg->flags()) & IPMSG_SENDCHECKOPT) {
        SendMsg sendMsg(msg->ipAddress(), msg->port(), msg->packetNoString(),
                ""/* extendedInfo */, IPMSG_RECVMSG);

        Global::msgThread->addSendMsg(Msg(sendMsg));
    }

    // If sender is not in our user list, add it.
    if (!Global::userManager->contains(msg->ip())) {
        emit newUserMsg(msg);
    }

    emit newMsg(msg);
}

void MsgServer::processSendMsg()
{
    // We are a friend of MsgThread class
    QMutexLocker locker(&Global::msgThread->m_lock);

    foreach (Msg msg, Global::msgThread->m_sendMsgMap) {
        handleMsg(msg);
    }
}

void MsgServer::handleMsg(Msg &msg)
{
    // Delete msg
    if (msg->state() == MsgBase::SendAckOk) {
        Global::msgThread->removeSendMsgNotLock(msg->packetNoString());
    }

    // Delete msg
    if (msg->sendTimes() >= MAX_RE_SEND_TIMES) {
        Global::msgThread->removeSendMsgNotLock(msg->packetNoString());
    }

    // Send msg
    if (msg->state() == MsgBase::NotSend
        || isResendNeeded(msg)) {

        msg->setState(MsgBase::Sending);
        msg->incrementSendTimes();

        switch (GET_MODE(msg->flags())) {
        case IPMSG_BR_ENTRY:
        case IPMSG_BR_EXIT:
        case IPMSG_BR_ABSENCE:
            broadcastUserMsg(msg);
            Global::msgThread->removeSendMsgNotLock(msg->packetNoString());
            break;

        case IPMSG_ANSENTRY:
        case IPMSG_ANSREADMSG:
        case IPMSG_RELEASEFILES:
            broadcastMsg(msg);
            Global::msgThread->removeSendMsgNotLock(msg->packetNoString());
            break;


        case IPMSG_READMSG:
            broadcastMsg(msg);
            // XXX TODO: If IPMSG_READCHECKOPT is on, we need to check if
            // sucessfully sended.
            break;

        case IPMSG_RECVMSG:
            broadcastMsg(msg);
            // XXX TODO: need to check if sucessfully sended.
            break;

        case IPMSG_SENDMSG:
            broadcastMsg(msg);
            // XXX TODO: need to check if sucessfully sended.
            break;

        default:
            Global::msgThread->removeSendMsgNotLock(msg->packetNoString());
            break;
        }
    }
}

bool MsgServer::isResendNeeded(Msg &msg) const
{
    if (msg->state() == MsgBase::SendFail
        && msg->sendTimes() < MAX_RE_SEND_TIMES) {
        return true;
    }

    return false;
}

void MsgServer::broadcastMsg(Msg &msg)
{
    QByteArray datagram
        = Global::transferCodec->codec()->fromUnicode(msg->packet());

    if (m_udpSocket.writeDatagram(datagram, msg->ipAddress(),
                                       msg->port()) == -1) {
        msg->setState(MsgBase::SendFail);
    } else { // Send Ok
        msg->setState(MsgBase::SendOk);
    }
}

void MsgServer::updateAddresses()
{
    m_broadcastAddresses.clear();
    m_ipAddresses.clear();

    foreach (QNetworkInterface interface, QNetworkInterface::allInterfaces()) {
        foreach (QNetworkAddressEntry entry, interface.addressEntries()) {
            QHostAddress broadcastAddress = entry.broadcast();
            if (broadcastAddress != QHostAddress::Null &&
                entry.ip() != QHostAddress::LocalHost) {
                m_broadcastAddresses << broadcastAddress;
                m_ipAddresses << entry.ip();
            }
        }
    }

    // Add broadcast address specified by user.
    foreach(QString s, Global::preferences->userSpecifiedBroadcastIpList) {
        QHostAddress h(s);
        if (h != QHostAddress::Null &&
            h != QHostAddress::LocalHost &&
            h != QHostAddress::LocalHostIPv6) {
            m_broadcastAddresses << h;
        }
    }

    // NOTE: Just to remove duplicate entries.
    QSet<QHostAddress> set = m_broadcastAddresses.toSet();
    m_broadcastAddresses.clear();
    m_broadcastAddresses = set.toList();
}

void MsgServer::broadcastUserMsg(Msg &msg)
{
    qDebug() << "MsgServer::broadcastUserMsg";

    updateAddresses();

    QByteArray datagram = Global::transferCodec->codec()
        ->fromUnicode(msg->packet());
    bool validBroadcastAddresses = true;
    foreach (QHostAddress address, m_broadcastAddresses) {
        if (m_udpSocket.writeDatagram(datagram, address,
                                       IPMSG_DEFAULT_PORT) == -1) {
            validBroadcastAddresses = false;
        }
    }

    if (!validBroadcastAddresses) {
        updateAddresses();
    }
}

bool MsgServer::isSupportedCommand(QByteArray &datagram) const
{
    QList<QByteArray> bl = datagram.split(':');

    if (bl.size() < MSG_NORMAL_FIELD_COUNT) {
        return false;
    }

    bool ok;
    long command = bl.at(MSG_FLAGS_POS).toLong(&ok);
    if (!ok) {
        return false;
    }

    bool isSupport = false;
    switch (GET_MODE(command)) {
    case IPMSG_NOOPERATION:
    case IPMSG_BR_ENTRY:
    case IPMSG_BR_EXIT:
    case IPMSG_ANSENTRY:
    case IPMSG_BR_ABSENCE:
    case IPMSG_BR_ISGETLIST:
    case IPMSG_OKGETLIST:
    case IPMSG_GETLIST:
    case IPMSG_ANSLIST:
    case IPMSG_BR_ISGETLIST2:
    case IPMSG_SENDMSG:
    case IPMSG_RECVMSG:
    case IPMSG_READMSG:
    case IPMSG_DELMSG:
    case IPMSG_ANSREADMSG:
    case IPMSG_GETINFO:
    case IPMSG_SENDINFO:
    case IPMSG_GETABSENCEINFO:
    case IPMSG_SENDABSENCEINFO:
    case IPMSG_GETFILEDATA:
    case IPMSG_RELEASEFILES:
    case IPMSG_GETDIRFILES:
    case IPMSG_GETPUBKEY:
    case IPMSG_ANSPUBKEY:
        isSupport = true;
        break;

    default:
        isSupport = false;
    }

    return isSupport;
}

void MsgServer::socketEerror(QAbstractSocket::SocketError errorCode)
{
    emit error(errorCode, m_udpSocket.errorString());
}

