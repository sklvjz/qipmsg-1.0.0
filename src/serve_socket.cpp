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

#include "serve_socket.h"
#include "constants.h"
#include "transfer_codec.h"
#include "send_file_map.h"
#include "send_file_manager.h"
#include "global.h"

#include <QDir>
#include <QDateTime>
#include <QTextCodec>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>

#define REQUST_FILE_PACKET_ID_POSITION      5
#define REQUST_FILE_FILE_ID_POSITION        6
#define REQUST_FILE_OFFSET_POSITION         7
#define MAXBUFF                             8192

#define BLOCK_SIZE                          1024*16

struct RequsetFile
{
    bool isFileSended;
    int fileType;
    QString filePath;
    qint64 offset;
    int fileId;
};

ServeSocket::ServeSocket(int socketDescriptor, QObject *parent)
    : QObject(parent)
{
  m_sockfd = socketDescriptor;
#if 0
    connect(&m_tcpSocket, SIGNAL(bytesWritten(qint64)),
            this, SLOT(updateBytesWrited(qint64)));
    connect(&m_tcpSocket, SIGNAL(error(QAbstractSocket::SocketError)),
            this, SLOT(socketError(QAbstractSocket::SocketError)));
#endif
}
ServeSocket::~ServeSocket()
{
  close( m_sockfd );
}

bool ServeSocket::startSendFile()
{
    qDebug() << "ServeSocket::startSendFile";

    QByteArray recvBlock;

    int  readlen = 0;
    char buff[MAXBUFF];
    while ( 1 ) {
      readlen = read( m_sockfd, buff, MAXBUFF-1 );
      if ( errno == EINTR ) {
        continue;
      }
      if ( readlen <= 0 ) {
        return false;
      }
      buff[readlen] = '\0';
      recvBlock.append( buff );
      if ( canParsePacket(recvBlock) ) {
        break;
      }
    }

//    forever {
//        if (!m_tcpSocket.waitForReadyRead(3000)) {
//            m_errorString = m_tcpSocket.errorString();
//            return false;
//        }
//
//        recvBlock.append(m_tcpSocket.read(m_tcpSocket.bytesAvailable()));
//
//        if (canParsePacket(recvBlock)) {
//            break;
//        }
//    }

    return handleRequest(recvBlock);
}

bool ServeSocket::canParsePacket(const QByteArray &requestPacket) const
{
    QList<QByteArray> bl = requestPacket.split(':');
    if (bl.count() < 8) {
        return false;
    }

    if (bl.count() >= 8) {
        // Only regular file have offset field.
        bool ok;
        int command = bl.at(MSG_FLAGS_POS).toInt(&ok, 16);

        if (command == IPMSG_GETFILEDATA && bl.count() < 9) {
            return false;
        }
    }

    return true;
}

bool ServeSocket::handleRequest(const QByteArray &requestPacket)
{
    qDebug() << "ServeSocket::handleRequest";

    struct RequsetFile requestFile;
    parseRequestPacket(requestPacket, requestFile);

    SendFileMap *map = 0;
    Global::sendFileManager->m_lock.lock();
    map = Global::sendFileManager->transferFileMap.value(m_packetNoString);
    if (map) {
        map->setState(SendFileMap::Transfer);
        Global::sendFileManager
            ->transferFileModel.updateTransfer(m_packetNoString);
    } else {
        Global::sendFileManager->m_lock.unlock();
        return false;
    }
    map->sem.release();
    Global::sendFileManager->m_lock.unlock();

    if (!requestFile.isFileSended) {
        m_errorString = "ServeSocket::handleRequest: Request file not sended";
        goto handle_request_fail;
    }

    if (requestFile.fileType == IPMSG_FILE_REGULAR) {
        if (!tcpSendFile(requestFile.filePath, requestFile.offset)) {
            goto handle_request_fail;
        }
    } else if (requestFile.fileType == IPMSG_FILE_DIR) {
        if (!tcpSendDir(requestFile.filePath)) {
            goto handle_request_fail;
        }
    } else {
        // unsupported type
        goto handle_request_fail;
    }

    Global::sendFileManager->m_lock.lock();
    map->m_map[requestFile.fileId]->setState(SendFile::SendOk);
    map->incrTransferCount();
    map->setState(SendFileMap::NotTransfer);
    Global::sendFileManager
        ->transferFileModel.updateTransfer(m_packetNoString);
    // if transfer finished, delete transfer
    if (map->isFinished()) {
        Global::sendFileManager->removeTransfer(m_packetNoString);
    }
    Global::sendFileManager->m_lock.unlock();

    return true;

handle_request_fail:
    qDebug() << "ServeSocket::handleRequest: send file error";

    // when this error happed, the file may have finished transfer by a
    // client's retry transfer, then file has been deleted from sendFileMap's
    // m_map.
    // we user handle class, so we can not test if recvFileHandle is bounded
    // how to test if a handle class is bounded????????????????
    Global::sendFileManager->m_lock.lock();
    map = Global::sendFileManager->transferFileMap.value(m_packetNoString);
    if (map) {
        qDebug() << "sem.available:" << map->sem.available();
        if (map->sem.available() == 1) {
            map->setState(SendFileMap::NotTransfer);
            Global::sendFileManager
                ->transferFileModel.updateTransfer(m_packetNoString);
            map->m_map[requestFile.fileId]->setState(SendFile::SendFail);
        }
        map->sem.acquire();
    }
    Global::sendFileManager->m_lock.unlock();

    return false;
}

