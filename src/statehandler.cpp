/***************************************************************************
 * Copyright (C) 2010 by Gopala Krishna A <krishna.ggk@gmail.com>          *
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

#include "statehandler.h"

#include "actionmanager.h"
#include "cgraphicsscene.h"
#include "cgraphicsview.h"
#include "global.h"
#include "library.h"
#include "settings.h"
#include "singletonowner.h"
#include "undocommands.h"

#include "paintings/painting.h"

#include "xmlutilities/xmlutilities.h"

#include <QApplication>
#include <QClipboard>
#include <QDebug>
#include <QPointer>
#include <QSet>

namespace Caneda
{
    struct StateHandlerPrivate
    {
        StateHandlerPrivate() {
            mouseAction = CGraphicsScene::Normal;
            paintingDrawItem = 0;

        }

        ~StateHandlerPrivate() {
            delete paintingDrawItem;
            clearInsertibles();
        }

        void updateToolbarInsertibles() {
            LibraryLoader *loader = LibraryLoader::instance();
            toolbarInsertibles.insert("insGround",
                    loader->newComponent("Ground", 0, "Passive"));
            toolbarInsertibles.insert("insPort",
                    loader->newComponent("Port", 0, "Passive"));
        }

        void clearInsertibles() {
            foreach (CGraphicsItem* item, insertibles) {
                if (item->scene()) {
                    item->scene()->removeItem(item);
                }
                delete item;
            }
            insertibles.clear();
        }

        CGraphicsScene::MouseAction mouseAction;
        QList<CGraphicsItem*> insertibles;
        Painting *paintingDrawItem;

        QSet<CGraphicsScene*> scenes;
        QSet<CGraphicsView*> widgets;

        QPointer<CGraphicsView> focussedWidget;
        QHash<QString, CGraphicsItem*> toolbarInsertibles;
    };

    static bool areItemsEquivalent(CGraphicsItem *a, CGraphicsItem *b)
    {
        if (!a || !b) {
            return false;
        }
        if (a->type() != b->type()) {
            return false;
        }

        if (a->isComponent()) {
            Component *ac = canedaitem_cast<Component*>(a);
            Component *bc = canedaitem_cast<Component*>(b);

            return ac->library() == bc->library() &&
                ac->name() == bc->name();
        }

        // Implement for other kinds of comparison required to compare
        // insertibles and toolbarInsertibles of
        // StateHandlerPrivate class.
        return false;
    }

    StateHandler::StateHandler(QObject *parent) : QObject(parent)
    {
        d = new StateHandlerPrivate;

        LibraryLoader *loader = LibraryLoader::instance();
        connect(loader, SIGNAL(basicLibrariesLoaded()), this, SLOT(slotUpdateToolbarInsertibles()));
    }

    StateHandler* StateHandler::instance()
    {
        static StateHandler *instance = 0;
        if (!instance) {
            instance = new StateHandler(SingletonOwner::instance());
        }
        return instance;
    }

    StateHandler::~StateHandler()
    {
        delete d;
    }

    void StateHandler::registerWidget(CGraphicsView *widget)
    {
        CGraphicsScene *scene = widget->cGraphicsScene();
        if (!scene) {
            qWarning() << Q_FUNC_INFO << "Widget doesn't have an associated scene";
            return;
        }
        if (!d->widgets.contains(widget)) {
            d->widgets << widget;
            connect(widget, SIGNAL(destroyed(QObject*)), SLOT(slotOnObjectDestroyed(QObject*)));
            connect(widget, SIGNAL(focussedIn(CGraphicsView*)),
                    SLOT(slotUpdateFocussedWidget(CGraphicsView*)));
        }

        if (!d->scenes.contains(scene)) {
            d->scenes << scene;
            connect(scene, SIGNAL(destroyed(QObject*)), SLOT(slotOnObjectDestroyed(QObject*)));
            connect(scene, SIGNAL(rotateInvokedWhileInserting()),
                    SLOT(slotRotateInsertibles()));
            connect(scene, SIGNAL(mirrorInvokedWhileInserting()),
                    SLOT(slotMirrorInsertibles()));
        }
    }

    void StateHandler::unregisterWidget(CGraphicsView *widget)
    {
        if (!widget) {
            return;
        }
        if (d->widgets.contains(widget)) {
            d->widgets.remove(widget);
            disconnect(widget, SIGNAL(destroyed(QObject*)), this, SLOT(slotOnObjectDestroyed(QObject*)));
            disconnect(widget, SIGNAL(focussedIn(CGraphicsView*)), this,
                    SLOT(slotUpdateFocussedWidget(CGraphicsView*)));
        }

        CGraphicsScene *scene = widget->cGraphicsScene();
        if (scene && d->scenes.contains(scene)) {
            d->scenes.remove(scene);
            disconnect(scene, SIGNAL(destroyed(QObject*)), this, SLOT(slotOnObjectDestroyed(QObject*)));
            disconnect(scene, SIGNAL(rotateInvokedWhileInserting()), this,
                    SLOT(slotRotateInsertibles()));
            disconnect(scene, SIGNAL(mirrorInvokedWhileInserting()), this,
                    SLOT(slotMirrorInsertibles()));
        }
    }

    void StateHandler::slotSidebarItemClicked(const QString& item,
            const QString& category)
    {
        if (category == "Paint Tools" || category == "Layout Tools") {
            if (d->paintingDrawItem) {
                // Clear old item first
                if (d->paintingDrawItem->scene()) {
                    d->paintingDrawItem->scene()->removeItem(d->paintingDrawItem);
                }
                delete d->paintingDrawItem;
            }

            d->paintingDrawItem = Painting::fromName(item);
            if (!d->paintingDrawItem) {
                slotSetNormalAction();
            } else {
                d->paintingDrawItem->setPaintingRect(QRectF(0, 0, 0, 0));
                slotPerformToggleAction("paintingDraw", true);
            }
        }
        else {
            d->clearInsertibles();
            LibraryLoader *libLoader = LibraryLoader::instance();
            CGraphicsItem *qItem = libLoader->newComponent(item, 0, category);
            if (!qItem) {
                slotSetNormalAction();
            } else {
                d->insertibles << qItem;
                slotPerformToggleAction("insertItem", true);
            }
        }
    }

    void StateHandler::slotHandlePaste()
    {
        const QString text = qApp->clipboard()->text();

        Caneda::XmlReader reader(text.toUtf8());

        while(!reader.atEnd()) {
            reader.readNext();

            if(reader.isStartElement() && reader.name() == "caneda") {
                break;
            }
        }

        if(reader.hasError() || !(reader.isStartElement() && reader.name() == "caneda")) {
            return;
        }

        if(!Caneda::checkVersion(reader.attributes().value("version").toString())) {
            return;
        }

        QList<CGraphicsItem*> _items;
        while(!reader.atEnd()) {
            reader.readNext();

            if(reader.isEndElement()) {
                break;
            }

            if(reader.isStartElement()) {
                CGraphicsItem *readItem = 0;
                if(reader.name() == "component") {
                    readItem = Component::loadComponentData(&reader, 0);
                }
                else if(reader.name() == "wire") {
                    readItem = Wire::loadWireData(&reader, 0);
                }
                else if(reader.name() == "painting")  {
                    readItem = Painting::loadPainting(&reader, 0);
                }

                if(readItem) {
                    _items << readItem;
                }
            }
        }

        if (_items.isEmpty() == false) {
            d->clearInsertibles();
            d->insertibles = _items;
            slotPerformToggleAction("insertItem", true);
        }
    }

    void StateHandler::slotRotateInsertibles()
    {
        if (d->mouseAction != CGraphicsScene::InsertingItems) {
            qDebug() << Q_FUNC_INFO << "Wrong mouse action mode!";
            return;
        }

        // Utilize code available in undo command :-P
        RotateItemsCmd cmd(d->insertibles, Caneda::Clockwise);
        cmd.redo();

        // Now start a fresh insertion
        slotPerformToggleAction("insertItem", true);
    }

    void StateHandler::slotMirrorInsertibles()
    {
        if (d->mouseAction != CGraphicsScene::InsertingItems) {
            qDebug() << Q_FUNC_INFO << "Wrong mouse action mode!";
            return;
        }

        // Utilize code available in undo command :-P
        MirrorItemsCmd cmd(d->insertibles, Qt::XAxis);
        cmd.redo();

        // Now start a fresh insertion
        slotPerformToggleAction("insertItem", true);
    }

    void StateHandler::slotInsertToolbarComponent(const QString& sender,
            bool on)
    {
        CGraphicsItem *item = d->toolbarInsertibles[sender];
        if (!on || !item) {
            slotSetNormalAction();
            return;
        }

        d->clearInsertibles();
        d->insertibles << item->copy();
        slotPerformToggleAction("insertItem", true);
    }

    void StateHandler::slotOnObjectDestroyed(QObject *object)
    {
        //HACK: Using static cast to convert QObject pointers to scene and widget
        //      respectively. This might result in invalid pointers, but the main
        //      purpose why we need them is just to remove the same from the lists.
        //
        //      Using of these pointers to access any method or variable will result
        //      in ugly crash!!
        CGraphicsScene *scene = static_cast<CGraphicsScene*>(object);
        CGraphicsView *widget = static_cast<CGraphicsView*>(object);


        d->scenes.remove(scene);
        d->widgets.remove(widget);
    }

    void StateHandler::slotUpdateFocussedWidget(CGraphicsView *widget)
    {
        d->focussedWidget = widget;
    }

    /*!
     * \brief Toogles the action perfomed.
     *
     * This method toggles the action and calls the function pointed by
     * \a func if on is true. This method takes care to preserve the mutual
     * exclusiveness off the checkable actions.
     */
    void StateHandler::slotPerformToggleAction(const QString& sender, bool on)
    {
        typedef void (CGraphicsScene::*pActionFunc) (QList<CGraphicsItem*>&, const Caneda::UndoOption);

        ActionManager *am = ActionManager::instance();

        Action *action = am->actionForName(sender);
        CGraphicsScene::MouseAction ma = am->mouseActionForAction(action);
        pActionFunc func = 0;

        if (sender == "editDelete") {
            func = &CGraphicsScene::deleteItems;
        } else if (sender == "editRotate") {
            func = &CGraphicsScene::rotateItems;
        } else if (sender == "editMirror") {
            func = &CGraphicsScene::mirrorXItems;
        } else if (sender == "editMirrorY") {
            func = &CGraphicsScene::mirrorYItems;
        } else if (sender == "editActivate") {
            func = &CGraphicsScene::toggleActiveStatus;
        }

        QList<Action*> mouseActions = ActionManager::instance()->mouseActions();

        //toggling off any action switches normal select action "on"
        if(!on) {
            // Normal action can't be turned off through UI by clicking
            // the selct action again.
            slotSetNormalAction();
            return;
        }

        //else part
        CGraphicsScene *scene = 0;
        if (d->focussedWidget.isNull() == false) {
            scene = d->focussedWidget->cGraphicsScene();
        }
        QList<QGraphicsItem*> selectedItems;
        if (scene) {
            selectedItems = scene->selectedItems();
        }

        do {
            if(!selectedItems.isEmpty() && func != 0) {
                QList<CGraphicsItem*> funcable = filterItems<CGraphicsItem>(selectedItems);

                if(funcable.isEmpty()) {
                    break;
                }

                (scene->*func)(funcable, Caneda::PushUndoCmd);

                // Turn off this action
                slotPerformToggleAction(action->objectName(), false);
                return;
            }
        } while(false); //For break

        // Just ensure all other action's are off.
        foreach(Action *act, mouseActions) {
            if(act != action) {
                act->blockSignals(true);
                act->setChecked(false);
                act->blockSignals(false);
            }
        }

        QHash<QString, CGraphicsItem*>::const_iterator it =
            d->toolbarInsertibles.begin();
        while (it != d->toolbarInsertibles.end()) {
            Action *act = am->actionForName(it.key());
            act->blockSignals(true);
            act->setChecked(false);
            act->blockSignals(false);
            ++it;
        }

        if (sender == "insertItem" && d->insertibles.size() == 1) {
            for (it = d->toolbarInsertibles.begin();
                    it != d->toolbarInsertibles.end(); ++it) {
                if (areItemsEquivalent(it.value(), d->insertibles.first())) {
                    Action *act = am->actionForName(it.key());
                    act->blockSignals(true);
                    act->setChecked(true);
                    act->blockSignals(false);
                }
            }
        }

        // Ensure current action is on visibly
        action->blockSignals(true);
        action->setChecked(true);
        action->blockSignals(false);

        d->mouseAction = ma;
        applyStateToAllWidgets();
    }

    void StateHandler::slotSetNormalAction()
    {
        slotPerformToggleAction("select", true);
    }

    void StateHandler::slotUpdateToolbarInsertibles()
    {
        d->updateToolbarInsertibles();
    }

    void StateHandler::applyCursor(CGraphicsView *widget)
    {
        QCursor cursor;

        switch (d->mouseAction) {
            case CGraphicsScene::Wiring:
                cursor.setShape(Qt::CrossCursor);
                break;

            case CGraphicsScene::Deleting:
                cursor = QCursor(Caneda::icon("draw-eraser").pixmap(20));
                break;

            case CGraphicsScene::Rotating:
                cursor = QCursor(Caneda::icon("object-rotate-left").pixmap(20));
                break;

            case CGraphicsScene::MirroringX:
                cursor.setShape(Qt::SizeVerCursor);
                break;

            case CGraphicsScene::MirroringY:
                cursor.setShape(Qt::SizeHorCursor);
                break;

            case CGraphicsScene::ZoomingAreaEvent:
                cursor = QCursor(Caneda::icon("zoom-in").pixmap(20));
                break;

            case CGraphicsScene::PaintingDrawEvent:
                cursor.setShape(Qt::CrossCursor);
                break;

            case CGraphicsScene::InsertingItems:
                cursor.setShape(Qt::ClosedHandCursor);
                break;

            default:
                cursor.setShape(Qt::ArrowCursor);
        }

        widget->setCursor(cursor);
    }

    void StateHandler::applyState(CGraphicsView *widget)
    {
        applyCursor(widget);
        CGraphicsScene *scene = widget->cGraphicsScene();
        if (!scene) {
            return;
        }

        scene->setMouseAction(d->mouseAction);
        if (d->mouseAction == CGraphicsScene::InsertingItems) {
            if (!d->insertibles.isEmpty()) {
                QList<CGraphicsItem*> copy;
                foreach (CGraphicsItem *it, d->insertibles) {
                    copy << it->copy(scene);
                }
                scene->beginInsertingItems(copy);
            }
        } else if (d->mouseAction == CGraphicsScene::PaintingDrawEvent) {
            if (d->paintingDrawItem) {
                Painting *copy = d->paintingDrawItem->copy();
                scene->beginPaintingDraw(copy);
            }
        }
    }

    void StateHandler::applyStateToAllWidgets()
    {
        foreach (CGraphicsView *widget, d->widgets) {
            applyState(widget);
        }
    }

} // namespace Caneda