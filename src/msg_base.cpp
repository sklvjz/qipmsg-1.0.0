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

#include "msg_base.h"
#include "helper.h"
#include "global.h"
#include "user_manager.h"
#include "constants.h"

#include <QStringList>


MsgBase::MsgBase(QString packet, QHostAddress address, quint16 port)
    : m_owner(packet, address, port), m_packet(packet),
    m_ipAddress(address), m_port(port)
{
    parsePacket();
}

MsgBase::MsgBase(QHostAddress address, quint16 port, QString additionalInfo,
                 QString extendedInfo, quint32 flags)
    : m_ipAddress(address), m_port(port), m_additionalInfo(additionalInfo),
    m_extendedInfo(extendedInfo), m_flags(flags)
{
    m_packetNoString = Helper::packetNoString();
    m_owner = Global::userManager->ourself();

    constructPacket();
}

MsgBase::~MsgBase()
{
}

void MsgBase::constructPacket()
{
    m_packet.append(QString("%1%2").arg(IPMSG_VERSION).arg(COMMAND_SEPERATOR));

    m_packet.append(QString("%1%2").arg(m_packetNoString)
                    .arg(COMMAND_SEPERATOR));

    m_packet.append(m_owner.loginName());
    m_packet.append(COMMAND_SEPERATOR);
    m_packet.append(m_owner.host());
    m_packet.append(COMMAND_SEPERATOR);
    m_packet.append(QString("%1%2").arg(m_flags).arg(COMMAND_SEPERATOR));
    m_packet.append(m_additionalInfo);
}

void MsgBase::parsePacket()
{
    QStringList list = m_packet.split(COMMAND_SEPERATOR);

    // sanity check
    if (list.count() < MSG_NORMAL_FIELD_COUNT) {
        return;
    }

    m_extendedInfo = m_packet.section(QChar(EXTEND_INFO_SEPERATOR), 1, 1);
    parseAdditionalInfo();

    m_packetNoString = list.at(MSG_PACKET_NO_POS);
    m_flags = list.at(MSG_FLAGS_POS).toUInt();
}

void MsgBase::parseAdditionalInfo()
{
    int cnt = 0;
    int index = 0;
    while (cnt < MSG_ADDITION_INFO_POS) {
        int id = m_packet.indexOf(QChar(COMMAND_SEPERATOR), index);
        if (index == -1) {
            break;
        }
        ++cnt;
        index = id + 1;
    }

    if (index == -1) {
        m_additionalInfo = "";
    } else {
        QString s = m_packet.right(m_packet.size() - index);
        m_additionalInfo = s.section(QChar(EXTEND_INFO_SEPERATOR), 0, 0);
    }
}

