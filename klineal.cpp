/***************************************************************************
                          klineal.cpp  -  description
                             -------------------
    Begin                : Fri Oct 13 2000
    Copyright            : 2000 by Till Krech <till@snafu.de>
                           2008 by Mathias Soeken <msoeken@informatik.uni-bremen.de>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "klineal.h"

#include <QAction>
#include <QApplication>
#include <QBitmap>
#include <QBrush>
#include <QClipboard>
#include <QDesktopWidget>
#include <QFontDatabase>
#include <QInputDialog>
#include <QMenu>
#include <QMouseEvent>
#include <QPainter>
#include <QScreen>
#include <QSlider>
#include <QToolButton>
#include <QWidgetAction>

#include <KAboutData>
#include <KActionCollection>
#include <KConfig>
#include <KConfigDialog>
#include <KHelpClient>
#include <KHelpMenu>
#include <KLocalizedString>
#include <KNotification>
#include <KShortcutsDialog>
#include <KStandardAction>
#include <KWindowSystem>

#include "krulerconfig.h"

#ifdef KRULER_HAVE_X11
#include <QX11Info>
#include <netwm.h>
#endif

#include "kruler.h"
#include "krulersystemtray.h"
#include "qautosizelabel.h"

#include "ui_cfg_appearance.h"
#include "ui_cfg_advanced.h"

static const int RESIZE_HANDLE_LEN = 7;
static const int RESIZE_HANDLE_SHORT_LEN = 24;
static const qreal RESIZE_HANDLE_OPACITY = 0.3;

/**
 * create the thingy with no borders and set up
 * its members
 */
