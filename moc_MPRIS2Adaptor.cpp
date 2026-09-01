/****************************************************************************
** Meta object code from reading C++ file 'MPRIS2Adaptor.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.4.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "src/desktop/MPRIS2Adaptor.h"
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'MPRIS2Adaptor.h' doesn't include <QObject>."
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
struct qt_meta_stringdata_Penguin__Desktop__MPRIS2RootAdaptor_t {
    uint offsetsAndSizes[34];
    char stringdata0[36];
    char stringdata1[16];
    char stringdata2[23];
    char stringdata3[20];
    char stringdata4[653];
    char stringdata5[6];
    char stringdata6[1];
    char stringdata7[5];
    char stringdata8[8];
    char stringdata9[11];
    char stringdata10[17];
    char stringdata11[9];
    char stringdata12[13];
    char stringdata13[9];
    char stringdata14[13];
    char stringdata15[20];
    char stringdata16[19];
};
#define QT_MOC_LITERAL(ofs, len) \
    uint(sizeof(qt_meta_stringdata_Penguin__Desktop__MPRIS2RootAdaptor_t::offsetsAndSizes) + ofs), len 
Q_CONSTINIT static const qt_meta_stringdata_Penguin__Desktop__MPRIS2RootAdaptor_t qt_meta_stringdata_Penguin__Desktop__MPRIS2RootAdaptor = {
    {
        QT_MOC_LITERAL(0, 35),  // "Penguin::Desktop::MPRIS2RootA..."
        QT_MOC_LITERAL(36, 15),  // "D-Bus Interface"
        QT_MOC_LITERAL(52, 22),  // "org.mpris.MediaPlayer2"
        QT_MOC_LITERAL(75, 19),  // "D-Bus Introspection"
        QT_MOC_LITERAL(95, 652),  // "  <interface name=\"org.mpris..."
        QT_MOC_LITERAL(748, 5),  // "Raise"
        QT_MOC_LITERAL(754, 0),  // ""
        QT_MOC_LITERAL(755, 4),  // "Quit"
        QT_MOC_LITERAL(760, 7),  // "CanQuit"
        QT_MOC_LITERAL(768, 10),  // "Fullscreen"
        QT_MOC_LITERAL(779, 16),  // "CanSetFullscreen"
        QT_MOC_LITERAL(796, 8),  // "CanRaise"
        QT_MOC_LITERAL(805, 12),  // "HasTrackList"
        QT_MOC_LITERAL(818, 8),  // "Identity"
        QT_MOC_LITERAL(827, 12),  // "DesktopEntry"
        QT_MOC_LITERAL(840, 19),  // "SupportedUriSchemes"
        QT_MOC_LITERAL(860, 18)   // "SupportedMimeTypes"
    },
    "Penguin::Desktop::MPRIS2RootAdaptor",
    "D-Bus Interface",
    "org.mpris.MediaPlayer2",
    "D-Bus Introspection",
    "  <interface name=\"org.mpris.MediaPlayer2\">\n    <method name=\"Rais"
    "e\"/>\n    <method name=\"Quit\"/>\n    <property name=\"CanQuit\" typ"
    "e=\"b\" access=\"read\"/>\n    <property name=\"Fullscreen\" type=\"b\""
    " access=\"readwrite\"/>\n    <property name=\"CanSetFullscreen\" type="
    "\"b\" access=\"read\"/>\n    <property name=\"CanRaise\" type=\"b\" ac"
    "cess=\"read\"/>\n    <property name=\"HasTrackList\" type=\"b\" access"
    "=\"read\"/>\n    <property name=\"Identity\" type=\"s\" access=\"read\""
    "/>\n    <property name=\"DesktopEntry\" type=\"s\" access=\"read\"/>\n"
    "    <property name=\"SupportedUriSchemes\" type=\"as\" access=\"read\""
    "/>\n    <property name=\"SupportedMimeTypes\" type=\"as\" access=\"rea"
    "d\"/>\n  </interface>\n",
    "Raise",
    "",
    "Quit",
    "CanQuit",
    "Fullscreen",
    "CanSetFullscreen",
    "CanRaise",
    "HasTrackList",
    "Identity",
    "DesktopEntry",
    "SupportedUriSchemes",
    "SupportedMimeTypes"
};
#undef QT_MOC_LITERAL
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_Penguin__Desktop__MPRIS2RootAdaptor[] = {

 // content:
      10,       // revision
       0,       // classname
       2,   14, // classinfo
       2,   18, // methods
       9,   32, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // classinfo: key, value
       1,    2,
       3,    4,

 // slots: name, argc, parameters, tag, flags, initial metatype offsets
       5,    0,   30,    6, 0x0a,   10 /* Public */,
       7,    0,   31,    6, 0x0a,   11 /* Public */,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,

 // properties: name, type, flags
       8, QMetaType::Bool, 0x00015001, uint(-1), 0,
       9, QMetaType::Bool, 0x00015103, uint(-1), 0,
      10, QMetaType::Bool, 0x00015001, uint(-1), 0,
      11, QMetaType::Bool, 0x00015001, uint(-1), 0,
      12, QMetaType::Bool, 0x00015001, uint(-1), 0,
      13, QMetaType::QString, 0x00015001, uint(-1), 0,
      14, QMetaType::QString, 0x00015001, uint(-1), 0,
      15, QMetaType::QStringList, 0x00015001, uint(-1), 0,
      16, QMetaType::QStringList, 0x00015001, uint(-1), 0,

       0        // eod
};

