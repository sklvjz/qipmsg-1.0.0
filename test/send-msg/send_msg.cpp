#include "send_msg.h"

#include <QtCore>
#include <QtGui>

#include <unistd.h>

SendMsg::SendMsg(QWidget *parent)
{
    textEdit = new QTextEdit;
    createButtonsLayout();

    createConnections();

    QGridLayout *mainLayout = new QGridLayout;

    mainLayout->addWidget(textEdit, 0, 0, 1, 2);
    mainLayout->addWidget(sendButton, 1, 0);
    mainLayout->addWidget(closeButton, 1, 1);

    setLayout(mainLayout);
}

void SendMsg::createButtonsLayout()
{
    sendButton = new QPushButton(tr("Send"));
    closeButton = new QPushButton(tr("Close"));
}

void SendMsg::createConnections()
{
    connect(sendButton, SIGNAL(clicked()),
            this, SLOT(send()));
    connect(closeButton, SIGNAL(clicked()),
            this, SLOT(close()));
}

void SendMsg::send()
{

#define IPMSG_SENDMSG           0x00000020UL
#define IPMSG_SENDCHECKOPT      0x00000100UL
#define IPMSG_SECRETOPT         0x00000200UL
#define IPMSG_READCHECKOPT      0x00100000UL
    int flag = 0;
    flag |= IPMSG_SENDMSG | IPMSG_SENDCHECKOPT
        | IPMSG_SECRETOPT | IPMSG_READCHECKOPT;
    QByteArray datagram;
    datagram.append("1:1001:logname:host:");
    datagram.append(QString("%1%2").arg(flag).arg(":"));
    datagram.append(textEdit->toPlainText());

    int cnt = 0;
    forever {
        if (m_udpSocket.writeDatagram(datagram, QHostAddress("192.168.1.104"), 2425) == -1) {
            qDebug() << "SendMsg::send: send datagram failed";
        }

        usleep(200000);

        ++cnt;
        if (cnt == 1) {
            break;
        }
    }
}

