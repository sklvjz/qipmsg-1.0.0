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

#include "recv_file_thread.h"
#include "recv_file_map.h"

void RecvFileThread::run()
{
    m_recvFileTransfer = new RecvFileTransfer(m_recvFileMap);

    connect(this, SIGNAL(stopTransfer()),
            m_recvFileTransfer, SLOT(stopTransfer()), Qt::DirectConnection);
    connect(this, SIGNAL(abortTransfer()),
            m_recvFileTransfer, SLOT(abortTransfer()), Qt::DirectConnection);
    connect(m_recvFileTransfer, SIGNAL(recvFileFinished()),
            this, SIGNAL(recvFileFinished()));
    connect(m_recvFileTransfer, SIGNAL(recvFileError(QString)),
            this, SIGNAL(recvFileError(QString)));

    m_recvFileTransfer->startTransfer();

    m_recvFileMap->setTransferState(RecvFileMap::NotTransfer);
}

void RecvFileThread::resumeTransfer()
{
    m_recvFileTransfer->resumeTransfer();
}

