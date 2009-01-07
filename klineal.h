/***************************************************************************
                          klineal.h  -  description
                             -------------------
    begin                : Fri Oct 13 2000
    Copyright            : (C) 2000 - 2008 by Till Krech <till@snafu.de>
                           (C) 2009        by Mathias Soeken <msoeken@tzi.de>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KLINEAL_H
#define KLINEAL_H

#include <QWidget>

#include <KColorDialog>

class QAutoSizeLabel;
class KMenu;

class KLineal : public QWidget {
  Q_OBJECT

public:
  enum { North = 0, West, South, East };

  KLineal( QWidget *parent = 0 );
  ~KLineal();

  void move( int x, int y );
  void move( const QPoint &p );
  QPoint pos();
  int x();
  int y();

protected:
  void keyPressEvent( QKeyEvent *e );
  void mousePressEvent( QMouseEvent *e );
  void mouseReleaseEvent( QMouseEvent *e );
  void mouseMoveEvent( QMouseEvent *e );
  void paintEvent( QPaintEvent *e );
  void enterEvent( QEvent *e );
  void leaveEvent( QEvent *e );


private:
  void drawScale( QPainter &painter );
  void drawBackground( QPainter &painter );
  void reLength( int percentOfScreen );
  bool mDragging;
  QPoint mLastClickPos;
  QPoint mDragOffset;
  QAutoSizeLabel *mLabel;
  QAutoSizeLabel *mColorLabel;
  int mOrientation;
  int mLongEdgeLen;
  int mShortEdgeLen;
  KMenu *mMenu;
  KMenu *mLenMenu;
  QAction *mFullScreenAction;
  QColor mColor;
  QColor mStoredColor;
  QCursor mCurrentCursor;
  QCursor mNorthCursor;
  QCursor mEastCursor;
  QCursor mWestCursor;
  QCursor mSouthCursor;
  QCursor mDragCursor;
  KColorDialog mColorSelector;
  QFont mScaleFont;
  bool _clicked;

public slots:
  void setOrientation( int );
  void setNorth();
  void setEast();
  void setSouth();
  void setWest();
  void turnLeft();
  void turnRight();
  void showMenu();
  void hideLabel();
  void showLabel();
  void adjustLabel();
  void setShortLength();
  void setMediumLength();
  void setTallLength();
  void setFullLength();
  void setColor();
  void setFont( const QFont &font );
  void setColor( const QColor &color );
  void chooseColor();
  void chooseFont();
  void restoreColor();
  void saveSettings();
  void slotQuit();
};
#endif
