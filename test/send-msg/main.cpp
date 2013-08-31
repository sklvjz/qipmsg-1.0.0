#include "send_msg.h"

#include <QApplication>

int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    SendMsg sendMsg;
    sendMsg.show();

    return app.exec();
}