Q_CONSTINIT const QMetaObject Penguin::Desktop::MPRIS2RootAdaptor::staticMetaObject = { {
    QMetaObject::SuperData::link<QDBusAbstractAdaptor::staticMetaObject>(),
    qt_meta_stringdata_Penguin__Desktop__MPRIS2RootAdaptor.offsetsAndSizes,
    qt_meta_data_Penguin__Desktop__MPRIS2RootAdaptor,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_stringdata_Penguin__Desktop__MPRIS2RootAdaptor_t,
        // property 'CanQuit'
        QtPrivate::TypeAndForceComplete<bool, std::true_type>,
        // property 'Fullscreen'
        QtPrivate::TypeAndForceComplete<bool, std::true_type>,
        // property 'CanSetFullscreen'
        QtPrivate::TypeAndForceComplete<bool, std::true_type>,
        // property 'CanRaise'
        QtPrivate::TypeAndForceComplete<bool, std::true_type>,
        // property 'HasTrackList'
        QtPrivate::TypeAndForceComplete<bool, std::true_type>,
        // property 'Identity'
        QtPrivate::TypeAndForceComplete<QString, std::true_type>,
        // property 'DesktopEntry'
        QtPrivate::TypeAndForceComplete<QString, std::true_type>,
        // property 'SupportedUriSchemes'
        QtPrivate::TypeAndForceComplete<QStringList, std::true_type>,
        // property 'SupportedMimeTypes'
        QtPrivate::TypeAndForceComplete<QStringList, std::true_type>,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<MPRIS2RootAdaptor, std::true_type>,
        // method 'Raise'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'Quit'
        QtPrivate::TypeAndForceComplete<void, std::false_type>
    >,
    nullptr
} };

void Penguin::Desktop::MPRIS2RootAdaptor::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<MPRIS2RootAdaptor *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->Raise(); break;
        case 1: _t->Quit(); break;
        default: ;
        }
    }else if (_c == QMetaObject::ReadProperty) {
        auto *_t = static_cast<MPRIS2RootAdaptor *>(_o);
        (void)_t;
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast< bool*>(_v) = _t->canQuit(); break;
        case 1: *reinterpret_cast< bool*>(_v) = _t->fullscreen(); break;
        case 2: *reinterpret_cast< bool*>(_v) = _t->canSetFullscreen(); break;
        case 3: *reinterpret_cast< bool*>(_v) = _t->canRaise(); break;
        case 4: *reinterpret_cast< bool*>(_v) = _t->hasTrackList(); break;
        case 5: *reinterpret_cast< QString*>(_v) = _t->identity(); break;
        case 6: *reinterpret_cast< QString*>(_v) = _t->desktopEntry(); break;
        case 7: *reinterpret_cast< QStringList*>(_v) = _t->supportedUriSchemes(); break;
        case 8: *reinterpret_cast< QStringList*>(_v) = _t->supportedMimeTypes(); break;
        default: break;
        }
    } else if (_c == QMetaObject::WriteProperty) {
        auto *_t = static_cast<MPRIS2RootAdaptor *>(_o);
        (void)_t;
        void *_v = _a[0];
        switch (_id) {
        case 1: _t->setFullscreen(*reinterpret_cast< bool*>(_v)); break;
        default: break;
        }
    } else if (_c == QMetaObject::ResetProperty) {
    } else if (_c == QMetaObject::BindableProperty) {
    }
    (void)_a;
}

