/***************************************************************************
 * Copyright (C) 2013 by Pablo Daniel Pareja Obregon                       *
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

namespace Caneda
{
/*!
 * \page DocumentViewFramework Document-View framework
 *
 * Caneda's Document-View framework provides a convenient approach
 * to model-view programming, much like Qt's Graphics View Architecture.
 * The main intent is to allow the program to add any document type (vhdl,
 * pcb, etc) without having to manually change the bulk of the existing
 * code.
 *
 * The general idea is to have a new document type implement the
 * following three interfaces (technically QObject subclasses):
 *
 *   * IContext - This class provides an interface for a context which is
 * used by IDocument and IView. Only one instance of this class per
 * document type is used during the whole life span of the program.
 * This class answers the general questions about each document type,
 * like which file suffixes it can handle, points to the appropiate
 * methods to create new documents of its type, etc. This class also
 * provides objects like the toolbar, statusbar, etc, which are specific
 * to the particular context. The context class can also be used to host
 * functionalites shared by all views and documents of same type.
 *
 *   * IDocument - This class represents the actual document interface
 * (scene), in a manner similar to Qt's Graphics View Architecture,
 * serving as an interface for all documents that can be handled by
 * Caneda. This class manages document specific methods like saving,
 * loading, exporting to different formats, as well as containing the
 * actual scene. The scene itself may be included as a pointer to
 * another class that contains all the scene specific methods (for
 * example a graphics scene). The scene, in its turn, serves as a
 * container for item objects and handles their manipulation.
 *
 *   * IView - This class represents the view for a document, in a manner
 * similar to Qt's Graphics View Architecture. The IView class provides
 * the view widget, which visualizes the contents of a scene. The view
 * itself may be included as a pointer to another class that contains
 * all the view specific methods (for example a graphics view). You can
 * attach several views to the same scene, to provide different viewports
 * into the same data set of the document (for example, when using split
 * views).
 *
 * DocumentViewManager in coordination with MainWindow, Tab and TabWidget
 * classes, handle the bulk of the file open, split and close actions.
 *
 * Under this scheme, in addition to the 3 classes aforementioned, there
 * are two extra classes where most of the burden is put (for example
 * SchematicScene and SchematicView). Although conceptually these should be
 * part of the corresponding IDocument and IView class implementations
 * respectively (SchematicDocument and SchematicView in the example), there
 * are some reasons for keeping the separate.
 *
 * Initially the interface classes would just contain pure virtual
 * functions. However, in that case, signals and slots may not be used in
 * the interface unless the respective classes were a subclass of QObject.
 * Signals and slots are quite useful to model certain characteristics
 * like, for example, handling the focus of a view in DocumentViewManager
 * to update the current active view. Although hacks could be used (like
 * implementing the signal and slots only in the implementation classes and
 * connecting them by getting a QObject* representation), the interface
 * doesn't serve the purpose and we end up using two pointers for every
 * element usage (one IView* and one QObject*, for that example). The
 * classes QGraphicsScene, QTextDocument, etc, are themselves QObject
 * subclasses and subclassing them for that usage is inevitable. As such
 * one has to resort to virtual inheritance and multiple inheritance
 * everywhere. This again results in too many casts (one to interface and
 * another to one of QGraphicsScene, QGraphicsView, etc).
 *
 * That is when we realized the benefits of QtCreator's approach. They have
 * all the interface classes inherit QObject, and the implementations just
 * have to satisfy the interfaces. The bulk of the work is done in other
 * classes and the implementation just passes on the responsibility to that
 * classes wherever required (for example the undo in the interface
 * implementation invokes undo in the other class the object has used).
 * This is way more cleaner, and is the implementation chosen. Although
 * there is an increase in number of classes used, the resulting code
 * quality and readability increases a lot.
 */

} // namespace Caneda