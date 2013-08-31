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

#include "global.h"
#include "preferences.h"
#include "setup_window.h"
#include "user_manager.h"
#include "transfer_codec.h"

#include <QtGui>
#include <QtCore>

DetailTab::DetailTab(QWidget *parent)
    : QWidget(parent)
{
    autoMinimizeCheckBox
        = new QCheckBox(tr("Minimize message readed alert window"));
    autoMinimizeCheckBox->setEnabled(false);

    singleClickCheckBox
        = new QCheckBox(tr("Single click to open send window (default"
                    " double click)"));
    if (Global::preferences->isSingleClickSendWindow) {
        singleClickCheckBox->setCheckState(Qt::Checked);
    }
    enableHotKeyCheckBox = new QCheckBox(tr("Enable HotKey (Ctrl+Alt+s, R)"));
    enableHotKeyCheckBox->setEnabled(false);

    buttonAliasCheckBox
        = new QCheckBox(tr("Change [Send/Reply] button name to"
                    " [Fire/Retrun Fire]"));
    buttonAliasCheckBox->setEnabled(false);

    noAutoCloseReplyCheckBox
        = new QCheckBox(tr("No auto close receive message window when reply"));
    if (Global::preferences->noAutoCloseMsgWindowWhenReply) {
        noAutoCloseReplyCheckBox->setCheckState(Qt::Checked);
    }

    noPopupMessageWhenAway = new QCheckBox(tr("Not popup message when away"));
    noPopupMessageWhenAway->setEnabled(false);

    QGridLayout *layout = new QGridLayout;
    layout->addWidget(autoMinimizeCheckBox, 0, 0, 1, 2);
    layout->addWidget(singleClickCheckBox, 1, 0, 1, 2);
    layout->addWidget(enableHotKeyCheckBox, 2, 0, 1, 2);
    layout->addWidget(buttonAliasCheckBox, 3, 0, 1, 2);
    layout->addWidget(noAutoCloseReplyCheckBox, 4, 0, 1, 2);
    layout->addWidget(noPopupMessageWhenAway, 5, 0, 1, 2);

    setLayout(layout);
}

void DetailTab::save()
{
    Global::preferences->isSingleClickSendWindow
        = singleClickCheckBox->isChecked();
    Global::preferences->noAutoCloseMsgWindowWhenReply
        = noAutoCloseReplyCheckBox->isChecked();
}

LogTab::LogTab(QWidget *parent)
    : QWidget(parent)
{
    isLogMsgCheckBox = new QCheckBox(tr("Enable log message"));
    if (Global::preferences->isLogMsg) {
        isLogMsgCheckBox->setCheckState(Qt::Checked);
    }
    noLogLockMsgBeforeOpenCheckBox
        = new QCheckBox(tr("Not log lock message before open"));
    noLogLockMsgBeforeOpenCheckBox->setEnabled(false);
    if (Global::preferences->noLogLockMsgBeforeOpen) {
        noLogLockMsgBeforeOpenCheckBox->setCheckState(Qt::Unchecked);
    }
    isLogLoginNameCheckBox = new QCheckBox(tr("Log nickname and login name"));
    if (Global::preferences->isLogLoginName) {
        isLogLoginNameCheckBox->setCheckState(Qt::Checked);
    }
    isLogIPCheckBox = new QCheckBox(tr("Log ip address"));
    if (Global::preferences->isLogIP) {
        isLogIPCheckBox->setCheckState(Qt::Checked);
    }

    logFileButton = new QPushButton(tr("Log File"));
    logFileLabel = new QLabel;
    int frameStyle = QFrame::Sunken | QFrame::Panel;
    logFileLabel->setFrameStyle(frameStyle);
    logFileLabel->setText(Global::preferences->logFilePath);

    connect(logFileButton, SIGNAL(clicked()),
            this, SLOT(getLogFilePath()));

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(isLogMsgCheckBox);
    mainLayout->addWidget(noLogLockMsgBeforeOpenCheckBox);
    mainLayout->addWidget(isLogLoginNameCheckBox);
    mainLayout->addWidget(isLogIPCheckBox);

    QGridLayout *layout = new QGridLayout;
    layout->addWidget(logFileButton, 0, 0);
    layout->addWidget(logFileLabel, 0, 1);
    layout->setColumnStretch(0, 10);
    layout->setColumnStretch(1, 30);

    mainLayout->addLayout(layout);

    setLayout(mainLayout);
}

