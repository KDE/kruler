////////////////////////////////////////////////////////////////////////////
//                                                                        //
// Copyright (C)  2008  Mathias Soeken <msoeken@informatik.uni-bremen.de> //
//                                                                        //
// This program is free software; you can redistribute it and/or          //
// modify it under the terms of the GNU General Public License            //
// as published by the Free Software Foundation; either version 2         //
// of the License, or (at your option) any later version.                 //
//                                                                        //
// This program is distributed in the hope that it will be useful,        //
// but WITHOUT ANY WARRANTY; without even the implied warranty of         //
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          //
// GNU General Public License for more details.                           //
//                                                                        //
// You should have received a copy of the GNU General Public License      //
// along with this program; if not, write to the Free Software            //
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA          //
// 02110-1301, USA.                                                       //
////////////////////////////////////////////////////////////////////////////

#include "qautosizelabel.h"

#include <QFontMetrics>

QAutoSizeLabel::QAutoSizeLabel( QWidget *parent, Qt::WindowFlags f )
  : QLabel( parent,  f )
{
  resize();
}

QAutoSizeLabel::QAutoSizeLabel( const QString &text, QWidget *parent, Qt::WindowFlags f )
  : QLabel( text, parent,  f )
{
  resize();
}

QAutoSizeLabel::~QAutoSizeLabel()
{
}

void QAutoSizeLabel::setText( const QString &text )
{
  QLabel::setText( text );
  resize();
}

void QAutoSizeLabel::resize()
{
  QFontMetrics fm( font(), this );
  QLabel::resize( fm.size( Qt::TextSingleLine, text() ) );
}

#include "qautosizelabel.moc"