const QMetaObject *Penguin::Desktop::MPRIS2RootAdaptor::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *Penguin::Desktop::MPRIS2RootAdaptor::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_Penguin__Desktop__MPRIS2RootAdaptor.stringdata0))
        return static_cast<void*>(this);
    return QDBusAbstractAdaptor::qt_metacast(_clname);
}

int Penguin::Desktop::MPRIS2RootAdaptor::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDBusAbstractAdaptor::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 2)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 2;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 2)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 2;
    }else if (_c == QMetaObject::ReadProperty || _c == QMetaObject::WriteProperty
            || _c == QMetaObject::ResetProperty || _c == QMetaObject::BindableProperty
            || _c == QMetaObject::RegisterPropertyMetaType) {
        qt_static_metacall(this, _c, _id, _a);
        _id -= 9;
    }
    return _id;
}
namespace {
struct qt_meta_stringdata_Penguin__Desktop__MPRIS2PlayerAdaptor_t {
    uint offsetsAndSizes[70];
    char stringdata0[38];
    char stringdata1[16];
    char stringdata2[30];
    char stringdata3[20];
    char stringdata4[1505];
    char stringdata5[7];
    char stringdata6[1];
    char stringdata7[9];
    char stringdata8[5];
    char stringdata9[9];
    char stringdata10[6];
    char stringdata11[10];
    char stringdata12[5];
    char stringdata13[5];
    char stringdata14[5];
    char stringdata15[7];
    char stringdata16[12];
    char stringdata17[16];
    char stringdata18[8];
    char stringdata19[8];
    char stringdata20[4];
    char stringdata21[15];
    char stringdata22[11];
    char stringdata23[5];
    char stringdata24[8];
    char stringdata25[9];
    char stringdata26[7];
    char stringdata27[12];
    char stringdata28[12];
    char stringdata29[10];
    char stringdata30[14];
    char stringdata31[8];
    char stringdata32[9];
    char stringdata33[8];
    char stringdata34[11];
};
#define QT_MOC_LITERAL(ofs, len) \
    uint(sizeof(qt_meta_stringdata_Penguin__Desktop__MPRIS2PlayerAdaptor_t::offsetsAndSizes) + ofs), len 
Q_CONSTINIT static const qt_meta_stringdata_Penguin__Desktop__MPRIS2PlayerAdaptor_t qt_meta_stringdata_Penguin__Desktop__MPRIS2PlayerAdaptor = {
    {
        QT_MOC_LITERAL(0, 37),  // "Penguin::Desktop::MPRIS2Playe..."
        QT_MOC_LITERAL(38, 15),  // "D-Bus Interface"
        QT_MOC_LITERAL(54, 29),  // "org.mpris.MediaPlayer2.Player"
        QT_MOC_LITERAL(84, 19),  // "D-Bus Introspection"
        QT_MOC_LITERAL(104, 1504),  // "  <interface name=\"org.mpris..."
        QT_MOC_LITERAL(1609, 6),  // "Seeked"
        QT_MOC_LITERAL(1616, 0),  // ""
        QT_MOC_LITERAL(1617, 8),  // "Position"
        QT_MOC_LITERAL(1626, 4),  // "Next"
        QT_MOC_LITERAL(1631, 8),  // "Previous"
        QT_MOC_LITERAL(1640, 5),  // "Pause"
        QT_MOC_LITERAL(1646, 9),  // "PlayPause"
        QT_MOC_LITERAL(1656, 4),  // "Stop"
        QT_MOC_LITERAL(1661, 4),  // "Play"
        QT_MOC_LITERAL(1666, 4),  // "Seek"
        QT_MOC_LITERAL(1671, 6),  // "Offset"
        QT_MOC_LITERAL(1678, 11),  // "SetPosition"
        QT_MOC_LITERAL(1690, 15),  // "QDBusObjectPath"
        QT_MOC_LITERAL(1706, 7),  // "TrackId"
        QT_MOC_LITERAL(1714, 7),  // "OpenUri"
        QT_MOC_LITERAL(1722, 3),  // "Uri"
        QT_MOC_LITERAL(1726, 14),  // "PlaybackStatus"
        QT_MOC_LITERAL(1741, 10),  // "LoopStatus"
        QT_MOC_LITERAL(1752, 4),  // "Rate"
        QT_MOC_LITERAL(1757, 7),  // "Shuffle"
        QT_MOC_LITERAL(1765, 8),  // "Metadata"
        QT_MOC_LITERAL(1774, 6),  // "Volume"
        QT_MOC_LITERAL(1781, 11),  // "MinimumRate"
        QT_MOC_LITERAL(1793, 11),  // "MaximumRate"
        QT_MOC_LITERAL(1805, 9),  // "CanGoNext"
        QT_MOC_LITERAL(1815, 13),  // "CanGoPrevious"
        QT_MOC_LITERAL(1829, 7),  // "CanPlay"
        QT_MOC_LITERAL(1837, 8),  // "CanPause"
        QT_MOC_LITERAL(1846, 7),  // "CanSeek"
        QT_MOC_LITERAL(1854, 10)   // "CanControl"
    },
    "Penguin::Desktop::MPRIS2PlayerAdaptor",
    "D-Bus Interface",
    "org.mpris.MediaPlayer2.Player",
    "D-Bus Introspection",
    "  <interface name=\"org.mpris.MediaPlayer2.Player\">\n    <method name"
    "=\"Next\"/>\n    <method name=\"Previous\"/>\n    <method name=\"Pause"
    "\"/>\n    <method name=\"PlayPause\"/>\n    <method name=\"Stop\"/>\n "
    "   <method name=\"Play\"/>\n    <method name=\"Seek\">\n      <arg dir"
    "ection=\"in\" type=\"x\" name=\"Offset\"/>\n    </method>\n    <method"
    " name=\"SetPosition\">\n      <arg direction=\"in\" type=\"o\" name=\""
    "TrackId\"/>\n      <arg direction=\"in\" type=\"x\" name=\"Position\"/"
    ">\n    </method>\n    <method name=\"OpenUri\">\n      <arg direction="
    "\"in\" type=\"s\" name=\"Uri\"/>\n    </method>\n    <signal name=\"Se"
    "eked\">\n      <arg type=\"x\" name=\"Position\"/>\n    </signal>\n   "
    " <property name=\"PlaybackStatus\" type=\"s\" access=\"read\"/>\n    <"
    "property name=\"LoopStatus\" type=\"s\" access=\"readwrite\"/>\n    <p"
    "roperty name=\"Rate\" type=\"d\" access=\"readwrite\"/>\n    <property"
    " name=\"Shuffle\" type=\"b\" access=\"readwrite\"/>\n    <property nam"
    "e=\"Metadata\" type=\"a{sv}\" access=\"read\"/>\n    <property name=\""
    "Volume\" type=\"d\" access=\"readwrite\"/>\n    <property name=\"Posit"
    "ion\" type=\"x\" access=\"read\"/>\n    <property name=\"MinimumRate\""
    " type=\"d\" access=\"read\"/>\n    <property name=\"MaximumRate\" type"
    "=\"d\" access=\"read\"/>\n    <property name=\"CanGoNext\" type=\"b\" "
    "access=\"read\"/>\n    <property name=\"CanGoPrevious\" type=\"b\" acc"
    "ess=\"read\"/>\n    <property name=\"CanPlay\" type=\"b\" access=\"rea"
    "d\"/>\n    <property name=\"CanPause\" type=\"b\" access=\"read\"/>\n "
    "   <property name=\"CanSeek\" type=\"b\" access=\"read\"/>\n    <prope"
    "rty name=\"CanControl\" type=\"b\" access=\"read\"/>\n  </interface>\n",
    "Seeked",
    "",
    "Position",
    "Next",
    "Previous",
    "Pause",
    "PlayPause",
    "Stop",
    "Play",
    "Seek",
    "Offset",
    "SetPosition",
    "QDBusObjectPath",
    "TrackId",
    "OpenUri",
    "Uri",
    "PlaybackStatus",
    "LoopStatus",
    "Rate",
    "Shuffle",
    "Metadata",
    "Volume",
    "MinimumRate",
    "MaximumRate",
    "CanGoNext",
    "CanGoPrevious",
    "CanPlay",
    "CanPause",
    "CanSeek",
    "CanControl"
};
#undef QT_MOC_LITERAL
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_Penguin__Desktop__MPRIS2PlayerAdaptor[] = {

 // content:
      10,       // revision
       0,       // classname
       2,   14, // classinfo
      10,   18, // methods
      15,   98, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // classinfo: key, value
       1,    2,
       3,    4,

 // signals: name, argc, parameters, tag, flags, initial metatype offsets
       5,    1,   78,    6, 0x06,   16 /* Public */,

 // slots: name, argc, parameters, tag, flags, initial metatype offsets
       8,    0,   81,    6, 0x0a,   18 /* Public */,
       9,    0,   82,    6, 0x0a,   19 /* Public */,
      10,    0,   83,    6, 0x0a,   20 /* Public */,
      11,    0,   84,    6, 0x0a,   21 /* Public */,
      12,    0,   85,    6, 0x0a,   22 /* Public */,
      13,    0,   86,    6, 0x0a,   23 /* Public */,
      14,    1,   87,    6, 0x0a,   24 /* Public */,
      16,    2,   90,    6, 0x0a,   26 /* Public */,
      19,    1,   95,    6, 0x0a,   29 /* Public */,

 // signals: parameters
    QMetaType::Void, QMetaType::LongLong,    7,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::LongLong,   15,
    QMetaType::Void, 0x80000000 | 17, QMetaType::LongLong,   18,    7,
    QMetaType::Void, QMetaType::QString,   20,

 // properties: name, type, flags
      21, QMetaType::QString, 0x00015001, uint(-1), 0,
      22, QMetaType::QString, 0x00015103, uint(-1), 0,
      23, QMetaType::Double, 0x00015103, uint(-1), 0,
      24, QMetaType::Bool, 0x00015103, uint(-1), 0,
      25, QMetaType::QVariantMap, 0x00015001, uint(-1), 0,
      26, QMetaType::Double, 0x00015103, uint(-1), 0,
       7, QMetaType::LongLong, 0x00015001, uint(-1), 0,
      27, QMetaType::Double, 0x00015001, uint(-1), 0,
      28, QMetaType::Double, 0x00015001, uint(-1), 0,
      29, QMetaType::Bool, 0x00015001, uint(-1), 0,
      30, QMetaType::Bool, 0x00015001, uint(-1), 0,
      31, QMetaType::Bool, 0x00015001, uint(-1), 0,
      32, QMetaType::Bool, 0x00015001, uint(-1), 0,
      33, QMetaType::Bool, 0x00015001, uint(-1), 0,
      34, QMetaType::Bool, 0x00015001, uint(-1), 0,

       0        // eod
};

Q_CONSTINIT const QMetaObject Penguin::Desktop::MPRIS2PlayerAdaptor::staticMetaObject = { {
    QMetaObject::SuperData::link<QDBusAbstractAdaptor::staticMetaObject>(),
    qt_meta_stringdata_Penguin__Desktop__MPRIS2PlayerAdaptor.offsetsAndSizes,
    qt_meta_data_Penguin__Desktop__MPRIS2PlayerAdaptor,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_stringdata_Penguin__Desktop__MPRIS2PlayerAdaptor_t,
        // property 'PlaybackStatus'
        QtPrivate::TypeAndForceComplete<QString, std::true_type>,
        // property 'LoopStatus'
        QtPrivate::TypeAndForceComplete<QString, std::true_type>,
        // property 'Rate'
        QtPrivate::TypeAndForceComplete<double, std::true_type>,
        // property 'Shuffle'
        QtPrivate::TypeAndForceComplete<bool, std::true_type>,
        // property 'Metadata'
        QtPrivate::TypeAndForceComplete<QVariantMap, std::true_type>,
        // property 'Volume'
        QtPrivate::TypeAndForceComplete<double, std::true_type>,
        // property 'Position'
        QtPrivate::TypeAndForceComplete<qlonglong, std::true_type>,
        // property 'MinimumRate'
        QtPrivate::TypeAndForceComplete<double, std::true_type>,
        // property 'MaximumRate'
        QtPrivate::TypeAndForceComplete<double, std::true_type>,
        // property 'CanGoNext'
        QtPrivate::TypeAndForceComplete<bool, std::true_type>,
        // property 'CanGoPrevious'
        QtPrivate::TypeAndForceComplete<bool, std::true_type>,
        // property 'CanPlay'
        QtPrivate::TypeAndForceComplete<bool, std::true_type>,
        // property 'CanPause'
        QtPrivate::TypeAndForceComplete<bool, std::true_type>,
        // property 'CanSeek'
        QtPrivate::TypeAndForceComplete<bool, std::true_type>,
        // property 'CanControl'
        QtPrivate::TypeAndForceComplete<bool, std::true_type>,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<MPRIS2PlayerAdaptor, std::true_type>,
        // method 'Seeked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<qlonglong, std::false_type>,
        // method 'Next'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'Previous'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'Pause'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'PlayPause'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'Stop'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'Play'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'Seek'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<qlonglong, std::false_type>,
        // method 'SetPosition'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QDBusObjectPath &, std::false_type>,
        QtPrivate::TypeAndForceComplete<qlonglong, std::false_type>,
        // method 'OpenUri'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>
    >,
    nullptr
} };

void Penguin::Desktop::MPRIS2PlayerAdaptor::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<MPRIS2PlayerAdaptor *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->Seeked((*reinterpret_cast< std::add_pointer_t<qlonglong>>(_a[1]))); break;
        case 1: _t->Next(); break;
        case 2: _t->Previous(); break;
        case 3: _t->Pause(); break;
        case 4: _t->PlayPause(); break;
        case 5: _t->Stop(); break;
        case 6: _t->Play(); break;
        case 7: _t->Seek((*reinterpret_cast< std::add_pointer_t<qlonglong>>(_a[1]))); break;
        case 8: _t->SetPosition((*reinterpret_cast< std::add_pointer_t<QDBusObjectPath>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<qlonglong>>(_a[2]))); break;
        case 9: _t->OpenUri((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        switch (_id) {
        default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
        case 8:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
            case 0:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< QDBusObjectPath >(); break;
            }
            break;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (MPRIS2PlayerAdaptor::*)(qlonglong );
            if (_t _q_method = &MPRIS2PlayerAdaptor::Seeked; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 0;
                return;
            }
        }
    }else if (_c == QMetaObject::ReadProperty) {
        auto *_t = static_cast<MPRIS2PlayerAdaptor *>(_o);
        (void)_t;
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast< QString*>(_v) = _t->playbackStatus(); break;
        case 1: *reinterpret_cast< QString*>(_v) = _t->loopStatus(); break;
        case 2: *reinterpret_cast< double*>(_v) = _t->rate(); break;
        case 3: *reinterpret_cast< bool*>(_v) = _t->shuffle(); break;
        case 4: *reinterpret_cast< QVariantMap*>(_v) = _t->metadata(); break;
        case 5: *reinterpret_cast< double*>(_v) = _t->volume(); break;
        case 6: *reinterpret_cast< qlonglong*>(_v) = _t->position(); break;
        case 7: *reinterpret_cast< double*>(_v) = _t->minimumRate(); break;
        case 8: *reinterpret_cast< double*>(_v) = _t->maximumRate(); break;
        case 9: *reinterpret_cast< bool*>(_v) = _t->canGoNext(); break;
        case 10: *reinterpret_cast< bool*>(_v) = _t->canGoPrevious(); break;
        case 11: *reinterpret_cast< bool*>(_v) = _t->canPlay(); break;
        case 12: *reinterpret_cast< bool*>(_v) = _t->canPause(); break;
        case 13: *reinterpret_cast< bool*>(_v) = _t->canSeek(); break;
        case 14: *reinterpret_cast< bool*>(_v) = _t->canControl(); break;
        default: break;
        }
    } else if (_c == QMetaObject::WriteProperty) {
        auto *_t = static_cast<MPRIS2PlayerAdaptor *>(_o);
        (void)_t;
        void *_v = _a[0];
        switch (_id) {
        case 1: _t->setLoopStatus(*reinterpret_cast< QString*>(_v)); break;
        case 2: _t->setRate(*reinterpret_cast< double*>(_v)); break;
        case 3: _t->setShuffle(*reinterpret_cast< bool*>(_v)); break;
        case 5: _t->setVolume(*reinterpret_cast< double*>(_v)); break;
        default: break;
        }
    } else if (_c == QMetaObject::ResetProperty) {
    } else if (_c == QMetaObject::BindableProperty) {
    }
}

