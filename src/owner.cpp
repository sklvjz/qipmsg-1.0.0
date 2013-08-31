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

#include "owner.h"
#include "constants.h"

#include <QStringList>

Owner::Owner(QString packet, QHostAddress address, quint16 port)
    : m_ipAddress(address), m_port(port)
{
    initOwner(packet);
}

#if 0
Owner::Owner(const Owner &rhs)
{
    m_name = rhs.m_name;
    m_loginName = rhs.m_loginName;
    m_group = rhs.m_group;
    m_host = rhs.m_host;
    m_ipAddress = rhs.m_ipAddress;
    m_port = rhs.m_port;
    m_displayLevel = rhs.m_displayLevel;
}

Owner& Owner::operator=(const Owner &rhs)
{
    if (&rhs == this) {
        return *this;
    }

    m_name = rhs.m_name;
    m_loginName = rhs.m_loginName;
    m_group = rhs.m_group;
    m_host = rhs.m_host;
    m_ipAddress = rhs.m_ipAddress;
    m_port = rhs.m_port;
    m_displayLevel = rhs.m_displayLevel;

    return *this;
}
#endif

void Owner::initOwner(QString &packet)
{
    QStringList list = packet.split(COMMAND_SEPERATOR);

    if (list.size() < MSG_NORMAL_FIELD_COUNT) {
        return;
    }

    m_loginName = list.at(MSG_LOG_NAME_POS);

    quint32 flag = list.at(MSG_FLAGS_POS).toUInt();
    if (flag & IPMSG_BR_ENTRY || flag & IPMSG_BR_ABSENCE) {
        m_group = packet.section(QChar(EXTEND_INFO_SEPERATOR), 1, 1);
        m_name = parseAdditionalInfo(packet);
        if (m_name.isEmpty()) {
            m_name = m_loginName;
        }
    }

    // No sender name in this situation, so we set sender name to
    // sender's login name.
    if (flag & IPMSG_SENDMSG) {
        m_name = m_loginName;
    }

    m_host = list.at(MSG_HOST_POS);
}

QString Owner::parseAdditionalInfo(QString &packet)
{
    int cnt = 0;
    int index = 0;
    while (cnt < MSG_ADDITION_INFO_POS) {
        int id = packet.indexOf(QChar(COMMAND_SEPERATOR), index);
        if (index == -1) {
            break;
        }
        ++cnt;
        index = id + 1;
    }

    QString s = packet.right(packet.size() - index);

    return s.section(QChar(EXTEND_INFO_SEPERATOR), 0, 0);
}

