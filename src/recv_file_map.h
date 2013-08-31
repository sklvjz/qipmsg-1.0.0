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

#ifndef RECV_FILE_MAP_H
#define RECV_FILE_MAP_H

#include "recv_file_handle.h"

#include <QDateTime>
#include <QTimer>

class RecvFileMap
{
public:
    friend class RecvFileTransfer;
    friend class RecvFileFinishDialog;
    friend class MsgWindow;

    enum States { Normal, Retry };
    enum TransferStates { NotTransfer, Transfer };

    RecvFileMap(): m_currentId(-1), m_dirCount(0), m_regularFileCount(0),
    m_totalRegularFileCount(0), m_totalBytesReaded(0),
    m_state(Normal), m_transferState(NotTransfer) {}

    void resetStats();

    void addFile(int id, RecvFileHandle handle) { m_map.insert(id, handle); }

    QString fileNameJoin(QString sep) const;

    void setCurrentId(int id) { m_currentId = id; }
    int currentId() const { return m_currentId; }

    RecvFileHandle currentFile() const { return m_map[m_currentId]; }

    void setSaveFilePath(QString path) { m_saveFilePath = path; }
    QString saveFilePath() const { return m_saveFilePath; }

    void incrDirCount() { ++m_dirCount; }
    int dirCount() const { return m_dirCount; }

    void incrRegularFileCount() { ++m_regularFileCount; }
    int regularFileCount() const { return m_regularFileCount; }

    void incrTotalRegularFileCount() { ++m_totalRegularFileCount; }
    int totalRegularFileCount() const { return m_totalRegularFileCount; }

    QString transferStatsInfo() const;

    QString secondStringUnit(int second) const;

    void addBytesReaded(qint64 size) { m_totalBytesReaded += size; }

    void setStartTime() { m_begin = QDateTime::currentDateTime(); }
    void setEndTime() { m_end = QDateTime::currentDateTime(); }

    quint32 elapse() const {
        return qMax(m_end.toTime_t() - m_begin.toTime_t(), (quint32)1);
    }

    double totalTransferRateAvg() const {
        return m_totalBytesReaded / (double)elapse();
    }

    void startTimer() { m_timer.start(1000); }
    void stopTimer() { m_timer.stop(); }

    States state() const { return m_state; }
    void setState(States state) { m_state = state; }

    TransferStates transferState() const { return m_transferState; }
    void setTransferState(TransferStates state) { m_transferState = state; }

private:
    // file id of file current transfered
    int m_currentId;
    QMap<int, RecvFileHandle> m_map;

    int m_dirCount;
    int m_regularFileCount;
    int m_totalRegularFileCount;

    qint64 m_totalBytesReaded;

    QDateTime m_begin;
    QDateTime m_end;

    QString m_saveFilePath;

    QTimer m_timer;

    States m_state;
    TransferStates m_transferState;
};

#endif // !RECV_FILE_MAP_H

