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

#include "lockfile.h"

#include <QFile>

#include <fcntl.h>
#include <unistd.h>

Q_GLOBAL_STATIC(LockFile, lockFile);

LockFile::LockFile(QObject *parent)
    : QObject(parent)
{
}

LockFile *LockFile::instance()
{
    return lockFile();
}

bool LockFile::lock()
{
#define LOCKMODE (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)
    int fd = open(fileName.toUtf8().data(), O_RDWR|O_CREAT, LOCKMODE);
    if (fd < 0) {
        qWarning("LockFile::lock: can not open %s for writing.",
                 fileName.toUtf8().data());
        return false;
    }

    if (lockfile(fd) < 0) {
        return false;
    }

    ftruncate(fd, 0);
    char *str = "qipmsg lock file";
    write(fd, str, strlen(str));

    return true;
}

int LockFile::lockfile(int fd)
{
    struct flock fl;

    fl.l_type = F_WRLCK;
    fl.l_start = 0;
    fl.l_whence = SEEK_SET;
    fl.l_len = 0;

    return (fcntl(fd, F_SETLK, &fl));
}

void LockFile::setLockFile(QString name)
{
    fileName = name;
}