KLineal::KLineal( QWidget *parent )
  : QWidget( parent ),
    mRulerState( StateNone ),
    mShortEdgeLen( 70 ),
    mCloseAction( 0 ),
    mLenMenu( 0 ),                // INFO This member could be eventually deleted
                                  // since if mFullScreenAction is initialized
                                  // mLenMenu should have been, too.
    mFullScreenAction( 0 ),
    mScaleDirectionAction( 0 ),
    mCenterOriginAction( 0 ),
    mOffsetAction( 0 ),
    mClicked( false ),
    mActionCollection( 0 ),
    mTrayIcon( 0 )
{
  setAttribute( Qt::WA_TranslucentBackground );
  KWindowSystem::setType( winId(), NET::Override );   // or NET::Normal
  KWindowSystem::setState( winId(), NET::KeepAbove );

  setWindowTitle( i18nc( "@title:window", "KRuler" ) );

  setMinimumSize( 60, 60 );
  setMaximumSize( 8000, 8000 );
  setWhatsThis( i18n( "This is a tool to measure pixel distances on the screen. "
                      "It is useful for working on layouts of dialogs, web pages etc." ) );
  setMouseTracking( true );

  mColor = RulerSettings::self()->bgColor();
  mScaleFont = RulerSettings::self()->scaleFont();
  mLongEdgeLen = RulerSettings::self()->length();
  mHorizontal = RulerSettings::self()->horizontal();
  mLeftToRight = RulerSettings::self()->leftToRight();
  mOffset = RulerSettings::self()->offset();
  mRelativeScale = RulerSettings::self()->relativeScale();
  mAlwaysOnTopLayer = RulerSettings::self()->alwaysOnTop();

  mLabel = new QAutoSizeLabel( this );
  mLabel->setWhatsThis( i18n( "This is the current distance measured in pixels." ) );

  resize( QSize( mLongEdgeLen, mShortEdgeLen ) );

  //BEGIN setup menu and actions
  mActionCollection = new KActionCollection( this );
  mActionCollection->setConfigGroup( QStringLiteral( "Actions" ) );

  mMenu = new QMenu( this );
  mMenu->addSection( i18n( "KRuler" ) );
  addAction( mMenu, QIcon::fromTheme( QStringLiteral( "object-rotate-left" ) ), i18n( "Rotate" ),
             this, SLOT(rotate()), Qt::Key_R, QStringLiteral( "turn_right" ) );

  mLenMenu = new QMenu( i18n( "&Length" ), this );
  addAction( mLenMenu, QIcon(), i18nc( "Make Kruler Height Short", "&Short" ),
             this, SLOT(setShortLength()), Qt::CTRL + Qt::Key_S, QStringLiteral( "length_short" ) );
  addAction( mLenMenu, QIcon(), i18nc( "Make Kruler Height Medium", "&Medium" ),
             this, SLOT(setMediumLength()), Qt::CTRL + Qt::Key_M, QStringLiteral( "length_medium" ) );
  addAction( mLenMenu, QIcon(), i18nc( "Make Kruler Height Tall", "&Tall" ),
             this, SLOT(setTallLength()), Qt::CTRL + Qt::Key_T, QStringLiteral( "length_tall" ) );
  addAction( mLenMenu, QIcon(), i18n("&Full Screen Width"),
             this, SLOT(setFullLength()), Qt::CTRL + Qt::Key_F, QStringLiteral( "length_full_length" ) );
  mLenMenu->addSeparator();
  addAction( mLenMenu, QIcon(), i18n( "Length..." ),
             this, SLOT(slotLength()), QKeySequence(), QStringLiteral( "set_length" ) );
  mMenu->addMenu( mLenMenu );

  QMenu* scaleMenu = new QMenu( i18n( "&Scale" ), this );
  mScaleDirectionAction = addAction( scaleMenu, QIcon(), i18n( "Right to Left" ),
                                     this, SLOT(switchDirection()), Qt::Key_D, QStringLiteral( "right_to_left" ) );
  mCenterOriginAction = addAction( scaleMenu, QIcon(), i18n( "Center Origin" ),
                                   this, SLOT(centerOrigin()), Qt::Key_C, QStringLiteral( "center_origin" ) );
  mCenterOriginAction->setEnabled( !mRelativeScale );
  mOffsetAction = addAction( scaleMenu, QIcon(), i18n( "Offset..." ),
                             this, SLOT(slotOffset()), Qt::Key_O, QStringLiteral( "set_offset" ) );
  mOffsetAction->setEnabled( !mRelativeScale );
  scaleMenu->addSeparator();
  QAction *relativeScaleAction = addAction( scaleMenu, QIcon(), i18n( "Percentage" ),
                                            0, 0, QKeySequence(), QStringLiteral( "toggle_percentage" ) );
  relativeScaleAction->setCheckable( true );
  relativeScaleAction->setChecked( mRelativeScale );
  connect(relativeScaleAction, &QAction::toggled, this, &KLineal::switchRelativeScale);
  mMenu->addMenu( scaleMenu );

  mOpacity = RulerSettings::self()->opacity();
  QMenu* opacityMenu = new QMenu( i18n( "O&pacity" ), this );
  QWidgetAction *opacityAction = new QWidgetAction( this );
  QSlider *slider = new QSlider( this );
  slider->setMinimum( 0 );
  slider->setMaximum( 255 );
  slider->setSingleStep( 1 );
  slider->setOrientation( Qt::Horizontal );
  slider->setValue( RulerSettings::self()->opacity() );
  connect(slider, &QSlider::valueChanged, this, &KLineal::slotOpacity);
  opacityAction->setDefaultWidget( slider );
  opacityMenu->addAction( opacityAction );
  mMenu->addMenu( opacityMenu );

  QAction *keyBindings = mActionCollection->addAction( KStandardAction::KeyBindings, this, SLOT(slotKeyBindings()) );
  mMenu->addAction( keyBindings );
  QAction *preferences = mActionCollection->addAction( KStandardAction::Preferences, this, SLOT(slotPreferences()) );
  mMenu->addAction( preferences );
  mMenu->addSeparator();
  mMenu->addMenu( ( new KHelpMenu( this, KAboutData::applicationData(), true ) )->menu() );
  mMenu->addSeparator();
  if ( RulerSettings::self()->trayIcon() ) {
      createSystemTray();
  }

  QAction *quit = mActionCollection->addAction( KStandardAction::Quit, qApp, SLOT(quit()) );
  mMenu->addAction( quit );

  mActionCollection->associateWidget( this );
  mActionCollection->readSettings();

  mLastClickPos = geometry().topLeft() + QPoint( width() / 2, height() / 2 );

  setWindowFlags( mAlwaysOnTopLayer ? Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint
                                    : Qt::FramelessWindowHint );

  setHorizontal( mHorizontal );
  adjustLabel();
}

KLineal::~KLineal()
{
  delete mTrayIcon;
}

void KLineal::createSystemTray()
{
  mCloseAction = mActionCollection->addAction( KStandardAction::Close, this, SLOT(slotClose()) );
  mMenu->addAction( mCloseAction );

  if ( !mTrayIcon ) {
    mTrayIcon = new KRulerSystemTray( QStringLiteral( "kruler" ), this, mActionCollection );
    mTrayIcon->setCategory( KStatusNotifierItem::ApplicationStatus );
  }
}