TransferTab::TransferTab(QWidget *parent)
    : QWidget(parent)
{
    createCodecComboBox();

    QLabel *label = new QLabel(tr("Transfer Codec:"));

    QGridLayout *mainLayout = new QGridLayout;
    mainLayout->addWidget(label, 0, 0);
    mainLayout->addWidget(codecComboBox, 0, 1);

    mainLayout->setColumnStretch(0, 10);
    mainLayout->setColumnStretch(1, 30);

    setLayout(mainLayout);
}

void TransferTab::createCodecComboBox()
{
    QMap<QString, QString> codecMap;
    QRegExp iso8859RegExp("ISO[- ]8859-([0-9]+).*");

    foreach (QByteArray name, QTextCodec::availableCodecs()) {
        QTextCodec *codec = QTextCodec::codecForName(name);

        QString sortKey = codec->name().toUpper();
        int rank;

        if (sortKey.startsWith("UTF-8")) {
            rank = 1;
        } else if (sortKey.startsWith("UTF-16")) {
            rank = 2;
        } else if (iso8859RegExp.exactMatch(sortKey)) {
            if (iso8859RegExp.cap(1).size() == 1)
                rank = 3;
            else
                rank = 4;
        } else {
            rank = 5;
        }
        sortKey.prepend(QChar('0' + rank));

        codecMap.insert(sortKey, QString(name));
    }
    QStringList codecNames = codecMap.values();

    codecComboBox = new QComboBox();
    codecComboBox->setDuplicatesEnabled(false);

    codecComboBox->addItems(codecNames);
    int i = codecComboBox->findText(Global::preferences->transferCodecName);
    codecComboBox->setCurrentIndex(i);
}

void TransferTab::save()
{
    Global::preferences->transferCodecName = codecComboBox->currentText();
    Global::transferCodec->setTransCodec(codecComboBox->currentText());
}

void LogTab::getLogFilePath()
{
    QFileDialog::Options options = QFileDialog::DontConfirmOverwrite;
    QString selectedFilter;
    QString path = Global::preferences->logFilePath;
    QString fileName
        = QFileDialog::getSaveFileName(this,
                tr("Select Log File"),
                path,
                tr("Text Files (*.log);;All Files (*)"),
                &selectedFilter,
                options);

    if (!fileName.isEmpty()) {
        logFileLabel->setText(fileName);
    }
}

void LogTab::save()
{
    Global::preferences->isLogMsg = isLogMsgCheckBox->isChecked();
    Global::preferences->noLogLockMsgBeforeOpen
        = noLogLockMsgBeforeOpenCheckBox->isChecked();
    Global::preferences->isLogLoginName = isLogLoginNameCheckBox->isChecked();
    Global::preferences->isLogIP = isLogIPCheckBox->isChecked();
    Global::preferences->logFilePath = logFileLabel->text();
    Global::preferences->openLogFile();
}

OtherTab::OtherTab(QWidget *parent)
    : QWidget(parent)
{
    quoteMarkButton = new QPushButton(tr("Quote Mark"));
    quoteMarkButton->setFlat(true);
    quoteMarkLineEdit = new QLineEdit;
    quoteMarkLineEdit->setText(Global::preferences->quoteMark);

    int frameStyle = QFrame::Sunken | QFrame::Panel;

    noticeSoundButton = new QPushButton(tr("Notice Sound"));
    noticeSoundLabel = new QLabel(Global::preferences->noticeSound);
    noticeSoundLabel->setFrameStyle(frameStyle);
    connect(noticeSoundButton, SIGNAL(clicked()),
            this, SLOT(getNoticeSoundPath()));

    onlineIconButton = new QPushButton(tr("Online Icon"));
    onlineIconButton->setDisabled(true);
    onlineIconLabel = new QLabel;
    onlineIconLabel->setFrameStyle(frameStyle);

    awayIconButton = new QPushButton(tr("Away Icon"));
    awayIconButton->setDisabled(true);
    awayIconLabel = new QLabel;
    awayIconLabel->setFrameStyle(frameStyle);

    QGridLayout *layout = new QGridLayout;
    layout->addWidget(quoteMarkButton, 0, 0);
    layout->addWidget(quoteMarkLineEdit, 0, 1);
    layout->addWidget(noticeSoundButton, 1, 0);
    layout->addWidget(noticeSoundLabel, 1, 1);
    layout->addWidget(onlineIconButton, 2, 0);
    layout->addWidget(onlineIconLabel, 2, 1);
    layout->addWidget(awayIconButton, 3, 0);
    layout->addWidget(awayIconLabel, 3, 1);

    layout->setColumnStretch(0, 10);
    layout->setColumnStretch(1, 30);

    setLayout(layout);
}

