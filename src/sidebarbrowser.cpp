/***************************************************************************
 * Copyright (C) 2006 by Gopala Krishna A <krishna.ggk@gmail.com>          *
 * Copyright (C) 2013-2015 by Pablo Daniel Pareja Obregon                  *
 *                                                                         *
 * This is free software; you can redistribute it and/or modify            *
 * it under the terms of the GNU General Public License as published by    *
 * the Free Software Foundation; either version 2, or (at your option)     *
 * any later version.                                                      *
 *                                                                         *
 * This software is distributed in the hope that it will be useful,        *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of          *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           *
 * GNU General Public License for more details.                            *
 *                                                                         *
 * You should have received a copy of the GNU General Public License       *
 * along with this package; see the file COPYING.  If not, write to        *
 * the Free Software Foundation, Inc., 51 Franklin Street - Fifth Floor,   *
 * Boston, MA 02110-1301, USA.                                             *
 ***************************************************************************/

#include "sidebarbrowser.h"

#include "component.h"
#include "global.h"
#include "library.h"

#include <QAction>
#include <QDebug>
#include <QDrag>
#include <QHeaderView>
#include <QIcon>
#include <QList>
#include <QLineEdit>
#include <QMimeData>
#include <QMouseEvent>
#include <QPainter>
#include <QVariant>
#include <QVBoxLayout>

namespace Caneda
{
    /*************************************************************************
     *                             CategoryItem                              *
     *************************************************************************/
    //! \brief Constructor.
    CategoryItem::CategoryItem(const QString& name, const QString& filename,
            const QPixmap& pixmap, bool isLibrary, CategoryItem *parent) :
        m_name(name),
        m_filename(filename),
        m_iconPixmap(pixmap),
        m_isLibrary(isLibrary),
        m_parentItem(parent)
    {
        if(m_parentItem) {
            m_parentItem->addChild(this);
        }
    }

    //! \brief Destructor.
    CategoryItem::~CategoryItem()
    {
        qDeleteAll(m_childItems);
    }

    CategoryItem* CategoryItem::child(int row) const
    {
        if(m_childItems.isEmpty() || row >= m_childItems.size()) {
            return 0;
        }
        return m_childItems.value(row);
    }

    void CategoryItem::addChild(CategoryItem *c)
    {
        c->m_parentItem = const_cast<CategoryItem*>(this);
        m_childItems << c;
    }

    void CategoryItem::removeChild(int c)
    {
        m_childItems.removeAt(c);
    }

    int CategoryItem::row() const
    {
        if(m_parentItem) {
            return m_parentItem->m_childItems.indexOf(const_cast<CategoryItem*>(this));
        }
        return 0;
    }


    /*************************************************************************
     *                             SidebarModel                              *
     *************************************************************************/
    //! \brief Constructor.
    SidebarModel::SidebarModel(QObject *parent) : QAbstractItemModel(parent)
    {
        rootItem = new CategoryItem("Root", "");
    }

    /*!
     * \brief Add a library to the sidebar
     *
     * \param libraryName Library name
     * \param category Category to place the library
     */
    void SidebarModel::plugLibrary(const QString& libraryName, const QString& category)
    {
        LibraryManager *manager = LibraryManager::instance();
        const Library *libItem = manager->library(libraryName);

        if(!libItem) {
            return;
        }

        CategoryItem *libRoot;
        if(category == "root") {
            libRoot = new CategoryItem(libItem->libraryName(), libItem->libraryPath(),
                    QPixmap(), true, rootItem);
        }
        else {
            for(int i = 0; i < rootItem->childCount(); i++) {
                if(rootItem->child(i)->name() == category) {
                    libRoot = new CategoryItem(libItem->libraryName(), libItem->libraryPath(),
                            QPixmap(), true, rootItem->child(i));
                    break;
                }
            }
        }

        // Get the components list and plug each component into the sidebar browser
        QStringList components(libItem->componentsList());
        foreach(const QString component, components) {
            ComponentDataPtr data = libItem->component(component);
            QPixmap pixmap = manager->pixmapCache(data->name, data->library);
            new CategoryItem(data->name, data->filename, pixmap, false, libRoot);
        }

        beginResetModel();
        endResetModel();
    }

