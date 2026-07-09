/****************************************************************************
** Meta object code from reading C++ file 'RecursosTab.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.5.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../src/gui/RecursosTab.h"
#include <QtCore/qmetatype.h>

#if __has_include(<QtCore/qtmochelpers.h>)
#include <QtCore/qtmochelpers.h>
#else
QT_BEGIN_MOC_NAMESPACE
#endif


#include <memory>

#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'RecursosTab.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 68
#error "This file was generated using the moc from 6.5.3. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

#ifndef Q_CONSTINIT
#define Q_CONSTINIT
#endif

QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
QT_WARNING_DISABLE_GCC("-Wuseless-cast")
namespace {

#ifdef QT_MOC_HAS_STRINGDATA
struct qt_meta_stringdata_CLASSRecursosTabENDCLASS_t {};
static constexpr auto qt_meta_stringdata_CLASSRecursosTabENDCLASS = QtMocHelpers::stringData(
    "RecursosTab",
    "measureResources",
    "",
    "handleMeasurementFinished",
    "std::vector<double>",
    "timesPeru",
    "memsPeru",
    "timesGeom",
    "memsGeom"
);
#else  // !QT_MOC_HAS_STRING_DATA
struct qt_meta_stringdata_CLASSRecursosTabENDCLASS_t {
    uint offsetsAndSizes[18];
    char stringdata0[12];
    char stringdata1[17];
    char stringdata2[1];
    char stringdata3[26];
    char stringdata4[20];
    char stringdata5[10];
    char stringdata6[9];
    char stringdata7[10];
    char stringdata8[9];
};
#define QT_MOC_LITERAL(ofs, len) \
    uint(sizeof(qt_meta_stringdata_CLASSRecursosTabENDCLASS_t::offsetsAndSizes) + ofs), len 
Q_CONSTINIT static const qt_meta_stringdata_CLASSRecursosTabENDCLASS_t qt_meta_stringdata_CLASSRecursosTabENDCLASS = {
    {
        QT_MOC_LITERAL(0, 11),  // "RecursosTab"
        QT_MOC_LITERAL(12, 16),  // "measureResources"
        QT_MOC_LITERAL(29, 0),  // ""
        QT_MOC_LITERAL(30, 25),  // "handleMeasurementFinished"
        QT_MOC_LITERAL(56, 19),  // "std::vector<double>"
        QT_MOC_LITERAL(76, 9),  // "timesPeru"
        QT_MOC_LITERAL(86, 8),  // "memsPeru"
        QT_MOC_LITERAL(95, 9),  // "timesGeom"
        QT_MOC_LITERAL(105, 8)   // "memsGeom"
    },
    "RecursosTab",
    "measureResources",
    "",
    "handleMeasurementFinished",
    "std::vector<double>",
    "timesPeru",
    "memsPeru",
    "timesGeom",
    "memsGeom"
};
#undef QT_MOC_LITERAL
#endif // !QT_MOC_HAS_STRING_DATA
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_CLASSRecursosTabENDCLASS[] = {

 // content:
      11,       // revision
       0,       // classname
       0,    0, // classinfo
       2,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags, initial metatype offsets
       1,    0,   26,    2, 0x08,    1 /* Private */,
       3,    4,   27,    2, 0x08,    2 /* Private */,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 4, 0x80000000 | 4, 0x80000000 | 4, 0x80000000 | 4,    5,    6,    7,    8,

       0        // eod
};

Q_CONSTINIT const QMetaObject RecursosTab::staticMetaObject = { {
    QMetaObject::SuperData::link<QWidget::staticMetaObject>(),
    qt_meta_stringdata_CLASSRecursosTabENDCLASS.offsetsAndSizes,
    qt_meta_data_CLASSRecursosTabENDCLASS,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_stringdata_CLASSRecursosTabENDCLASS_t,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<RecursosTab, std::true_type>,
        // method 'measureResources'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'handleMeasurementFinished'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const std::vector<double> &, std::false_type>,
        QtPrivate::TypeAndForceComplete<const std::vector<double> &, std::false_type>,
        QtPrivate::TypeAndForceComplete<const std::vector<double> &, std::false_type>,
        QtPrivate::TypeAndForceComplete<const std::vector<double> &, std::false_type>
    >,
    nullptr
} };

void RecursosTab::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<RecursosTab *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->measureResources(); break;
        case 1: _t->handleMeasurementFinished((*reinterpret_cast< std::add_pointer_t<std::vector<double>>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<std::vector<double>>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<std::vector<double>>>(_a[3])),(*reinterpret_cast< std::add_pointer_t<std::vector<double>>>(_a[4]))); break;
        default: ;
        }
    }
}

const QMetaObject *RecursosTab::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *RecursosTab::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_CLASSRecursosTabENDCLASS.stringdata0))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int RecursosTab::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
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
QT_WARNING_POP
