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

#include <QSettings>
#include <QIcon>
#include <QtGui>

#include "global.h"
#include "helper.h"
#include "preferences.h"
#include "user_manager.h"
#include "msg_thread.h"
#include "transfer_codec.h"
#include "systray.h"
#include "window_manager.h"
#include "file_server.h"
#include "constants.h"
#include "send_file_manager.h"
#include "sound_thread.h"

SendFileManager *Global::sendFileManager = 0;
QSettings *Global::settings = 0;
UserManager *Global::userManager = 0;
WindowManager *Global::windowManager = 0;
Preferences *Global::preferences = 0;
TransferCodec *Global::transferCodec = 0;
Systray *Global::systray = 0;
QMap<QString, QIcon *> Global::iconSet;
FileServer *Global::fileServer = 0;
MsgThread *Global::msgThread = 0;
SoundThread *Global::soundThread = 0;

static void createIconSet();

using namespace Global;

void Global::globalInit(QString path)
{
    qDebug("Global::globalInit");

    createIconSet();

    Helper::setIniPath(path);

    sendFileManager = new SendFileManager;

    if (path.isEmpty()) {
        settings = new QSettings(QSettings::IniFormat, QSettings::UserScope,
                                 QString(COMPANY), QString(PROGRAM));
    } else {
        QString fileName = path + "/qipmsg.ini";
        settings = new QSettings(fileName, QSettings::IniFormat);
    }

    preferences = new Preferences;

    transferCodec = new TransferCodec;

    fileServer = new FileServer;

    msgThread = new MsgThread;
    soundThread = new SoundThread;

    userManager = new UserManager;
    windowManager = new WindowManager;

    systray = new Systray;
}

void Global::globalEnd()
{
    qDebug("Global::globalEnd");

    delete sendFileManager;

    delete userManager;

    delete windowManager;

    delete msgThread;
    delete soundThread;

    delete systray;

    delete fileServer;

    delete transferCodec;

    // We must delete settings after delete preferences, preferences need
    // to save settings when delete.
    delete preferences;
    delete settings;
}

static void createIconSet()
{
    iconSet.insert("normal", new QIcon(QString(":/icons/") + "qipmsg.xpm"));
    iconSet.insert("receive",
                   new QIcon(QString(":/icons/") + "qipmsg_recv.xpm"));
}

QString Global::fileCountString(int i)
{
    QString s;
    if (i <= 1) {
        s = QObject::tr("%1 file").arg(i);
    } else {
        s = QObject::tr("%1 files").arg(i);
    }

    return s;
}

QPoint Global::randomNearMiddlePoint()
{
    QRect rect(QApplication::desktop()->screenGeometry());
    int x = qrand() % (rect.width() / 4);
    int y = qrand() % (rect.height() / 6);
    return QPoint(rect.width()/6 + x, rect.height()/8 + y);
}

