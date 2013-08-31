#ifndef SEND_MSG_H
#define SEND_MSG_H

#include <QWidget>
#include <QUdpSocket>

class QTextEdit;
class QPushButton;


class SendMsg : public QWidget
{
    Q_OBJECT

public:
    SendMsg(QWidget *parent = 0);
    ~SendMsg() {}

private slots:
    void send();

private:
    void createButtonsLayout();
    void createConnections();

    QTextEdit *textEdit;
    QPushButton *sendButton;
    QPushButton *closeButton;

    QUdpSocket m_udpSocket;
};

#endif // !SEND_MSG_H

