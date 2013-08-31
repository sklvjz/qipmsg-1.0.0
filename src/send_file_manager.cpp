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

#include "send_file_manager.h"

#include <QMutexLocker>

SendFileManager::~SendFileManager()
{
    // XXX NOTE: program is abort to quit. so we let the os to care about the
    // memory.
    // foreach (SendFileMap *m, transferFileMap) {
    //     delete m;
    //     m = 0;
    // }
}

void SendFileManager::addTransfer(QString key, SendFileMap *value)
{
    transferFileMap.insert(key, value);
    transferFileModel.insertTransfer(value);

    emit transferCountChanged(transferFileMap.count());
}

void SendFileManager::addTransferLocked(QString key, SendFileMap *value)
{
    // XXX NOTE: we need lock here, 'SendFileThread' call removeTransfer()
    // directly
    QMutexLocker locker(&m_lock);

    transferFileMap.insert(key, value);
    transferFileModel.insertTransfer(value);

    emit transferCountChanged(transferFileMap.count());
}

void SendFileManager::removeTransfer(QString key)
{
    // XXX NOTE: we need lock here, 'SendFileThread' call this directly.
    // QMutexLocker locker(&m_lock);

    SendFileMap *map = transferFileMap.value(key);
    transferFileMap.remove(key);
    if (map) {
        delete map;
    }

    transferFileModel.removeRow(key);

    emit transferCountChanged(transferFileMap.count());
}

void SendFileManager::removeTransferLocked(QString key)
{
    QMutexLocker locker(&m_lock);

    SendFileMap *map = transferFileMap.value(key);
    transferFileMap.remove(key);
    if (map) {
        delete map;
    }

    transferFileModel.removeRow(key);

    emit transferCountChanged(transferFileMap.count());
}

