/***************************************************************************
                          klineal.h  -  description
                             -------------------
    begin                : Fri Oct 13 2000
    copyright            : (C) 2000 by Till Krech
    email                : till@snafu.de
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

#include <kapplication.h>
#include <kpopupmenu.h>
#include <kmainwindow.h>

#include <kcolordialog.h>
#include <kfontdialog.h>

#include <qlabel.h>
#include <qpainter.h>
#include <qwidget.h>
#include <qcursor.h>

class KLineal : public KMainWindow {
  Q_OBJECT
public:
  enum { North=0, West=1, South=2, East=3 };
  /** constructor */
  KLineal(QWidget*parent=0,const char* name=0);
  /** destructor */
  ~KLineal();
  void move(int x, int y);
  void move(const QPoint &p);
  QPoint pos();
  int x();
  int y();
protected:
	void keyPressEvent(QKeyEvent *e);
  void mousePressEvent(QMouseEvent *e);
  void mouseReleaseEvent(QMouseEvent *e);
  void mouseMoveEvent(QMouseEvent *e);
  void paintEvent(QPaintEvent *e);
  void enterEvent(QEvent *e);
  void leaveEvent(QEvent *e);
  void setupBackground();


private:
  void drawScale(QPainter &painter);
  void reLength(int percentOfScreen);
  bool mDragging;
  QPoint mLastClickPos;
  QPoint mDragOffset;
  QLabel *mLabel;
  QLabel *mColorLabel;
  QFrame *mColorRect;
  int mOrientation;
  int mLongEdgeLen;
  int mShortEdgeLen;
  KPopupMenu *mMenu;
  KPopupMenu *mLenMenu;
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
  void setOrientation(int);
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
  void setFont(QFont &);
  void setColor(const QColor &color);
  void choseColor();
  void choseFont();
  void restoreColor();
  void saveSettings();
};
#endif
