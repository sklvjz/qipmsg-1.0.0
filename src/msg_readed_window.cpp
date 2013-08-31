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

#include "msg_readed_window.h"
#include "global.h"
#include "window_manager.h"

#include <QtCore>
#include <QtGui>

MsgReadedWindow::MsgReadedWindow(QString nickName)
{
    createUi();
    // Delete when close
    setAttribute(Qt::WA_DeleteOnClose, true);

    createConnections();

    QString title(tr("Envelope Opened: "));
    title.append(nickName);
    setWindowTitle(title);
    setWindowIcon(*Global::iconSet.value("normal"));

    setWindowModality(Qt::NonModal);

    adjustSize();
    move(Global::randomNearMiddlePoint());
}

QSize MsgReadedWindow::sizeHint() const
{
    return QSize(180, 100);
}

void MsgReadedWindow::createUi()
{
    QVBoxLayout *mainLayout = new QVBoxLayout;;

    QString s = QString(tr("Envelope Opened"));
    s.append("\n");

    QDateTime now = QDateTime::currentDateTime();
    s.append("(");
    s.append(now.toString("dddd hh:mm"));
    s.append(")");

    QLabel *label = new QLabel(s);
    label->setAlignment(Qt::AlignHCenter);

    okButton = new QPushButton(QString(tr("Ok")));
    okButton->setSizePolicy(QSizePolicy(QSizePolicy::Fixed,
                                        QSizePolicy::Fixed));
    okButton->setFocusPolicy(Qt::StrongFocus);

    mainLayout->addWidget(label);
    mainLayout->addWidget(okButton, 0, Qt::AlignHCenter);

    setLayout(mainLayout);
}

void MsgReadedWindow::createConnections()
{
    connect(okButton, SIGNAL(clicked()),
            this, SLOT(close()));
}

void MsgReadedWindow::closeEvent(QCloseEvent *event)
{
    Global::windowManager->removeMsgReadedWindow(this);

    event->accept();
}

