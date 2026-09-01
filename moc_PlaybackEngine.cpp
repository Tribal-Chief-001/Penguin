/****************************************************************************
** Meta object code from reading C++ file 'PlaybackEngine.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.4.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "src/core/PlaybackEngine.h"
#include <QtCore/qmetatype.h>
#include <QtCore/QList>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'PlaybackEngine.h' doesn't include <QObject>."
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
struct qt_meta_stringdata_Penguin__Core__PlaybackEngine_t {
    uint offsetsAndSizes[110];
    char stringdata0[30];
    char stringdata1[21];
    char stringdata2[1];
    char stringdata3[14];
    char stringdata4[6];
    char stringdata5[16];
    char stringdata6[11];
    char stringdata7[14];
    char stringdata8[16];
    char stringdata9[11];
    char stringdata10[13];
    char stringdata11[6];
    char stringdata12[14];
    char stringdata13[7];
    char stringdata14[12];
    char stringdata15[6];
    char stringdata16[14];
    char stringdata17[17];
    char stringdata18[12];
    char stringdata19[10];
    char stringdata20[16];
    char stringdata21[14];
    char stringdata22[9];
    char stringdata23[17];
    char stringdata24[16];
    char stringdata25[10];
    char stringdata26[16];
    char stringdata27[11];
    char stringdata28[12];
    char stringdata29[10];
    char stringdata30[11];
    char stringdata31[17];
    char stringdata32[13];
    char stringdata33[3];
    char stringdata34[19];
    char stringdata35[9];
    char stringdata36[10];
    char stringdata37[22];
    char stringdata38[13];
    char stringdata39[12];
    char stringdata40[4];
    char stringdata41[14];
    char stringdata42[14];
    char stringdata43[13];
    char stringdata44[22];
    char stringdata45[12];
    char stringdata46[25];
    char stringdata47[25];
    char stringdata48[25];
    char stringdata49[5];
    char stringdata50[23];
    char stringdata51[28];
    char stringdata52[5];
    char stringdata53[19];
    char stringdata54[14];
};
#define QT_MOC_LITERAL(ofs, len) \
    uint(sizeof(qt_meta_stringdata_Penguin__Core__PlaybackEngine_t::offsetsAndSizes) + ofs), len 
Q_CONSTINIT static const qt_meta_stringdata_Penguin__Core__PlaybackEngine_t qt_meta_stringdata_Penguin__Core__PlaybackEngine = {
    {
        QT_MOC_LITERAL(0, 29),  // "Penguin::Core::PlaybackEngine"
        QT_MOC_LITERAL(30, 20),  // "playbackStateChanged"
        QT_MOC_LITERAL(51, 0),  // ""
        QT_MOC_LITERAL(52, 13),  // "PlaybackState"
        QT_MOC_LITERAL(66, 5),  // "state"
        QT_MOC_LITERAL(72, 15),  // "positionChanged"
        QT_MOC_LITERAL(88, 10),  // "positionMs"
        QT_MOC_LITERAL(99, 13),  // "smpteTimecode"
        QT_MOC_LITERAL(113, 15),  // "durationChanged"
        QT_MOC_LITERAL(129, 10),  // "durationMs"
        QT_MOC_LITERAL(140, 12),  // "speedChanged"
        QT_MOC_LITERAL(153, 5),  // "speed"
        QT_MOC_LITERAL(159, 13),  // "volumeChanged"
        QT_MOC_LITERAL(173, 6),  // "volume"
        QT_MOC_LITERAL(180, 11),  // "muteChanged"
        QT_MOC_LITERAL(192, 5),  // "muted"
        QT_MOC_LITERAL(198, 13),  // "tracksChanged"
        QT_MOC_LITERAL(212, 16),  // "QList<TrackInfo>"
        QT_MOC_LITERAL(229, 11),  // "audioTracks"
        QT_MOC_LITERAL(241, 9),  // "subTracks"
        QT_MOC_LITERAL(251, 15),  // "metadataChanged"
        QT_MOC_LITERAL(267, 13),  // "MediaMetadata"
        QT_MOC_LITERAL(281, 8),  // "metadata"
        QT_MOC_LITERAL(290, 16),  // "telemetryUpdated"
        QT_MOC_LITERAL(307, 15),  // "DiagnosticsData"
        QT_MOC_LITERAL(323, 9),  // "telemetry"
        QT_MOC_LITERAL(333, 15),  // "vuLevelsChanged"
        QT_MOC_LITERAL(349, 10),  // "leftPeakDb"
        QT_MOC_LITERAL(360, 11),  // "rightPeakDb"
        QT_MOC_LITERAL(372, 9),  // "leftRmsDb"
        QT_MOC_LITERAL(382, 10),  // "rightRmsDb"
        QT_MOC_LITERAL(393, 16),  // "equalizerChanged"
        QT_MOC_LITERAL(410, 12),  // "EqualizerDSP"
        QT_MOC_LITERAL(423, 2),  // "eq"
        QT_MOC_LITERAL(426, 18),  // "activeLyricChanged"
        QT_MOC_LITERAL(445, 8),  // "cueIndex"
        QT_MOC_LITERAL(454, 9),  // "lyricText"
        QT_MOC_LITERAL(464, 21),  // "activeSubtitleChanged"
        QT_MOC_LITERAL(486, 12),  // "subtitleText"
        QT_MOC_LITERAL(499, 11),  // "mediaLoaded"
        QT_MOC_LITERAL(511, 3),  // "uri"
        QT_MOC_LITERAL(515, 13),  // "mediaFinished"
        QT_MOC_LITERAL(529, 13),  // "errorOccurred"
        QT_MOC_LITERAL(543, 12),  // "errorMessage"
        QT_MOC_LITERAL(556, 21),  // "onBackendStateChanged"
        QT_MOC_LITERAL(578, 11),  // "EngineState"
        QT_MOC_LITERAL(590, 24),  // "onBackendPositionChanged"
        QT_MOC_LITERAL(615, 24),  // "onBackendDurationChanged"
        QT_MOC_LITERAL(640, 24),  // "onBackendMetadataChanged"
        QT_MOC_LITERAL(665, 4),  // "meta"
        QT_MOC_LITERAL(670, 22),  // "onBackendTracksChanged"
        QT_MOC_LITERAL(693, 27),  // "onBackendDiagnosticsChanged"
        QT_MOC_LITERAL(721, 4),  // "diag"
        QT_MOC_LITERAL(726, 18),  // "onBackendEndOfFile"
        QT_MOC_LITERAL(745, 13)   // "onVuTimerTick"
    },
    "Penguin::Core::PlaybackEngine",
    "playbackStateChanged",
    "",
    "PlaybackState",
    "state",
    "positionChanged",
    "positionMs",
    "smpteTimecode",
    "durationChanged",
    "durationMs",
    "speedChanged",
    "speed",
    "volumeChanged",
    "volume",
    "muteChanged",
    "muted",
    "tracksChanged",
    "QList<TrackInfo>",
    "audioTracks",
    "subTracks",
    "metadataChanged",
    "MediaMetadata",
    "metadata",
    "telemetryUpdated",
    "DiagnosticsData",
    "telemetry",
    "vuLevelsChanged",
    "leftPeakDb",
    "rightPeakDb",
    "leftRmsDb",
    "rightRmsDb",
    "equalizerChanged",
    "EqualizerDSP",
    "eq",
    "activeLyricChanged",
    "cueIndex",
    "lyricText",
    "activeSubtitleChanged",
    "subtitleText",
    "mediaLoaded",
    "uri",
    "mediaFinished",
    "errorOccurred",
    "errorMessage",
    "onBackendStateChanged",
    "EngineState",
    "onBackendPositionChanged",
    "onBackendDurationChanged",
    "onBackendMetadataChanged",
    "meta",
    "onBackendTracksChanged",
    "onBackendDiagnosticsChanged",
    "diag",
    "onBackendEndOfFile",
    "onVuTimerTick"
};
#undef QT_MOC_LITERAL
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_Penguin__Core__PlaybackEngine[] = {

 // content:
      10,       // revision
       0,       // classname
       0,    0, // classinfo
      24,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
      16,       // signalCount

 // signals: name, argc, parameters, tag, flags, initial metatype offsets
       1,    1,  158,    2, 0x06,    1 /* Public */,
       5,    2,  161,    2, 0x06,    3 /* Public */,
       8,    2,  166,    2, 0x06,    6 /* Public */,
      10,    1,  171,    2, 0x06,    9 /* Public */,
      12,    1,  174,    2, 0x06,   11 /* Public */,
      14,    1,  177,    2, 0x06,   13 /* Public */,
      16,    2,  180,    2, 0x06,   15 /* Public */,
      20,    1,  185,    2, 0x06,   18 /* Public */,
      23,    1,  188,    2, 0x06,   20 /* Public */,
      26,    4,  191,    2, 0x06,   22 /* Public */,
      31,    1,  200,    2, 0x06,   27 /* Public */,
      34,    2,  203,    2, 0x06,   29 /* Public */,
      37,    1,  208,    2, 0x06,   32 /* Public */,
      39,    1,  211,    2, 0x06,   34 /* Public */,
      41,    0,  214,    2, 0x06,   36 /* Public */,
      42,    1,  215,    2, 0x06,   37 /* Public */,

 // slots: name, argc, parameters, tag, flags, initial metatype offsets
      44,    1,  218,    2, 0x08,   39 /* Private */,
      46,    1,  221,    2, 0x08,   41 /* Private */,
      47,    1,  224,    2, 0x08,   43 /* Private */,
      48,    1,  227,    2, 0x08,   45 /* Private */,
      50,    2,  230,    2, 0x08,   47 /* Private */,
      51,    1,  235,    2, 0x08,   50 /* Private */,
      53,    0,  238,    2, 0x08,   52 /* Private */,
      54,    0,  239,    2, 0x08,   53 /* Private */,

 // signals: parameters
    QMetaType::Void, 0x80000000 | 3,    4,
    QMetaType::Void, QMetaType::LongLong, QMetaType::QString,    6,    7,
    QMetaType::Void, QMetaType::LongLong, QMetaType::QString,    9,    7,
    QMetaType::Void, QMetaType::Double,   11,
    QMetaType::Void, QMetaType::Int,   13,
    QMetaType::Void, QMetaType::Bool,   15,
    QMetaType::Void, 0x80000000 | 17, 0x80000000 | 17,   18,   19,
    QMetaType::Void, 0x80000000 | 21,   22,
    QMetaType::Void, 0x80000000 | 24,   25,
    QMetaType::Void, QMetaType::Double, QMetaType::Double, QMetaType::Double, QMetaType::Double,   27,   28,   29,   30,
    QMetaType::Void, 0x80000000 | 32,   33,
    QMetaType::Void, QMetaType::Int, QMetaType::QString,   35,   36,
    QMetaType::Void, QMetaType::QString,   38,
    QMetaType::Void, QMetaType::QString,   40,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString,   43,

 // slots: parameters
    QMetaType::Void, 0x80000000 | 45,    4,
    QMetaType::Void, QMetaType::LongLong,    6,
    QMetaType::Void, QMetaType::LongLong,    9,
    QMetaType::Void, 0x80000000 | 21,   49,
    QMetaType::Void, 0x80000000 | 17, 0x80000000 | 17,   18,   19,
    QMetaType::Void, 0x80000000 | 24,   52,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

