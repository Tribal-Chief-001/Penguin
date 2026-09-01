/****************************************************************************
** Meta object code from reading C++ file 'TeleprompterWidget.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.4.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "src/ui/TeleprompterWidget.h"
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'TeleprompterWidget.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 68
#error "This file was generated using the moc from 6.4.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

#ifndef Q_CONSTINIT
#define Q_CONSTINIT
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
namespace {
struct qt_meta_stringdata_Penguin__UI__TeleprompterWidget_t {
    uint offsetsAndSizes[36];
    char stringdata0[32];
    char stringdata1[14];
    char stringdata2[1];
    char stringdata3[11];
    char stringdata4[17];
    char stringdata5[9];
    char stringdata6[5];
    char stringdata7[15];
    char stringdata8[11];
    char stringdata9[12];
    char stringdata10[9];
    char stringdata11[13];
    char stringdata12[16];
    char stringdata13[7];
    char stringdata14[14];
    char stringdata15[18];
    char stringdata16[12];
    char stringdata17[13];
};
#define QT_MOC_LITERAL(ofs, len) \
    uint(sizeof(qt_meta_stringdata_Penguin__UI__TeleprompterWidget_t::offsetsAndSizes) + ofs), len 
Q_CONSTINIT static const qt_meta_stringdata_Penguin__UI__TeleprompterWidget_t qt_meta_stringdata_Penguin__UI__TeleprompterWidget = {
    {
        QT_MOC_LITERAL(0, 31),  // "Penguin::UI::TeleprompterWidget"
        QT_MOC_LITERAL(32, 13),  // "seekRequested"
        QT_MOC_LITERAL(46, 0),  // ""
        QT_MOC_LITERAL(47, 10),  // "positionMs"
        QT_MOC_LITERAL(58, 16),  // "activeCueChanged"
        QT_MOC_LITERAL(75, 8),  // "cueIndex"
        QT_MOC_LITERAL(84, 4),  // "text"
        QT_MOC_LITERAL(89, 14),  // "loadLrcContent"
        QT_MOC_LITERAL(104, 10),  // "lrcContent"
        QT_MOC_LITERAL(115, 11),  // "loadLrcFile"
        QT_MOC_LITERAL(127, 8),  // "filePath"
        QT_MOC_LITERAL(136, 12),  // "setLrcParser"
        QT_MOC_LITERAL(149, 15),  // "Core::LrcParser"
        QT_MOC_LITERAL(165, 6),  // "parser"
        QT_MOC_LITERAL(172, 13),  // "setPositionMs"
        QT_MOC_LITERAL(186, 17),  // "setActiveCueIndex"
        QT_MOC_LITERAL(204, 11),  // "clearLyrics"
        QT_MOC_LITERAL(216, 12)   // "scrollOffset"
    },
    "Penguin::UI::TeleprompterWidget",
    "seekRequested",
    "",
    "positionMs",
    "activeCueChanged",
    "cueIndex",
    "text",
    "loadLrcContent",
    "lrcContent",
    "loadLrcFile",
    "filePath",
    "setLrcParser",
    "Core::LrcParser",
    "parser",
    "setPositionMs",
    "setActiveCueIndex",
    "clearLyrics",
    "scrollOffset"
};
#undef QT_MOC_LITERAL
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_Penguin__UI__TeleprompterWidget[] = {

 // content:
      10,       // revision
       0,       // classname
       0,    0, // classinfo
       8,   14, // methods
       1,   86, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       2,       // signalCount

 // signals: name, argc, parameters, tag, flags, initial metatype offsets
       1,    1,   62,    2, 0x06,    2 /* Public */,
       4,    2,   65,    2, 0x06,    4 /* Public */,

 // slots: name, argc, parameters, tag, flags, initial metatype offsets
       7,    1,   70,    2, 0x0a,    7 /* Public */,
       9,    1,   73,    2, 0x0a,    9 /* Public */,
      11,    1,   76,    2, 0x0a,   11 /* Public */,
      14,    1,   79,    2, 0x0a,   13 /* Public */,
      15,    1,   82,    2, 0x0a,   15 /* Public */,
      16,    0,   85,    2, 0x0a,   17 /* Public */,

 // signals: parameters
    QMetaType::Void, QMetaType::LongLong,    3,
    QMetaType::Void, QMetaType::Int, QMetaType::QString,    5,    6,

 // slots: parameters
    QMetaType::Bool, QMetaType::QString,    8,
    QMetaType::Bool, QMetaType::QString,   10,
    QMetaType::Void, 0x80000000 | 12,   13,
    QMetaType::Void, QMetaType::LongLong,    3,
    QMetaType::Void, QMetaType::Int,    5,
    QMetaType::Void,

 // properties: name, type, flags
      17, QMetaType::Double, 0x00015103, uint(-1), 0,

       0        // eod
};

