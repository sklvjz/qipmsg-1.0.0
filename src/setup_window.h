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

#ifndef SETUP_WINDOW_H
#define SETUP_WINDOW_H

#include <QWidget>
#include <QDialog>

class QCheckBox;
class QComboBox;
class QDialogButtonBox;
class QGroupBox;
class QHBoxLayout;
class QLabel;
class QLineEdit;
class QPushButton;
class QSize;
class QTabWidget;
class QWidget;

class DetailTab : public QWidget
{
    Q_OBJECT

public:
    DetailTab(QWidget *parent = 0);

    void save();

private:
    QCheckBox *autoMinimizeCheckBox;
    QCheckBox *singleClickCheckBox;
    QCheckBox *enableHotKeyCheckBox;
    QCheckBox *buttonAliasCheckBox;
    QCheckBox *noAutoCloseReplyCheckBox;
    QCheckBox *noPopupMessageWhenAway;
};

class LogTab : public QWidget
{
    Q_OBJECT

public:
    LogTab(QWidget *parent = 0);

    void save();

private slots:
    void getLogFilePath();

private:
    QCheckBox *isLogMsgCheckBox;
    QCheckBox *noLogLockMsgBeforeOpenCheckBox;
    QCheckBox *isLogLoginNameCheckBox;
    QCheckBox *isLogIPCheckBox;
    QPushButton *logFileButton;
    QLabel *logFileLabel;
};

class OtherTab : public QWidget
{
    Q_OBJECT

public:
    OtherTab(QWidget *parent = 0);

    void save();

private slots:
    void getNoticeSoundPath();

private:
    QPushButton *quoteMarkButton;
    QLineEdit *quoteMarkLineEdit;
    QPushButton *noticeSoundButton;
    QLabel *noticeSoundLabel;
    QPushButton *onlineIconButton;
    QLabel *onlineIconLabel;
    QPushButton *awayIconButton;
    QLabel *awayIconLabel;
};

class TransferTab : public QWidget
{
    Q_OBJECT

public:
    TransferTab(QWidget *parent = 0);

    void save();

private:
    void createCodecComboBox();

    QComboBox *codecComboBox;
    QLabel *codecLabel;
};

class DetailSetupDialog : public QDialog
{
    Q_OBJECT

public:
    DetailSetupDialog(QWidget *parent = 0);

    virtual ~DetailSetupDialog() {}

    virtual QSize sizeHint() const;

private slots:
    void save();
    void apply();

private:
    DetailTab *detailTab;
    LogTab *logTab;
    OtherTab *otherTab;
    TransferTab *transferTab;

    QTabWidget *tabWidget;
    QDialogButtonBox *buttonBox;
};

class QListWidget;
class SetupWindow : public QWidget
{
    Q_OBJECT

public:
    SetupWindow(QWidget *parent = 0);

    virtual ~SetupWindow() {}

    virtual QSize sizeHint() const;

private slots:
    void savePreferences();
    void applyPreferences();
    void detailSetup();
    void AddBroadcast();
    void DelBroadcast();

private:
    void createNicknameGroupBox();
    void createGroupNameGroupBox();
    void createSendRecvSettingsGroupBox();
    void createMiscSettingGroupBox();
    void createBroadcastGroupBox();

    void createButtonLayout();

    void createConnections();

    QGroupBox *nicknameGroupBox;
    QGroupBox *groupNameGroupBox;
    QGroupBox *sendReceiveSettingsGroupBox;
    QGroupBox *miscSettingGroupBox;
    QGroupBox *broadcastGroupBox;

    QPushButton *okButton;
    QPushButton *applyButton;
    QHBoxLayout *buttonsLayout;

    QLineEdit *nickNameEdit;
    QComboBox *groupNameComboBox;

    QCheckBox *readCheck;
    QCheckBox *noAutoPopupMsg;
    QCheckBox *noSoundAlarm;
    QCheckBox *sealedSend;
    QCheckBox *quoteMsg;

    QPushButton *detailSetupButton;
    QPushButton *urlLinkSetupButton;
    QPushButton *passwordSetupButton;

    QListWidget *broadcast_list_widget_;
    QLineEdit* line_edit_;
};

#endif // !SETUP_WINDOW_H