QAction* KLineal::addAction( QMenu *menu, const QIcon& icon, const QString& text,
                             const QObject* receiver, const char* member,
                             const QKeySequence &shortcut, const QString& name )
{
  QAction *action = new QAction( icon, text, mActionCollection );
  mActionCollection->setDefaultShortcut( action, shortcut );
  if ( receiver ) {
    connect( action, SIGNAL(triggered()), receiver, member );
  }
  menu->addAction( action );
  mActionCollection->addAction( name, action );
  return action;
}

void KLineal::slotClose()
{
  hide();
}

void KLineal::slotQuit()
{
   qApp->quit();
}

void KLineal::move( int x, int y )
{
  move( QPoint( x, y ) );
}

void KLineal::move(const QPoint &p)
{
  setGeometry( QRect( p, size() ) );
}

QPoint KLineal::pos() const
{
  return frameGeometry().topLeft();
}

int KLineal::x() const
{
  return pos().x();
}

int KLineal::y() const
{
  return pos().y();
}

void KLineal::drawBackground( QPainter& painter )
{
  QColor a, b, bg = mColor;
  QLinearGradient gradient;
  if ( mHorizontal ) {
    a = bg.light( 120 );
    b = bg.dark( 130 );
    gradient = QLinearGradient( 1, 0, 1, height() );
  } else {
    a = bg.light( 120 );
    b = bg.dark( 130 );
    gradient = QLinearGradient( 0, 1, width(), 1 );
  }
  a.setAlpha( mOpacity );
  b.setAlpha( mOpacity );
  gradient.setColorAt( 0, a );
  gradient.setColorAt( 1, b );
  painter.fillRect( rect(), QBrush( gradient ) );
}

void KLineal::setHorizontal( bool horizontal )
{
  QRect r = frameGeometry();
  mHorizontal = horizontal;
  QPoint center = mLastClickPos;

  if ( mClicked ) {
    center = mLastClickPos;
    mClicked = false;
  } else {
    center = r.topLeft() + QPoint( width() / 2, height() / 2 );
  }

  if ( mHorizontal ) {
    r.setSize( QSize( mLongEdgeLen, mShortEdgeLen ) );
  } else {
    r.setSize( QSize( mShortEdgeLen, mLongEdgeLen ) );
  }

  QPoint newTopLeft = QPoint( center.x() - height() / 2, center.y() - width() / 2 );
  r.moveTo(newTopLeft);

  QRect desktop = QApplication::desktop()->screenGeometry( this );

  if ( r.top() < desktop.top() ) {
    r.moveTop( desktop.top() );
  }

  if ( r.bottom() > desktop.bottom() ) {
    r.moveBottom( desktop.bottom() );
  }

  if ( r.left() < desktop.left() ) {
    r.moveLeft( desktop.left() );
  }

  if ( r.right() > desktop.right() ) {
    r.moveRight( desktop.right() );
  }

  setGeometry( r );
  adjustLabel();

  if ( mLenMenu && mFullScreenAction ) {
    mFullScreenAction->setText( mHorizontal ? i18n( "&Full Screen Width" ) : i18n( "&Full Screen Height" ) );
  }

  updateScaleDirectionMenuItem();

  repaint();
  saveSettings();
}

void KLineal::rotate()
{
  setHorizontal( !mHorizontal );
}

void KLineal::reLength( int percentOfScreen )
{
  if ( percentOfScreen < 10 ) {
    return;
  }

  QRect r = QApplication::desktop()->screenGeometry( this );

  if ( mHorizontal ) {
    mLongEdgeLen = r.width() * percentOfScreen / 100;
    resize( mLongEdgeLen, height() );
  } else {
    mLongEdgeLen = r.height() * percentOfScreen / 100;
    resize( width(), mLongEdgeLen );
  }

  if ( x() + width() < 10 ) {
    move( 10, y() );
  }

  if ( y() + height() < 10 ) {
    move( x(), 10 );
  }

  saveSettings();
}

void KLineal::reLengthAbsolute( int length )
{
  if ( length < 100 ) {
    return;
  }

  mLongEdgeLen = length;
  if ( mHorizontal ) {
    resize( mLongEdgeLen, height() );
  } else {
    resize( width(), mLongEdgeLen );
  }

  if ( x() + width() < 10 ) {
    move( 10, y() );
  }

  if ( y() + height() < 10 ) {
    move( x(), 10 );
  }

  saveSettings();
}

