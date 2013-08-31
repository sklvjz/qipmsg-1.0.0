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

#ifndef SEND_FILE_H
#define SEND_FILE_H

#include <QString>
#include <QFileInfo>

class SendFile : public QFileInfo
{
public:
    enum States { NotSend, SendOk, SendFail };
    SendFile(QString path);

    virtual ~SendFile();

    virtual SendFile* clone() const { return new SendFile(*this); }

    States state() const { return m_state; }
    void setState(States state) { m_state = state; }

    int type() const;

private:
    enum States m_state;

    int m_fileId;
    time_t m_mtime;      // save this to trace if a file changed
};

#endif // !SEND_FILE_H

