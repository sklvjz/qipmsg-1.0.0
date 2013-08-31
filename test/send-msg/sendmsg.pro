TEMPLATE = app

CONFIG += qt warn_on debug

QT += network

HEADERS += \
    send_msg.h

SOURCES += \
    main.cpp \
    send_msg.cpp

#TRANSLATIONS = translations/sendmsg_zh_CN.ts

unix {
  UI_DIR = .ui
  MOC_DIR = .moc
  OBJECTS_DIR = .obj
}

win32 {
}