void OtherTab::getNoticeSoundPath()
{
    QFileDialog::Options options;
    QString selectedFilter;
    QString path = Global::preferences->noticeSound;
    QString fileName
        = QFileDialog::getOpenFileName(this,
                tr("Select Sound File"),
                path,
                tr("Audio Files (*.wav)"),
                &selectedFilter,
                options);

    if (!fileName.isEmpty()) {
        noticeSoundLabel->setText(fileName);
    }
}

void OtherTab::save()
{
    if (!quoteMarkLineEdit->text().isEmpty()) {
        Global::preferences->quoteMark = quoteMarkLineEdit->text();
    }

    if (!noticeSoundLabel->text().isEmpty()) {
        Global::preferences->noticeSound = noticeSoundLabel->text();
    }
}

DetailSetupDialog::DetailSetupDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(tr("Detail/Log Settings"));
    setWindowModality(Qt::WindowModal);

    detailTab = new DetailTab;
    logTab = new LogTab;
    otherTab = new OtherTab;
    transferTab = new TransferTab;

    tabWidget = new QTabWidget;
    tabWidget->setUsesScrollButtons(true);
    tabWidget->addTab(detailTab, tr("Detail"));
    tabWidget->addTab(logTab, tr("Log"));
    tabWidget->addTab(otherTab, tr("Other"));
    tabWidget->addTab(transferTab, tr("Transfer"));

    buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok
            | QDialogButtonBox::Apply);

    connect(buttonBox->button(QDialogButtonBox::Ok), SIGNAL(clicked()),
            this, SLOT(save()));
    connect(buttonBox->button(QDialogButtonBox::Apply), SIGNAL(clicked()),
            this, SLOT(apply()));

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(tabWidget);
    mainLayout->addWidget(buttonBox);

    setLayout(mainLayout);

    adjustSize();
}

QSize DetailSetupDialog::sizeHint() const
{
    return QSize(360, 240);
}

void DetailSetupDialog::save()
{
    apply();

    close();
}

void DetailSetupDialog::apply()
{
    detailTab->save();
    otherTab->save();
    logTab->save();
    transferTab->save();

    Global::preferences->save();
}

SetupWindow::SetupWindow(QWidget *parent)
    : QWidget(parent)
{
    // setModal(true);
    setAttribute(Qt::WA_DeleteOnClose, true);

    createNicknameGroupBox();
    createGroupNameGroupBox();
    createSendRecvSettingsGroupBox();
    createBroadcastGroupBox();

    createButtonLayout();

    createMiscSettingGroupBox();

    QGridLayout *mainLayout = new QGridLayout(this);

    mainLayout->addWidget(nicknameGroupBox, 0, 0);
    mainLayout->addWidget(groupNameGroupBox, 0, 1);
    mainLayout->addWidget(sendReceiveSettingsGroupBox, 1, 0, 1, 1);
    mainLayout->addWidget(miscSettingGroupBox, 1, 1);
    mainLayout->addWidget(broadcastGroupBox, 2, 0, 1, 2);
    mainLayout->addLayout(buttonsLayout, 3, 0, 1, 2);

    createConnections();

    setWindowTitle(tr("Setup Preferences"));
    setWindowIcon(*Global::iconSet.value("normal"));

    setLayout(mainLayout);

    adjustSize();

    // Put the dialog in the middle of screen
    QRect rect(QApplication::desktop()->screenGeometry());
    move((rect.width() - width()) / 2, (rect.height() - height()) / 2);
}

void SetupWindow::createNicknameGroupBox()
{
    nicknameGroupBox = new QGroupBox(tr("Nickname"));
    nicknameGroupBox->setAlignment(Qt::AlignHCenter);

    nickNameEdit = new QLineEdit();
    nickNameEdit->setText(Global::preferences->userName);

    QHBoxLayout *hbox = new QHBoxLayout();
    hbox->addWidget(nickNameEdit);

    nicknameGroupBox->setLayout(hbox);
}

