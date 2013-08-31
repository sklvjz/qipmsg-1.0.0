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

#include "retry_recv_file_dialog.h"
#include "constants.h"
#include "recv_file_map.h"

RetryRecvFileDialog::RetryRecvFileDialog(RecvFileMap *recvFileMap,
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

    setWindowTitle(tr("Sorry! Get File Failed!"));

    setLayout(mainLayout);
}

void RetryRecvFileDialog::createGroupBox()
{
    statsGroupBox = new QGroupBox();
    QVBoxLayout *vbox = new QVBoxLayout;

    statsLabel = new QLabel(m_recvFileMap->transferStatsInfo());
    statsLabel->setAlignment(Qt::AlignHCenter);
    vbox->addWidget(statsLabel);

    statsGroupBox->setLayout(vbox);
}

void RetryRecvFileDialog::createButtonLayout()
{
    retryButton = new QPushButton(tr("Retry"));
    cancelButton = new QPushButton(tr("Cancel"));

    buttonsLayout = new QHBoxLayout;
    buttonsLayout->addWidget(retryButton);
    buttonsLayout->addSpacing(10);
    buttonsLayout->addWidget(cancelButton);
}

void RetryRecvFileDialog::createConnections()
{
    connect(retryButton, SIGNAL(clicked()),
            this, SLOT(retryGetFile()));
    connect(cancelButton, SIGNAL(clicked()),
            this, SLOT(close()));
}

void RetryRecvFileDialog::retryGetFile()
{
    emit retry();

    close();
}

void RetryRecvFileDialog::closeEvent(QCloseEvent *event)
{
    emit retryChecked();

    event->accept();
}