    /*!
     * \brief Remove a library from the sidebar
     *
     * \param libraryName Library name
     * \param category Category of the library to be removed
     */
    void SidebarModel::unPlugLibrary(const QString& libraryName, const QString& category)
    {
        const Library *libItem = LibraryManager::instance()->library(libraryName);
        if(!libItem) {
            return;
        }

        if(category == "root") {
            for(int i = 0; i < rootItem->childCount(); i++) {
                if(rootItem->child(i)->filename() == libItem->libraryPath()) {
                    rootItem->removeChild(i);
                    break;
                }
            }
        }
        else {
            for(int i = 0; i < rootItem->childCount(); i++) {
                if(rootItem->child(i)->name() == category) {
                    for(int j = 0; j < rootItem->child(i)->childCount(); j++) {
                        if(rootItem->child(i)->child(j)->filename() ==
                                libItem->libraryPath()) {
                            rootItem->child(i)->removeChild(j);
                            break;
                        }
                    }
                }
            }
        }

        beginResetModel();
        endResetModel();
    }

    /*!
     * \brief Returns the library row upon success, -1 if not found
     *
     * \param libraryName library to look for
     * \param category library's category
     */
    int SidebarModel::libraryRow(const QString &libraryName, const QString &category)
    {
        if(category == "root") {
            for(int i = 0; i < rootItem->childCount(); i++) {
                if(rootItem->child(i)->name() == libraryName) {
                    return i;
                }
            }
        }
        else {
            for(int i = 0; i < rootItem->childCount(); i++) {
                if(rootItem->child(i)->name() == category) {
                    for(int j = 0; j < rootItem->child(i)->childCount(); j++) {
                        if(rootItem->child(i)->child(j)->name() == libraryName) {
                            return j;
                        }
                    }
                }
            }
        }

        return -1;
    }

    void SidebarModel::plugItem(QString itemName, const QPixmap& itemPixmap, QString category)
    {
        CategoryItem *catItem = 0;

        if(category == "root") {
            catItem = new CategoryItem(itemName, "",
                    QPixmap(), true, rootItem);
        }
        else {
            for(int i = 0; i < rootItem->childCount(); i++) {
                if(rootItem->child(i)->name() == category) {
                    catItem = rootItem->child(i);
                    break;
                }
            }

            if(!catItem) {
                catItem = new CategoryItem(category, category, QPixmap(), false, rootItem);
            }

            new CategoryItem(itemName, category, itemPixmap, false, catItem);

            beginResetModel();
            endResetModel();
        }
    }

    void SidebarModel::plugItems(const QList<QPair<QString, QPixmap> > &items,
            QString category)
    {
        CategoryItem *catItem = 0;

        for(int i = 0; i < rootItem->childCount(); i++) {
            if(rootItem->child(i)->name() == category) {
                catItem = rootItem->child(i);
                break;
            }
        }

        if(!catItem) {
            catItem = new CategoryItem(category, category, QPixmap(), false, rootItem);
        }

        QList<QPair<QString, QPixmap> >::const_iterator it = items.begin(),
            end = items.end();

        while(it != end) {
            new CategoryItem(it->first, category, it->second, false, catItem);
            ++it;
        }

        beginResetModel();
        endResetModel();
    }

    QModelIndex SidebarModel::index(int row, int column, const QModelIndex & parent) const
    {
        if(column != 0) {
            return QModelIndex();
        }
        CategoryItem *parentItem;
        parentItem = !parent.isValid() ? rootItem : static_cast<CategoryItem*>(parent.internalPointer());
        CategoryItem *childItem = parentItem->child(row);
        return childItem ? createIndex(row, 0, childItem) : QModelIndex();
    }

    int SidebarModel::rowCount(const QModelIndex & parent) const
    {
        CategoryItem *parentItem;
        parentItem = !parent.isValid() ? rootItem :
            static_cast<CategoryItem*>(parent.internalPointer());
        return parentItem->childCount();
    }

    QModelIndex SidebarModel::parent(const QModelIndex & index) const
    {
        if(!index.isValid()) {
            return QModelIndex();
        }

        CategoryItem *childItem = static_cast<CategoryItem*>(index.internalPointer());
        CategoryItem *parentItem = childItem->parent();

        if(parentItem == rootItem) {
            return QModelIndex();
        }

        return createIndex(parentItem->row(), 0, parentItem);
    }

