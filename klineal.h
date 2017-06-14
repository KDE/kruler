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

class QAction;
class QAutoSizeLabel;
class QIcon;
class QMenu;
class QToolButton;

class KActionCollection;
class KRulerSystemTray;

class KLineal : public QWidget {
  Q_OBJECT

public:
  enum { North = 0, West, South, East };

  KLineal( QWidget *parent = 0 );
  ~KLineal();

  void move( int x, int y );
  void move( const QPoint &p );
  QPoint pos() const;
  int x() const;
  int y() const;

protected:
  void keyPressEvent( QKeyEvent *e );
  void mousePressEvent( QMouseEvent *e );
  void mouseReleaseEvent( QMouseEvent *e );
  void mouseMoveEvent( QMouseEvent *e );
  void wheelEvent( QWheelEvent *e );
  void paintEvent( QPaintEvent *e );
  void enterEvent( QEvent *e );
  void leaveEvent( QEvent *e );

  void createSystemTray();

private:
  QAction* addAction( QMenu *menu, const QIcon& icon, const QString& text,
                      const QObject* receiver, const char* member,
                      const QKeySequence &shortcut, const QString& name );
  void drawScale( QPainter &painter );
  void drawBackground( QPainter &painter );
  void drawScaleText( QPainter &painter, int x, const QString &text );
  void drawScaleTick( QPainter &painter, int x, int length );
  void reLength( int percentOfScreen );
  void reLengthAbsolute( int length );
  void updateScaleDirectionMenuItem();

  bool mDragging;
  QPoint mLastClickPos;
  QPoint mDragOffset;
  QAutoSizeLabel *mLabel;
  int mOrientation;
  int mLongEdgeLen;
  int mShortEdgeLen;
  QMenu *mMenu;
  QAction *mCloseAction;
  QMenu *mLenMenu;
  QAction *mFullScreenAction;
  QAction *mScaleDirectionAction;
  QAction *mCenterOriginAction;
  QAction *mOffsetAction;
  QColor mColor;
  QCursor mCurrentCursor;
  QCursor mVerticalCursor;
  QCursor mHorizontalCursor;
  QCursor mDragCursor;
  QFont mScaleFont;
  bool mAlwaysOnTopLayer;
  bool mClicked;
  bool mLeftToRight;
  int mOffset;
  bool mRelativeScale;
  KActionCollection *mActionCollection;
  int mOpacity;
  KRulerSystemTray *mTrayIcon;

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
  void switchDirection();
  void centerOrigin();
  void slotOffset();
  void slotLength();
  void slotOpacity( int value );
  void slotKeyBindings();
  void slotPreferences();
  void switchRelativeScale( bool checked );
  void saveSettings();
  void slotClose();
  void slotQuit();
  void loadConfig();
};
#endif
