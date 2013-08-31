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

#include <QApplication>
#include <QUdpSocket>
#include <QMetaType>

#include "qipmsg.h"
#include "helper.h"
#include "systray.h"
#include "user_manager.h"
#include "msg_thread.h"
#include "constants.h"
#include "global.h"
#include "recv_msg.h"
#include "send_msg.h"
#include "send_file_manager.h"

QIpMsg::QIpMsg(QObject *parent)
    : QObject(parent)
{
    Global::globalInit(Helper::iniPath());

    createConnections();

    Global::systray->show();
}

QIpMsg::~QIpMsg()
{
    // delete global variable and save settings.
    Global::globalEnd();
}

void QIpMsg::createConnections()
{
    connect(Global::sendFileManager, SIGNAL(transferCountChanged(int)),
            Global::systray, SLOT(updateTransferCount(int)));
}

