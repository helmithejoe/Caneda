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

#include "textcontext.h"

#include "singletonowner.h"
#include "textdocument.h"
#include "textview.h"

#include <QFileInfo>
#include <QStringList>

namespace Caneda
{
    TextContext::TextContext(QObject *parent) : IContext(parent)
    {
    }

    TextContext* TextContext::instance()
    {
        static TextContext *instance = 0;
        if (!instance) {
            instance = new TextContext(SingletonOwner::instance());
        }

        return instance;
    }

    TextContext::~TextContext()
    {
    }

    void TextContext::init()
    {
    }

    bool TextContext::canOpen(const QFileInfo& info) const
    {
        return info.suffix() == "txt";
    }

    QStringList TextContext::fileNameFilters() const
    {
        return QStringList();
    }

    IDocument* TextContext::newDocument()
    {
        return new TextDocument;
    }

    IDocument* TextContext::open(const QString& fileName, QString *errorMessage)
    {
        TextDocument *document = new TextDocument();
        document->setFileName(fileName);

        if (!document->load(errorMessage)) {
            delete document;
            document = 0;
        }

        return document;
    }

} // namespace Caneda