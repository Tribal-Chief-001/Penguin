/****************************************************************************
** Meta object code from reading C++ file 'MainWindow.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.4.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "src/ui/MainWindow.h"
#include <QtGui/qtextcursor.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'MainWindow.h' doesn't include <QObject>."
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
struct qt_meta_stringdata_Penguin__UI__MainWindow_t {
    uint offsetsAndSizes[40];
    char stringdata0[24];
    char stringdata1[8];
    char stringdata2[1];
    char stringdata3[7];
    char stringdata4[5];
    char stringdata5[11];
    char stringdata6[10];
    char stringdata7[9];
    char stringdata8[9];
    char stringdata9[14];
    char stringdata10[8];
    char stringdata11[15];
    char stringdata12[20];
    char stringdata13[17];
    char stringdata14[24];
    char stringdata15[20];
    char stringdata16[5];
    char stringdata17[22];
    char stringdata18[13];
    char stringdata19[22];
};
#define QT_MOC_LITERAL(ofs, len) \
    uint(sizeof(qt_meta_stringdata_Penguin__UI__MainWindow_t::offsetsAndSizes) + ofs), len 
Q_CONSTINIT static const qt_meta_stringdata_Penguin__UI__MainWindow_t qt_meta_stringdata_Penguin__UI__MainWindow = {
    {
        QT_MOC_LITERAL(0, 23),  // "Penguin::UI::MainWindow"
        QT_MOC_LITERAL(24, 7),  // "setMode"
        QT_MOC_LITERAL(32, 0),  // ""
        QT_MOC_LITERAL(33, 6),  // "UIMode"
        QT_MOC_LITERAL(40, 4),  // "mode"
        QT_MOC_LITERAL(45, 10),  // "toggleMode"
        QT_MOC_LITERAL(56, 9),  // "openMedia"
        QT_MOC_LITERAL(66, 8),  // "filePath"
        QT_MOC_LITERAL(75, 8),  // "autoPlay"
        QT_MOC_LITERAL(84, 13),  // "openDirectory"
        QT_MOC_LITERAL(98, 7),  // "dirPath"
        QT_MOC_LITERAL(106, 14),  // "openFileDialog"
        QT_MOC_LITERAL(121, 19),  // "openDirectoryDialog"
        QT_MOC_LITERAL(141, 16),  // "toggleFullscreen"
        QT_MOC_LITERAL(158, 23),  // "onEngineMetadataChanged"
        QT_MOC_LITERAL(182, 19),  // "Core::MediaMetadata"
        QT_MOC_LITERAL(202, 4),  // "meta"
        QT_MOC_LITERAL(207, 21),  // "onEngineErrorOccurred"
        QT_MOC_LITERAL(229, 12),  // "errorMessage"
        QT_MOC_LITERAL(242, 21)   // "onEngineMediaFinished"
    },
    "Penguin::UI::MainWindow",
    "setMode",
    "",
    "UIMode",
    "mode",
    "toggleMode",
    "openMedia",
    "filePath",
    "autoPlay",
    "openDirectory",
    "dirPath",
    "openFileDialog",
    "openDirectoryDialog",
    "toggleFullscreen",
    "onEngineMetadataChanged",
    "Core::MediaMetadata",
    "meta",
    "onEngineErrorOccurred",
    "errorMessage",
    "onEngineMediaFinished"
};
#undef QT_MOC_LITERAL
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_Penguin__UI__MainWindow[] = {

 // content:
      10,       // revision
       0,       // classname
       0,    0, // classinfo
      11,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags, initial metatype offsets
       1,    1,   80,    2, 0x0a,    1 /* Public */,
       5,    0,   83,    2, 0x0a,    3 /* Public */,
       6,    2,   84,    2, 0x0a,    4 /* Public */,
       6,    1,   89,    2, 0x2a,    7 /* Public | MethodCloned */,
       9,    1,   92,    2, 0x0a,    9 /* Public */,
      11,    0,   95,    2, 0x0a,   11 /* Public */,
      12,    0,   96,    2, 0x0a,   12 /* Public */,
      13,    0,   97,    2, 0x0a,   13 /* Public */,
      14,    1,   98,    2, 0x08,   14 /* Private */,
      17,    1,  101,    2, 0x08,   16 /* Private */,
      19,    0,  104,    2, 0x08,   18 /* Private */,

 // slots: parameters
    QMetaType::Void, 0x80000000 | 3,    4,
    QMetaType::Void,
    QMetaType::Bool, QMetaType::QString, QMetaType::Bool,    7,    8,
    QMetaType::Bool, QMetaType::QString,    7,
    QMetaType::Void, QMetaType::QString,   10,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 15,   16,
    QMetaType::Void, QMetaType::QString,   18,
    QMetaType::Void,

       0        // eod
};

Q_CONSTINIT const QMetaObject Penguin::UI::MainWindow::staticMetaObject = { {
    QMetaObject::SuperData::link<QMainWindow::staticMetaObject>(),
    qt_meta_stringdata_Penguin__UI__MainWindow.offsetsAndSizes,
    qt_meta_data_Penguin__UI__MainWindow,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_stringdata_Penguin__UI__MainWindow_t,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<MainWindow, std::true_type>,
        // method 'setMode'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<UIMode, std::false_type>,
        // method 'toggleMode'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'openMedia'
        QtPrivate::TypeAndForceComplete<bool, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        QtPrivate::TypeAndForceComplete<bool, std::false_type>,
        // method 'openMedia'
        QtPrivate::TypeAndForceComplete<bool, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'openDirectory'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'openFileDialog'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'openDirectoryDialog'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'toggleFullscreen'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onEngineMetadataChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const Core::MediaMetadata &, std::false_type>,
        // method 'onEngineErrorOccurred'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'onEngineMediaFinished'
        QtPrivate::TypeAndForceComplete<void, std::false_type>
    >,
    nullptr
} };

void Penguin::UI::MainWindow::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<MainWindow *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->setMode((*reinterpret_cast< std::add_pointer_t<UIMode>>(_a[1]))); break;
        case 1: _t->toggleMode(); break;
        case 2: { bool _r = _t->openMedia((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<bool>>(_a[2])));
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = std::move(_r); }  break;
        case 3: { bool _r = _t->openMedia((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1])));
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = std::move(_r); }  break;
        case 4: _t->openDirectory((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 5: _t->openFileDialog(); break;
        case 6: _t->openDirectoryDialog(); break;
        case 7: _t->toggleFullscreen(); break;
        case 8: _t->onEngineMetadataChanged((*reinterpret_cast< std::add_pointer_t<Core::MediaMetadata>>(_a[1]))); break;
        case 9: _t->onEngineErrorOccurred((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 10: _t->onEngineMediaFinished(); break;
        default: ;
        }
    }
}

const QMetaObject *Penguin::UI::MainWindow::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *Penguin::UI::MainWindow::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_Penguin__UI__MainWindow.stringdata0))
        return static_cast<void*>(this);
    return QMainWindow::qt_metacast(_clname);
}

int Penguin::UI::MainWindow::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QMainWindow::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 11)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 11;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 11)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 11;
    }
    return _id;
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