void KLineal::updateScaleDirectionMenuItem()
{
  if ( !mScaleDirectionAction ) return;

  QString label;

  if ( mHorizontal ) {
    label = mLeftToRight ? i18n( "Right to Left" ) : i18n( "Left to Right" );
  } else {
    label = mLeftToRight ? i18n( "Bottom to Top" ) : i18n( "Top to Bottom" );
  }

  mScaleDirectionAction->setText( label );
}

QRect KLineal::beginRect() const
{
  int shortLen = RESIZE_HANDLE_SHORT_LEN;
  return mHorizontal
    ? QRect( 0, ( height() - shortLen ) / 2 + 1, RESIZE_HANDLE_LEN, shortLen )
    : QRect( ( width() - shortLen ) / 2, 0, shortLen, RESIZE_HANDLE_LEN );
}

QRect KLineal::endRect() const
{
  int shortLen = RESIZE_HANDLE_SHORT_LEN;
  return mHorizontal
    ? QRect( width() - RESIZE_HANDLE_LEN, ( height() - shortLen ) / 2 + 1, RESIZE_HANDLE_LEN, shortLen )
    : QRect( ( width() - shortLen ) / 2, height() - RESIZE_HANDLE_LEN, shortLen, RESIZE_HANDLE_LEN );
}

Qt::CursorShape KLineal::resizeCursor() const
{
  return mHorizontal ? Qt::SizeHorCursor : Qt::SizeVerCursor;
}

void KLineal::setShortLength()
{
  reLength( 30 );
}

void KLineal::setMediumLength()
{
  reLength( 50 );
}

void KLineal::setTallLength()
{
  reLength( 75 );
}

void KLineal::setFullLength()
{
  reLength( 100 );
}

void KLineal::switchDirection()
{
  mLeftToRight = !mLeftToRight;
  updateScaleDirectionMenuItem();
  repaint();
  adjustLabel();
  saveSettings();
}

void KLineal::centerOrigin()
{
  mOffset = -( mLongEdgeLen / 2 );
  repaint();
  adjustLabel();
  saveSettings();
}

void KLineal::slotOffset()
{
  bool ok;
  int newOffset = QInputDialog::getInt( this, i18nc( "@title:window", "Scale Offset" ),
                                            i18n( "Offset:" ), mOffset,
                                            -2147483647, 2147483647, 1, &ok );

  if ( ok ) {
    mOffset = newOffset;
    repaint();
    adjustLabel();
    saveSettings();
  }
}

void KLineal::slotLength()
{
  bool ok;
  QRect r = QApplication::desktop()->screenGeometry( this );
  int width = mHorizontal ? r.width() : r.height();
  int newLength = QInputDialog::getInt( this, i18nc( "@title:window", "Ruler Length" ),
                                            i18n( "Length:" ), mLongEdgeLen,
                                            0, width, 1, &ok );

  if ( ok ) {
    reLengthAbsolute( newLength );
  }
}

void KLineal::slotOpacity( int value )
{
  mOpacity = value;
  repaint();
  RulerSettings::self()->setOpacity( value );
  RulerSettings::self()->save();
}

void KLineal::slotKeyBindings()
{
  KShortcutsDialog::configure( mActionCollection );
}

void KLineal::slotPreferences()
{
  KConfigDialog *dialog = new KConfigDialog( this, QStringLiteral( "settings" ), RulerSettings::self() );

  Ui::ConfigAppearance appearanceConfig;
  QWidget *appearanceConfigWidget = new QWidget( dialog );
  appearanceConfig.setupUi( appearanceConfigWidget );
  dialog->addPage( appearanceConfigWidget, i18n( "Appearance" ), QStringLiteral( "preferences-desktop-default-applications" ) );

#ifdef KRULER_HAVE_X11
  // Advanced page only contains the "Native moving" and "Always on top" settings, disable when not running on X11
  if ( QX11Info::isPlatformX11() ) {
    Ui::ConfigAdvanced advancedConfig;
    QWidget *advancedConfigWidget = new QWidget( dialog );
    advancedConfig.setupUi( advancedConfigWidget );
    dialog->addPage( advancedConfigWidget, i18n( "Advanced" ), QStringLiteral( "preferences-other" ) );
  }
#endif

  connect(dialog, &KConfigDialog::settingsChanged, this, &KLineal::loadConfig);
  dialog->exec();
  delete dialog;
}

