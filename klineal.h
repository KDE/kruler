/*
    SPDX-FileCopyrightText: 2000-2008 Till Krech <till@snafu.de>
    SPDX-FileCopyrightText: 2009 Mathias Soeken <msoeken@tzi.de>
    SPDX-FileCopyrightText: 2017 Aurélien Gâteau <agateau@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KLINEAL_H
#define KLINEAL_H

#include <QWidget>

class QAction;
class QIcon;
class QMenu;

class KActionCollection;
class KRulerSystemTray;

class KLineal : public QWidget {
  Q_OBJECT

public:
  explicit KLineal( QWidget *parent = nullptr );
  ~KLineal() override;

  void move( int x, int y );
  void move( const QPoint &p );
  QPoint pos() const;
  int x() const;
  int y() const;

protected:
  void keyPressEvent( QKeyEvent *e ) override;
  void leaveEvent( QEvent *e ) override;
  void mousePressEvent( QMouseEvent *e ) override;
  void mouseMoveEvent( QMouseEvent *e ) override;
  void wheelEvent( QWheelEvent *e ) override;
  void paintEvent( QPaintEvent *e ) override;

  void createSystemTray();

private:
  void createCrossCursor();
  QAction* addAction( QMenu *menu, const QIcon& icon, const QString& text,
                      const QObject* receiver, const char* member,
                      const QKeySequence &shortcut, const QString& name );
  void drawScale( QPainter &painter );
  void drawBackground( QPainter &painter );
  void drawScaleText( QPainter &painter, int x, const QString &text );
  void drawScaleTick( QPainter &painter, int x, int length );
  void drawResizeHandle( QPainter &painter, Qt::Edge edge );
  void drawIndicatorOverlay( QPainter &painter, int xy );
  void drawIndicatorText( QPainter &painter, int xy );
  void updateScaleDirectionMenuItem();

  QRect beginRect() const;
  QRect endRect() const;
  Qt::CursorShape resizeCursor() const;
  bool nativeMove() const;
  void startNativeMove( QMouseEvent *e );
  void stopNativeMove( QMouseEvent *e );
  QString indicatorText() const;

  QCursor mCrossCursor;
  QPoint mLastClickPos;
  QPoint mDragOffset;
  bool mHorizontal = false;
  QMenu *mMenu = nullptr;
  QAction *mCloseAction = nullptr;
  QAction *mScaleDirectionAction = nullptr;
  QAction *mCenterOriginAction = nullptr;
  QAction *mOffsetAction = nullptr;
  QColor mColor;
  QFont mScaleFont;
  bool mAlwaysOnTopLayer = false;
  bool mClicked = false;
  bool mLeftToRight = false;
  int mOffset = 0;
  bool mRelativeScale = false;
  KActionCollection *mActionCollection = nullptr;
  int mOpacity = 0;
  KRulerSystemTray *mTrayIcon = nullptr;
  bool mWayland;

  void setHorizontal( bool horizontal );

  int length() const;
  QPoint localCursorPos() const;
  qreal pixelRatio() const;

public Q_SLOTS:
  void rotate();
  void showMenu();
  void switchDirection();
  void centerOrigin();
  void slotOffset();
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
