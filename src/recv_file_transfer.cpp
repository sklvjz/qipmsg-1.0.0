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

#include "recv_file_transfer.h"
#include "recv_file_handle.h"
#include "recv_file_map.h"
#include "constants.h"
#include "helper.h"
#include "global.h"
#include "user_manager.h"
#include "transfer_codec.h"

#include <QFile>
#include <QDir>
#include <QTextCodec>

#include <sys/types.h>
#include <utime.h>

struct TransferFile
{
    QString name;
    qint64 size;
    int type;
    QMap<int, QString> extendAttr;
};

void RecvFileTransfer::startTransfer()
{
    m_recvFileMap->setStartTime();
    m_recvFileMap->resetStats();

    foreach (RecvFileHandle h, m_recvFileMap->m_map) {
        if (h->state() == RecvFile::RecvOk
            || h->state() == RecvFile::NotRecv) {
            continue;
        }

        h->resetStats();

        m_recvFileMap->setCurrentId(h->fileId());
        m_recvFileMap->startTimer();

        m_tcpSocket.disconnectFromHost();
        m_tcpSocket.connectToHost(h->ipAddress(), IPMSG_DEFAULT_PORT);
        if (!m_tcpSocket.waitForConnected(1000)) {
            m_errorString = m_tcpSocket.errorString();
            goto transfer_fail;
        }

        if (h->type() == IPMSG_FILE_REGULAR) {
            if (!recvFileRegular(h)) {
                goto transfer_fail;
            }
        } else if (h->type() == IPMSG_FILE_DIR) {
            if (!recvFileDir(h)) {
                goto transfer_fail;
            }
        }

        m_recvFileMap->stopTimer();
    }

    m_tcpSocket.disconnectFromHost();
    m_recvFileMap->setEndTime();
    emit recvFileFinished();
    return;

transfer_fail:
    m_tcpSocket.disconnectFromHost();
    m_recvFileMap->setEndTime();
    m_recvFileMap->stopTimer();
    // if 'isAbortTransfer' is true, it's a manually stop, not emit error
    if (!isAbortTransfer) {
        emit recvFileError(m_errorString);
    }
    return;
}

bool RecvFileTransfer::recvFileRegular(RecvFileHandle h)
{
    h->setStartTime();

    QByteArray sendBlock(constructRecvFileDatagram(h));
    m_tcpSocket.write(sendBlock);
    if (m_tcpSocket.waitForBytesWritten(3000) == -1) {
        m_errorString = m_tcpSocket.errorString();
        return false;
    }

    QFile file(m_recvFileMap->saveFilePath() + "/" + h->name());
    QFlags<QIODevice::OpenModeFlag> flags;
    if (m_recvFileMap->state() == RecvFileMap::Retry) {
        flags = QIODevice::WriteOnly | QIODevice::Append;
    } else {
        flags = QIODevice::WriteOnly;
    }
    if (!file.open(flags)) {
        m_errorString = file.errorString();
        return false;
    }

    qint64 bytesReaded = 0;
    qint64 bytesToRead;
    if (m_recvFileMap->state() == RecvFileMap::Retry) {
        bytesToRead = h->size() - h->offset();
    } else {
        bytesToRead = h->size();
    }
    while (bytesReaded < bytesToRead) {
        if (!m_tcpSocket.waitForReadyRead(3000)) {
            m_errorString = m_tcpSocket.errorString();
            goto recv_file_error;
        }

        qint64 bytesAvailable = m_tcpSocket.bytesAvailable();
        bytesReaded += bytesAvailable;

        if (!saveData(m_tcpSocket.read(m_tcpSocket.bytesAvailable()), file)) {
            goto recv_file_error;
        }

        // we need this because QTcpSocket error signal may happend any time
        h->setBytesReaded(bytesReaded);
        h->addOffset(bytesAvailable);

        if (isStopTransfer) {
            m_lock.lock();
            m_cond.wait(&m_lock);
            m_lock.unlock();
        }

        if (isAbortTransfer) {
            goto recv_file_error;
        }
    }

    file.close();
    // set modify time
    setLastModified(h);
    h->setState(RecvFile::RecvOk);
    m_recvFileMap->addBytesReaded(bytesReaded);

    h->incrRegularFileCount();
    m_recvFileMap->incrRegularFileCount();
    m_recvFileMap->incrTotalRegularFileCount();

    return true;

recv_file_error:
    file.close();
    h->setState(RecvFile::RecvFail);
    m_recvFileMap->addBytesReaded(bytesReaded);

    return false;
}

void RecvFileTransfer::setLastModified(QString path, QString secondString)
{
    if (!secondString.isEmpty()) {
        bool ok;
        quint64 secs = secondString.toULongLong(&ok, 16);
        if (ok) {
            struct utimbuf buf;
            // XXX NOTE: we set both access time and modify time to modify time
            buf.actime = secs;
            buf.modtime = secs;
            utime(path.toLocal8Bit(), &buf);
        }
    }
}