void KLineal::loadConfig()
{
  mColor = RulerSettings::self()->bgColor();
  mScaleFont = RulerSettings::self()->scaleFont();
  mAlwaysOnTopLayer = RulerSettings::self()->alwaysOnTop();
  saveSettings();

  setWindowFlags( mAlwaysOnTopLayer ? Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint
                                    : Qt::FramelessWindowHint );

  if ( RulerSettings::self()->trayIcon() ) {
    if ( !mTrayIcon ) {
      createSystemTray();
    }
  } else {
    delete mTrayIcon;
    mTrayIcon = 0;

    if ( mCloseAction ) {
      mCloseAction->setVisible( false );
    }
  }
  show();
  repaint();
}


void KLineal::switchRelativeScale( bool checked )
{
  mRelativeScale = checked;

  mCenterOriginAction->setEnabled( !mRelativeScale );
  mOffsetAction->setEnabled( !mRelativeScale );

  repaint();
  adjustLabel();
  saveSettings();
}

/**
 * save the ruler color to the config file
 */
void KLineal::saveSettings()
{
  RulerSettings::self()->setBgColor( mColor );
  RulerSettings::self()->setScaleFont( mScaleFont );
  RulerSettings::self()->setLength( mLongEdgeLen );
  RulerSettings::self()->setHorizontal( mHorizontal );
  RulerSettings::self()->setLeftToRight( mLeftToRight );
  RulerSettings::self()->setOffset( mOffset );
  RulerSettings::self()->setRelativeScale( mRelativeScale );
  RulerSettings::self()->setAlwaysOnTop( mAlwaysOnTopLayer );
  RulerSettings::self()->save();
}

/**
 * lets the context menu appear at current cursor position
 */
void KLineal::showMenu()
{
  QPoint pos = QCursor::pos();
  mMenu->popup( pos );
}

/**
 * updates the current value label
 */
void KLineal::adjustLabel()
{
  QString s;

  int digit = mHorizontal ? width() : height();

  if ( !mRelativeScale ) {
    if ( mLeftToRight ) {
      digit += mOffset;
    } else {
      digit = mLongEdgeLen - digit + mOffset;
    }
  } else {
    // INFO: Perhaps use float also for displaying relative value
    digit = (int)( ( digit * 100.f ) / mLongEdgeLen );

    if ( !mLeftToRight ) {
      digit = 100 - digit;
    }
  }

  s.sprintf( "%d%s", digit, ( mRelativeScale ? "%" : " px" ) );
  mLabel->setText( s );

  QFontMetrics fm = mLabel->fontMetrics();
  QPoint pos = mHorizontal
    ? QPoint( height() / 2, ( height() - fm.ascent() ) / 2 )
    : QPoint( ( width() - mLabel->width() ) / 2, width() / 2 );
  mLabel->move( pos );
}

void KLineal::keyPressEvent( QKeyEvent *e )
{
  QPoint dist;

  switch ( e->key() ) {
  case Qt::Key_F1:
    KHelpClient::invokeHelp();
    return;

  case Qt::Key_Left:
    dist.setX( -1 );
    break;

  case Qt::Key_Right:
    dist.setX( 1 );
    break;

  case Qt::Key_Up:
    dist.setY( -1 );
    break;

  case Qt::Key_Down:
    dist.setY( 1 );
    break;

  default:
    QWidget::keyPressEvent(e);
    return;
  }

  if ( e->modifiers() & Qt::ShiftModifier ) {
    dist *= 10;
  }

  move( pos() + dist );
  KNotification::event( QString(), QStringLiteral( "cursormove" ), QString() );
}

/**
 * overwritten to handle the line cursor which is a separate widget outside the main
 * window. Also used for dragging.
 */