void ServeSocket::parseRequestPacket(const QByteArray &requestPacket,
        struct RequsetFile &requestFile)
{
    QList<QByteArray> list = requestPacket.split(':');

    // XXX NOTE: canParsePacket() make sure we have enough items in list,
    // so we do not need to check size of list before call list.at()
    bool ok;
    long id = list.at(REQUST_FILE_PACKET_ID_POSITION).toLong(&ok, 16);
    m_packetNoString = QString("%1").arg(id);
    int fileId = list.at(REQUST_FILE_FILE_ID_POSITION).toLong(&ok, 16);
    int command = list.at(MSG_FLAGS_POS).toInt(&ok, 16);

    QMutexLocker locker(&Global::sendFileManager->m_lock);

    SendFileMap *sendFileMap
        = Global::sendFileManager->transferFileMap.value(m_packetNoString);
    requestFile.isFileSended = false;
    if (sendFileMap && sendFileMap->canSendFile(fileId)) {
        requestFile.isFileSended = true;
        requestFile.fileType = sendFileMap->m_map[fileId]->type();
        requestFile.filePath = sendFileMap->m_map[fileId]->absoluteFilePath();
        requestFile.fileId = fileId;
        if (command == IPMSG_GETFILEDATA) {
            requestFile.offset
                = list.at(REQUST_FILE_OFFSET_POSITION).toLongLong(&ok, 16);
        } else {
            requestFile.offset = 0;
        }
    }
}

bool ServeSocket::tcpSendFile(QString filePath, qint64 offset)
{
    QFile file(filePath);

    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }

    file.seek(offset);
    while (!file.atEnd()) {
        QByteArray block = file.read(BLOCK_SIZE);
        if (!tcpWriteBlock(block)) {
            return false;
        }
    }

    return true;
}

bool ServeSocket::tcpWriteBlock(QByteArray &block)
{
  size_t  nbytes = block.size();
  char*   buff   = block.data();

  size_t  sent = 0;
  ssize_t n    = 0;
  while ( sent < nbytes ) {
    n = send(m_sockfd, buff+sent, nbytes-sent, 0);
    if ( n > 0 ) {
      sent += n;
    } else if ( n < 0 ) {
      if ( errno == EINTR ) {
        continue;
      }
      return false;
    } else {
      return false;
    }
  }
  return true;
//    qint32 bytesToWrite = block.size();
//    forever {
//        qint32 n = m_tcpSocket.write(block);
//        if (n == -1) {
//            return false;
//        }
//        if (!m_tcpSocket.waitForBytesWritten(-1)) {
//            m_errorString = m_tcpSocket.errorString();
//            return false;
//        }
//        bytesToWrite -= n;
//        block.remove(0, n);
//        if (bytesToWrite == 0) {
//            break;
//        }
//    }
//    return true;
}

