/****************************************************************************
** Meta object code from reading C++ file 'AudioDeckWidget.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.4.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "src/ui/AudioDeckWidget.h"
#include <QtGui/qtextcursor.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'AudioDeckWidget.h' doesn't include <QObject>."
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
struct qt_meta_stringdata_Penguin__UI__AudioDeckWidget_t {
    uint offsetsAndSizes[78];
    char stringdata0[29];
    char stringdata1[20];
    char stringdata2[1];
    char stringdata3[19];
    char stringdata4[17];
    char stringdata5[6];
    char stringdata6[10];
    char stringdata7[14];
    char stringdata8[14];
    char stringdata9[16];
    char stringdata10[19];
    char stringdata11[22];
    char stringdata12[21];
    char stringdata13[22];
    char stringdata14[6];
    char stringdata15[14];
    char stringdata16[29];
    char stringdata17[20];
    char stringdata18[6];
    char stringdata19[24];
    char stringdata20[6];
    char stringdata21[6];
    char stringdata22[24];
    char stringdata23[6];
    char stringdata24[24];
    char stringdata25[20];
    char stringdata26[5];
    char stringdata27[24];
    char stringdata28[6];
    char stringdata29[6];
    char stringdata30[5];
    char stringdata31[5];
    char stringdata32[27];
    char stringdata33[9];
    char stringdata34[10];
    char stringdata35[22];
    char stringdata36[29];
    char stringdata37[13];
    char stringdata38[5];
};
#define QT_MOC_LITERAL(ofs, len) \
    uint(sizeof(qt_meta_stringdata_Penguin__UI__AudioDeckWidget_t::offsetsAndSizes) + ofs), len 
Q_CONSTINIT static const qt_meta_stringdata_Penguin__UI__AudioDeckWidget_t qt_meta_stringdata_Penguin__UI__AudioDeckWidget = {
    {
        QT_MOC_LITERAL(0, 28),  // "Penguin::UI::AudioDeckWidget"
        QT_MOC_LITERAL(29, 19),  // "switchModeRequested"
        QT_MOC_LITERAL(49, 0),  // ""
        QT_MOC_LITERAL(50, 18),  // "updateUIFromEngine"
        QT_MOC_LITERAL(69, 16),  // "playTrackAtIndex"
        QT_MOC_LITERAL(86, 5),  // "index"
        QT_MOC_LITERAL(92, 9),  // "nextTrack"
        QT_MOC_LITERAL(102, 13),  // "previousTrack"
        QT_MOC_LITERAL(116, 13),  // "toggleShuffle"
        QT_MOC_LITERAL(130, 15),  // "cycleRepeatMode"
        QT_MOC_LITERAL(146, 18),  // "onPlayPauseClicked"
        QT_MOC_LITERAL(165, 21),  // "onJumpBackwardClicked"
        QT_MOC_LITERAL(187, 20),  // "onJumpForwardClicked"
        QT_MOC_LITERAL(208, 21),  // "onVolumeSliderChanged"
        QT_MOC_LITERAL(230, 5),  // "value"
        QT_MOC_LITERAL(236, 13),  // "onMuteClicked"
        QT_MOC_LITERAL(250, 28),  // "onEnginePlaybackStateChanged"
        QT_MOC_LITERAL(279, 19),  // "Core::PlaybackState"
        QT_MOC_LITERAL(299, 5),  // "state"
        QT_MOC_LITERAL(305, 23),  // "onEnginePositionChanged"
        QT_MOC_LITERAL(329, 5),  // "posMs"
        QT_MOC_LITERAL(335, 5),  // "smpte"
        QT_MOC_LITERAL(341, 23),  // "onEngineDurationChanged"
        QT_MOC_LITERAL(365, 5),  // "durMs"
        QT_MOC_LITERAL(371, 23),  // "onEngineMetadataChanged"
        QT_MOC_LITERAL(395, 19),  // "Core::MediaMetadata"
        QT_MOC_LITERAL(415, 4),  // "meta"
        QT_MOC_LITERAL(420, 23),  // "onEngineVuLevelsChanged"
        QT_MOC_LITERAL(444, 5),  // "lPeak"
        QT_MOC_LITERAL(450, 5),  // "rPeak"
        QT_MOC_LITERAL(456, 4),  // "lRms"
        QT_MOC_LITERAL(461, 4),  // "rRms"
        QT_MOC_LITERAL(466, 26),  // "onEngineActiveLyricChanged"
        QT_MOC_LITERAL(493, 8),  // "cueIndex"
        QT_MOC_LITERAL(502, 9),  // "lyricText"
        QT_MOC_LITERAL(512, 21),  // "onEngineMediaFinished"
        QT_MOC_LITERAL(534, 28),  // "onPlaylistTrackDoubleClicked"
        QT_MOC_LITERAL(563, 12),  // "PlaylistItem"
        QT_MOC_LITERAL(576, 4)   // "item"
    },
    "Penguin::UI::AudioDeckWidget",
    "switchModeRequested",
    "",
    "updateUIFromEngine",
    "playTrackAtIndex",
    "index",
    "nextTrack",
    "previousTrack",
    "toggleShuffle",
    "cycleRepeatMode",
    "onPlayPauseClicked",
    "onJumpBackwardClicked",
    "onJumpForwardClicked",
    "onVolumeSliderChanged",
    "value",
    "onMuteClicked",
    "onEnginePlaybackStateChanged",
    "Core::PlaybackState",
    "state",
    "onEnginePositionChanged",
    "posMs",
    "smpte",
    "onEngineDurationChanged",
    "durMs",
    "onEngineMetadataChanged",
    "Core::MediaMetadata",
    "meta",
    "onEngineVuLevelsChanged",
    "lPeak",
    "rPeak",
    "lRms",
    "rRms",
    "onEngineActiveLyricChanged",
    "cueIndex",
    "lyricText",
    "onEngineMediaFinished",
    "onPlaylistTrackDoubleClicked",
    "PlaylistItem",
    "item"
};
#undef QT_MOC_LITERAL
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_Penguin__UI__AudioDeckWidget[] = {

 // content:
      10,       // revision
       0,       // classname
       0,    0, // classinfo
      20,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: name, argc, parameters, tag, flags, initial metatype offsets
       1,    0,  134,    2, 0x06,    1 /* Public */,

 // slots: name, argc, parameters, tag, flags, initial metatype offsets
       3,    0,  135,    2, 0x0a,    2 /* Public */,
       4,    1,  136,    2, 0x0a,    3 /* Public */,
       6,    0,  139,    2, 0x0a,    5 /* Public */,
       7,    0,  140,    2, 0x0a,    6 /* Public */,
       8,    0,  141,    2, 0x0a,    7 /* Public */,
       9,    0,  142,    2, 0x0a,    8 /* Public */,
      10,    0,  143,    2, 0x08,    9 /* Private */,
      11,    0,  144,    2, 0x08,   10 /* Private */,
      12,    0,  145,    2, 0x08,   11 /* Private */,
      13,    1,  146,    2, 0x08,   12 /* Private */,
      15,    0,  149,    2, 0x08,   14 /* Private */,
      16,    1,  150,    2, 0x08,   15 /* Private */,
      19,    2,  153,    2, 0x08,   17 /* Private */,
      22,    2,  158,    2, 0x08,   20 /* Private */,
      24,    1,  163,    2, 0x08,   23 /* Private */,
      27,    4,  166,    2, 0x08,   25 /* Private */,
      32,    2,  175,    2, 0x08,   30 /* Private */,
      35,    0,  180,    2, 0x08,   33 /* Private */,
      36,    2,  181,    2, 0x08,   34 /* Private */,

 // signals: parameters
    QMetaType::Void,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int,    5,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int,   14,
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 17,   18,
    QMetaType::Void, QMetaType::LongLong, QMetaType::QString,   20,   21,
    QMetaType::Void, QMetaType::LongLong, QMetaType::QString,   23,   21,
    QMetaType::Void, 0x80000000 | 25,   26,
    QMetaType::Void, QMetaType::Double, QMetaType::Double, QMetaType::Double, QMetaType::Double,   28,   29,   30,   31,
    QMetaType::Void, QMetaType::Int, QMetaType::QString,   33,   34,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int, 0x80000000 | 37,    5,   38,

       0        // eod
};

