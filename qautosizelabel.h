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

#ifndef QAUTOSIZELABEL_H
#define QAUTOSIZELABEL_H

#include <QLabel>

class QAutoSizeLabel : public QLabel {
  Q_OBJECT

  public:
    explicit QAutoSizeLabel( QWidget *parent = 0, Qt::WindowFlags f = 0 );
    explicit QAutoSizeLabel( const QString &text, QWidget *parent = 0, Qt::WindowFlags f = 0 );
    virtual ~QAutoSizeLabel();

  public Q_SLOTS:
    void setText( const QString &text );

  private:
    void resize();
};

#endif
