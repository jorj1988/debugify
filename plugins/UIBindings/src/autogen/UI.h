// Autogenerated - do not change.

#pragma once
#include "MessageBox.h"
#include "Menu.h"
#include "MainWindow.h"
#include "FileEditor.h"
#include "Editor.h"
#include "TypeEditor.h"
#include "EditableTextWindow.h"
#include "Dialog.h"
#include "Console.h"
#include "Dockable.h"
#include "ModuleExplorer.h"
#include "Terminal.h"
#include "ToolBar.h"
#include "Application.h"
#include "../../UIBindings/QtBindings/Qt.h"
#include "../../DebugifyBindings/src/autogen/LldbDriver.h"
#include "../../BindingGenerator/EksBindings/EksBindings.h"
#include "bondage/RuntimeHelpers.h"

namespace UI
{
EXPORT_DEBUGIFY const bondage::Library &bindings();
}

BONDAGE_EXPOSED_CLASS_COPYABLE(EXPORT_DEBUGIFY, ::UI::MessageBox)
BONDAGE_EXPOSED_ENUM(EXPORT_DEBUGIFY, ::UI::MessageBox::StandardButton)
BONDAGE_EXPOSED_CLASS_UNMANAGED(EXPORT_DEBUGIFY, ::UI::AboutToShowNotifier)
BONDAGE_EXPOSED_CLASS_DERIVED_MANAGED(EXPORT_DEBUGIFY, ::UI::MainWindow, ::QMainWindow, ::QWidget)
BONDAGE_EXPOSED_CLASS_UNMANAGED(EXPORT_DEBUGIFY, ::UI::DebugNotifier)
BONDAGE_EXPOSED_CLASS_UNMANAGED(EXPORT_DEBUGIFY, ::UI::TypeNotifier)
BONDAGE_EXPOSED_CLASS_UNMANAGED(EXPORT_DEBUGIFY, ::UI::AboutToCloseNotifier)
BONDAGE_EXPOSED_CLASS_UNMANAGED(EXPORT_DEBUGIFY, ::UI::EditorNotifier)
BONDAGE_EXPOSED_CLASS_UNMANAGED(EXPORT_DEBUGIFY, ::UI::OutputNotifier)
BONDAGE_EXPOSED_CLASS_DERIVED_MANAGED(EXPORT_DEBUGIFY, ::UI::Menu, ::QMenu, ::QWidget)
BONDAGE_EXPOSED_CLASS_UNMANAGED(EXPORT_DEBUGIFY, ::UI::MarginClickNotifier)
BONDAGE_EXPOSED_DERIVED_PARTIAL_CLASS(EXPORT_DEBUGIFY, ::UI::LineNumberArea, ::QWidget, ::QWidget)
BONDAGE_EXPOSED_CLASS_DERIVED_UNMANAGED(EXPORT_DEBUGIFY, ::UI::Editor, ::QWidget, ::QWidget)
BONDAGE_EXPOSED_DERIVED_PARTIAL_CLASS(EXPORT_DEBUGIFY, ::UI::TypeEditor, ::UI::Editor, ::QWidget)
BONDAGE_EXPOSED_CLASS_DERIVED_MANAGED(EXPORT_DEBUGIFY, ::UI::FileEditor, ::UI::Editor, ::QWidget)
BONDAGE_EXPOSED_ENUM(EXPORT_DEBUGIFY, ::UI::FileEditor::MarkerType)
BONDAGE_EXPOSED_DERIVED_PARTIAL_CLASS(EXPORT_DEBUGIFY, ::UI::EditableTextEdit, ::QTextEdit, ::QWidget)
BONDAGE_EXPOSED_CLASS_UNMANAGED(EXPORT_DEBUGIFY, ::UI::ClickNotifier)
BONDAGE_EXPOSED_CLASS_DERIVED_MANAGED(EXPORT_DEBUGIFY, ::UI::Dialog, ::QDialog, ::QWidget)
BONDAGE_EXPOSED_ENUM(EXPORT_DEBUGIFY, ::UI::Dialog::Result)
BONDAGE_EXPOSED_CLASS_UNMANAGED(EXPORT_DEBUGIFY, ::UI::ChangedNotifier)
BONDAGE_EXPOSED_CLASS_UNMANAGED(EXPORT_DEBUGIFY, ::UI::InputNotifier)
BONDAGE_EXPOSED_CLASS_DERIVED_UNMANAGED(EXPORT_DEBUGIFY, ::UI::Dockable, ::QDockWidget, ::QWidget)
BONDAGE_EXPOSED_CLASS_DERIVED_MANAGED(EXPORT_DEBUGIFY, ::UI::ModuleExplorerDock, ::UI::Dockable, ::QWidget)
BONDAGE_EXPOSED_CLASS_DERIVED_MANAGED(EXPORT_DEBUGIFY, ::UI::EditableTextWindow, ::UI::Dockable, ::QWidget)
BONDAGE_EXPOSED_CLASS_DERIVED_MANAGED(EXPORT_DEBUGIFY, ::UI::Console, ::UI::Dockable, ::QWidget)
BONDAGE_EXPOSED_CLASS_DERIVED_MANAGED(EXPORT_DEBUGIFY, ::UI::Terminal, ::UI::Console, ::QWidget)
BONDAGE_EXPOSED_CLASS_DERIVED_MANAGED(EXPORT_DEBUGIFY, ::UI::ToolBar, ::QToolBar, ::QWidget)
BONDAGE_EXPOSED_CLASS_DERIVED_MANAGED(EXPORT_DEBUGIFY, ::UI::Application, ::QApplication, ::QApplication)

