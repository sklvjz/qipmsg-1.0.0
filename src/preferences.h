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

#ifndef PREFERENCES_H
#define PREFERENCES_H

#include <QString>
#include <QSettings>
#include <QStringList>
#include <QMutex>
#include <QFile>


class Preferences
{
public:
    Preferences();
    ~Preferences();

    void reset();
    void load();
    void save();
    void openLogFile();

    QString displayLevel;

    QString userName;
    QString groupName;
    QString groupNameHistory;
    QStringList groupNameList;
    bool isSealed;
    bool isReadCheck;
    bool isNoSoundAlarm;
    bool isNoAutoPopupMsg;
    bool isQuoteMsg;
    bool noAutoCloseMsgWindowWhenReply;

    bool isSingleClickSendWindow;

    QString quoteMark;
    QString noticeSound;

    bool isLogMsg;
    bool noLogLockMsgBeforeOpen;
    bool isLogLoginName;
    bool isLogIP;
    QString logFilePath;
    QFile logFile;

    QString lastSaveFilePath;
    QString lastSendFilePath;
    QString lastSendDirPath;

    QStringList userSearchList;
    QString userSearchHistory;
    bool isSearchAllColumns;

    QString transferCodecName;

    QStringList userSpecifiedBroadcastIpList;
    QString userSpecifiedBroadcastIp;
};

#endif // !PREFERENCES_H

