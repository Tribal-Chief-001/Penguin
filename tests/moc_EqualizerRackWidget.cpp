/****************************************************************************
** Meta object code from reading C++ file 'EqualizerRackWidget.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.4.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../src/ui/EqualizerRackWidget.h"
#include <QtGui/qtextcursor.h>
#include <QtCore/qmetatype.h>
#include <QtCore/QList>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'EqualizerRackWidget.h' doesn't include <QObject>."
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
struct qt_meta_stringdata_Penguin__UI__EqualizerRackWidget_t {
    uint offsetsAndSizes[34];
    char stringdata0[33];
    char stringdata1[16];
    char stringdata2[1];
    char stringdata3[10];
    char stringdata4[7];
    char stringdata5[16];
    char stringdata6[14];
    char stringdata7[6];
    char stringdata8[14];
    char stringdata9[11];
    char stringdata10[12];
    char stringdata11[12];
    char stringdata12[10];
    char stringdata13[10];
    char stringdata14[21];
    char stringdata15[6];
    char stringdata16[23];
};
#define QT_MOC_LITERAL(ofs, len) \
    uint(sizeof(qt_meta_stringdata_Penguin__UI__EqualizerRackWidget_t::offsetsAndSizes) + ofs), len 
Q_CONSTINIT static const qt_meta_stringdata_Penguin__UI__EqualizerRackWidget_t qt_meta_stringdata_Penguin__UI__EqualizerRackWidget = {
    {
        QT_MOC_LITERAL(0, 32),  // "Penguin::UI::EqualizerRackWidget"
        QT_MOC_LITERAL(33, 15),  // "bandGainChanged"
        QT_MOC_LITERAL(49, 0),  // ""
        QT_MOC_LITERAL(50, 9),  // "bandIndex"
        QT_MOC_LITERAL(60, 6),  // "gainDb"
        QT_MOC_LITERAL(67, 15),  // "allGainsChanged"
        QT_MOC_LITERAL(83, 13),  // "QList<double>"
        QT_MOC_LITERAL(97, 5),  // "gains"
        QT_MOC_LITERAL(103, 13),  // "presetChanged"
        QT_MOC_LITERAL(117, 10),  // "presetName"
        QT_MOC_LITERAL(128, 11),  // "setBandGain"
        QT_MOC_LITERAL(140, 11),  // "setAllGains"
        QT_MOC_LITERAL(152, 9),  // "setPreset"
        QT_MOC_LITERAL(162, 9),  // "resetFlat"
        QT_MOC_LITERAL(172, 20),  // "onSliderValueChanged"
        QT_MOC_LITERAL(193, 5),  // "value"
        QT_MOC_LITERAL(199, 22)   // "onPresetComboActivated"
    },
    "Penguin::UI::EqualizerRackWidget",
    "bandGainChanged",
    "",
    "bandIndex",
    "gainDb",
    "allGainsChanged",
    "QList<double>",
    "gains",
    "presetChanged",
    "presetName",
    "setBandGain",
    "setAllGains",
    "setPreset",
    "resetFlat",
    "onSliderValueChanged",
    "value",
    "onPresetComboActivated"
};
#undef QT_MOC_LITERAL
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_Penguin__UI__EqualizerRackWidget[] = {

 // content:
      10,       // revision
       0,       // classname
       0,    0, // classinfo
       9,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       3,       // signalCount

 // signals: name, argc, parameters, tag, flags, initial metatype offsets
       1,    2,   68,    2, 0x06,    1 /* Public */,
       5,    1,   73,    2, 0x06,    4 /* Public */,
       8,    1,   76,    2, 0x06,    6 /* Public */,

 // slots: name, argc, parameters, tag, flags, initial metatype offsets
      10,    2,   79,    2, 0x0a,    8 /* Public */,
      11,    1,   84,    2, 0x0a,   11 /* Public */,
      12,    1,   87,    2, 0x0a,   13 /* Public */,
      13,    0,   90,    2, 0x0a,   15 /* Public */,
      14,    1,   91,    2, 0x08,   16 /* Private */,
      16,    1,   94,    2, 0x08,   18 /* Private */,

 // signals: parameters
    QMetaType::Void, QMetaType::Int, QMetaType::Double,    3,    4,
    QMetaType::Void, 0x80000000 | 6,    7,
    QMetaType::Void, QMetaType::QString,    9,

 // slots: parameters
    QMetaType::Void, QMetaType::Int, QMetaType::Double,    3,    4,
    QMetaType::Void, 0x80000000 | 6,    7,
    QMetaType::Void, QMetaType::QString,    9,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int,   15,
    QMetaType::Void, QMetaType::QString,    9,

       0        // eod
};

Q_CONSTINIT const QMetaObject Penguin::UI::EqualizerRackWidget::staticMetaObject = { {
    QMetaObject::SuperData::link<QWidget::staticMetaObject>(),
    qt_meta_stringdata_Penguin__UI__EqualizerRackWidget.offsetsAndSizes,
    qt_meta_data_Penguin__UI__EqualizerRackWidget,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_stringdata_Penguin__UI__EqualizerRackWidget_t,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<EqualizerRackWidget, std::true_type>,
        // method 'bandGainChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        QtPrivate::TypeAndForceComplete<double, std::false_type>,
        // method 'allGainsChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QVector<double> &, std::false_type>,
        // method 'presetChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'setBandGain'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        QtPrivate::TypeAndForceComplete<double, std::false_type>,
        // method 'setAllGains'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QVector<double> &, std::false_type>,
        // method 'setPreset'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'resetFlat'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onSliderValueChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'onPresetComboActivated'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>
    >,
    nullptr
} };

void Penguin::UI::EqualizerRackWidget::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<EqualizerRackWidget *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->bandGainChanged((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<double>>(_a[2]))); break;
        case 1: _t->allGainsChanged((*reinterpret_cast< std::add_pointer_t<QList<double>>>(_a[1]))); break;
        case 2: _t->presetChanged((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 3: _t->setBandGain((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<double>>(_a[2]))); break;
        case 4: _t->setAllGains((*reinterpret_cast< std::add_pointer_t<QList<double>>>(_a[1]))); break;
        case 5: _t->setPreset((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 6: _t->resetFlat(); break;
        case 7: _t->onSliderValueChanged((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 8: _t->onPresetComboActivated((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        switch (_id) {
        default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
        case 1:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
            case 0:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< QList<double> >(); break;
            }
            break;
        case 4:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
            case 0:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< QList<double> >(); break;
            }
            break;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (EqualizerRackWidget::*)(int , double );
            if (_t _q_method = &EqualizerRackWidget::bandGainChanged; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (EqualizerRackWidget::*)(const QVector<double> & );
            if (_t _q_method = &EqualizerRackWidget::allGainsChanged; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (EqualizerRackWidget::*)(const QString & );
            if (_t _q_method = &EqualizerRackWidget::presetChanged; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 2;
                return;
            }
        }
    }
}

const QMetaObject *Penguin::UI::EqualizerRackWidget::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *Penguin::UI::EqualizerRackWidget::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_Penguin__UI__EqualizerRackWidget.stringdata0))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int Penguin::UI::EqualizerRackWidget::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 9)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 9;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 9)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 9;
    }
    return _id;
}

// SIGNAL 0
void Penguin::UI::EqualizerRackWidget::bandGainChanged(int _t1, double _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void Penguin::UI::EqualizerRackWidget::allGainsChanged(const QVector<double> & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void Penguin::UI::EqualizerRackWidget::presetChanged(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
