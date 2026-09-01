/****************************************************************************
** Meta object code from reading C++ file 'VUMeterWidget.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.4.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../src/ui/VUMeterWidget.h"
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'VUMeterWidget.h' doesn't include <QObject>."
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
struct qt_meta_stringdata_Penguin__UI__VUMeterWidget_t {
    uint offsetsAndSizes[28];
    char stringdata0[27];
    char stringdata1[10];
    char stringdata2[1];
    char stringdata3[11];
    char stringdata4[12];
    char stringdata5[10];
    char stringdata6[11];
    char stringdata7[12];
    char stringdata8[15];
    char stringdata9[7];
    char stringdata10[14];
    char stringdata11[3];
    char stringdata12[15];
    char stringdata13[6];
};
#define QT_MOC_LITERAL(ofs, len) \
    uint(sizeof(qt_meta_stringdata_Penguin__UI__VUMeterWidget_t::offsetsAndSizes) + ofs), len 
Q_CONSTINIT static const qt_meta_stringdata_Penguin__UI__VUMeterWidget_t qt_meta_stringdata_Penguin__UI__VUMeterWidget = {
    {
        QT_MOC_LITERAL(0, 26),  // "Penguin::UI::VUMeterWidget"
        QT_MOC_LITERAL(27, 9),  // "setLevels"
        QT_MOC_LITERAL(37, 0),  // ""
        QT_MOC_LITERAL(38, 10),  // "leftPeakDb"
        QT_MOC_LITERAL(49, 11),  // "rightPeakDb"
        QT_MOC_LITERAL(61, 9),  // "leftRmsDb"
        QT_MOC_LITERAL(71, 10),  // "rightRmsDb"
        QT_MOC_LITERAL(82, 11),  // "setVULevels"
        QT_MOC_LITERAL(94, 14),  // "Core::VULevels"
        QT_MOC_LITERAL(109, 6),  // "levels"
        QT_MOC_LITERAL(116, 13),  // "setLeftPeakDb"
        QT_MOC_LITERAL(130, 2),  // "db"
        QT_MOC_LITERAL(133, 14),  // "setRightPeakDb"
        QT_MOC_LITERAL(148, 5)   // "reset"
    },
    "Penguin::UI::VUMeterWidget",
    "setLevels",
    "",
    "leftPeakDb",
    "rightPeakDb",
    "leftRmsDb",
    "rightRmsDb",
    "setVULevels",
    "Core::VULevels",
    "levels",
    "setLeftPeakDb",
    "db",
    "setRightPeakDb",
    "reset"
};
#undef QT_MOC_LITERAL
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_Penguin__UI__VUMeterWidget[] = {

 // content:
      10,       // revision
       0,       // classname
       0,    0, // classinfo
       7,   14, // methods
       2,   87, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags, initial metatype offsets
       1,    4,   56,    2, 0x0a,    3 /* Public */,
       1,    3,   65,    2, 0x2a,    8 /* Public | MethodCloned */,
       1,    2,   72,    2, 0x2a,   12 /* Public | MethodCloned */,
       7,    1,   77,    2, 0x0a,   15 /* Public */,
      10,    1,   80,    2, 0x0a,   17 /* Public */,
      12,    1,   83,    2, 0x0a,   19 /* Public */,
      13,    0,   86,    2, 0x0a,   21 /* Public */,

 // slots: parameters
    QMetaType::Void, QMetaType::Double, QMetaType::Double, QMetaType::Double, QMetaType::Double,    3,    4,    5,    6,
    QMetaType::Void, QMetaType::Double, QMetaType::Double, QMetaType::Double,    3,    4,    5,
    QMetaType::Void, QMetaType::Double, QMetaType::Double,    3,    4,
    QMetaType::Void, 0x80000000 | 8,    9,
    QMetaType::Void, QMetaType::Double,   11,
    QMetaType::Void, QMetaType::Double,   11,
    QMetaType::Void,

 // properties: name, type, flags
       3, QMetaType::Double, 0x00015103, uint(-1), 0,
       4, QMetaType::Double, 0x00015103, uint(-1), 0,

       0        // eod
};

