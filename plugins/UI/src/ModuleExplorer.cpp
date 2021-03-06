#include "ModuleExplorer.h"
#include "Module.h"
#include "CompileUnit.h"
#include "Type.h"
#include "QtCore/QFileInfo"
#include "QtCore/QDateTime"
#include "QtWidgets/QHeaderView"
#include "QtWidgets/QTreeWidgetItem"
#include "QtWidgets/QLineEdit"
#include "QtWidgets/QVBoxLayout"
#include "QDebug"

namespace UI
{

class FilterHelper : public QSortFilterProxyModel
  {
public:
  FilterHelper(QObject *parent) : QSortFilterProxyModel(parent) { }

  bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const X_OVERRIDE
    {
    if (filterRegExp().isEmpty())
      {
      return true;
      }

    auto result = QSortFilterProxyModel::filterAcceptsRow(sourceRow, sourceParent);

    return result || hasAcceptedChildren(sourceRow, sourceParent);
    }

  bool hasAcceptedChildren(int sourceRow, const QModelIndex &sourceParent) const
    {
    QModelIndex item = sourceModel()->index(sourceRow, 0, sourceParent);
    if (!item.isValid())
      {
      return false;
      }

    //check if there are children
    int childCount = item.model()->rowCount(item);
    if (childCount == 0)
      {
      return false;
      }

    for (int i = 0; i < childCount; ++i)
      {
      if (QSortFilterProxyModel::filterAcceptsRow(i, item))
        {
        return true;
        }

      if (hasAcceptedChildren(i, item))
        {
        return true;
        }
      }

    return false;
    }
  };

void ModuleWorker::loadFiles(const Module::Pointer &ptr)
  {
  auto vec = ptr->files();

  QStringList result;
  xForeach(auto &i, vec)
    {
    result << i.data();
    }

  qSort(result);

  emit loadedFiles(ptr, result);
  }

ModuleExplorer::ModuleExplorer(TypeManager *types)
    : _types(types)
  {
  qRegisterMetaType<Module::Pointer>();

  _widget = new QWidget();

  auto layout = new QVBoxLayout(_widget);
  layout->setContentsMargins(4, 4, 4, 4);
  layout->setSpacing(4);

  auto edit = new QLineEdit(_widget);
  edit->setPlaceholderText("Search");
  layout->addWidget(edit);
  connect(edit, SIGNAL(textChanged(QString)), this, SLOT(filterChanged(QString)));

  _tree = new QTreeView(_widget);
  layout->addWidget(_tree);
  _tree->setEditTriggers(QAbstractItemView::NoEditTriggers);
  _tree->header()->hide();

  _filter = new FilterHelper(this);
  _filter->setFilterKeyColumn(0);
  _tree->setModel(_filter);
  _filter->setSourceModel(this);

  _worker = new ModuleWorker;
  _workerThread = new QThread(this);
  _worker->moveToThread(_workerThread);
  _workerThread->start();

  connect(this, SIGNAL(loadFiles(Module::Pointer)), _worker, SLOT(loadFiles(Module::Pointer)));
  connect(_worker, SIGNAL(loadedFiles(Module::Pointer,QStringList)), this, SLOT(loadedFiles(Module::Pointer,QStringList)));

  connect(_tree, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(itemDoubleClicked(QModelIndex)));

  connect(types, SIGNAL(typeAdded(Module::Pointer, UI::CachedType::Pointer)), this, SLOT(typeAdded(Module::Pointer, UI::CachedType::Pointer)));
  connect(types, SIGNAL(typeDeclarationAdded(Module::Pointer, UI::CachedType::Pointer, const Type*)), this, SLOT(typeDeclarationAdded(Module::Pointer, UI::CachedType::Pointer, const Type*)));
  }

ModuleExplorer::~ModuleExplorer()
  {
  _workerThread->quit();
  while (!_workerThread->isFinished())
    {
    QThread::currentThread()->msleep(1);
    }
  }

void ModuleExplorer::setTarget(const Target::Pointer &tar)
  {
  _target = tar;

  rebuildTree();
  }

bool ModuleExplorer::isSystemModule(const QString &path, const Module::Pointer &)
  {
#ifdef X_OSX
  return path.startsWith("/usr/lib") || path.startsWith("/System/Library");
#else
# error Define system library paths?
#endif
  }

QString ModuleExplorer::makeFileTooltip(const QString file, const QFileInfo &info)
  {
  auto lastModified = info.lastModified();
  QDateTime now = QDateTime::currentDateTime();

  QString type = "secs";
  qint64 to = lastModified.secsTo(now);
  if (to > 60)
    {
    to /= 60;
    type = "mins";
    }
  if (to > 60)
    {
    to /= 60;
    type = "hours";
    }
  if (to > 24)
    {
    to /= 24;
    type = "days";
    }
  if (to > 365)
    {
    to /= 365;
    type = "years";
    }

  return QString("Path: %1\nLast modified: (%2 %3 ago) %4")
      .arg(file)
      .arg(to)
      .arg(type)
      .arg(lastModified.toString());
  }

void ModuleExplorer::loadedFiles(const Module::Pointer &module, const QStringList &files)
  {
  if (!files.size())
    {
    return;
    }

  auto foundModule = _moduleMap.find(module);
  if (foundModule == _moduleMap.end())
    {
    return;
    }

  auto &moduleData = foundModule->second;

  moduleData.filesItem = new QStandardItem("Files");
  moduleData.moduleItem->appendRow(moduleData.filesItem);

  xForeach(auto file, files)
    {
    QFileInfo info(file);
    auto fileItem = new QStandardItem(info.fileName());
    fileItem->setData(file, PathRole);
    fileItem->setData(Source, TypeRole);
    fileItem->setData(QVariant::fromValue(module), ModuleRole);

    fileItem->setToolTip(makeFileTooltip(file, info));

    moduleData.filesItem->appendRow(fileItem);

    _fileMap[file] = fileItem;

    auto beg = _unownedTypeMap.lowerBound(file);
    auto end = _unownedTypeMap.upperBound(file);
    for (; beg != end; ++beg)
      {
      insertTypeReference(module, fileItem, beg.value());
      }
    _unownedTypeMap.remove(file);
    }

  moduleData.filesItem->sortChildren(0);
  }

void ModuleExplorer::itemDoubleClicked(const QModelIndex &index)
  {
  auto type = index.data(TypeRole);
  if (type == Source)
    {
    emit sourceFileActivated(index.data(ModuleRole).value<Module::Pointer>(), index.data(PathRole).toString());
    }
  else if (type == DataType)
    {
    emit dataTypeActivated(index.data(ModuleRole).value<Module::Pointer>(), index.data(PathRole).toString());
    }
  }

void ModuleExplorer::typeAdded(const Module::Pointer &module, const CachedType::Pointer &type)
  {
  auto &item = _moduleMap[module];
  if (!item.moduleItem)
    {
    // Possibly we could hit this if target changed rapidly.. probably not good.
    return;
    }

  if (!item.typeItem)
    {	
    item.typeItem = new QStandardItem("Types");
    item.moduleItem->appendRow(item.typeItem);
    }

  auto parent = item.typeItem;
  auto it = item.items.find(type->parent);
  if (it != item.items.end())
    {
    parent = it->second;
    }

  auto typeItem = new QStandardItem(type->basename + type->specialisation);
  parent->appendRow(typeItem);
  typeItem->setData(type->path, PathRole);
  typeItem->setData(DataType, TypeRole);
  typeItem->setData(QVariant::fromValue(module), ModuleRole);
  item.items[type] = typeItem;

  typeItem->setToolTip(type->path);

  parent->sortChildren(0);
  }

void ModuleExplorer::typeDeclarationAdded(const Module::Pointer &module, const CachedType::Pointer &type, const Type *)
  {
  auto &moduleItem = _moduleMap[module];
  if (!moduleItem.moduleItem)
    {
    // Possibly we could hit this if target changed rapidly.. probably not good.
    return;
    }

  auto item = _fileMap[type->file];
  if (item)
    {
    insertTypeReference(module, item, type);
    return;
    }

  _unownedTypeMap.insert(type->file, type);
  }

void ModuleExplorer::insertTypeReference(const Module::Pointer &module, QStandardItem *parent, const CachedType::Pointer &type)
  {
  for (int i = 0; i < parent->rowCount(); ++i)
    {
    QStandardItem *child = parent->child(i);
    if (child->data(PathRole).toString() == type->path)
      {
      return;
      }
    }

  auto typeItem = new QStandardItem(type->basename + type->specialisation);
  typeItem->setData(DataType, TypeRole);
  typeItem->setData(type->path, PathRole);
  typeItem->setData(QVariant::fromValue(module), ModuleRole);

  parent->appendRow(typeItem);
  }

void ModuleExplorer::filterChanged(const QString &filter)
  {
  Qt::CaseSensitivity caseSensitivity = Qt::CaseInsensitive;
  QString strPattern = "^" + filter;
  QRegExp regExp(strPattern, caseSensitivity);

  _filter->setFilterRegExp(regExp);
  }

void ModuleExplorer::clearTree()
  {
  _fileMap.clear();
  _unownedTypeMap.clear();
  _moduleMap.clear();
  QStandardItemModel::clear();
  }

void ModuleExplorer::rebuildTree()
  {
  clearTree();
  if (!_target)
    {
    return;
    }

  QStandardItem *systemItems = new QStandardItem("System");

  for(size_t i = 0; i < _target->moduleCount(); ++i)
    {
    auto module = _target->moduleAt(i);

    bool isSystem = false;
    auto item = new QStandardItem;
    buildModule(item, module, isSystem);

    if (isSystem)
      {
      systemItems->appendRow(QList<QStandardItem*>() << item);
      }
    else
      {
      appendRow(item);
      }

    _moduleMap[module] = { item, nullptr, nullptr, { } };
    }

  if (systemItems->hasChildren())
    {
    appendRow(QList<QStandardItem*>() << systemItems);
    }
  }

void ModuleExplorer::buildModule(QStandardItem *item, const Module::Pointer &module, bool &isSystem)
  {
  QString path = module->path().data();
  QFileInfo info(path);

  item->setData(Module, TypeRole);
  item->setData(path, PathRole);

  QString name(info.fileName());
  item->setText(name);

  item->setToolTip(makeFileTooltip(path, info));

  isSystem = isSystemModule(path, module);

  emit loadFiles(module);
  }

ModuleExplorerDock::ModuleExplorerDock(TypeManager *types, bool toolbar)
    : Dockable(toolbar)
  {
  _explorer = new ModuleExplorer(types);
  setWidget(_explorer->widget());
  }

void ModuleExplorerDock::setTarget(const Target::Pointer &ptr)
  {
  _explorer->setTarget(ptr);
  }

}
