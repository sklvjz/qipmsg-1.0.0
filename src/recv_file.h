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

#ifndef RECV_FILE_H
#define RECV_FILE_H

#include <QString>
#include <QtGlobal>
#include <QMap>
#include <QHostAddress>
#include <QDateTime>

class RecvFile
{
public:
    enum States { NotRecv, ToRecv, RecvOk, RecvFail };

    RecvFile(const QString ip, const QString packetNoString,
             const QString info);
    ~RecvFile();

    virtual RecvFile* clone() const { return new RecvFile(*this); }

    int fileId() const { return m_fileId; }
    QString fileIdString() const { return QString("%1").arg(m_fileId); }

    QString name() const { return m_name; }

    QString sizeString() const;
    qint64 size() const { return m_size; }

    QString packetNoString() const { return m_packetNoString; }
    qint64 packetNo() const {
        bool ok;
        return m_packetNoString.toLongLong(&ok, 10);
    }

    int type() const { return m_type; }

    qint64 offset() const { return m_offset; }
    void addOffset(qint64 offset) { m_offset += offset; }

    qint64 bytesReaded() const { return m_bytesReaded; }
    void setBytesReaded(qint64 bytesReaded) { m_bytesReaded = bytesReaded; }
    void addBytesReaded(qint64 size) { m_bytesReaded += size; }

    QHostAddress ipAddress() const { return QHostAddress(m_ip); }
    QString ip() const { return m_ip; }

    void setState(States state) { m_state = state; }
    States state() const { return m_state; }

    void setStartTime() { m_begin = QDateTime::currentDateTime(); }
    void setEndTime() { m_end = QDateTime::currentDateTime(); }
    int elapse() const
    {
        return qMax(QDateTime::currentDateTime().toTime_t()
                    - m_begin.toTime_t(), (unsigned int)1);
    }

    void incrRegularFileCount() { ++m_regularFileCount; }

    QString transferStatsInfo() const;

    void resetStats();

    const QMap<int, QString>& attrMap() { return m_attrMap; }

private:
    double transferRateAvg() const;
    double percent() const;

    QString m_ip;
    QString m_packetNoString;
    int m_fileId;
    QString m_name;
    qint64 m_size;
    int m_type;
    QMap<int, QString> m_attrMap;

    qint64 m_offset;
    qint64 m_bytesReaded;
    int m_regularFileCount;

    enum States m_state;

    QDateTime m_begin;
    QDateTime m_end;
};

#endif // !RECV_FILE_H

