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

#include "send_file_map.h"
#include "send_file.h"
#include "constants.h"
#include "helper.h"

#include <QStringList>
#include <QList>
#include <QDateTime>
#include <QtDebug>

SendFileMap::SendFileMap(QObject *parent)
    : QObject(parent), m_transferedCount(0), m_state(NotTransfer)
{
}

void SendFileMap::addFile(const QStringList &pathList)
{
    int id = 0;
    foreach (QString path, pathList) {
        SendFile sendFile(path);
        m_map.insert(id, SendFileHandle(sendFile));
        ++id;
    }
}

QString SendFileMap::packetString() const
{
    QList<int> keys = m_map.keys();
    QString fileInfoString;

    foreach (int key, keys) {
        SendFileHandle h = m_map.value(key);

        QString s = QString("%1").arg(key);
        s.append(COMMAND_SEPERATOR);
        s.append(h->fileName().replace(":", "::"));
        s.append(COMMAND_SEPERATOR);
        s.append(QString("%1").arg(h->size(), 2, 16, QChar('0')));
        s.append(COMMAND_SEPERATOR);
        s.append(QString("%1").arg(h->lastModified().toTime_t(),
                    0, 16));
        s.append(COMMAND_SEPERATOR);
        if (h->isFile()) {
            s.append(QString("%1").arg(IPMSG_FILE_REGULAR));
        } else if (h->isDir()) {
            s.append(QString("%1").arg(IPMSG_FILE_DIR));
        }
        s.append(COMMAND_SEPERATOR);
        s.append(QString("%1=").arg(IPMSG_FILE_MTIME, 0, 16));
        s.append(QString("%1").arg(h->lastModified().toTime_t(), 0, 16));
        s.append(COMMAND_SEPERATOR);
        s.append(QString("%1=").arg(IPMSG_FILE_CREATETIME, 0, 16));
        s.append(QString("%1").arg(h->created().toTime_t(), 0, 16));
        s.append(COMMAND_SEPERATOR);
        s.append(FILELIST_SEPARATOR);

        fileInfoString.append(s);
    }

    return fileInfoString;
}

QString SendFileMap::fileNames() const
{
    QStringList items;

    foreach (SendFileHandle h, m_map) {
        items << h->fileName();
    }

    return items.join(" ");
}

QString SendFileMap::sizeInfo() const
{
    int dirCount = 0;
    int regularFileCount = 0;
    qint64 size = 0;

    foreach (SendFileHandle h, m_map) {
        if (h->isDir()) {
            ++dirCount;
        } else {
            ++regularFileCount;
            size += h->size();
        }
    }

    QString s;
    if (regularFileCount > 0) {
        s = Helper::sizeStringUnit(size);
    }

    if (dirCount > 0) {
        s = s + (regularFileCount > 0 ? "/" : "") + QString("%1").arg(dirCount)
            + " " + (dirCount > 1 ? tr("Folders") : tr("Folder"));
    }

    return s;
}

QString SendFileMap::sendStats() const
{
    return QString("%1/%2/%3/").arg(m_map.count(), 1, 10)
        .arg(m_transferedCount)
        .arg(isTransfer() ? 1 : 0);
}

bool SendFileMap::isTransfer() const
{
    return (m_state == Transfer);
}

QString SendFileMap::recvUserInfo() const
{
    return m_recvUser + "(" + m_recvHostname + ")";
}

bool SendFileMap::canSendFile(int fileId) const
{
    if (m_map.contains(fileId)
        && m_map[fileId]->state() != SendFile::SendOk) {
        return true;
    }

    return false;
}

bool SendFileMap::isFinished() const
{
    foreach (SendFileHandle h, m_map) {
        if (h->state() != SendFile::SendOk) {
            return false;
        }
    }

    return true;
}