const QMetaObject *Penguin::Desktop::MPRIS2PlayerAdaptor::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *Penguin::Desktop::MPRIS2PlayerAdaptor::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_Penguin__Desktop__MPRIS2PlayerAdaptor.stringdata0))
        return static_cast<void*>(this);
    return QDBusAbstractAdaptor::qt_metacast(_clname);
}

int Penguin::Desktop::MPRIS2PlayerAdaptor::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDBusAbstractAdaptor::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 10)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 10;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 10)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 10;
    }else if (_c == QMetaObject::ReadProperty || _c == QMetaObject::WriteProperty
            || _c == QMetaObject::ResetProperty || _c == QMetaObject::BindableProperty
            || _c == QMetaObject::RegisterPropertyMetaType) {
        qt_static_metacall(this, _c, _id, _a);
        _id -= 15;
    }
    return _id;
}

// SIGNAL 0
void Penguin::Desktop::MPRIS2PlayerAdaptor::Seeked(qlonglong _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
namespace {
struct qt_meta_stringdata_Penguin__Desktop__MPRIS2Service_t {
    uint offsetsAndSizes[16];
    char stringdata0[32];
    char stringdata1[21];
    char stringdata2[1];
    char stringdata3[24];
    char stringdata4[24];
    char stringdata5[22];
    char stringdata6[21];
    char stringdata7[18];
};
#define QT_MOC_LITERAL(ofs, len) \
    uint(sizeof(qt_meta_stringdata_Penguin__Desktop__MPRIS2Service_t::offsetsAndSizes) + ofs), len 
Q_CONSTINIT static const qt_meta_stringdata_Penguin__Desktop__MPRIS2Service_t qt_meta_stringdata_Penguin__Desktop__MPRIS2Service = {
    {
        QT_MOC_LITERAL(0, 31),  // "Penguin::Desktop::MPRIS2Service"
        QT_MOC_LITERAL(32, 20),  // "onEngineStateChanged"
        QT_MOC_LITERAL(53, 0),  // ""
        QT_MOC_LITERAL(54, 23),  // "onEngineMetadataChanged"
        QT_MOC_LITERAL(78, 23),  // "onEngineDurationChanged"
        QT_MOC_LITERAL(102, 21),  // "onEngineVolumeChanged"
        QT_MOC_LITERAL(124, 20),  // "onEngineSpeedChanged"
        QT_MOC_LITERAL(145, 17)   // "onPlaylistUpdated"
    },
    "Penguin::Desktop::MPRIS2Service",
    "onEngineStateChanged",
    "",
    "onEngineMetadataChanged",
    "onEngineDurationChanged",
    "onEngineVolumeChanged",
    "onEngineSpeedChanged",
    "onPlaylistUpdated"
};
#undef QT_MOC_LITERAL
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_Penguin__Desktop__MPRIS2Service[] = {

 // content:
      10,       // revision
       0,       // classname
       0,    0, // classinfo
       6,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags, initial metatype offsets
       1,    0,   50,    2, 0x0a,    1 /* Public */,
       3,    0,   51,    2, 0x0a,    2 /* Public */,
       4,    0,   52,    2, 0x0a,    3 /* Public */,
       5,    0,   53,    2, 0x0a,    4 /* Public */,
       6,    0,   54,    2, 0x0a,    5 /* Public */,
       7,    0,   55,    2, 0x0a,    6 /* Public */,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

Q_CONSTINIT const QMetaObject Penguin::Desktop::MPRIS2Service::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_meta_stringdata_Penguin__Desktop__MPRIS2Service.offsetsAndSizes,
    qt_meta_data_Penguin__Desktop__MPRIS2Service,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_stringdata_Penguin__Desktop__MPRIS2Service_t,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<MPRIS2Service, std::true_type>,
        // method 'onEngineStateChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onEngineMetadataChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onEngineDurationChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onEngineVolumeChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onEngineSpeedChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onPlaylistUpdated'
        QtPrivate::TypeAndForceComplete<void, std::false_type>
    >,
    nullptr
} };

void Penguin::Desktop::MPRIS2Service::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<MPRIS2Service *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->onEngineStateChanged(); break;
        case 1: _t->onEngineMetadataChanged(); break;
        case 2: _t->onEngineDurationChanged(); break;
        case 3: _t->onEngineVolumeChanged(); break;
        case 4: _t->onEngineSpeedChanged(); break;
        case 5: _t->onPlaylistUpdated(); break;
        default: ;
        }
    }
    (void)_a;
}

const QMetaObject *Penguin::Desktop::MPRIS2Service::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *Penguin::Desktop::MPRIS2Service::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_Penguin__Desktop__MPRIS2Service.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int Penguin::Desktop::MPRIS2Service::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 6)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 6;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 6)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 6;
    }
    return _id;
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
