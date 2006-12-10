/***************************************************************************
 * Copyright (C) 2006 by Gopala Krishna A <krishna.ggk@gmail.com>          *
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

#include "resistor.h"
#include "propertytext.h"
#include "schematicscene.h"

#include <QtGui/QPainter>
#include <QtGui/QStyle>
#include <QtGui/QStyleOptionGraphicsItem>

Resistor::Resistor(QGraphicsScene *s) : Component(0l,s)
{
   m_ports.append(new ComponentPort(this,QPointF(-30.0,0.0)));
   m_ports.append(new ComponentPort(this,QPointF(30.0,0.0)));
   
   PropertyText *t1 = new PropertyText("R","100k","Simple resistor",0l,scene());
   m_properties.append(t1);
   t1->setPos(scenePos() + QPointF(0,-35));
}

QString Resistor::name() const
{
   return QString("R");
}

QString Resistor::model() const
{
   return QString("R");
}

QString Resistor::text() const
{
   return QObject::tr("R");
}

QString Resistor::netlist() const
{
   return QString("R");
}

void Resistor::paint(QPainter *p, const QStyleOptionGraphicsItem *o, QWidget *w)
{
   Q_UNUSED(w);
   p->drawLine(-30,  0,-18,  0);
   p->drawLine(-18,  0,-15, -7);
   p->drawLine(-15, -7, -9,  7);
   p->drawLine( -9,  7, -3, -7);
   p->drawLine( -3, -7,  3,  7);
   p->drawLine(  3,  7,  9, -7);
   p->drawLine(  9, -7, 15,  7);
   p->drawLine( 15,  7, 18,  0);
   p->drawLine( 18,  0, 30,  0);

   if(o->state & QStyle::State_Selected)
   {
      p->setPen(QPen(Qt::darkGray,2));
      p->drawRect(boundingRect());
   }
      
}

QRectF Resistor::boundingRect() const
{
   return QRectF(-30,-7,60,14).adjusted(-6,-1,+6,+1);
}
