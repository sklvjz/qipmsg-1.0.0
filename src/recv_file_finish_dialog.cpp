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

#include <QtGui>
#include <QtCore>

#include "recv_file_finish_dialog.h"
#include "global.h"
#include "helper.h"
#include "constants.h"
#include "recv_file_map.h"

RecvFileFinishDialog::RecvFileFinishDialog(RecvFileMap *recvFileMap,
                                           QWidget *parent)
    : QDialog(parent), m_recvFileMap(recvFileMap)
{
    setWindowModality(Qt::WindowModal);

    createGroupBox();
    createButtonLayout();

    createConnections();

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(statsGroupBox);
    mainLayout->addSpacing(5);
    mainLayout->addLayout(buttonsLayout);

    if (m_recvFileMap->dirCount() > 0
        || m_recvFileMap->regularFileCount() > 1) {
        openFileButton->setDisabled(true);
    }

    setWindowTitle(tr("Congratulation! Get File Successfully!"));

    setLayout(mainLayout);
}

void RecvFileFinishDialog::createGroupBox()
{
    statsGroupBox = new QGroupBox();
    QVBoxLayout *vbox = new QVBoxLayout;

    statsLabel = new QLabel(m_recvFileMap->transferStatsInfo());
    statsLabel->setAlignment(Qt::AlignHCenter);
    vbox->addWidget(statsLabel);

    statsGroupBox->setLayout(vbox);
}

void RecvFileFinishDialog::createButtonLayout()
{
    closeButton = new QPushButton(tr("Close"));
    openFileButton = new QPushButton(tr("Open File"));
    openDirButton = new QPushButton(tr("Open Folder"));

    buttonsLayout = new QHBoxLayout;
    buttonsLayout->addWidget(closeButton);
    buttonsLayout->addSpacing(10);
    buttonsLayout->addWidget(openFileButton);
    buttonsLayout->addSpacing(10);
    buttonsLayout->addWidget(openDirButton);
}

void RecvFileFinishDialog::createConnections()
{
    connect(closeButton, SIGNAL(clicked()),
            this, SLOT(close()));
    connect(openFileButton, SIGNAL(clicked()),
            this, SLOT(openFile()));
    connect(openDirButton, SIGNAL(clicked()),
            this, SLOT(openDir()));
}

void RecvFileFinishDialog::openFile()
{
    // XXX NOTE: Qt use 'xdg-open' to open url, but 'xdg-open' not work
    // properly, so we use the 'qipmsg-xdg-open' to open url. 'qipmsg-xdg-open'
    // use gvfs-open to open url if gvfs-open exist.
    QString filePath = m_recvFileMap->saveFilePath() + "/"
        + m_recvFileMap->currentFile()->name();
    QProcess process;
    process.closeReadChannel(QProcess::StandardOutput);
    process.closeReadChannel(QProcess::StandardError);
    process.startDetached(Helper::openUrlProgram(),
            QStringList() << filePath);

    close();
}

void RecvFileFinishDialog::openDir()
{
    QString filePath;
    if (m_recvFileMap->regularFileCount() < 1
        && m_recvFileMap->dirCount() == 1) {
        filePath = m_recvFileMap->saveFilePath() + "/" +
            m_recvFileMap->currentFile()->name();
    } else {
        filePath = m_recvFileMap->saveFilePath();
    }
    QProcess process;
    process.startDetached(Helper::openUrlProgram(),
            QStringList() << filePath);

    close();
}

void RecvFileFinishDialog::closeEvent(QCloseEvent *event)
{
    emit fileChecked();

    event->accept();
}

