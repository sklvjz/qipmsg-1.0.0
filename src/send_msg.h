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

#ifndef SEND_MSG_H
#define SEND_MSG_H

#include "msg_base.h"

class SendMsg : public MsgBase
{
public:
    SendMsg() {}
    SendMsg(QHostAddress address, quint16 port, QString additionalInfo,
            QString extendedInfo, qint32 flags)
        : MsgBase(address, port, additionalInfo, extendedInfo,
                  flags), m_state(NotSend), m_sendTimes(0) {}

    virtual ~SendMsg();

    virtual SendMsg* clone() const { return new SendMsg(*this); }

    void setState(States state) { m_state = state; }
    MsgBase::States state() const { return m_state; }

    int sendTimes() const { return m_sendTimes; }
    void incrementSendTimes() { ++m_sendTimes; }

private:
    States m_state;
    int m_sendTimes;
};

#endif // !SEND_MSG_H

