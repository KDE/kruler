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

class QAutoSizeLabel;
class QToolButton;

class KAction;
class KActionCollection;
class KIcon;
class KMenu;
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
  KAction* addAction( KMenu *menu, KIcon icon, const QString& text,
                      const QObject* receiver, const char* member,
                      const QKeySequence &shortcut, const QString& name );
  void drawScale( QPainter &painter );
  void drawBackground( QPainter &painter );
  void reLength( int percentOfScreen );
  void reLengthAbsolute( int length );
  void updateScaleDirectionMenuItem();

  bool mDragging;
  QPoint mLastClickPos;
  QPoint mDragOffset;
  QAutoSizeLabel *mLabel;
  QAutoSizeLabel *mColorLabel;
  int mOrientation;
  int mLongEdgeLen;
  int mShortEdgeLen;
  KMenu *mMenu;
  QAction *mCloseAction;
  KMenu *mLenMenu;
  QAction *mFullScreenAction;
  QAction *mScaleDirectionAction;
  QAction *mCenterOriginAction;
  QAction *mOffsetAction;
  QColor mColor;
  QCursor mCurrentCursor;
  QCursor mNorthCursor;
  QCursor mEastCursor;
  QCursor mWestCursor;
  QCursor mSouthCursor;
  QCursor mDragCursor;
  QFont mScaleFont;
  bool mClicked;
  bool mLeftToRight;
  int mOffset;
  bool mRelativeScale;
  KActionCollection *mActionCollection;
  int mOpacity;
  QToolButton *mBtnRotateLeft, *mBtnRotateRight;
  QToolButton *mCloseButton;
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
  void adjustButtons();
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
  void copyColor();
  void saveSettings();
  void slotClose();
  void slotQuit();
  void loadConfig();
};
#endif