void SetupWindow::createGroupNameGroupBox()
{
    groupNameGroupBox = new QGroupBox(tr("Group"));
    groupNameGroupBox->setAlignment(Qt::AlignHCenter);

    groupNameComboBox = new QComboBox();
    groupNameComboBox->setEditable(true);
    groupNameComboBox->setDuplicatesEnabled(false);
    QCompleter *completer
        = new QCompleter(Global::preferences->groupNameList, this);
    groupNameComboBox->setCompleter(completer);

    // groupNameComboBox->addItem("");
    groupNameComboBox->addItems(Global::preferences->groupNameList);
    int i = groupNameComboBox->findText(Global::preferences->groupName);
    groupNameComboBox->setCurrentIndex(i);

    QHBoxLayout *hbox = new QHBoxLayout();
    hbox->addWidget(groupNameComboBox);

    groupNameGroupBox->setLayout(hbox);
}

void SetupWindow::createSendRecvSettingsGroupBox()
{
    sendReceiveSettingsGroupBox = new QGroupBox(tr("Send/Receive Settings"));

    readCheck = new QCheckBox(tr("Envelope open check"));
    if (Global::preferences->isReadCheck) {
        readCheck->setCheckState(Qt::Checked);
    }
    noAutoPopupMsg = new QCheckBox(tr("Not popup message"));
    if (Global::preferences->isNoAutoPopupMsg) {
        noAutoPopupMsg->setCheckState(Qt::Checked);
    }
    noSoundAlarm = new QCheckBox(tr("No sound alarm"));
    if (Global::preferences->isNoSoundAlarm) {
        noSoundAlarm->setCheckState(Qt::Checked);
    }
    sealedSend = new QCheckBox(tr("Send sealed message by default"));
    if (Global::preferences->isSealed) {
        sealedSend->setCheckState(Qt::Checked);
    }
    quoteMsg = new QCheckBox(tr("Quote message by default"));
    if (Global::preferences->isQuoteMsg) {
        quoteMsg->setCheckState(Qt::Checked);
    }

    QVBoxLayout *vbox = new QVBoxLayout();
    vbox->addWidget(readCheck);
    vbox->addWidget(noAutoPopupMsg);
    vbox->addWidget(noSoundAlarm);
    vbox->addWidget(sealedSend);
    vbox->addWidget(quoteMsg);

    sendReceiveSettingsGroupBox->setLayout(vbox);
}

void SetupWindow::createBroadcastGroupBox()
{
    broadcastGroupBox = new QGroupBox(tr("None Local Broadcast Address Settings"));

    broadcast_list_widget_ = new QListWidget;
    broadcast_list_widget_->setSelectionMode(
            QAbstractItemView::ExtendedSelection);
    foreach(QString s, Global::preferences->userSpecifiedBroadcastIpList) {
        QListWidgetItem* item = new QListWidgetItem(s, broadcast_list_widget_);
        broadcast_list_widget_->insertItem(0, item);
    }

    QLabel* label = new QLabel(tr("Add Broadcast Address"));
    line_edit_ = new QLineEdit;
    QCheckBox* dial_up = new QCheckBox(tr("dial up connection"));
    dial_up->setEnabled(false);

    QPushButton* add_button = new QPushButton(">>");
    QPushButton* del_button = new QPushButton("<<");
    QFontMetrics fm = fontMetrics();
    add_button->setFixedSize(fm.width(">>") + 6, fm.width(">>") + 2);
    del_button->setFixedSize(fm.width(">>") + 6, fm.width(">>") + 2);
    connect(add_button, SIGNAL(clicked()), this, SLOT(AddBroadcast()));
    connect(del_button, SIGNAL(clicked()), this, SLOT(DelBroadcast()));
    QVBoxLayout* button_layout = new QVBoxLayout;
    button_layout->addSpacing(20);
    button_layout->addWidget(add_button);
    button_layout->addWidget(del_button);
    button_layout->addSpacing(20);

    QGridLayout *grid_layout = new QGridLayout();
    grid_layout->addWidget(label, 0, 0, 1, 1);
    grid_layout->addWidget(line_edit_, 1, 0, 1, 1);
    grid_layout->addWidget(dial_up, 2, 0, 1, 1);
    grid_layout->addLayout(button_layout, 0, 1, 3, 1);
    grid_layout->addWidget(broadcast_list_widget_, 0, 2, 3, 1);

    broadcastGroupBox->setLayout(grid_layout);
}

