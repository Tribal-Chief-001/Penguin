/****************************************************************************
** Meta object code from reading C++ file 'ViewfinderWidget.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.4.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "src/ui/ViewfinderWidget.h"
#include <QtGui/qtextcursor.h>
#include <QtCore/qmetatype.h>
#include <QtCore/QList>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'ViewfinderWidget.h' doesn't include <QObject>."
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
struct qt_meta_stringdata_Penguin__UI__VideoSurfaceWidget_t {
    uint offsetsAndSizes[8];
    char stringdata0[32];
    char stringdata1[8];
    char stringdata2[1];
    char stringdata3[14];
};
#define QT_MOC_LITERAL(ofs, len) \
    uint(sizeof(qt_meta_stringdata_Penguin__UI__VideoSurfaceWidget_t::offsetsAndSizes) + ofs), len 
Q_CONSTINIT static const qt_meta_stringdata_Penguin__UI__VideoSurfaceWidget_t qt_meta_stringdata_Penguin__UI__VideoSurfaceWidget = {
    {
        QT_MOC_LITERAL(0, 31),  // "Penguin::UI::VideoSurfaceWidget"
        QT_MOC_LITERAL(32, 7),  // "clicked"
        QT_MOC_LITERAL(40, 0),  // ""
        QT_MOC_LITERAL(41, 13)   // "doubleClicked"
    },
    "Penguin::UI::VideoSurfaceWidget",
    "clicked",
    "",
    "doubleClicked"
};
#undef QT_MOC_LITERAL
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_Penguin__UI__VideoSurfaceWidget[] = {

 // content:
      10,       // revision
       0,       // classname
       0,    0, // classinfo
       2,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       2,       // signalCount

 // signals: name, argc, parameters, tag, flags, initial metatype offsets
       1,    0,   26,    2, 0x06,    1 /* Public */,
       3,    0,   27,    2, 0x06,    2 /* Public */,

 // signals: parameters
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

Q_CONSTINIT const QMetaObject Penguin::UI::VideoSurfaceWidget::staticMetaObject = { {
    QMetaObject::SuperData::link<QWidget::staticMetaObject>(),
    qt_meta_stringdata_Penguin__UI__VideoSurfaceWidget.offsetsAndSizes,
    qt_meta_data_Penguin__UI__VideoSurfaceWidget,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_stringdata_Penguin__UI__VideoSurfaceWidget_t,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<VideoSurfaceWidget, std::true_type>,
        // method 'clicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'doubleClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>
    >,
    nullptr
} };

void Penguin::UI::VideoSurfaceWidget::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<VideoSurfaceWidget *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->clicked(); break;
        case 1: _t->doubleClicked(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (VideoSurfaceWidget::*)();
            if (_t _q_method = &VideoSurfaceWidget::clicked; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (VideoSurfaceWidget::*)();
            if (_t _q_method = &VideoSurfaceWidget::doubleClicked; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 1;
                return;
            }
        }
    }
    (void)_a;
}

const QMetaObject *Penguin::UI::VideoSurfaceWidget::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *Penguin::UI::VideoSurfaceWidget::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_Penguin__UI__VideoSurfaceWidget.stringdata0))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int Penguin::UI::VideoSurfaceWidget::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
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
    }
    return _id;
}

// SIGNAL 0
void Penguin::UI::VideoSurfaceWidget::clicked()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}

