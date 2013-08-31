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

#ifndef MSG_BASE_H
#define MSG_BASE_H

#include <QHostAddress>

#include "owner.h"

class MsgBase
{
public:
    enum States {
        NotSend = 0, Sending = 1, SendOk = 2, SendFail = 3, SendAckOk = 4
    };

    MsgBase() {}

    // Create receive message from receive packet
    MsgBase(QString packet, QHostAddress address, quint16 port);

    // Create send message
    MsgBase(QHostAddress address, quint16 port, QString additionalInfo,
        QString extendedInfo, quint32 flags);

    virtual ~MsgBase();

    virtual MsgBase* clone() const { return new MsgBase(*this); }

    virtual QString packet() const { return m_packet; }

    virtual QString packetNoString() const { return m_packetNoString; }

    virtual quint32 flags() const { return m_flags; }

    virtual quint16 port() const { return m_port; }

    virtual const Owner& owner() const { return m_owner; }

    virtual QString ip() const { return m_ipAddress.toString(); }
    virtual QHostAddress ipAddress() const { return m_ipAddress; }

    virtual QString additionalInfo() const { return m_additionalInfo; }
    virtual QString extendedInfo() const { return m_extendedInfo; }

    // We set all these methods to empty
    // XXX NOTE: only send msg use these functions
    virtual void setState(States state) {}
    virtual MsgBase::States state() const {}
    virtual int sendTimes() const {}
    virtual void incrementSendTimes() {}

private:
    void parsePacket();
    void parseAdditionalInfo();
    void constructPacket();

    Owner m_owner;
    QString m_packet;
    QString m_extendedInfo;
    QString m_additionalInfo;
    QString m_packetNoString;
    quint32 m_flags;
    QHostAddress m_ipAddress;
    quint16 m_port;
};

#endif // !MSG_BASE_H

