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

#ifndef FILE_SERVER_H
#define FILE_SERVER_H

#include <QTcpServer>

class FileServer : public QTcpServer
{
    Q_OBJECT

public:
    FileServer(QObject *parent = 0);
    ~FileServer();

private slots:

protected:
    void incomingConnection(int socketDescriptor);

private:
    void startListening();
};

#endif // !FILE_SERVER_H
