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

#include "recv_file.h"
#include "constants.h"
#include "helper.h"

#include <QStringList>

RecvFile::RecvFile(QString ip, QString packetNoString, QString info)
    : m_ip(ip), m_packetNoString(packetNoString), m_offset(0),
    m_bytesReaded(0), m_regularFileCount(0), m_state(NotRecv)
{
    info.replace(QString(FILE_NAME_BEFORE), QString(FILE_NAME_ESCAPE));

    QStringList list = info.split(QChar(COMMAND_SEPERATOR));

    if (list.size() >= RECV_FILE_EXTEND_ATTR_POS) {
        bool ok;
        m_fileId = list.at(RECV_FILE_ID_POS).toInt(&ok, 10);
        m_name = list.at(RECV_FILE_NAME_POS);
        m_name.replace(QString(FILE_NAME_ESCAPE), QString(FILE_NAME_AFTER));
        m_size = list.at(RECV_FILE_SIZE_POS).toLongLong(&ok, 16);
        m_type = list.at(RECV_FILE_ATTR_POS).toInt(&ok, 16);

        // Extended file attribution like mtime, atime...
        for (int i = RECV_FILE_EXTEND_ATTR_POS; i < list.size(); ++i) {
            QString s = list.at(i);
            QStringList l = s.split(QChar('='));
            if (l.size() == 2) {
                int i = l.at(0).toInt(&ok, 16);
                if (ok) {
                    m_attrMap.insert(i, l.at(1));
                }
            }
        }
    }
}

RecvFile::~RecvFile()
{
}

QString RecvFile::sizeString() const
{
    if (m_type == IPMSG_FILE_DIR) {
        return QString("Folder");
    }

    return QString("%1").arg(Helper::sizeStringUnit(m_size, " "));
}

QString RecvFile::transferStatsInfo() const
{
    if (m_type == IPMSG_FILE_REGULAR) {
        QString bytesReadedString;
        if (m_size >= ONE_MB) {
            bytesReadedString = QString("%1")
                .arg(qMax(m_bytesReaded/ONE_MB, 0.1), 0, 'f', 1);
        } else {
            bytesReadedString = QString("%1")
                .arg(qMax(m_bytesReaded/ONE_KB, 1.0), 0, 'f', 0);
        }
        QString fileSizeString = bytesReadedString + "/"
            + Helper::sizeStringUnit(m_size);

        QString transferRateString
            = Helper::sizeStringUnit(transferRateAvg()) + "/s";

        QString transferPercentString = QString("(%1%2)")
            .arg(percent(), 0, 'f', 0)
            .arg("%");

        return (fileSizeString + " " + transferRateString
                + " " + transferPercentString);
    } else if (m_type == IPMSG_FILE_DIR) {
            return (QObject::tr("Total") + " "
                    + Helper::sizeStringUnit((double)m_bytesReaded) + "/"
                    + Helper::fileCountString(m_regularFileCount) + " ("
                    + Helper::sizeStringUnit(transferRateAvg()) + "/s)");
    }

    return QString();
}

double RecvFile::transferRateAvg() const
{
    return m_bytesReaded / (double)elapse();
}

double RecvFile::percent() const
{
    // If a empty file
    if (m_size == 0) {
        return 100;
    }

    return ((double)m_bytesReaded / m_size) * 100;
}

void RecvFile::resetStats()
{
    m_bytesReaded = 0;
    m_regularFileCount = 0;
}