    QVariant SidebarModel::data(const QModelIndex & index, int role) const
    {
        if(!index.isValid()) {
            return QVariant();
        }

        CategoryItem *item = static_cast<CategoryItem*>(index.internalPointer());

        if(role == Qt::DisplayRole) {
            return QVariant(item->name());
        }

        if(item->isLeaf() && !item->isLibrary()) {
            switch(role) {
                case Qt::DecorationRole :
                    return QVariant(QIcon(item->iconPixmap()));

                case DragPixmapRole:
                    return QVariant(QPixmap(item->iconPixmap()));
            }
        }

        if(role == Qt::SizeHintRole) {
            return QSize(150, 32);
        }
        return QVariant();
    }

    bool SidebarModel::isLeaf(const QModelIndex& index) const
    {
        return (index.isValid() &&
                static_cast<CategoryItem*>(index.internalPointer())->isLeaf());
    }

    bool SidebarModel::isLibrary(const QModelIndex& index) const
    {
        return (index.isValid() &&
                static_cast<CategoryItem*>(index.internalPointer())->isLibrary());
    }

    Qt::ItemFlags SidebarModel::flags(const QModelIndex& index) const
    {
        Qt::ItemFlags flag = Qt::ItemIsEnabled;
        if(isLeaf(index) && !isLibrary(index)) {
            flag |= Qt::ItemIsSelectable;
            flag |= Qt::ItemIsDragEnabled;
        }
        return flag;
    }

    QStringList SidebarModel::mimeTypes() const
    {
        return QStringList() << "application/caneda.sidebarItem";
    }

    QMimeData* SidebarModel::mimeData(const QModelIndexList &indexes) const
    {
        QMimeData *mimeData = new QMimeData();
        QByteArray encodedData;

        QDataStream stream(&encodedData, QIODevice::WriteOnly);

        foreach(QModelIndex index, indexes) {
            if(index.isValid()) {
                CategoryItem *item = static_cast<CategoryItem*>(index.internalPointer());
                if(item->isLeaf() && !item->isLibrary()) {
                    QString category = item->parent()->name();
                    stream << item->name() << category;
                }
            }
        }

        mimeData->setData("application/caneda.sidebarItem", encodedData);
        return mimeData;
    }


    /*************************************************************************
     *                          FilterProxyModel                             *
     *************************************************************************/
    //! \brief Constructor.
    FilterProxyModel::FilterProxyModel(QObject *parent) : QSortFilterProxyModel(parent)
    {
    }

    bool FilterProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
    {
        QModelIndex index0 = sourceModel()->index(sourceRow, 0, sourceParent);
        SidebarModel *sm = static_cast<SidebarModel*>(sourceModel());

        if(sm->isLeaf(index0) == false) {
            return true;
        }
        return QSortFilterProxyModel::filterAcceptsRow(sourceRow, sourceParent);
    }


    /*************************************************************************
     *                                TreeView                               *
     *************************************************************************/
    //! \brief Constructor.
    TreeView::TreeView(QWidget *parent) :
        QTreeView(parent),
        invalidPressed(false)
    {
        header()->hide();

        setDragDropMode(QAbstractItemView::DragOnly);
        setDragEnabled(true);
        setAlternatingRowColors(true);
        setAnimated(true);
        setIconSize(QSize(24, 24));
    }

    void TreeView::mousePressEvent(QMouseEvent *event)
    {
        invalidPressed = !indexAt(event->pos()).isValid();
        QTreeView::mousePressEvent(event);
    }

    void TreeView::mouseMoveEvent(QMouseEvent *event)
    {
        QTreeView::mouseMoveEvent(event);
    }

    void TreeView::mouseReleaseEvent(QMouseEvent *event)
    {
        if(invalidPressed && !indexAt(event->pos()).isValid()) {
            emit invalidAreaClicked(QModelIndex());
        }
        QTreeView::mouseReleaseEvent(event);
    }