void KLineal::mouseMoveEvent( QMouseEvent *inEvent )
{
  Q_UNUSED( inEvent );

  if ( mRulerState != StateNone && this == mouseGrabber() ) {
    if ( mRulerState == StateMove ) {
      move( QCursor::pos() - mDragOffset );
    } else if ( mRulerState == StateBegin ) {
      QRect r = geometry();
      if ( mHorizontal ) {
        r.setLeft( QCursor::pos().x() - mDragOffset.x() );
      } else {
        r.setTop( QCursor::pos().y() - mDragOffset.y() );
      }
      setGeometry( r );

      adjustLabel();
    } else if ( mRulerState == StateEnd ) {
      QPoint end = QCursor::pos() + mDragOffset - pos();
      QSize size = mHorizontal
        ? QSize( end.x(), height() )
        : QSize( width(), end.y() );
      resize( size );
      adjustLabel();
    }
  } else {
    QPoint cpos = mapFromGlobal( QCursor::pos() );
    mRulerState = StateNone;
    if ( beginRect().contains( cpos ) || endRect().contains( cpos) ) {
      setCursor( resizeCursor() );
    } else {
      setCursor( Qt::SizeAllCursor );
    }
  }
}

/**
 * overwritten for dragging and context menu
 */
void KLineal::mousePressEvent( QMouseEvent *inEvent )
{
  mLastClickPos = QCursor::pos();

  QRect gr = geometry();
  mDragOffset = mLastClickPos - gr.topLeft();
  if ( inEvent->button() == Qt::LeftButton ) {
    if ( mRulerState < StateMove ) {
      if ( beginRect().contains( mDragOffset ) ) {
        mRulerState = StateBegin;
        grabMouse( resizeCursor() );
      } else if ( endRect().contains( mDragOffset ) ) {
        mDragOffset = gr.bottomRight() - mLastClickPos;
        mRulerState = StateEnd;
        grabMouse( resizeCursor() );
      } else {
        if ( nativeMove() ) {
          startNativeMove( inEvent );
        } else {
          mRulerState = StateMove;
          grabMouse( Qt::SizeAllCursor );
        }
      }
    }
  } else if ( inEvent->button() == Qt::MidButton ) {
    mClicked = true;
    rotate();
  } else if ( inEvent->button() == Qt::RightButton ) {
    showMenu();
  }
}

#ifdef KRULER_HAVE_X11
bool KLineal::nativeMove() const
{
  return QX11Info::isPlatformX11() && RulerSettings::self()->nativeMoving();
}

void KLineal::startNativeMove( QMouseEvent *inEvent )
{
  xcb_ungrab_pointer( QX11Info::connection(), QX11Info::appTime() );
  NETRootInfo wm_root( QX11Info::connection(), NET::WMMoveResize );
  wm_root.moveResizeRequest( winId(), inEvent->globalX(), inEvent->globalY(), NET::Move );
}

void KLineal::stopNativeMove( QMouseEvent *inEvent )
{
  NETRootInfo wm_root( QX11Info::connection(), NET::WMMoveResize );
  wm_root.moveResizeRequest( winId(), inEvent->globalX(), inEvent->globalY(), NET::MoveResizeCancel );
}
#else
bool KLineal::nativeMove() const
{
  return false;
}

void KLineal::startNativeMove( QMouseEvent *inEvent )
{
  Q_UNUSED( inEvent );
}

void KLineal::stopNativeMove( QMouseEvent *inEvent )
{
  Q_UNUSED( inEvent );
}
#endif

/**
 * overwritten for dragging
 */
void KLineal::mouseReleaseEvent( QMouseEvent *inEvent )
{
  if ( mRulerState != StateNone ) {
    mRulerState = StateNone;
    releaseMouse();
  } else if ( nativeMove() ) {
    stopNativeMove( inEvent );
  }
}

void KLineal::wheelEvent( QWheelEvent *e )
{
  int numDegrees = e->delta() / 8;
  int numSteps = numDegrees / 15;

  // changing offset
  if ( e->buttons() == Qt::LeftButton ) {
    if ( !mRelativeScale ) {
      mLabel->show();
      mOffset += numSteps;

      repaint();
      mLabel->setText( i18n( "Offset: %1", mOffset ) );
      saveSettings();
    }
  } else { // changing length
    int oldLen = mLongEdgeLen;
    int newLength = mLongEdgeLen + numSteps;
    reLengthAbsolute( newLength );
    mLabel->setText( i18n( "Length: %1 px", mLongEdgeLen ) );

    // when holding shift relength at the other side
    if ( e->modifiers() & Qt::ShiftModifier ) {
      int change = mLongEdgeLen - oldLen;

      QPoint dist;

      if ( mHorizontal ) {
        dist.setX( -change );
      } else {
        dist.setY( -change );
      }

      move( pos() + dist );
    }
  }

  QWidget::wheelEvent( e );
}