void RecvFileTransfer::setLastModified(RecvFileHandle h)
{
    QString path = m_recvFileMap->saveFilePath() + "/" + h->name();
    QString s = h->attrMap().value(IPMSG_FILE_MTIME);
    if (!s.isEmpty()) {
        bool ok;
        quint64 secs = s.toULongLong(&ok, 16);
        if (ok) {
            struct utimbuf buf;
            // XXX NOTE: we set both access time and modify time to modify time
            buf.actime = secs;
            buf.modtime = secs;
            utime(path.toLocal8Bit(), &buf);
        }
    }
}

bool RecvFileTransfer::recvFileDir(RecvFileHandle h)
{
    h->setStartTime();

    QByteArray sendBlock(constructRecvFileDatagram(h));
    m_tcpSocket.write(sendBlock);
    if (m_tcpSocket.waitForBytesWritten(3000) == -1) {
        m_errorString = m_tcpSocket.errorString();
        return false;
    }

    QDir dir(m_recvFileMap->saveFilePath());
    QByteArray recvBlock;
    qint64 bytesToWrite = 0;
    bool isRecvContentData = false;
    QFile file;
    struct TransferFile transferFile;
    forever {
        if (isStopTransfer) {
            m_lock.lock();
            m_cond.wait(&m_lock);
            m_lock.unlock();
        }
        if (isAbortTransfer) {
            h->setState(RecvFile::RecvFail);
            return false;
        }

        if (!m_tcpSocket.waitForReadyRead(3000)) {
            m_errorString = m_tcpSocket.errorString();
            return false;
        }

        recvBlock.append(m_tcpSocket.read(m_tcpSocket.bytesAvailable()));

        if (isRecvContentData) {
            if (bytesToWrite > recvBlock.size()) {
                saveData(recvBlock, file);
                h->addBytesReaded(recvBlock.size());
                m_recvFileMap->addBytesReaded(recvBlock.size());
                bytesToWrite -= recvBlock.size();
                recvBlock.clear();
                continue;
            } else {
                isRecvContentData = false;
                saveData(recvBlock.left(bytesToWrite), file);
                h->addBytesReaded(bytesToWrite);
                m_recvFileMap->addBytesReaded(bytesToWrite);
                recvBlock.remove(0, bytesToWrite);
                bytesToWrite = 0;
                m_recvFileMap->incrTotalRegularFileCount();
                h->incrRegularFileCount();
                file.close();   // successfully get file
                setLastModified(dir.absolutePath() + "/" + transferFile.name,
                                transferFile.extendAttr.value(IPMSG_FILE_MTIME));
            }
        }

        while (!isRecvContentData && canParseHeader(recvBlock)) {
            if (!parseHeader(recvBlock, transferFile)) {
                return false;
            }

            if (transferFile.type == IPMSG_FILE_REGULAR) {
                bytesToWrite = transferFile.size;
                file.setFileName(dir.absolutePath() + "/" + transferFile.name);
                if (!file.open(QIODevice::WriteOnly)) {
                    m_errorString = "RecvFileTransfer::recvFileDir:"
                        + file.errorString();
                    return false;
                }

                if (bytesToWrite <= recvBlock.size()) {
                    saveData(recvBlock.left(bytesToWrite), file);
                    h->addBytesReaded(bytesToWrite);
                    m_recvFileMap->addBytesReaded(bytesToWrite);
                    recvBlock.remove(0, bytesToWrite);
                    m_recvFileMap->incrTotalRegularFileCount();
                    h->incrRegularFileCount();
                    file.close();   // successfully get file
                    setLastModified(dir.absolutePath() + "/"
                                    + transferFile.name,
                                    transferFile.extendAttr.value(IPMSG_FILE_MTIME));
                } else {
                    isRecvContentData = true;
                    saveData(recvBlock, file);
                    h->addBytesReaded(recvBlock.size());
                    m_recvFileMap->addBytesReaded(recvBlock.size());
                    bytesToWrite -= recvBlock.size();
                    recvBlock.clear();
                }
            } else if (transferFile.type == IPMSG_FILE_DIR) {
                if (m_recvFileMap->state() == RecvFileMap::Retry
                    && dir.exists(transferFile.name)) {
                    // nothing to do
                } else if (!dir.mkdir(transferFile.name)) {
                    m_errorString
                        = "RecvFileTransfer::recvFileDir: mkdir error";
                    return false;
                }
                dir.cd(transferFile.name);
                // set folder modify name
                setLastModified(dir.absolutePath(),
                                transferFile.extendAttr.value(IPMSG_FILE_MTIME));
            } else if (transferFile.type == IPMSG_FILE_RETPARENT) {
                if (!dir.cdUp()) {
                    m_errorString
                        = "RecvFileTransfer::recvFileDir: cdUp error";
                    return false;
                }

                QString path;
                if (!dir.isRoot()) {
                    path = dir.absolutePath() + "/";
                } else {
                    path = dir.absolutePath();
                }
                if (QDir::cleanPath(path) == QDir::cleanPath(m_recvFileMap->saveFilePath())) {
                    h->setState(RecvFile::RecvOk);
                    m_recvFileMap->incrDirCount();
                    return true;
                }
            } else {
                m_errorString
                    = "RecvFileTransfer::recvFileDir: unsupported file type";
                return false;
            }
        }
    }
}