    void TreeView::startDrag(Qt::DropActions supportedActions)
    {
        QModelIndex index = selectedIndexes().first();
        QPixmap pix = model()->data(index, SidebarModel::DragPixmapRole).value<QPixmap>();

        QDrag *drag = new QDrag(this);

        drag->setPixmap(pix);
        drag->setMimeData(model()->mimeData(selectedIndexes()));
        drag->setHotSpot(QPoint(pix.width()/2, pix.height()/2));
        drag->exec(supportedActions);
    }


    /*************************************************************************
     *                            SidebarBrowser                             *
     *************************************************************************/
    //! \brief Constructor.
    SidebarBrowser::SidebarBrowser(QWidget *parent) : QWidget(parent)
    {
        QVBoxLayout *layout = new QVBoxLayout(this);

        m_filterEdit = new QLineEdit(this);
        m_filterEdit->setClearButtonEnabled(true);
        layout->addWidget(m_filterEdit);

        m_treeView = new TreeView();
        layout->addWidget(m_treeView);

        m_model = new SidebarModel(this);
        m_proxyModel = new FilterProxyModel(this);
        m_proxyModel->setDynamicSortFilter(true);
        m_proxyModel->setSourceModel(m_model);
        m_proxyModel->setSortCaseSensitivity(Qt::CaseInsensitive);
        m_treeView->setModel(m_proxyModel);

        connect(m_filterEdit, SIGNAL(textChanged(const QString &)),
                this, SLOT(filterTextChanged()));

        connect(m_model, SIGNAL(modelReset()), m_treeView, SLOT(expandAll()));
        connect(m_treeView, SIGNAL(clicked(const QModelIndex&)), this,
                SLOT(slotOnClicked(const QModelIndex&)));
        connect(m_treeView, SIGNAL(invalidAreaClicked(const QModelIndex&)), this,
                SLOT(slotOnClicked(const QModelIndex&)));
        connect(m_treeView, SIGNAL(activated(const QModelIndex&)), this,
                SLOT(slotOnDoubleClicked(const QModelIndex&)));

        setWindowTitle(tr("Schematic Items"));
        m_currentComponent = "";
    }

    //! \brief Destructor.
    SidebarBrowser::~SidebarBrowser()
    {
        m_treeView->setModel(0);
    }

    void SidebarBrowser::plugLibrary(QString libraryName, QString category)
    {
        m_model->plugLibrary(libraryName, category);
    }

    void SidebarBrowser::unPlugLibrary(QString libraryName, QString category)
    {
        m_model->unPlugLibrary(libraryName, category);
    }

    void SidebarBrowser::plugItem(QString itemName, const QPixmap& itemPixmap,
            QString category)
    {
        m_model->plugItem(itemName, itemPixmap, category);
    }

    void SidebarBrowser::plugItems(const QList<QPair<QString, QPixmap> > &items,
            QString category)
    {
        m_model->plugItems(items, category);
    }

    QString SidebarBrowser::currentComponent()
    {
        return m_currentComponent;
    }

    void SidebarBrowser::filterTextChanged()
    {
        QString text = m_filterEdit->text();
        QRegExp regExp(text, Qt::CaseInsensitive, QRegExp::RegExp);
        m_proxyModel->setFilterRegExp(regExp);
    }

    void SidebarBrowser::slotOnClicked(const QModelIndex& index)
    {
        if(index.isValid()) {
            QMimeData *mime = index.model()->mimeData(QModelIndexList() << index);
            if(mime) {
                QByteArray encodedData = mime->data("application/caneda.sidebarItem");
                QDataStream stream(&encodedData, QIODevice::ReadOnly);
                QString item, category;
                stream >> item >> category;
                emit itemClicked(item, category);
                m_currentComponent = item;
            }
        }
    }

    void SidebarBrowser::slotOnDoubleClicked(const QModelIndex& index)
    {
        if(index.isValid()) {
            QMimeData *mime = index.model()->mimeData(QModelIndexList() << index);
            if(mime) {
                QByteArray encodedData = mime->data("application/caneda.sidebarItem");
                QDataStream stream(&encodedData, QIODevice::ReadOnly);
                QString item, category;
                stream >> item >> category;
                emit itemDoubleClicked(item, category);
                m_currentComponent = item;
            }
        }
    }

} // namespace Caneda