Q_CONSTINIT const QMetaObject Penguin::Core::PlaybackEngine::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_meta_stringdata_Penguin__Core__PlaybackEngine.offsetsAndSizes,
    qt_meta_data_Penguin__Core__PlaybackEngine,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_stringdata_Penguin__Core__PlaybackEngine_t,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<PlaybackEngine, std::true_type>,
        // method 'playbackStateChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<PlaybackState, std::false_type>,
        // method 'positionChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<qint64, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'durationChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<qint64, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'speedChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<double, std::false_type>,
        // method 'volumeChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'muteChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<bool, std::false_type>,
        // method 'tracksChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QList<TrackInfo> &, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QList<TrackInfo> &, std::false_type>,
        // method 'metadataChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const MediaMetadata &, std::false_type>,
        // method 'telemetryUpdated'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const DiagnosticsData &, std::false_type>,
        // method 'vuLevelsChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<double, std::false_type>,
        QtPrivate::TypeAndForceComplete<double, std::false_type>,
        QtPrivate::TypeAndForceComplete<double, std::false_type>,
        QtPrivate::TypeAndForceComplete<double, std::false_type>,
        // method 'equalizerChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const EqualizerDSP &, std::false_type>,
        // method 'activeLyricChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'activeSubtitleChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'mediaLoaded'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'mediaFinished'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'errorOccurred'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'onBackendStateChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<EngineState, std::false_type>,
        // method 'onBackendPositionChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<qint64, std::false_type>,
        // method 'onBackendDurationChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<qint64, std::false_type>,
        // method 'onBackendMetadataChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const MediaMetadata &, std::false_type>,
        // method 'onBackendTracksChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QList<TrackInfo> &, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QList<TrackInfo> &, std::false_type>,
        // method 'onBackendDiagnosticsChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const DiagnosticsData &, std::false_type>,
        // method 'onBackendEndOfFile'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onVuTimerTick'
        QtPrivate::TypeAndForceComplete<void, std::false_type>
    >,
    nullptr
} };

