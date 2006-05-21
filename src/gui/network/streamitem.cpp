/****************************************************************
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

/** 
 * \file streamitem.cpp
 * \version $Id$
 */

#include "streamitem.h"
#include "circuitlistwidget.h"


/** Constructor */
StreamItem::StreamItem(Stream stream)
{
  /* Save the stream's id */
  _id = stream.id();
  
  /* Update the status and target */
  setText(CircuitListWidget::ConnectionColumn, stream.target());
  setText(CircuitListWidget::StatusColumn, stream.statusString());
}

/** Updates the status of this stream item. */
void
StreamItem::update(Stream stream)
{
  /* Only update the status. We leave the target alone so we can still see the
   * hostname even after the target address has been resolved. */
  setText(CircuitListWidget::StatusColumn, stream.statusString());
}

