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

#include "window_manager.h"
#include "global.h"
#include "msg_window.h"
#include "msg_readed_window.h"
#include "msg_thread.h"
#include "constants.h"
#include "helper.h"
#include "sound_thread.h"
#include "sound.h"
#include "preferences.h"

#include <QPoint>
#include <QSize>

WindowManager::WindowManager(QObject *parent)
    : QObject(parent)
{
}

WindowManager::~WindowManager()
{
    destroyMsgWindowList();
    destroyMsgReadedWindowList();
}

void WindowManager::newMsg(Msg msg)
{
    switch (GET_MODE(msg->flags())) {
    case IPMSG_SENDMSG:
        createMsgWindow(msg);
        break;
    case IPMSG_READMSG:
        createMsgReadedWindow(msg);
        break;

    default:
        break;
    }
}

void WindowManager::createMsgWindow(Msg msg)
{
        MsgWindow *msgWindow = new MsgWindow(msg);

        m_msgWindowList.insert(0, msgWindow);

        // play sound
        if (!Global::preferences->isNoSoundAlarm) {
            QString soundFile = Global::preferences->noticeSound;
#ifdef Q_OS_LINUX
            Global::soundThread->play(soundFile);
#else // !Q_OS_LINUX
            if (QSound::isAvailable()) {
                QSound::play();
            }
#endif
        }

        if (!Global::preferences->isNoAutoPopupMsg) {
            msgWindow->show();
        }
}

void WindowManager::createMsgReadedWindow(Msg msg)
{
        MsgReadedWindow *window = new MsgReadedWindow(msg->owner().name());

        m_msgReadedWindowList.insert(0, window);

        window->show();
}

void WindowManager::destroyMsgReadedWindowList()
{
    while (!m_msgReadedWindowList.isEmpty()) {
        delete m_msgReadedWindowList.takeFirst();
    }
}

void WindowManager::destroyMsgWindowList()
{
    while (!m_msgWindowList.isEmpty()) {
        delete m_msgWindowList.takeFirst();
    }
}

void WindowManager::visibleAllMsgWindow()
{
    foreach (MsgWindow *w, m_msgWindowList) {
        // this make the window on top
        w->setWindowFlags(w->windowFlags() | Qt::WindowStaysOnTopHint);
        QPoint pos = w->pos();
        w->move(pos);
        w->show();
        w->setWindowFlags(w->windowFlags() & ~Qt::WindowStaysOnTopHint);
        w->move(pos);
        w->show();
    }
}

void WindowManager::visibleMsgWindow()
{
    foreach (MsgWindow *w, m_msgWindowList) {
        if (!w->isVisible()) {
            w->show();
        }
    }
}

void WindowManager::visibleAllMsgReadedWindow()
{
    foreach (MsgReadedWindow *w, m_msgReadedWindowList) {
        w->setWindowFlags(w->windowFlags() | Qt::WindowStaysOnTopHint);
        QPoint pos = w->pos();
        w->move(pos);
        w->show();
        w->setWindowFlags(w->windowFlags() & ~Qt::WindowStaysOnTopHint);
        w->move(pos);
        w->show();
    }
}

int WindowManager::hidedMsgWindowCount() const
{
    int count = 0;
    foreach (MsgWindow *w, m_msgWindowList) {
        if (!w->isVisible()) {
            ++count;
        }
    }

    return count;
}