void Penguin::Core::PlaybackEngine::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<PlaybackEngine *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->playbackStateChanged((*reinterpret_cast< std::add_pointer_t<PlaybackState>>(_a[1]))); break;
        case 1: _t->positionChanged((*reinterpret_cast< std::add_pointer_t<qint64>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QString>>(_a[2]))); break;
        case 2: _t->durationChanged((*reinterpret_cast< std::add_pointer_t<qint64>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QString>>(_a[2]))); break;
        case 3: _t->speedChanged((*reinterpret_cast< std::add_pointer_t<double>>(_a[1]))); break;
        case 4: _t->volumeChanged((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 5: _t->muteChanged((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1]))); break;
        case 6: _t->tracksChanged((*reinterpret_cast< std::add_pointer_t<QList<TrackInfo>>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QList<TrackInfo>>>(_a[2]))); break;
        case 7: _t->metadataChanged((*reinterpret_cast< std::add_pointer_t<MediaMetadata>>(_a[1]))); break;
        case 8: _t->telemetryUpdated((*reinterpret_cast< std::add_pointer_t<DiagnosticsData>>(_a[1]))); break;
        case 9: _t->vuLevelsChanged((*reinterpret_cast< std::add_pointer_t<double>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<double>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<double>>(_a[3])),(*reinterpret_cast< std::add_pointer_t<double>>(_a[4]))); break;
        case 10: _t->equalizerChanged((*reinterpret_cast< std::add_pointer_t<EqualizerDSP>>(_a[1]))); break;
        case 11: _t->activeLyricChanged((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QString>>(_a[2]))); break;
        case 12: _t->activeSubtitleChanged((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 13: _t->mediaLoaded((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 14: _t->mediaFinished(); break;
        case 15: _t->errorOccurred((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 16: _t->onBackendStateChanged((*reinterpret_cast< std::add_pointer_t<EngineState>>(_a[1]))); break;
        case 17: _t->onBackendPositionChanged((*reinterpret_cast< std::add_pointer_t<qint64>>(_a[1]))); break;
        case 18: _t->onBackendDurationChanged((*reinterpret_cast< std::add_pointer_t<qint64>>(_a[1]))); break;
        case 19: _t->onBackendMetadataChanged((*reinterpret_cast< std::add_pointer_t<MediaMetadata>>(_a[1]))); break;
        case 20: _t->onBackendTracksChanged((*reinterpret_cast< std::add_pointer_t<QList<TrackInfo>>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QList<TrackInfo>>>(_a[2]))); break;
        case 21: _t->onBackendDiagnosticsChanged((*reinterpret_cast< std::add_pointer_t<DiagnosticsData>>(_a[1]))); break;
        case 22: _t->onBackendEndOfFile(); break;
        case 23: _t->onVuTimerTick(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (PlaybackEngine::*)(PlaybackState );
            if (_t _q_method = &PlaybackEngine::playbackStateChanged; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (PlaybackEngine::*)(qint64 , const QString & );
            if (_t _q_method = &PlaybackEngine::positionChanged; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (PlaybackEngine::*)(qint64 , const QString & );
            if (_t _q_method = &PlaybackEngine::durationChanged; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 2;
                return;
            }
        }
        {
            using _t = void (PlaybackEngine::*)(double );
            if (_t _q_method = &PlaybackEngine::speedChanged; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 3;
                return;
            }
        }
        {
            using _t = void (PlaybackEngine::*)(int );
            if (_t _q_method = &PlaybackEngine::volumeChanged; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 4;
                return;
            }
        }
        {
            using _t = void (PlaybackEngine::*)(bool );
            if (_t _q_method = &PlaybackEngine::muteChanged; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 5;
                return;
            }
        }
        {
            using _t = void (PlaybackEngine::*)(const QList<TrackInfo> & , const QList<TrackInfo> & );
            if (_t _q_method = &PlaybackEngine::tracksChanged; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 6;
                return;
            }
        }
        {
            using _t = void (PlaybackEngine::*)(const MediaMetadata & );
            if (_t _q_method = &PlaybackEngine::metadataChanged; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 7;
                return;
            }
        }
        {
            using _t = void (PlaybackEngine::*)(const DiagnosticsData & );
            if (_t _q_method = &PlaybackEngine::telemetryUpdated; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 8;
                return;
            }
        }
        {
            using _t = void (PlaybackEngine::*)(double , double , double , double );
            if (_t _q_method = &PlaybackEngine::vuLevelsChanged; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 9;
                return;
            }
        }
        {
            using _t = void (PlaybackEngine::*)(const EqualizerDSP & );
            if (_t _q_method = &PlaybackEngine::equalizerChanged; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 10;
                return;
            }
        }
        {
            using _t = void (PlaybackEngine::*)(int , const QString & );
            if (_t _q_method = &PlaybackEngine::activeLyricChanged; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 11;
                return;
            }
        }
        {
            using _t = void (PlaybackEngine::*)(const QString & );
            if (_t _q_method = &PlaybackEngine::activeSubtitleChanged; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 12;
                return;
            }
        }
        {
            using _t = void (PlaybackEngine::*)(const QString & );
            if (_t _q_method = &PlaybackEngine::mediaLoaded; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 13;
                return;
            }
        }
        {
            using _t = void (PlaybackEngine::*)();
            if (_t _q_method = &PlaybackEngine::mediaFinished; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 14;
                return;
            }
        }
        {
            using _t = void (PlaybackEngine::*)(const QString & );
            if (_t _q_method = &PlaybackEngine::errorOccurred; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 15;
                return;
            }
        }
    }
}

const QMetaObject *Penguin::Core::PlaybackEngine::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *Penguin::Core::PlaybackEngine::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_Penguin__Core__PlaybackEngine.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int Penguin::Core::PlaybackEngine::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
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
void Penguin::Core::PlaybackEngine::playbackStateChanged(PlaybackState _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void Penguin::Core::PlaybackEngine::positionChanged(qint64 _t1, const QString & _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void Penguin::Core::PlaybackEngine::durationChanged(qint64 _t1, const QString & _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void Penguin::Core::PlaybackEngine::speedChanged(double _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}

// SIGNAL 4
void Penguin::Core::PlaybackEngine::volumeChanged(int _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 4, _a);
}

// SIGNAL 5
void Penguin::Core::PlaybackEngine::muteChanged(bool _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 5, _a);
}

// SIGNAL 6
void Penguin::Core::PlaybackEngine::tracksChanged(const QList<TrackInfo> & _t1, const QList<TrackInfo> & _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 6, _a);
}

// SIGNAL 7
void Penguin::Core::PlaybackEngine::metadataChanged(const MediaMetadata & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 7, _a);
}

// SIGNAL 8
void Penguin::Core::PlaybackEngine::telemetryUpdated(const DiagnosticsData & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 8, _a);
}

// SIGNAL 9
void Penguin::Core::PlaybackEngine::vuLevelsChanged(double _t1, double _t2, double _t3, double _t4)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t3))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t4))) };
    QMetaObject::activate(this, &staticMetaObject, 9, _a);
}

// SIGNAL 10
void Penguin::Core::PlaybackEngine::equalizerChanged(const EqualizerDSP & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 10, _a);
}

// SIGNAL 11
void Penguin::Core::PlaybackEngine::activeLyricChanged(int _t1, const QString & _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 11, _a);
}

// SIGNAL 12
void Penguin::Core::PlaybackEngine::activeSubtitleChanged(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 12, _a);
}

// SIGNAL 13
void Penguin::Core::PlaybackEngine::mediaLoaded(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 13, _a);
}

// SIGNAL 14
void Penguin::Core::PlaybackEngine::mediaFinished()
{
    QMetaObject::activate(this, &staticMetaObject, 14, nullptr);
}

// SIGNAL 15
void Penguin::Core::PlaybackEngine::errorOccurred(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 15, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
