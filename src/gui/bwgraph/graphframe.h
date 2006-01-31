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

#ifndef _GRAPHFRAME_H
#define _GRAPHFRAME_H

#include <QApplication>
#include <QDesktopWidget>
#include <QFrame>
#include <QPainter>
#include <QPen>
#include <QVector>
#include "linetypes.h"
#include "../../config/vidaliasettings.h"

#define GRID_X        12  // Width of a grid cell
#define GRID_Y        20  // Height of a grid cell
#define HOR_SPC       2   // Space between data points
#define SCALE_WIDTH   68  // Width of the scale 

#define RECV_COLOR    90,90,0
#define SEND_COLOR    60,60,255

#define FONT_SIZE     10
#define FONT_FACE     "Arial"

#define ABS(x)  ((x)<0 ? -(x) : (x))

class GraphFrame : public QFrame
{
  Q_OBJECT

public:
  /** Default Constructor **/
  GraphFrame(QWidget *parent = 0);
  /** Default Destructor **/
  ~GraphFrame();
  /** Add data points **/
  void addPoints(quint64 send, quint64 recv);
  /** Clears the graph **/
  void resetGraph();

protected:
  /** Overloaded QWidget::paintEvent() **/
  void paintEvent(QPaintEvent *event);

private:
  /** Gets the width of the desktop, the max # of points **/
  int getNumPoints();
  /** Paints grid lines in the graph **/
  void paintGrid(QPainter* painter);
  /** Paints send/receive rate indicators **/
  void paintRates(QPainter* painter, uint filter);
  /** Paints the scale in the graph **/
  void paintScale(QPainter* painter);

  /** Holds the received data points **/
  QVector<quint64> *_recvData;
  /** Holds the sent data points **/
  QVector<quint64> *_sendData;
  /** The maximum number of points to store **/
  int maxPoints;
  
  /** A VidaliaSettings object that tells us what to draw **/
  VidaliaSettings* _settings;
};

#endif
