/***************************************************************************
 * Copyright (C) 2007 by Gopala Krishna A <krishna.ggk@gmail.com>          *
 * Copyright (C) 2012-2013 by Pablo Daniel Pareja Obregon                  *
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

#ifndef XMLUTILITIES_H
#define XMLUTILITIES_H

#include <QByteArray>
#include <QMap>
#include <QPolygonF>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

// Forward declarations
class QBrush;
class QFont;
class QLineF;
class QPen;
class QRectF;
class QSize;
class QTransform;

namespace Caneda
{
    /*!
     * \brief This class adds some helper methods to Qt's QXmlStreamReader
     * class.
     *
     * This class contains some methods used to extend the functionality of
     * Qt's QXmlStreamReader class, for example to read point attributes,
     * graphic items transforms, double attributes, etc.
     *
     * \warning QXmlStreamReader doesn't have virtual destructor. Don't delete
     * any instance of this class from base pointer.
     *
     * \sa XmlWriter
     */
    class XmlReader : public QXmlStreamReader
    {
    public:
        //! Constructs an xml stream reader acting on \a data.
        explicit XmlReader(const QByteArray & data) : QXmlStreamReader(data) {}

        int readInt();
        double readDouble();

        QPointF readPoint();

        QPointF readPointAttribute(QString tag = "point");
        QLineF readLineAttribute(QString tag = "line");
        QRectF readRectAttribute(QLatin1String tag = QLatin1String("rect"));
        QTransform readTransformAttribute(QString tag = "transform");
        qreal readDoubleAttribute(QString tag);

        QSize readSize();

        QRectF readRect();
        QTransform readTransform();

        QPen readPen();
        QBrush readBrush();
        QFont readFont();

        QString readLocaleText(const QString& localePrefix = QString("C"));

        void readFurther();
        void readUnknownElement();
    };

    /*!
     * \brief This class adds some helper methods to Qt's QXmlStreamWriter
     * class.
     *
     * This class contains some methods used to extend the functionality of
     * Qt's QXmlStreamWriter class, for example to write point attributes,
     * graphic items, etc.

     * \warning QXmlStreamWriter doesn't have virtual destructor. Don't delete
     * any instance of this class from base pointer.
     *
     * \sa XmlReader
     */
    class XmlWriter : public QXmlStreamWriter
    {
    public:
        //! Constructs an xml stream writer acting on \a device.
        explicit XmlWriter(QIODevice *device) : QXmlStreamWriter(device) {}
        //! Constructs an xml stream writer acting on \a bytearray.
        explicit XmlWriter(QByteArray *bytearray) : QXmlStreamWriter(bytearray) {}
        //! Constructs an xml stream writer acting on \a string.
        explicit XmlWriter(QString *string) : QXmlStreamWriter(string) {}

        void writeElement(const QString& tag, const QString& value);
        void writeElement(const QString& tag, int value);
        void writeElement(const QString& tag, qreal value);
        void writeElement(const QString& tag, bool value);

        void writeRect(const QRectF& rect, QString tag = "rect");
        void writeTransform(const QTransform& transform);
        void writeSize(const QSize& size, QString tag = "size");
        void writePoint(const QPointF& point, QString tag = "point");

        void writePointAttribute(const QPointF& point, QString tag = "point");
        void writeLineAttribute(const QLineF& line, QLatin1String tag = QLatin1String("line"));
        void writeRectAttribute(const QRectF& rect, QLatin1String tag = QLatin1String("rect"));
        void writePolygonAttribute(const QPolygonF& polygon, QString tag="polygon");
        void writeTransformAttribute(const QTransform& transform, QString tag = "transform");

        void writePen(const QPen& pen, QLatin1String tag = QLatin1String("pen"));
        void writeBrush(const QBrush& brush, QLatin1String tag = QLatin1String("brush"));
        void writeFont(const QFont& font, QLatin1String tag = QLatin1String("font"));

        void writeLocaleText(const QString &lang, const QString& value);
    };

} // namespace Caneda

#endif //XMLUTILITIES_H