Q_CONSTINIT const QMetaObject Penguin::UI::AudioDeckWidget::staticMetaObject = { {
    QMetaObject::SuperData::link<QWidget::staticMetaObject>(),
    qt_meta_stringdata_Penguin__UI__AudioDeckWidget.offsetsAndSizes,
    qt_meta_data_Penguin__UI__AudioDeckWidget,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_stringdata_Penguin__UI__AudioDeckWidget_t,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<AudioDeckWidget, std::true_type>,
        // method 'switchModeRequested'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'updateUIFromEngine'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'playTrackAtIndex'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'nextTrack'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'previousTrack'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'toggleShuffle'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'cycleRepeatMode'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onPlayPauseClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onJumpBackwardClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onJumpForwardClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onVolumeSliderChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'onMuteClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
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
        // method 'onEngineMetadataChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const Core::MediaMetadata &, std::false_type>,
        // method 'onEngineVuLevelsChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<double, std::false_type>,
        QtPrivate::TypeAndForceComplete<double, std::false_type>,
        QtPrivate::TypeAndForceComplete<double, std::false_type>,
        QtPrivate::TypeAndForceComplete<double, std::false_type>,
        // method 'onEngineActiveLyricChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'onEngineMediaFinished'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onPlaylistTrackDoubleClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        QtPrivate::TypeAndForceComplete<const PlaylistItem &, std::false_type>
    >,
    nullptr
} };