// SIGNAL 1
void Penguin::UI::VideoSurfaceWidget::doubleClicked()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}
namespace {
struct qt_meta_stringdata_Penguin__UI__ViewfinderWidget_t {
    uint offsetsAndSizes[84];
    char stringdata0[30];
    char stringdata1[20];
    char stringdata2[1];
    char stringdata3[26];
    char stringdata4[10];
    char stringdata5[15];
    char stringdata6[14];
    char stringdata7[8];
    char stringdata8[19];
    char stringdata9[19];
    char stringdata10[19];
    char stringdata11[14];
    char stringdata12[22];
    char stringdata13[21];
    char stringdata14[22];
    char stringdata15[21];
    char stringdata16[15];
    char stringdata17[9];
    char stringdata18[22];
    char stringdata19[6];
    char stringdata20[14];
    char stringdata21[20];
    char stringdata22[6];
    char stringdata23[23];
    char stringdata24[29];
    char stringdata25[20];
    char stringdata26[6];
    char stringdata27[24];
    char stringdata28[6];
    char stringdata29[6];
    char stringdata30[24];
    char stringdata31[6];
    char stringdata32[22];
    char stringdata33[23];
    char stringdata34[12];
    char stringdata35[10];
    char stringdata36[25];
    char stringdata37[22];
    char stringdata38[5];
    char stringdata39[24];
    char stringdata40[20];
    char stringdata41[5];
};
#define QT_MOC_LITERAL(ofs, len) \
    uint(sizeof(qt_meta_stringdata_Penguin__UI__ViewfinderWidget_t::offsetsAndSizes) + ofs), len 
Q_CONSTINIT static const qt_meta_stringdata_Penguin__UI__ViewfinderWidget_t qt_meta_stringdata_Penguin__UI__ViewfinderWidget = {
    {
        QT_MOC_LITERAL(0, 29),  // "Penguin::UI::ViewfinderWidget"
        QT_MOC_LITERAL(30, 19),  // "switchModeRequested"
        QT_MOC_LITERAL(50, 0),  // ""
        QT_MOC_LITERAL(51, 25),  // "fullscreenToggleRequested"
        QT_MOC_LITERAL(77, 9),  // "toggleOsd"
        QT_MOC_LITERAL(87, 14),  // "toggleReticles"
        QT_MOC_LITERAL(102, 13),  // "setOsdVisible"
        QT_MOC_LITERAL(116, 7),  // "visible"
        QT_MOC_LITERAL(124, 18),  // "setReticlesVisible"
        QT_MOC_LITERAL(143, 18),  // "updateUIFromEngine"
        QT_MOC_LITERAL(162, 18),  // "onPlayPauseClicked"
        QT_MOC_LITERAL(181, 13),  // "onStopClicked"
        QT_MOC_LITERAL(195, 21),  // "onStepBackwardClicked"
        QT_MOC_LITERAL(217, 20),  // "onStepForwardClicked"
        QT_MOC_LITERAL(238, 21),  // "onJumpBackwardClicked"
        QT_MOC_LITERAL(260, 20),  // "onJumpForwardClicked"
        QT_MOC_LITERAL(281, 14),  // "onSpeedChanged"
        QT_MOC_LITERAL(296, 8),  // "speedStr"
        QT_MOC_LITERAL(305, 21),  // "onVolumeSliderChanged"
        QT_MOC_LITERAL(327, 5),  // "value"
        QT_MOC_LITERAL(333, 13),  // "onMuteClicked"
        QT_MOC_LITERAL(347, 19),  // "onAudioTrackChanged"
        QT_MOC_LITERAL(367, 5),  // "index"
        QT_MOC_LITERAL(373, 22),  // "onSubtitleTrackChanged"
        QT_MOC_LITERAL(396, 28),  // "onEnginePlaybackStateChanged"
        QT_MOC_LITERAL(425, 19),  // "Core::PlaybackState"
        QT_MOC_LITERAL(445, 5),  // "state"
        QT_MOC_LITERAL(451, 23),  // "onEnginePositionChanged"
        QT_MOC_LITERAL(475, 5),  // "posMs"
        QT_MOC_LITERAL(481, 5),  // "smpte"
        QT_MOC_LITERAL(487, 23),  // "onEngineDurationChanged"
        QT_MOC_LITERAL(511, 5),  // "durMs"
        QT_MOC_LITERAL(517, 21),  // "onEngineTracksChanged"
        QT_MOC_LITERAL(539, 22),  // "QList<Core::TrackInfo>"
        QT_MOC_LITERAL(562, 11),  // "audioTracks"
        QT_MOC_LITERAL(574, 9),  // "subTracks"
        QT_MOC_LITERAL(584, 24),  // "onEngineTelemetryUpdated"
        QT_MOC_LITERAL(609, 21),  // "Core::DiagnosticsData"
        QT_MOC_LITERAL(631, 4),  // "diag"
        QT_MOC_LITERAL(636, 23),  // "onEngineMetadataChanged"
        QT_MOC_LITERAL(660, 19),  // "Core::MediaMetadata"
        QT_MOC_LITERAL(680, 4)   // "meta"
    },
    "Penguin::UI::ViewfinderWidget",
    "switchModeRequested",
    "",
    "fullscreenToggleRequested",
    "toggleOsd",
    "toggleReticles",
    "setOsdVisible",
    "visible",
    "setReticlesVisible",
    "updateUIFromEngine",
    "onPlayPauseClicked",
    "onStopClicked",
    "onStepBackwardClicked",
    "onStepForwardClicked",
    "onJumpBackwardClicked",
    "onJumpForwardClicked",
    "onSpeedChanged",
    "speedStr",
    "onVolumeSliderChanged",
    "value",
    "onMuteClicked",
    "onAudioTrackChanged",
    "index",
    "onSubtitleTrackChanged",
    "onEnginePlaybackStateChanged",
    "Core::PlaybackState",
    "state",
    "onEnginePositionChanged",
    "posMs",
    "smpte",
    "onEngineDurationChanged",
    "durMs",
    "onEngineTracksChanged",
    "QList<Core::TrackInfo>",
    "audioTracks",
    "subTracks",
    "onEngineTelemetryUpdated",
    "Core::DiagnosticsData",
    "diag",
    "onEngineMetadataChanged",
    "Core::MediaMetadata",
    "meta"
};
#undef QT_MOC_LITERAL
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_Penguin__UI__ViewfinderWidget[] = {

 // content:
      10,       // revision
       0,       // classname
       0,    0, // classinfo
      24,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       2,       // signalCount

 // signals: name, argc, parameters, tag, flags, initial metatype offsets
       1,    0,  158,    2, 0x06,    1 /* Public */,
       3,    0,  159,    2, 0x06,    2 /* Public */,

 // slots: name, argc, parameters, tag, flags, initial metatype offsets
       4,    0,  160,    2, 0x0a,    3 /* Public */,
       5,    0,  161,    2, 0x0a,    4 /* Public */,
       6,    1,  162,    2, 0x0a,    5 /* Public */,
       8,    1,  165,    2, 0x0a,    7 /* Public */,
       9,    0,  168,    2, 0x0a,    9 /* Public */,
      10,    0,  169,    2, 0x08,   10 /* Private */,
      11,    0,  170,    2, 0x08,   11 /* Private */,
      12,    0,  171,    2, 0x08,   12 /* Private */,
      13,    0,  172,    2, 0x08,   13 /* Private */,
      14,    0,  173,    2, 0x08,   14 /* Private */,
      15,    0,  174,    2, 0x08,   15 /* Private */,
      16,    1,  175,    2, 0x08,   16 /* Private */,
      18,    1,  178,    2, 0x08,   18 /* Private */,
      20,    0,  181,    2, 0x08,   20 /* Private */,
      21,    1,  182,    2, 0x08,   21 /* Private */,
      23,    1,  185,    2, 0x08,   23 /* Private */,
      24,    1,  188,    2, 0x08,   25 /* Private */,
      27,    2,  191,    2, 0x08,   27 /* Private */,
      30,    2,  196,    2, 0x08,   30 /* Private */,
      32,    2,  201,    2, 0x08,   33 /* Private */,
      36,    1,  206,    2, 0x08,   36 /* Private */,
      39,    1,  209,    2, 0x08,   38 /* Private */,

 // signals: parameters
    QMetaType::Void,
    QMetaType::Void,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Bool,    7,
    QMetaType::Void, QMetaType::Bool,    7,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString,   17,
    QMetaType::Void, QMetaType::Int,   19,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int,   22,
    QMetaType::Void, QMetaType::Int,   22,
    QMetaType::Void, 0x80000000 | 25,   26,
    QMetaType::Void, QMetaType::LongLong, QMetaType::QString,   28,   29,
    QMetaType::Void, QMetaType::LongLong, QMetaType::QString,   31,   29,
    QMetaType::Void, 0x80000000 | 33, 0x80000000 | 33,   34,   35,
    QMetaType::Void, 0x80000000 | 37,   38,
    QMetaType::Void, 0x80000000 | 40,   41,

       0        // eod
};

Q_CONSTINIT const QMetaObject Penguin::UI::ViewfinderWidget::staticMetaObject = { {
    QMetaObject::SuperData::link<QWidget::staticMetaObject>(),
    qt_meta_stringdata_Penguin__UI__ViewfinderWidget.offsetsAndSizes,
    qt_meta_data_Penguin__UI__ViewfinderWidget,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_stringdata_Penguin__UI__ViewfinderWidget_t,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<ViewfinderWidget, std::true_type>,
        // method 'switchModeRequested'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'fullscreenToggleRequested'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'toggleOsd'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'toggleReticles'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'setOsdVisible'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<bool, std::false_type>,
        // method 'setReticlesVisible'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<bool, std::false_type>,
        // method 'updateUIFromEngine'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onPlayPauseClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onStopClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onStepBackwardClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onStepForwardClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onJumpBackwardClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onJumpForwardClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onSpeedChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'onVolumeSliderChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'onMuteClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onAudioTrackChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'onSubtitleTrackChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'onEnginePlaybackStateChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<Core::PlaybackState, std::false_type>,
        // method 'onEnginePositionChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<qint64, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'onEngineDurationChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<qint64, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'onEngineTracksChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QList<Core::TrackInfo> &, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QList<Core::TrackInfo> &, std::false_type>,
        // method 'onEngineTelemetryUpdated'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const Core::DiagnosticsData &, std::false_type>,
        // method 'onEngineMetadataChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const Core::MediaMetadata &, std::false_type>
    >,
    nullptr
} };

void Penguin::UI::ViewfinderWidget::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<ViewfinderWidget *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->switchModeRequested(); break;
        case 1: _t->fullscreenToggleRequested(); break;
        case 2: _t->toggleOsd(); break;
        case 3: _t->toggleReticles(); break;
        case 4: _t->setOsdVisible((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1]))); break;
        case 5: _t->setReticlesVisible((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1]))); break;
        case 6: _t->updateUIFromEngine(); break;
        case 7: _t->onPlayPauseClicked(); break;
        case 8: _t->onStopClicked(); break;
        case 9: _t->onStepBackwardClicked(); break;
        case 10: _t->onStepForwardClicked(); break;
        case 11: _t->onJumpBackwardClicked(); break;
        case 12: _t->onJumpForwardClicked(); break;
        case 13: _t->onSpeedChanged((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 14: _t->onVolumeSliderChanged((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 15: _t->onMuteClicked(); break;
        case 16: _t->onAudioTrackChanged((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 17: _t->onSubtitleTrackChanged((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 18: _t->onEnginePlaybackStateChanged((*reinterpret_cast< std::add_pointer_t<Core::PlaybackState>>(_a[1]))); break;
        case 19: _t->onEnginePositionChanged((*reinterpret_cast< std::add_pointer_t<qint64>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QString>>(_a[2]))); break;
        case 20: _t->onEngineDurationChanged((*reinterpret_cast< std::add_pointer_t<qint64>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QString>>(_a[2]))); break;
        case 21: _t->onEngineTracksChanged((*reinterpret_cast< std::add_pointer_t<QList<Core::TrackInfo>>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QList<Core::TrackInfo>>>(_a[2]))); break;
        case 22: _t->onEngineTelemetryUpdated((*reinterpret_cast< std::add_pointer_t<Core::DiagnosticsData>>(_a[1]))); break;
        case 23: _t->onEngineMetadataChanged((*reinterpret_cast< std::add_pointer_t<Core::MediaMetadata>>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (ViewfinderWidget::*)();
            if (_t _q_method = &ViewfinderWidget::switchModeRequested; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (ViewfinderWidget::*)();
            if (_t _q_method = &ViewfinderWidget::fullscreenToggleRequested; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 1;
                return;
            }
        }
    }
}

const QMetaObject *Penguin::UI::ViewfinderWidget::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *Penguin::UI::ViewfinderWidget::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_Penguin__UI__ViewfinderWidget.stringdata0))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int Penguin::UI::ViewfinderWidget::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 24)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 24;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 24)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 24;
    }
    return _id;
}

// SIGNAL 0
void Penguin::UI::ViewfinderWidget::switchModeRequested()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}

// SIGNAL 1
void Penguin::UI::ViewfinderWidget::fullscreenToggleRequested()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
