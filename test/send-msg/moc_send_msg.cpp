/****************************************************************************
** Meta object code from reading C++ file 'send_msg.h'
**
** Created: Tue Sep 16 00:45:00 2008
**      by: The Qt Meta Object Compiler version 59 (Qt 4.3.4)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "send_msg.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'send_msg.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 59
#error "This file was generated using the moc from 4.3.4. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

static const uint qt_meta_data_SendMsg[] = {

 // content:
       1,       // revision
       0,       // classname
       0,    0, // classinfo
       1,   10, // methods
       0,    0, // properties
       0,    0, // enums/sets

 // slots: signature, parameters, type, tag, flags
       9,    8,    8,    8, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_SendMsg[] = {
    "SendMsg\0\0send()\0"
};

const QMetaObject SendMsg::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_SendMsg,
      qt_meta_data_SendMsg, 0 }
};

const QMetaObject *SendMsg::metaObject() const
{
    return &staticMetaObject;
}

void *SendMsg::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_SendMsg))
	return static_cast<void*>(const_cast< SendMsg*>(this));
    return QWidget::qt_metacast(_clname);
}

int SendMsg::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: send(); break;
        }
        _id -= 1;
    }
    return _id;
}
