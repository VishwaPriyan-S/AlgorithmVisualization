/****************************************************************************
** Meta object code from reading C++ file 'algorithm_manager.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.9.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../algorithm_manager.h"
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'algorithm_manager.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 69
#error "This file was generated using the moc from 6.9.1. It"
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
struct qt_meta_tag_ZN16AlgorithmManagerE_t {};
} // unnamed namespace

template <> constexpr inline auto AlgorithmManager::qt_create_metaobjectdata<qt_meta_tag_ZN16AlgorithmManagerE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "AlgorithmManager",
        "algorithmChanged",
        "",
        "newAlgorithmName",
        "executionStarted",
        "stepChanged",
        "currentStep",
        "totalSteps",
        "algorithmFinished",
        "algorithmError",
        "error",
        "playbackStateChanged",
        "isPlaying",
        "onAlgorithmStateChanged",
        "AlgorithmBase::ExecutionState",
        "state",
        "autoStep"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'algorithmChanged'
        QtMocHelpers::SignalData<void(const QString &)>(1, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 3 },
        }}),
        // Signal 'executionStarted'
        QtMocHelpers::SignalData<void()>(4, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'stepChanged'
        QtMocHelpers::SignalData<void(int, int)>(5, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 6 }, { QMetaType::Int, 7 },
        }}),
        // Signal 'algorithmFinished'
        QtMocHelpers::SignalData<void()>(8, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'algorithmError'
        QtMocHelpers::SignalData<void(const QString &)>(9, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 10 },
        }}),
        // Signal 'playbackStateChanged'
        QtMocHelpers::SignalData<void(bool)>(11, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Bool, 12 },
        }}),
        // Slot 'onAlgorithmStateChanged'
        QtMocHelpers::SlotData<void(AlgorithmBase::ExecutionState)>(13, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { 0x80000000 | 14, 15 },
        }}),
        // Slot 'autoStep'
        QtMocHelpers::SlotData<void()>(16, 2, QMC::AccessPrivate, QMetaType::Void),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<AlgorithmManager, qt_meta_tag_ZN16AlgorithmManagerE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject AlgorithmManager::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN16AlgorithmManagerE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN16AlgorithmManagerE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN16AlgorithmManagerE_t>.metaTypes,
    nullptr
} };

void AlgorithmManager::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<AlgorithmManager *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->algorithmChanged((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 1: _t->executionStarted(); break;
        case 2: _t->stepChanged((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        case 3: _t->algorithmFinished(); break;
        case 4: _t->algorithmError((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 5: _t->playbackStateChanged((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1]))); break;
        case 6: _t->onAlgorithmStateChanged((*reinterpret_cast< std::add_pointer_t<AlgorithmBase::ExecutionState>>(_a[1]))); break;
        case 7: _t->autoStep(); break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (AlgorithmManager::*)(const QString & )>(_a, &AlgorithmManager::algorithmChanged, 0))
            return;
        if (QtMocHelpers::indexOfMethod<void (AlgorithmManager::*)()>(_a, &AlgorithmManager::executionStarted, 1))
            return;
        if (QtMocHelpers::indexOfMethod<void (AlgorithmManager::*)(int , int )>(_a, &AlgorithmManager::stepChanged, 2))
            return;
        if (QtMocHelpers::indexOfMethod<void (AlgorithmManager::*)()>(_a, &AlgorithmManager::algorithmFinished, 3))
            return;
        if (QtMocHelpers::indexOfMethod<void (AlgorithmManager::*)(const QString & )>(_a, &AlgorithmManager::algorithmError, 4))
            return;
        if (QtMocHelpers::indexOfMethod<void (AlgorithmManager::*)(bool )>(_a, &AlgorithmManager::playbackStateChanged, 5))
            return;
    }
}

const QMetaObject *AlgorithmManager::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *AlgorithmManager::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN16AlgorithmManagerE_t>.strings))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int AlgorithmManager::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 8)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 8;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 8)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 8;
    }
    return _id;
}

// SIGNAL 0
void AlgorithmManager::algorithmChanged(const QString & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 0, nullptr, _t1);
}

// SIGNAL 1
void AlgorithmManager::executionStarted()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}

// SIGNAL 2
void AlgorithmManager::stepChanged(int _t1, int _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 2, nullptr, _t1, _t2);
}

// SIGNAL 3
void AlgorithmManager::algorithmFinished()
{
    QMetaObject::activate(this, &staticMetaObject, 3, nullptr);
}

// SIGNAL 4
void AlgorithmManager::algorithmError(const QString & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 4, nullptr, _t1);
}

// SIGNAL 5
void AlgorithmManager::playbackStateChanged(bool _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 5, nullptr, _t1);
}
QT_WARNING_POP