void SetupWindow::createButtonLayout()
{
    okButton = new QPushButton(tr("Ok"));
    applyButton = new QPushButton(tr("Apply"));

    buttonsLayout = new QHBoxLayout;
    buttonsLayout->addStretch();
    buttonsLayout->addWidget(okButton);
    buttonsLayout->addWidget(applyButton);
}

void SetupWindow::createMiscSettingGroupBox()
{
    miscSettingGroupBox = new QGroupBox(tr("Other Settings"));

    int frameStyle = QFrame::Sunken | QFrame::Panel;
    QLabel *label = new QLabel(tr("Click right button on send\nwindow to set display level"));
    label->setFrameStyle(frameStyle);

    detailSetupButton = new QPushButton(tr("Detail/Log settings"));
    urlLinkSetupButton = new QPushButton(tr("URL link settings"));
    urlLinkSetupButton->setDisabled(true);
    passwordSetupButton = new QPushButton(tr("Password settings"));
    passwordSetupButton->setDisabled(true);

    QVBoxLayout *vbox = new QVBoxLayout;
    vbox->addWidget(label);
    vbox->addWidget(detailSetupButton);
    vbox->addWidget(urlLinkSetupButton);
    vbox->addWidget(passwordSetupButton);

    miscSettingGroupBox->setLayout(vbox);
}

void SetupWindow::createConnections()
{
    connect(okButton, SIGNAL(clicked()),
            this, SLOT(savePreferences()));
    connect(applyButton, SIGNAL(clicked()),
            this, SLOT(applyPreferences()));

    connect(detailSetupButton, SIGNAL(clicked()),
            this, SLOT(detailSetup()));
}

void SetupWindow::savePreferences()
{
    applyPreferences();

    close();
}

void SetupWindow::applyPreferences()
{
    Global::preferences->userName = nickNameEdit->text();

    if (!groupNameComboBox->currentText().isEmpty() &&
        groupNameComboBox->findText(groupNameComboBox->currentText())
        == -1) { // Group not exist
        groupNameComboBox->insertItem(0, groupNameComboBox->currentText());

        for (int i = 0; i < groupNameComboBox->count(); ++i) {
            QString s = groupNameComboBox->itemText(i);
            if (!Global::preferences->groupNameList.contains(s)) {
                Global::preferences->groupNameList.prepend(s);
            }
        }

        QCompleter *completer
            = new QCompleter(Global::preferences->groupNameList, this);
        groupNameComboBox->setCompleter(completer);
    }
    Global::preferences->groupName = groupNameComboBox->currentText();

    Global::preferences->isReadCheck = readCheck->isChecked();
    Global::preferences->isNoSoundAlarm = noSoundAlarm->isChecked();
    Global::preferences->isSealed = sealedSend->isChecked();
    Global::preferences->isQuoteMsg = quoteMsg->isChecked();
    Global::preferences->isNoAutoPopupMsg = noAutoPopupMsg->isChecked();

    Global::userManager->updateOurself();
    Global::userManager->broadcastEntry();

    Global::preferences->save();
}

void SetupWindow::detailSetup()
{
    DetailSetupDialog *dialog = new DetailSetupDialog(this);
    dialog->show();
}

void SetupWindow::AddBroadcast()
{
    QString s = line_edit_->text();
    if (!Global::preferences->userSpecifiedBroadcastIpList.contains(s)) {
        // NOTE: Just use QHostAddress to verify user input is legal ip
        // address.
        QHostAddress* address = new QHostAddress;
        if (address->setAddress(s)) {
            Global::preferences->userSpecifiedBroadcastIpList.prepend(s);
            QListWidgetItem* item = new QListWidgetItem(s, broadcast_list_widget_);
            broadcast_list_widget_->insertItem(0, item);
        }
    }
}

void SetupWindow::DelBroadcast()
{
    foreach(QListWidgetItem* item, broadcast_list_widget_->selectedItems()) {
        int row = broadcast_list_widget_->row(item);
        broadcast_list_widget_->takeItem(row);
        line_edit_->setText(item->text());
        Global::preferences->userSpecifiedBroadcastIpList.removeOne(
                item->text());
    }
}

QSize SetupWindow::sizeHint() const
{
    return QSize(360, 240);
}

