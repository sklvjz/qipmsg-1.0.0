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

#include "send_file_thread.h"
#include "file_server.h"
#include "constants.h"
#include "global.h"
#include "send_file_model.h"
#include "send_file_manager.h"

#include <QMessageBox>
#include <QApplication>

FileServer::FileServer(QObject *parent)
    : QTcpServer(parent)
{
    startListening();
}

FileServer::~FileServer()
{
}

void FileServer::incomingConnection(int socketDescriptor)
{
    SendFileThread *sendFileThread
        = new SendFileThread(socketDescriptor, this);

    connect(sendFileThread, SIGNAL(finished()),
            sendFileThread, SLOT(deleteLater()));

    sendFileThread->start();
}

void FileServer::startListening()
{
    listen(QHostAddress::Any, IPMSG_DEFAULT_PORT);
}