Q_CONSTINIT const QMetaObject Penguin::UI::TeleprompterWidget::staticMetaObject = { {
    QMetaObject::SuperData::link<QWidget::staticMetaObject>(),
    qt_meta_stringdata_Penguin__UI__TeleprompterWidget.offsetsAndSizes,
    qt_meta_data_Penguin__UI__TeleprompterWidget,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_stringdata_Penguin__UI__TeleprompterWidget_t,
        // property 'scrollOffset'
        QtPrivate::TypeAndForceComplete<double, std::true_type>,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<TeleprompterWidget, std::true_type>,
        // method 'seekRequested'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<qint64, std::false_type>,
        // method 'activeCueChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'loadLrcContent'
        QtPrivate::TypeAndForceComplete<bool, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'loadLrcFile'
        QtPrivate::TypeAndForceComplete<bool, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'setLrcParser'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const Core::LrcParser &, std::false_type>,
        // method 'setPositionMs'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<qint64, std::false_type>,
        // method 'setActiveCueIndex'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'clearLyrics'
        QtPrivate::TypeAndForceComplete<void, std::false_type>
    >,
    nullptr
} };

void Penguin::UI::TeleprompterWidget::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<TeleprompterWidget *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->seekRequested((*reinterpret_cast< std::add_pointer_t<qint64>>(_a[1]))); break;
        case 1: _t->activeCueChanged((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QString>>(_a[2]))); break;
        case 2: { bool _r = _t->loadLrcContent((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1])));
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = std::move(_r); }  break;
        case 3: { bool _r = _t->loadLrcFile((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1])));
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = std::move(_r); }  break;
        case 4: _t->setLrcParser((*reinterpret_cast< std::add_pointer_t<Core::LrcParser>>(_a[1]))); break;
        case 5: _t->setPositionMs((*reinterpret_cast< std::add_pointer_t<qint64>>(_a[1]))); break;
        case 6: _t->setActiveCueIndex((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 7: _t->clearLyrics(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (TeleprompterWidget::*)(qint64 );
            if (_t _q_method = &TeleprompterWidget::seekRequested; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (TeleprompterWidget::*)(int , const QString & );
            if (_t _q_method = &TeleprompterWidget::activeCueChanged; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 1;
                return;
            }
        }
    }else if (_c == QMetaObject::ReadProperty) {
        auto *_t = static_cast<TeleprompterWidget *>(_o);
        (void)_t;
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast< double*>(_v) = _t->scrollOffset(); break;
        default: break;
        }
    } else if (_c == QMetaObject::WriteProperty) {
        auto *_t = static_cast<TeleprompterWidget *>(_o);
        (void)_t;
        void *_v = _a[0];
        switch (_id) {
        case 0: _t->setScrollOffset(*reinterpret_cast< double*>(_v)); break;
        default: break;
        }
    } else if (_c == QMetaObject::ResetProperty) {
    } else if (_c == QMetaObject::BindableProperty) {
    }
}

const QMetaObject *Penguin::UI::TeleprompterWidget::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *Penguin::UI::TeleprompterWidget::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_Penguin__UI__TeleprompterWidget.stringdata0))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int Penguin::UI::TeleprompterWidget::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 8)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 8;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 8)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 8;
    }else if (_c == QMetaObject::ReadProperty || _c == QMetaObject::WriteProperty
            || _c == QMetaObject::ResetProperty || _c == QMetaObject::BindableProperty
            || _c == QMetaObject::RegisterPropertyMetaType) {
        qt_static_metacall(this, _c, _id, _a);
        _id -= 1;
    }
    return _id;
}

// SIGNAL 0
void Penguin::UI::TeleprompterWidget::seekRequested(qint64 _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void Penguin::UI::TeleprompterWidget::activeCueChanged(int _t1, const QString & _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