/**
 * draws the scale according to the orientation
 */
void KLineal::drawScale( QPainter &painter )
{
  painter.setPen( Qt::black );
  QFont font = mScaleFont;
  painter.setFont( font );
  int longLen;
  int w = width();
  int h = height();

  // draw a frame around the whole thing
  // (for some unknown reason, this doesn't show up anymore)
  if ( mHorizontal ) {
    longLen = w;
    painter.drawLine( 0, 0, 0, h - 1 );
    painter.drawLine( w - 1, h - 1, w - 1, 0 );
  } else {
    longLen = h;
    painter.drawLine( 0, h - 1, w - 1, h - 1 );
    painter.drawLine( w - 1, 0, 0, 0 );
  }

  if ( !mRelativeScale ) {
    int digit;
    int len;
    for ( int x = 0; x < longLen; ++x ) {
      if ( mLeftToRight ) {
        digit = x + mOffset;
      } else {
        digit = longLen - x + mOffset;
      }

      if ( digit % 2 ) continue;

      len = 6;

      if ( digit % 10 == 0 ) len = 10;
      if ( digit % 20 == 0 ) len = 15;
      if ( digit % 100 == 0 ) len = 18;

      if ( digit % 100 == 0 ) {
        QString units;
        units.sprintf( "%d", digit );
        drawScaleText( painter, x, units );
      }

      drawScaleTick( painter, x, len );
    }
  } else {
    float step = longLen / 100.f;
    int len;

    for ( int i = 0; i <= 100; ++i ) {
      int x = (int)( i * step );
      len = ( i % 10 ) ? 6 : 15;

      if ( i % 10 == 0 ) {
        QString units;
        int value = mLeftToRight ? i : ( 100 - i );
        units.sprintf( "%d%%", value );
        drawScaleText( painter, x, units );
      }

      drawScaleTick( painter, x, len );
    }
  }
}

void KLineal::drawScaleText( QPainter &painter, int x, const QString &text )
{
  QFontMetrics metrics = painter.fontMetrics();
  QSize textSize = metrics.size( Qt::TextSingleLine, text );
  int w = width();
  int h = height();
  int tw = textSize.width();
  int th = metrics.ascent();

  if ( mHorizontal ) {
    painter.drawText( x - tw / 2, (h + th) / 2, text );
  } else {
    painter.drawText( (w - tw) / 2, x + th / 2, text );
  }
}

void KLineal::drawScaleTick( QPainter &painter, int x, int len )
{
  int w = width();
  int h = height();
  if ( mHorizontal ) {
    painter.drawLine( x, 0, x, len );
    painter.drawLine( x, h, x, h - len );
  } else {
    painter.drawLine( 0, x, len, x );
    painter.drawLine( w, x, w - len, x );
  }
}

void KLineal::drawResizeHandle( QPainter &painter, Qt::Edge edge )
{
  QRect rect;
  switch ( edge ) {
  case Qt::LeftEdge:
  case Qt::TopEdge:
    rect = beginRect();
    break;
  case Qt::RightEdge:
  case Qt::BottomEdge:
    rect = endRect();
    break;
  }
  painter.setOpacity( RESIZE_HANDLE_OPACITY );
  if ( mHorizontal ) {
    int y1 = ( mShortEdgeLen - RESIZE_HANDLE_SHORT_LEN ) / 2;
    int y2 = y1 + RESIZE_HANDLE_SHORT_LEN - 1;
    for ( int x = rect.left() + 1; x < rect.right(); x += 2 ) {
      painter.drawLine( x, y1, x, y2 );
    }
  } else {
    int x1 = ( mShortEdgeLen - RESIZE_HANDLE_SHORT_LEN ) / 2;
    int x2 = x1 + RESIZE_HANDLE_SHORT_LEN - 1;
    for ( int y = rect.top() + 1; y < rect.bottom(); y += 2 ) {
      painter.drawLine( x1, y, x2, y );
    }
  }
  painter.setOpacity( 1 );
}

/**
 * actually draws the ruler
 */
void KLineal::paintEvent(QPaintEvent *inEvent )
{
  Q_UNUSED( inEvent );

  QPainter painter( this );
  drawBackground( painter );
  drawScale( painter );

  drawResizeHandle( painter, mHorizontal ? Qt::LeftEdge : Qt::TopEdge );
  drawResizeHandle( painter, mHorizontal ? Qt::RightEdge : Qt::BottomEdge );
}
