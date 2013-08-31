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

#include "send_file.h"
#include "ipmsg.h"


SendFile::SendFile(QString path)
    : QFileInfo(path), m_state(NotSend), m_fileId(-1), m_mtime(0)
{
    // XXX TODO: finish this
    // m_mtime =
}

SendFile::~SendFile()
{
}

int SendFile::type() const
{
    if (isFile()) {
        return IPMSG_FILE_REGULAR;
    }

    return IPMSG_FILE_DIR;
}