bool ServeSocket::tcpSendDir(QString filePath)
{
    QByteArray dirBlock = constructDirSendBlock(filePath, NormalBlockMode);
    if (!tcpWriteBlock(dirBlock)) {
        return false;
    }

    QDir dir(filePath);
    QFileInfoList fileInfoList = dir.entryInfoList();

    foreach (QFileInfo fi, fileInfoList) {
        // Skip '.' and '..' directory
        if (fi.fileName() == "." || fi.fileName() == "..") {
            continue;
        }

        if (fi.isFile()) {
            QByteArray fileBlock
                = constructFileSendBlock(fi.absoluteFilePath());
            if (!tcpWriteBlock(fileBlock)) {
                return false;
            }
            if (!tcpSendFile(fi.absoluteFilePath(), 0)) {
                return false;
            }
        }
        if (fi.isDir()) {
            if (!tcpSendDir(fi.absoluteFilePath())) {
                return false;
            }
        }
    }

    QByteArray retParentBlock
        = constructDirSendBlock(filePath, RetParentBlockMode);
    if (!tcpWriteBlock(retParentBlock)) {
        return false;
    }

    return true;
}

QByteArray ServeSocket::constructDirSendBlock(QString filePath,
        DirBlockModes mode)
{
    QFileInfo fi(filePath);

    QString fileName;
    if (mode == NormalBlockMode) {
        fileName = fi.fileName();
        // If send directory is root directory, we change its name to
        // "(root-directory)"
        if (fileName == "/") {
            fileName = "(root-directory)";
        }
    } else if (mode == RetParentBlockMode) {
        fileName = ".";
    }

    QString str(":");
    str.append(fileName);
    str.append(":");
    str.append(QString("%1").arg(0, TRANSFER_FILE_FILE_SIZE_LENGTH, 16, QChar('0')));
    str.append(":");
    if (mode == RetParentBlockMode) {
        str.append(QString("%1").arg(IPMSG_FILE_RETPARENT, 0, 16));
    } else if (mode == NormalBlockMode) {
        str.append(QString("%1").arg(IPMSG_FILE_DIR, 0, 16));
    }
    str.append(":");
    str.append(QString("%1=%2").arg(IPMSG_FILE_MTIME, 0, 16)
            .arg(fi.lastModified().toTime_t(), 0, 16));
    str.append(":");
    str.append(QString("%1=%2").arg(IPMSG_FILE_CREATETIME, 0, 16)
            .arg(fi.created().toTime_t(), 0, 16));
    str.append(":");

    QByteArray block = Global::transferCodec->codec()->fromUnicode(str);
    QString s = QString("%1").arg(block.size() + TRANSFER_FILE_HEADER_SIZE_LENGTH,
            TRANSFER_FILE_HEADER_SIZE_LENGTH, 16, QChar('0'));

    QByteArray ba = Global::transferCodec->codec()->fromUnicode(s);
    return ba + block;
}

QByteArray ServeSocket::constructFileSendBlock(QString filePath) const
{
    QFileInfo fi(filePath);

    QString fileName;
    fileName = fi.fileName();

    QString str(":");
    str.append(fileName);
    str.append(":");
    str.append(QString("%1").arg(fi.size(), TRANSFER_FILE_FILE_SIZE_LENGTH,
                16, QChar('0')));
    str.append(":");
    str.append(QString("%1").arg(IPMSG_FILE_REGULAR, 0, 16));
    str.append(":");
    str.append(QString("%1=%2").arg(IPMSG_FILE_MTIME, 0, 16)
            .arg(fi.lastModified().toTime_t(), 0, 16));
    str.append(":");
    str.append(QString("%1=%2").arg(IPMSG_FILE_CREATETIME, 0, 16)
            .arg(fi.created().toTime_t(), 0, 16));
    str.append(":");

    QByteArray block = Global::transferCodec->codec()->fromUnicode(str);
    QString s = QString("%1").arg(block.size() + TRANSFER_FILE_HEADER_SIZE_LENGTH,
            TRANSFER_FILE_HEADER_SIZE_LENGTH, 16, QChar('0'));

    QByteArray ba = Global::transferCodec->codec()->fromUnicode(s);
    return ba + block;
}