bool RecvFileTransfer::parseHeader(QByteArray &recvBlock,
                                   struct TransferFile &transferFile)
{
    bool ok;
    int headerSize
        = recvBlock.left(TRANSFER_FILE_HEADER_SIZE_LENGTH).toInt(&ok, 16);
    if (!ok) {
        m_errorString
            = "RecvFileTransfer::parseHeader: get headerSize error";
        return false;
    }

    QByteArray header = recvBlock.left(headerSize);

    QList<QByteArray> list = header.split(':');

#define TRANSFERFILE_NAME_POS           1
#define TRANSFERFILE_SIZE_POS           2
#define TRANSFERFILE_TYPE_POS           3
#define TRANSFERFILE_ATTR_BEGIN_POS     4

    // XXX NOTE: canParseHeader() make sure we have enough items in list,
    // so we do not need to check size of list before call list.at()
    transferFile.name = Global::transferCodec->codec()
        ->toUnicode(list.at(TRANSFERFILE_NAME_POS));
    transferFile.size = list.at(TRANSFERFILE_SIZE_POS).toLongLong(&ok, 16);
    if (!ok) {
        m_errorString = "RecvFileTransfer::parseHeader: get file size error";
        return false;
    }
    transferFile.type = list.at(TRANSFERFILE_TYPE_POS).toInt(&ok, 16);
    if (!ok) {
        m_errorString = "RecvFileTransfer::parseHeader: get file type error";
        return false;
    }

    // Extended file attribution like mtime, atime...
    for (int i = TRANSFERFILE_ATTR_BEGIN_POS; i < list.size(); ++i) {
        QString s = list.at(i);
        QStringList l = s.split(QChar('='));
        if (l.size() == 2) {
            int i = l.at(0).toInt(&ok, 16);
            if (ok) {
                transferFile.extendAttr.insert(i, l.at(1));
            }
        }
    }

    recvBlock.remove(0, headerSize);

    return true;
}

bool RecvFileTransfer::canParseHeader(QByteArray &recvBlock)
{
    if (recvBlock.size() < TRANSFER_FILE_HEADER_SIZE_LENGTH)
        return false;

    bool ok;
    int headerSize
        = recvBlock.left(TRANSFER_FILE_HEADER_SIZE_LENGTH).toInt(&ok, 16);

    if (recvBlock.size() >= headerSize) {
        return true;
    }

    return false;
}

bool RecvFileTransfer::saveData(QByteArray recvBlock, QFile &file)
{
    qint64 bytesToWrite = recvBlock.size();

    forever {
        qint64 bytesWrited = file.write(recvBlock);

        if (bytesWrited == -1) {
            m_errorString = file.errorString();
            return false;
        }
        bytesToWrite -= bytesWrited;
        if (bytesToWrite == 0) {
            break;
        }
        recvBlock.remove(0, bytesWrited);
    }

    return true;
}

QByteArray RecvFileTransfer::constructRecvFileDatagram(RecvFileHandle h)
{
    QString s = QString("%1:%2:%3:%4").arg(IPMSG_VERSION)
        .arg(Helper::packetNoString())
        .arg(Global::userManager->ourself().loginName())
        .arg(Global::userManager->ourself().host());

    int flags = 0;
    if (h->type() == IPMSG_FILE_REGULAR) {
        flags = IPMSG_GETFILEDATA;
    } else if (h->type() == IPMSG_FILE_DIR) {
        flags = IPMSG_GETDIRFILES;
    }

    s.append(":");
    s.append(QString("%1:%2:%3:").arg(flags, 0, 10)
            .arg(h->packetNo(), 0, 16)
            .arg(h->fileId(), 0, 16));

    if (h->type() == IPMSG_FILE_REGULAR) {
        if (m_recvFileMap->state() == RecvFileMap::Retry) {
            s.append(QString("%1:").arg(h->offset(), 0, 16));
        } else {
            s.append(QString("%1:").arg(0/* offset */, 0, 16));
        }
    }

    QByteArray ba;
    ba.append(s);
    return ba;
}

void RecvFileTransfer::stopTransfer()
{
    qDebug() << "RecvFileTransfer::stopTransfer";

    isStopTransfer = true;
}

void RecvFileTransfer::abortTransfer()
{
    qDebug() << "RecvFileTransfer::abortTransfer";

    isAbortTransfer = true;
}

void RecvFileTransfer::resumeTransfer()
{
    qDebug() << "RecvFileTransfer::resumeTransfer";

    if (isStopTransfer) {
        // XXX NOTE: we need first set 'isStopTransfer' to false to avoid
        // m_cond.wait() be called.
        m_lock.lock();
        isStopTransfer = false;
        m_cond.wakeOne();
        m_lock.unlock();
    }
}

