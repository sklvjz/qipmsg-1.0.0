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
#include <QApplication>
#include <QtDebug>
#include <QDir>

#include "preferences.h"
#include "constants.h"
#include "global.h"
#include "helper.h"

Preferences::Preferences()
{
    reset();
    load();
}

Preferences::~Preferences()
{
    save();
}

void Preferences::reset()
{
    userName = "";
    groupName = "";
    isSealed = false;
    isNoAutoPopupMsg = false;
    isQuoteMsg = true;
    isReadCheck = true;
    isNoSoundAlarm = false;
    groupNameHistory = "";
    groupNameList.clear();

    userSearchHistory = "";
    userSearchList.clear();
    userSpecifiedBroadcastIpList.clear();

    displayLevel = "";

    // Detail settings
    isSingleClickSendWindow = false;
    noAutoCloseMsgWindowWhenReply = false;
    // End detail settings

    // Other settings
    noticeSound = Helper::soundPath() + "/info.wav";
    quoteMark = ">";

    // Log settings
    isLogMsg = true;
    noLogLockMsgBeforeOpen = true;
    isLogLoginName = false;
    isLogIP = false;
    logFilePath = "";
    openLogFile();
    // End log settings

    // Internel use
    lastSaveFilePath = QDir::homePath();
    lastSendFilePath = QDir::homePath();
    lastSendDirPath = QDir::homePath();
    // End internel use

    transferCodecName = "GB2312";
}

void Preferences::load()
{
    qDebug("Preferences::load");

    QSettings *set = Global::settings;

    set->beginGroup("NickName");
    userName = set->value("userName", userName).toString();
    set->endGroup();

    set->beginGroup("GroupName");
    groupName = set->value("groupName", groupName).toString();
    groupNameHistory = set->value("groupNameHistory", groupNameHistory)
        .toString();
    groupNameList = groupNameHistory.split(QChar('\r'));
    set->endGroup();

    set->beginGroup("SendReceive");
    isSealed = set->value("isSealed", isSealed).toBool();
    isReadCheck = set->value("isReadCheck", isReadCheck).toBool();
    isNoSoundAlarm = set->value("isNoSoundAlarm", isNoSoundAlarm).toBool();
    isNoAutoPopupMsg = set->value("isNoAutoPopupMsg",
            isNoAutoPopupMsg).toBool();
    isQuoteMsg = set->value("isQuoteMsg", isQuoteMsg).toBool();
    set->endGroup();

    set->beginGroup("Other");
    quoteMark = set->value("quoteMark", quoteMark).toString();
    noticeSound = set->value("noticeSound", noticeSound).toString();
    set->endGroup();

    set->beginGroup("Detail");
    isSingleClickSendWindow = set->value("isSingleClickSendWindow", isSingleClickSendWindow).toBool();
    noAutoCloseMsgWindowWhenReply = set->value("noAutoCloseMsgWindowWhenReply", noAutoCloseMsgWindowWhenReply).toBool();
    set->endGroup();

    set->beginGroup("Log");
    isLogMsg = set->value("isLogMsg", isLogMsg).toBool();
    noLogLockMsgBeforeOpen
        = set->value("noLogLockMsgBeforeOpen",
                noLogLockMsgBeforeOpen).toBool();
    isLogLoginName = set->value("isLogLoginName",
            isLogLoginName).toBool();
    isLogIP = set->value("isLogIP",
            isLogIP).toBool();
    logFilePath = set->value("logFilePath", logFilePath).toString();
    openLogFile();
    set->endGroup();

    set->beginGroup("InternalUse");
    lastSaveFilePath = set->value("lastSaveFilePath", lastSaveFilePath).toString();
    lastSendFilePath = set->value("lastSendFilePath", lastSendFilePath).toString();
    lastSendDirPath = set->value("lastSendDirPath", lastSendDirPath).toString();
    set->endGroup();

    set->beginGroup("UserSearch");
    userSearchHistory = set->value("userSearchHistory", userSearchHistory)
        .toString();
    userSearchList = userSearchHistory.split(QChar('\r'));
    set->endGroup();

    set->beginGroup("BroadcastIp");
    userSpecifiedBroadcastIp =
        set->value("userSpecifiedBroadcastIp",
                   userSpecifiedBroadcastIp).toString();
    userSpecifiedBroadcastIpList =
        userSpecifiedBroadcastIp.split(QChar('\r'), QString::SkipEmptyParts);
    set->endGroup();

    set->beginGroup("Transfer");
    transferCodecName
        = set->value("transferCodecName", transferCodecName).toString();
    set->endGroup();

}

void Preferences::save()
{
    qDebug("Preferences::save");

    QSettings *set = Global::settings;

    set->beginGroup("NickName");
    set->setValue("userName", userName);
    set->endGroup();

    set->beginGroup("GroupName");
    set->setValue("groupName", groupName);
    set->setValue("groupNameHistory", groupNameList.join("\r"));
    set->endGroup();

    set->beginGroup("SendReceive");
    set->setValue("isSealed", QString("%1").arg(isSealed));
    set->setValue("isNoAutoPopupMsg", QString("%1").arg(isNoAutoPopupMsg));
    set->setValue("isQuoteMsg", QString("%1").arg(isQuoteMsg));
    set->setValue("isReadCheck", QString("%1").arg(isReadCheck));
    set->setValue("isNoSoundAlarm", QString("%1").arg(isNoSoundAlarm));
    set->endGroup();

    set->beginGroup("Detail");
    set->setValue("isSingleClickSendWindow", isSingleClickSendWindow);
    set->setValue("noAutoCloseMsgWindowWhenReply", noAutoCloseMsgWindowWhenReply);
    set->endGroup();

    set->beginGroup("Other");
    set->setValue("quoteMark", quoteMark);
    set->setValue("noticeSound", noticeSound);
    set->endGroup();

    set->beginGroup("Log");
    set->setValue("isLogMsg", isLogMsg);
    set->setValue("noLogLockMsgBeforeOpen", noLogLockMsgBeforeOpen);
    set->setValue("isLogLoginName", isLogLoginName);
    set->setValue("isLogIP", isLogIP);
    set->setValue("logFilePath", logFilePath);
    set->endGroup();

    set->beginGroup("InternalUse");
    set->setValue("lastSaveFilePath", lastSaveFilePath);
    set->setValue("lastSendFilePath", lastSendFilePath);
    set->setValue("lastSendDirPath", lastSendDirPath);
    set->endGroup();

    set->beginGroup("UserSearch");
    set->setValue("userSearchHistory", userSearchList.join("\r"));
    set->endGroup();

    set->beginGroup("BroadcastIp");
    set->setValue("userSpecifiedBroadcastIp",
                  userSpecifiedBroadcastIpList.join("\r"));
    set->endGroup();

    set->beginGroup("Transfer");
    set->setValue("transferCodecName", transferCodecName);
    set->endGroup();
}

void Preferences::openLogFile()
{
    if (logFile.isOpen()) {
        logFile.close();
    }

    logFile.setFileName(logFilePath);

    logFile.open(QIODevice::Append);
}

