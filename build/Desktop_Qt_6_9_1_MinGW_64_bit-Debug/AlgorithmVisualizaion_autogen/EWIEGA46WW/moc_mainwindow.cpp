/****************************************************************************
** Meta object code from reading C++ file 'mainwindow.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.9.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../mainwindow.h"
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'mainwindow.h' doesn't include <QObject>."
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
struct qt_meta_tag_ZN10MainWindowE_t {};
} // unnamed namespace

template <> constexpr inline auto MainWindow::qt_create_metaobjectdata<qt_meta_tag_ZN10MainWindowE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "MainWindow",
        "onAlgorithmChanged",
        "",
        "index",
        "onExecuteClicked",
        "onGenerateDataClicked",
        "onPlayClicked",
        "onPauseClicked",
        "onStepForward",
        "onStepBackward",
        "onResetClicked",
        "onSpeedChanged",
        "speed",
        "onAlgorithmStepChanged",
        "currentStep",
        "totalSteps",
        "onAlgorithmFinished",
        "onAlgorithmError",
        "error",
        "onExecutionStarted",
        "onNewData",
        "onLoadData",
        "onSaveData",
        "onExportImage",
        "onAbout",
        "onSettings",
        "onShowStatistics",
        "show",
        "onShowOperationInfo",
        "onEnableAnimations",
        "enable",
        "onDataInputChanged",
        "onArraySizeChanged",
        "size",
        "onPlaybackStateChanged",
        "isPlaying"
    };

    QtMocHelpers::UintData qt_methods {
        // Slot 'onAlgorithmChanged'
        QtMocHelpers::SlotData<void(int)>(1, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Int, 3 },
        }}),
        // Slot 'onExecuteClicked'
        QtMocHelpers::SlotData<void()>(4, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onGenerateDataClicked'
        QtMocHelpers::SlotData<void()>(5, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onPlayClicked'
        QtMocHelpers::SlotData<void()>(6, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onPauseClicked'
        QtMocHelpers::SlotData<void()>(7, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onStepForward'
        QtMocHelpers::SlotData<void()>(8, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onStepBackward'
        QtMocHelpers::SlotData<void()>(9, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onResetClicked'
        QtMocHelpers::SlotData<void()>(10, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onSpeedChanged'
        QtMocHelpers::SlotData<void(int)>(11, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Int, 12 },
        }}),
        // Slot 'onAlgorithmStepChanged'
        QtMocHelpers::SlotData<void(int, int)>(13, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Int, 14 }, { QMetaType::Int, 15 },
        }}),
        // Slot 'onAlgorithmFinished'
        QtMocHelpers::SlotData<void()>(16, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onAlgorithmError'
        QtMocHelpers::SlotData<void(const QString &)>(17, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QString, 18 },
        }}),
        // Slot 'onExecutionStarted'
        QtMocHelpers::SlotData<void()>(19, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onNewData'
        QtMocHelpers::SlotData<void()>(20, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onLoadData'
        QtMocHelpers::SlotData<void()>(21, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onSaveData'
        QtMocHelpers::SlotData<void()>(22, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onExportImage'
        QtMocHelpers::SlotData<void()>(23, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onAbout'
        QtMocHelpers::SlotData<void()>(24, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onSettings'
        QtMocHelpers::SlotData<void()>(25, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onShowStatistics'
        QtMocHelpers::SlotData<void(bool)>(26, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Bool, 27 },
        }}),
        // Slot 'onShowOperationInfo'
        QtMocHelpers::SlotData<void(bool)>(28, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Bool, 27 },
        }}),
        // Slot 'onEnableAnimations'
        QtMocHelpers::SlotData<void(bool)>(29, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Bool, 30 },
        }}),
        // Slot 'onDataInputChanged'
        QtMocHelpers::SlotData<void()>(31, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onArraySizeChanged'
        QtMocHelpers::SlotData<void(int)>(32, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Int, 33 },
        }}),
        // Slot 'onPlaybackStateChanged'
        QtMocHelpers::SlotData<void(bool)>(34, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Bool, 35 },
        }}),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<MainWindow, qt_meta_tag_ZN10MainWindowE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject MainWindow::staticMetaObject = { {
    QMetaObject::SuperData::link<QMainWindow::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN10MainWindowE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN10MainWindowE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN10MainWindowE_t>.metaTypes,
    nullptr
} };

void MainWindow::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<MainWindow *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->onAlgorithmChanged((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 1: _t->onExecuteClicked(); break;
        case 2: _t->onGenerateDataClicked(); break;
        case 3: _t->onPlayClicked(); break;
        case 4: _t->onPauseClicked(); break;
        case 5: _t->onStepForward(); break;
        case 6: _t->onStepBackward(); break;
        case 7: _t->onResetClicked(); break;
        case 8: _t->onSpeedChanged((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 9: _t->onAlgorithmStepChanged((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        case 10: _t->onAlgorithmFinished(); break;
        case 11: _t->onAlgorithmError((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 12: _t->onExecutionStarted(); break;
        case 13: _t->onNewData(); break;
        case 14: _t->onLoadData(); break;
        case 15: _t->onSaveData(); break;
        case 16: _t->onExportImage(); break;
        case 17: _t->onAbout(); break;
        case 18: _t->onSettings(); break;
        case 19: _t->onShowStatistics((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1]))); break;
        case 20: _t->onShowOperationInfo((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1]))); break;
        case 21: _t->onEnableAnimations((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1]))); break;
        case 22: _t->onDataInputChanged(); break;
        case 23: _t->onArraySizeChanged((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 24: _t->onPlaybackStateChanged((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObject *MainWindow::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *MainWindow::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN10MainWindowE_t>.strings))
        return static_cast<void*>(this);
    return QMainWindow::qt_metacast(_clname);
}

int MainWindow::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QMainWindow::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 25)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 25;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 25)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 25;
    }
    return _id;
}
QT_WARNING_POP
