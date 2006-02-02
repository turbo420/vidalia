/****************************************************************
 *  $Id$
 * 
 *  Vidalia is distributed under the following license:
 *
 *  Copyright (C) 2006,  Matt Edman, Justin Hipple
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version 2
 *  of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, 
 *  Boston, MA  02110-1301, USA.
 ****************************************************************/

#include "graphframe.h"

/** Default contructor **/
GraphFrame::GraphFrame(QWidget *parent)
: QFrame(parent)
{
  /* Create Graph Frame related objects */
  _recvData = new QList<qreal>();
  _sendData = new QList<qreal>();
  
  /* Initialize graph values */
  _maxPoints = getNumPoints();  
  _showRecv = true;
  _showSend = true;
  _maxValue = MIN_SCALE; 
}

/** Default destructor **/
GraphFrame::~GraphFrame()
{
  if (_recvData) {
    delete _recvData;
  }

  if (_sendData) {
    delete _sendData;
  }
}

/**
 Gets the width of the desktop, which is the maximum 
 number of points we can plot in the graph
**/
int
GraphFrame::getNumPoints()
{
  QDesktopWidget *desktop = QApplication::desktop();
  int width = desktop->width();
  return width;
}

/**
 Adds new data points to the graph
**/
void
GraphFrame::addPoints(qreal send, qreal recv)
{
  /* If maximum number of points plotted, remove oldest */
  if (_sendData->size() == _maxPoints) {
    _sendData->removeLast();
    _recvData->removeLast();
  }

  /* Add the points to their respective lists */
  _sendData->prepend(send);
  _recvData->prepend(recv);
  
  /* Check for a new maximum value */
  if (send > _maxValue) _maxValue = send;
  if (recv > _maxValue) _maxValue = recv;

  this->update();
}

/**
 Clears the graph
**/
void
GraphFrame::resetGraph()
{
  _recvData->clear();
  _sendData->clear();
  _maxValue = MIN_SCALE;
  this->update();
}

/**
 Toggles display of respective graph lines and counters
**/
void
GraphFrame::setShowCounters(bool showRecv, bool showSend)
{
  _showRecv = showRecv;
  _showSend = showSend;
  this->update();
}

/** 
 Overloads default QWidget::paintEvent
 Draws the actual bandwidth graph 
**/
void
GraphFrame::paintEvent(QPaintEvent *event)
{
  QPainter* painter = new QPainter(this);
  
  /* Fill in the background */
  painter->fillRect(this->frameRect(), QBrush(Qt::black));
  painter->drawRect(this->frameRect());
  
  /* Paint the gridlines */
  paintGrid(painter);
  
  /* Paint the scale */
  paintScale(painter);
  
  /* Paint the send/receive lines */
  paintLines(painter);

  /* Paint the send/receive rate indicators */
  paintRates(painter);

  delete painter;
}

/**
 Paints the selected graph lines on the graph
**/
void
GraphFrame::paintLines(QPainter* painter)
{
  /* If show received rate is selected */
  if (_showRecv) {
    painter->setPen(QColor(RECV_COLOR));
    // Draw line
  }

  /* If show send rate is selected */
  if (_showSend) {
    painter->setPen(QColor(SEND_COLOR));
    // Draw line
  }
}

/**
 Paints selected rate indicators on the graph
**/
void
GraphFrame::paintRates(QPainter* painter)
{
  int rateX = SCALE_WIDTH + FONT_SIZE;
  int rateY = this->frameRect().y() + FONT_SIZE;
  
  /* If received rate is selected */
  if (_showRecv) {
    painter->setPen(QColor(RECV_COLOR));
    qreal recv = _recvData->value(0);
    painter->drawText(QPoint(rateX, rateY), tr("%1 kB/s").arg(recv));
  }

  /* If send rate is selected */
  if (_showSend) {
    painter->setPen(QColor(SEND_COLOR));
    qreal send = _sendData->value(0);
    painter->drawText(QPoint(rateX, rateY+FONT_SIZE), tr("%1 kB/s").arg(send));
  }
}

/**
 Paints the scale on the graph
**/
void
GraphFrame::paintScale(QPainter* painter)
{
  painter->setPen(Qt::green);
  
  qreal markStep = _maxValue * .25;
  int top = this->frameRect().y();
  int bottom = this->frameRect().y() + this->frameRect().height();
  int paintStep = bottom / 4;
  
  /* Draw vertical separator */
  painter->drawLine(SCALE_WIDTH, top, SCALE_WIDTH, bottom);
  
  /* Draw the other marks in their correctly scaled locations */
  qreal scale;
  int pos;
  for (int i = 1; i < 5; i++) {
    pos = bottom - ((i * paintStep));
    scale = i * markStep;
    painter->drawLine(SCALE_WIDTH-5, pos, SCALE_WIDTH, pos);
    painter->drawText(QPoint(5, pos+FONT_SIZE), 
                      tr("%1 kB/s").arg(scale, 0, 'f', 2));
  }
}

/**
 Paints grid lines in the bandwidth graph
**/
void
GraphFrame::paintGrid(QPainter* painter)
{
  painter->setPen(Qt::darkGreen);
  QRect rec = this->frameRect();
  int x, y;
  
  /* Draw horizontal grid lines */
  y = (rec.y() + rec.height()) - GRID_Y;
  while (y > rec.y()) {
    painter->drawLine(SCALE_WIDTH, y, rec.width(), y);
    y -= GRID_Y;
  }

  /* Draw vertical grid lines */
  x = (rec.x() + rec.width()) - GRID_X;
  while (x > SCALE_WIDTH) {
    painter->drawLine(x, rec.y(), x, (rec.y() + rec.height()));
    x -= GRID_X;
  }
}

