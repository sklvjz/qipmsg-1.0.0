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

#ifndef SERVE_SOCKET_H
#define SERVE_SOCKET_H

#include <QObject>
#include <QTcpSocket>


struct RequsetFile;


class ServeSocket : public QObject
{
    Q_OBJECT;

public:
    enum DirBlockModes {
        NormalBlockMode,
        RetParentBlockMode
    };

    ServeSocket(int socketDescriptor, QObject *parent = 0);
    ~ServeSocket();

    bool startSendFile();

private:
    bool canParsePacket(const QByteArray &requestPacket) const;
    bool handleRequest(const QByteArray &requestPacket);
    void parseRequestPacket(const QByteArray&, struct RequsetFile&);
    bool tcpSendFile(QString filePath, qint64 offset);
    bool tcpSendDir(QString filePath);
    bool tcpWriteBlock(QByteArray &block);
    QByteArray constructDirSendBlock(QString filePath, DirBlockModes mode);
    QByteArray constructFileSendBlock(QString filePath) const;

    QString m_errorString;
    QString m_packetNoString;
    int     m_sockfd;
};

#endif // !SERVE_SOCKET_H

