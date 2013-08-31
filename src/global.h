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

#ifndef GLOBAL_H
#define GLOBAL_H

#include <QString>
#include <QMap>
#include <QPoint>

class QSettings;

class SendFileManager;
class UserManager;
class MsgThread;
class SoundThread;
class Preferences;
class TransferCodec;
class Systray;
class WindowManager;
class QIcon;
class FileServer;

namespace Global
{
    extern QSettings *settings;
    extern SendFileManager *sendFileManager;
    extern UserManager *userManager;
    extern MsgThread *msgThread;
    extern SoundThread *soundThread;
    extern WindowManager *windowManager;
    extern Preferences *preferences;
    extern TransferCodec *transferCodec;
    extern Systray *systray;
    extern QMap<QString, QIcon *> iconSet;
    extern FileServer *fileServer;

    void globalInit(QString path);
    void globalEnd();

    QString fileCountString(int i);
    QPoint randomNearMiddlePoint();
}; // namespace Global

#endif // !GLOBAL_H

