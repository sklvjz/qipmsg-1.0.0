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

#ifndef WINDOW_MANAGER_H
#define WINDOW_MANAGER_H

#include <QObject>
#include <QList>
#include <QMutex>

#include "owner.h"
#include "msg.h"

class MsgWindow;
class MsgReadedWindow;

class WindowManager : public QObject
{
    Q_OBJECT

public:
    WindowManager(QObject *parent = 0);
    ~WindowManager();

    void visibleMsgWindow();
    void visibleAllMsgWindow();
    void visibleAllMsgReadedWindow();

    void removeMsgWindow(MsgWindow *w) {
        m_msgWindowList.removeAll(w);
    }

    void removeMsgReadedWindow(MsgReadedWindow *w) {
        m_msgReadedWindowList.removeAll(w);
    }

    int hidedMsgWindowCount() const;

private slots:
    void newMsg(Msg msg);
    void destroyMsgReadedWindowList();

private:
    void createMsgWindow(Msg msg);
    void createMsgReadedWindow(Msg msg);

    void destroyMsgWindowList();

    QList<MsgWindow *> m_msgWindowList;
    QList<MsgReadedWindow *> m_msgReadedWindowList;
};

#endif // !WINDOW_MANAGER_H
