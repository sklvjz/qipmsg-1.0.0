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

#include "owner.h"
#include "global.h"
#include "helper.h"
#include "send_msg.h"
#include "user_manager.h"
#include "preferences.h"
#include "constants.h"
#include "msg_thread.h"

#include <QStringList>
#include <QStandardItemModel>

Owner UserManager::m_ourself;

UserManager::UserManager(QObject *parent)
    : QObject(parent)
{
    updateOurself();
    createModel();
}

void UserManager::updateOurself()
{
    if (Global::preferences->userName.isEmpty()) {
        m_ourself.setName(Helper::loginName());
    } else {
        m_ourself.setName(Global::preferences->userName);
    }

    m_ourself.setGroup(Global::preferences->groupName);
    m_ourself.setLoginName(Helper::loginName());
    m_ourself.setHost(Helper::hostname());
    m_ourself.setDisplayLevel(Global::preferences->displayLevel);
}

void UserManager::createModel()
{
    QStringList labels;
    labels << tr("user") << tr("group") << tr("host") << tr("ip")
        << tr("logname") << tr("display level");

    m_model = new QStandardItemModel(0, labels.size(), this);
    m_model->setHorizontalHeaderLabels(labels);
}

void UserManager::newUserMsg(Msg msg)
{
    if (!msg->owner().group().isEmpty()
            && !Global::preferences->groupNameList
            .contains(msg->owner().group())) {
        Global::preferences->groupNameList.prepend(msg->owner().group());
    }

    if (contains(msg->ip())) {
        updateUser(msg->owner(), ipToRow(msg->ip()));
    } else {
        int row = m_model->rowCount();
        m_model->insertRow(row);
        addUser(msg->owner(), row);

        emit userCountUpdated(m_model->rowCount());
    }
}

void UserManager::newExitMsg(Msg msg)
{
    int row;
    if ((row = ipToRow(msg->ip())) != -1) {
        m_model->removeRow(row);

        emit userCountUpdated(m_model->rowCount());
    }
}

void UserManager::updateUser(const Owner &owner, int row)
{
    m_model->setData(m_model->index(row, USER_VIEW_NAME_COLUMN),
                       owner.name());
    m_model->setData(m_model->index(row, USER_VIEW_GROUP_COLUMN),
                       owner.group());
    m_model->setData(m_model->index(row, USER_VIEW_HOST_COLUMN),
                       owner.host());
    m_model->setData(m_model->index(row, USER_VIEW_IP_COLUMN),
                       owner.ip());
    m_model->setData(m_model->index(row, USER_VIEW_LOGIN_NAME_COLUMN),
                       owner.loginName());
    m_model->setData(m_model->index(row, USER_VIEW_DISPLAY_LEVEL_COLUMN),
                       owner.displayLevel());
}

void UserManager::addUser(const Owner &owner, int row)
{
    updateUser(owner, row);
}

void UserManager::broadcastExit() const
{
    qDebug("UserManager::broadcastExit");

    quint32 flags = 0;
    flags |= IPMSG_BR_EXIT | QIPMSG_CAPACITY;

    SendMsg sendMsg(QHostAddress::Null, 0/* port */,
                    exitMessage(), ""/* extendedInfo */, flags);

    Global::msgThread->addSendMsg(Msg(sendMsg));
}

QString UserManager::entryMessage() const
{
    return QString("%1%2%3%4").arg(m_ourself.name())
        .arg(QChar('\0'))
        .arg(m_ourself.group())
        .arg(QChar('\0'));
}

QString UserManager::exitMessage() const
{
    return entryMessage();
}

void UserManager::broadcastEntry() const
{
    qDebug("UserManager::broadcastEntry");

    quint32 flags = 0;
    flags |= IPMSG_BR_ENTRY | QIPMSG_CAPACITY;

    SendMsg sendMsg(QHostAddress::Null, 0/* port */,
                    entryMessage(), ""/* extendedInfo */, flags);

    Global::msgThread->addSendMsg(Msg(sendMsg));
}

bool UserManager::contains(QString ip) const
{
    for (int row = 0; row < m_model->rowCount(); ++row) {
        if (this->ip(row) == ip) {
            return true;
        }
    }

    return false;
}

QString UserManager::ip(int row) const
{
    return m_model->data(m_model->index(row, USER_VIEW_IP_COLUMN))
        .toString();
}

int UserManager::ipToRow(QString ip) const
{
    for (int row = 0; row < m_model->rowCount(); ++row) {
        if (this->ip(row) == ip) {
            return row;
        }
    }

    return -1;
}

QString UserManager::name(int row) const
{
    return m_model->data(m_model->index(row, USER_VIEW_NAME_COLUMN))
        .toString();
}

QString UserManager::group(int row) const
{
    return m_model->data(m_model->index(row, USER_VIEW_GROUP_COLUMN))
        .toString();
}

QString UserManager::host(int row) const
{
    return m_model->data(m_model->index(row, USER_VIEW_HOST_COLUMN))
        .toString();
}

QString UserManager::loginName(int row) const
{
    return m_model->data(m_model->index(row, USER_VIEW_LOGIN_NAME_COLUMN))
        .toString();
}

