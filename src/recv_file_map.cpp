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

#include "recv_file_map.h"
#include "constants.h"
#include "helper.h"

#include <QStringList>

QString RecvFileMap::fileNameJoin(QString sep) const
{
    QStringList l;
    foreach (RecvFileHandle h, m_map) {
        l << h->name();
    }

    return l.join(sep);
}

QString RecvFileMap::transferStatsInfo() const
{
    QString fileString;
    if (m_dirCount < 1 && m_regularFileCount == 1) {
        fileString = currentFile()->name();
    } else {
        fileString = Helper::fileCountString(m_totalRegularFileCount);
    }

    return (QObject::tr("Total:") + " " + Helper::sizeStringUnit(m_totalBytesReaded)
           + " (" + Helper::sizeStringUnit(totalTransferRateAvg()) + "/s)"
           + "\n" + Helper::secondStringUnit(elapse()) + "    "
           + fileString);
}

void RecvFileMap::resetStats()
{
    m_currentId = -1;
    m_dirCount = 0;
    m_regularFileCount = 0;
    m_totalRegularFileCount = 0;
    m_totalBytesReaded = 0;
}