Q_CONSTINIT const QMetaObject Penguin::UI::VUMeterWidget::staticMetaObject = { {
    QMetaObject::SuperData::link<QWidget::staticMetaObject>(),
    qt_meta_stringdata_Penguin__UI__VUMeterWidget.offsetsAndSizes,
    qt_meta_data_Penguin__UI__VUMeterWidget,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_stringdata_Penguin__UI__VUMeterWidget_t,
        // property 'leftPeakDb'
        QtPrivate::TypeAndForceComplete<double, std::true_type>,
        // property 'rightPeakDb'
        QtPrivate::TypeAndForceComplete<double, std::true_type>,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<VUMeterWidget, std::true_type>,
        // method 'setLevels'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<double, std::false_type>,
        QtPrivate::TypeAndForceComplete<double, std::false_type>,
        QtPrivate::TypeAndForceComplete<double, std::false_type>,
        QtPrivate::TypeAndForceComplete<double, std::false_type>,
        // method 'setLevels'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<double, std::false_type>,
        QtPrivate::TypeAndForceComplete<double, std::false_type>,
        QtPrivate::TypeAndForceComplete<double, std::false_type>,
        // method 'setLevels'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<double, std::false_type>,
        QtPrivate::TypeAndForceComplete<double, std::false_type>,
        // method 'setVULevels'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const Core::VULevels &, std::false_type>,
        // method 'setLeftPeakDb'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<double, std::false_type>,
        // method 'setRightPeakDb'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<double, std::false_type>,
        // method 'reset'
        QtPrivate::TypeAndForceComplete<void, std::false_type>
    >,
    nullptr
} };

void Penguin::UI::VUMeterWidget::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<VUMeterWidget *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->setLevels((*reinterpret_cast< std::add_pointer_t<double>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<double>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<double>>(_a[3])),(*reinterpret_cast< std::add_pointer_t<double>>(_a[4]))); break;
        case 1: _t->setLevels((*reinterpret_cast< std::add_pointer_t<double>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<double>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<double>>(_a[3]))); break;
        case 2: _t->setLevels((*reinterpret_cast< std::add_pointer_t<double>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<double>>(_a[2]))); break;
        case 3: _t->setVULevels((*reinterpret_cast< std::add_pointer_t<Core::VULevels>>(_a[1]))); break;
        case 4: _t->setLeftPeakDb((*reinterpret_cast< std::add_pointer_t<double>>(_a[1]))); break;
        case 5: _t->setRightPeakDb((*reinterpret_cast< std::add_pointer_t<double>>(_a[1]))); break;
        case 6: _t->reset(); break;
        default: ;
        }
    }else if (_c == QMetaObject::ReadProperty) {
        auto *_t = static_cast<VUMeterWidget *>(_o);
        (void)_t;
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast< double*>(_v) = _t->leftPeakDb(); break;
        case 1: *reinterpret_cast< double*>(_v) = _t->rightPeakDb(); break;
        default: break;
        }
    } else if (_c == QMetaObject::WriteProperty) {
        auto *_t = static_cast<VUMeterWidget *>(_o);
        (void)_t;
        void *_v = _a[0];
        switch (_id) {
        case 0: _t->setLeftPeakDb(*reinterpret_cast< double*>(_v)); break;
        case 1: _t->setRightPeakDb(*reinterpret_cast< double*>(_v)); break;
        default: break;
        }
    } else if (_c == QMetaObject::ResetProperty) {
    } else if (_c == QMetaObject::BindableProperty) {
    }
}

const QMetaObject *Penguin::UI::VUMeterWidget::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *Penguin::UI::VUMeterWidget::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_Penguin__UI__VUMeterWidget.stringdata0))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int Penguin::UI::VUMeterWidget::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 7)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 7;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 7)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 7;
    }else if (_c == QMetaObject::ReadProperty || _c == QMetaObject::WriteProperty
            || _c == QMetaObject::ResetProperty || _c == QMetaObject::BindableProperty
            || _c == QMetaObject::RegisterPropertyMetaType) {
        qt_static_metacall(this, _c, _id, _a);
        _id -= 2;
    }
    return _id;
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
