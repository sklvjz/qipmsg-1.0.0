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

#ifndef RECV_MSG_H
#define RECV_MSG_H

#include "msg_base.h"

class RecvMsg : public MsgBase
{
public:
    RecvMsg() {}
    RecvMsg(QString packet, QHostAddress address, quint16 port)
        : MsgBase(packet, address, port) {}

    virtual ~RecvMsg() {}

    virtual RecvMsg* clone() const { return new RecvMsg(*this); }
};

#endif // !RECV_MSG_H