void Penguin::UI::AudioDeckWidget::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<AudioDeckWidget *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->switchModeRequested(); break;
        case 1: _t->updateUIFromEngine(); break;
        case 2: _t->playTrackAtIndex((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 3: _t->nextTrack(); break;
        case 4: _t->previousTrack(); break;
        case 5: _t->toggleShuffle(); break;
        case 6: _t->cycleRepeatMode(); break;
        case 7: _t->onPlayPauseClicked(); break;
        case 8: _t->onJumpBackwardClicked(); break;
        case 9: _t->onJumpForwardClicked(); break;
        case 10: _t->onVolumeSliderChanged((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 11: _t->onMuteClicked(); break;
        case 12: _t->onEnginePlaybackStateChanged((*reinterpret_cast< std::add_pointer_t<Core::PlaybackState>>(_a[1]))); break;
        case 13: _t->onEnginePositionChanged((*reinterpret_cast< std::add_pointer_t<qint64>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QString>>(_a[2]))); break;
        case 14: _t->onEngineDurationChanged((*reinterpret_cast< std::add_pointer_t<qint64>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QString>>(_a[2]))); break;
        case 15: _t->onEngineMetadataChanged((*reinterpret_cast< std::add_pointer_t<Core::MediaMetadata>>(_a[1]))); break;
        case 16: _t->onEngineVuLevelsChanged((*reinterpret_cast< std::add_pointer_t<double>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<double>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<double>>(_a[3])),(*reinterpret_cast< std::add_pointer_t<double>>(_a[4]))); break;
        case 17: _t->onEngineActiveLyricChanged((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QString>>(_a[2]))); break;
        case 18: _t->onEngineMediaFinished(); break;
        case 19: _t->onPlaylistTrackDoubleClicked((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<PlaylistItem>>(_a[2]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (AudioDeckWidget::*)();
            if (_t _q_method = &AudioDeckWidget::switchModeRequested; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 0;
                return;
            }
        }
    }
}

const QMetaObject *Penguin::UI::AudioDeckWidget::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *Penguin::UI::AudioDeckWidget::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_Penguin__UI__AudioDeckWidget.stringdata0))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int Penguin::UI::AudioDeckWidget::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 20)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 20;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 20)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 20;
    }
    return _id;
}

// SIGNAL 0
void Penguin::UI::AudioDeckWidget::switchModeRequested()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
