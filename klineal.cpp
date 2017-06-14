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

/**
 * this is our cursor bitmap:
 * a line 48 pixels long with an arrow pointing down
 * and a sqare with a one pixel hole at the top (end)
 */
static const uchar cursorBits[] = {
  0x38, 0x28, 0x38, 0x10, 0x10, 0x10, 0x10, 0x10,
  0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10,
  0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10,
  0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10,
  0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10,
  0xfe, 0xfe, 0x7c, 0x7c, 0x38, 0x38, 0x10, 0x10,
};

/**
 * create the thingy with no borders and set up
 * its members
 */
KLineal::KLineal( QWidget *parent )
  : QWidget( parent ),
    mDragging( false ),
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
    mCloseButton( 0 ),
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

  QBitmap bim = QBitmap::fromData( QSize( 8, 48 ), cursorBits, QImage::Format_Mono );
  QMatrix m;
  m.rotate( 90.0 );
  mVerticalCursor = QCursor( bim, bim, 4, 24 );
  bim = bim.transformed( m );
  mHorizontalCursor = QCursor( bim, bim, 24, 4 );

  mCurrentCursor = mVerticalCursor;
  mColor = RulerSettings::self()->bgColor();
  mScaleFont = RulerSettings::self()->scaleFont();
  mLongEdgeLen = RulerSettings::self()->length();
  mOrientation = RulerSettings::self()->orientation();
  mLeftToRight = RulerSettings::self()->leftToRight();
  mOffset = RulerSettings::self()->offset();
  mRelativeScale = RulerSettings::self()->relativeScale();
  mAlwaysOnTopLayer = RulerSettings::self()->alwaysOnTop();

  mLabel = new QAutoSizeLabel( this );
  mLabel->setGeometry( 0, height() - 12, 32, 12 );
  mLabel->setWhatsThis( i18n( "This is the current distance measured in pixels." ) );

  mBtnRotateLeft = new QToolButton( this );
  mBtnRotateLeft->setIcon( QIcon::fromTheme( QStringLiteral(  "object-rotate-left" ) ) );
  mBtnRotateLeft->setToolTip( i18n( "Turn Left" ) );
  connect(mBtnRotateLeft, &QToolButton::clicked, this, &KLineal::turnLeft);

  mBtnRotateRight = new QToolButton( this );
  mBtnRotateRight->setIcon( QIcon::fromTheme( QStringLiteral(  "object-rotate-right" ) ) );
  mBtnRotateRight->setToolTip( i18n( "Turn Right" ) );
  connect(mBtnRotateRight, &QToolButton::clicked, this, &KLineal::turnRight);

  resize( QSize( mLongEdgeLen, mShortEdgeLen ) );

  //BEGIN setup menu and actions
  mActionCollection = new KActionCollection( this );
  mActionCollection->setConfigGroup( QStringLiteral( "Actions" ) );

  mMenu = new QMenu( this );
  mMenu->addSection( i18n( "KRuler" ) );
  addAction( mMenu, QIcon::fromTheme( QStringLiteral( "object-rotate-left" ) ), i18n( "Rotate" ),
             this, SLOT(turnRight()), Qt::Key_R, QStringLiteral( "turn_right" ) );

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

  hideLabel();
  setOrientation( mOrientation );

}

KLineal::~KLineal()
{
  delete mTrayIcon;
}

void KLineal::createSystemTray()
{
  if ( !mCloseAction ) {
    mCloseAction = mActionCollection->addAction( KStandardAction::Close, this, SLOT(slotClose()) );
    mMenu->addAction( mCloseAction );

    mCloseButton = new QToolButton( this );
    mCloseButton->setIcon( mCloseAction->icon() );
    mCloseButton->setToolTip( mCloseAction->text().remove( QLatin1Char(  '&' ) ) );
    connect(mCloseButton, &QToolButton::clicked, this, &KLineal::slotClose);
  } else {
    mCloseAction->setVisible( true );
  }

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
  switch ( mOrientation ) {
  case North:
    a = bg.light( 120 );
    b = bg.dark( 130 );
    gradient = QLinearGradient( 1, 0, 1, height() );
    break;

  case South:
    b = bg.light( 120 );
    a = bg.dark( 130 );
    gradient = QLinearGradient( 1, 0, 1, height() );
    break;

  case West:
    a = bg.light( 120 );
    b = bg.dark( 130 );
    gradient = QLinearGradient( 0, 1, width(), 1 );
    break;

  case East:
    b = bg.light( 120 );
    a = bg.dark( 130 );
    gradient = QLinearGradient( 0, 1, width(), 1 );
    break;
  }
  a.setAlpha( mOpacity );
  b.setAlpha( mOpacity );
  gradient.setColorAt( 0, a );
  gradient.setColorAt( 1, b );
  painter.fillRect( rect(), QBrush( gradient ) );
}

void KLineal::setOrientation( int inOrientation )
{
  QRect r = frameGeometry();
  int nineties = (int)inOrientation - (int)mOrientation;
  mOrientation = ( inOrientation + 4 ) % 4;
  QPoint center = mLastClickPos, newTopLeft;

  if ( mClicked ) {
    center = mLastClickPos;
    mClicked = false;
  } else {
    center = r.topLeft() + QPoint( width() / 2, height() / 2 );
  }

  if ( nineties % 2 ) {
    newTopLeft = QPoint( center.x() - height() / 2, center.y() - width() / 2 );
  } else {
    newTopLeft = r.topLeft();
  }

  if ( mOrientation == North || mOrientation == South ) {
    r.setSize( QSize( mLongEdgeLen, mShortEdgeLen ) );
  } else {
    r.setSize( QSize( mShortEdgeLen, mLongEdgeLen ) );
  }

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
  mLabel->move( ( width() - mLabel->width() ) / 2,
                ( height() - mLabel->height() ) / 2 );

  switch( mOrientation ) {
  case North:
  case South:
    mCurrentCursor = mVerticalCursor;
    break;

  case West:
  case East:
    mCurrentCursor = mHorizontalCursor;
    break;
  }

  adjustButtons();

  if ( mLenMenu && mFullScreenAction ) {
    mFullScreenAction->setText( mOrientation % 2 ? i18n( "&Full Screen Height" ) : i18n( "&Full Screen Width" ) );
  }

  updateScaleDirectionMenuItem();

  setCursor( mCurrentCursor );
  repaint();
  saveSettings();
}

void KLineal::setNorth()
{
  setOrientation( North );
}

void KLineal::setEast()
{
  setOrientation( East );
}

void KLineal::setSouth()
{
  setOrientation( South );
}

void KLineal::setWest()
{
  setOrientation( West );
}

void KLineal::turnRight()
{
  setOrientation( mOrientation - 1 );
}

void KLineal::turnLeft()
{
  setOrientation( mOrientation + 1 );
}

void KLineal::reLength( int percentOfScreen )
{
  if ( percentOfScreen < 10 ) {
    return;
  }

  QRect r = QApplication::desktop()->screenGeometry( this );

  if ( mOrientation == North || mOrientation == South ) {
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

  adjustButtons();
  saveSettings();
}

void KLineal::reLengthAbsolute( int length )
{
  if ( length < 100 ) {
    return;
  }

  mLongEdgeLen = length;
  if ( mOrientation == North || mOrientation == South ) {
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

  adjustButtons();
  saveSettings();
}

void KLineal::updateScaleDirectionMenuItem()
{
  if ( !mScaleDirectionAction ) return;

  QString label;

  if ( mOrientation == North || mOrientation == South ) {
    label = mLeftToRight ? i18n( "Right to Left" ) : i18n( "Left to Right" );
  } else {
    label = mLeftToRight ? i18n( "Bottom to Top" ) : i18n( "Top to Bottom" );
  }

  mScaleDirectionAction->setText( label );
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
  int width = ( ( mOrientation == North ) || ( mOrientation == South ) ) ? r.width() : r.height();
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
  appearanceConfig.kcfg_CloseButtonVisible->setEnabled( appearanceConfig.kcfg_TrayIcon->isChecked() );
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
      //need to adjust button
      adjustButtons();
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
  RulerSettings::self()->setOrientation( mOrientation );
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
 * overwritten to switch the value label and line cursor on
 */
void KLineal::enterEvent( QEvent *inEvent )
{
  Q_UNUSED( inEvent );

  if ( !mDragging ) {
    showLabel();
  }
}

/**
 * overwritten to switch the value label and line cursor off
 */
void KLineal::leaveEvent( QEvent *inEvent )
{
  Q_UNUSED( inEvent );

  if ( !geometry().contains( QCursor::pos() ) ) {
    hideLabel();
  }
}

/**
 * shows the value lable
 */
void KLineal::showLabel()
{
  adjustLabel();
  mLabel->show();
  if ( RulerSettings::self()->rotateButtonsVisible() ) {
    mBtnRotateLeft->show();
    mBtnRotateRight->show();
  }
  if ( mCloseButton &&
       RulerSettings::self()->closeButtonVisible() &&
       RulerSettings::self()->trayIcon()) {
    mCloseButton->show();
  }
}

/**
 * hides the value label
 */
void KLineal::hideLabel()
{
  mLabel->hide();
  mBtnRotateLeft->hide();
  mBtnRotateRight->hide();
  if ( mCloseButton ) {
    mCloseButton->hide();
  }
}

/**
 * updates the current value label
 */
void KLineal::adjustLabel()
{
  QString s;
  QPoint cpos = QCursor::pos();

  int digit = ( mOrientation == North || mOrientation == South ) ? cpos.x() - x() : cpos.y() - y();

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
}

/**
 * Updates the position of the tool buttons
 */
void KLineal::adjustButtons()
{
  switch( mOrientation ) {
  case North:
    mBtnRotateLeft->move( mLongEdgeLen / 2 - 28, height() - 31 );
    mBtnRotateRight->move( mLongEdgeLen / 2 + 2, height() - 31 );
    if ( mCloseButton ) {
      mCloseButton->move( width() - 31, height() - 31 );
    }
    break;

  case South:
    mBtnRotateLeft->move( mLongEdgeLen / 2 - 28, 5 );
    mBtnRotateRight->move( mLongEdgeLen / 2 + 2, 5 );
    if ( mCloseButton ) {
      mCloseButton->move( width() - 31, 5 );
    }
    break;

  case East:
    mBtnRotateLeft->move( 5, mLongEdgeLen / 2 - 28 );
    mBtnRotateRight->move( 5, mLongEdgeLen / 2 + 2 );
    if ( mCloseButton ) {
      mCloseButton->move( 5, height() - 31 );
    }
    break;

  case West:
    mBtnRotateLeft->move( width() - 31, mLongEdgeLen / 2 - 28 );
    mBtnRotateRight->move( width() - 31, mLongEdgeLen / 2 + 2 );
    if ( mCloseButton ) {
      mCloseButton->move( width() - 31, height() - 31 );
    }
    break;
  }
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

  if ( mDragging && this == mouseGrabber() ) {
#ifdef KRULER_HAVE_X11
    if ( !QX11Info::isPlatformX11() || !RulerSettings::self()->nativeMoving() ) {
#endif
      move( QCursor::pos() - mDragOffset );
#ifdef KRULER_HAVE_X11
    }
#endif
  } else {
    QPoint p = QCursor::pos();

    switch ( mOrientation ) {
    case North:
      p.setY( p.y() - 46 );
      break;

    case East:
      p.setX( p.x() + 46 );
      break;

    case West:
      p.setX( p.x() - 46 );
      break;

    case South:
      p.setY( p.y() + 46 );
      break;
    }

    adjustLabel();
  }
}

/**
 * overwritten for dragging and context menu
 */
void KLineal::mousePressEvent( QMouseEvent *inEvent )
{
  mLastClickPos = QCursor::pos();
  hideLabel();

  QRect gr = geometry();
  mDragOffset = mLastClickPos - QPoint( gr.left(), gr.top() );
  if ( inEvent->button() == Qt::LeftButton ) {
#ifdef KRULER_HAVE_X11
    if ( QX11Info::isPlatformX11() && RulerSettings::self()->nativeMoving() ) {
      xcb_ungrab_pointer( QX11Info::connection(), QX11Info::appTime() );
      NETRootInfo wm_root( QX11Info::connection(), NET::WMMoveResize );
      wm_root.moveResizeRequest( winId(), inEvent->globalX(), inEvent->globalY(), NET::Move );
    } else {
#endif
      if ( !mDragging ) {
        grabMouse( Qt::SizeAllCursor );
        mDragging = true;
      }
#ifdef KRULER_HAVE_X11
    }
#endif
  } else if ( inEvent->button() == Qt::MidButton ) {
    mClicked = true;
    turnLeft();
  } else if ( inEvent->button() == Qt::RightButton ) {
    showMenu();
  }
}

/**
 * overwritten for dragging
 */
void KLineal::mouseReleaseEvent( QMouseEvent *inEvent )
{
  Q_UNUSED( inEvent );

#ifdef KRULER_HAVE_X11
  if ( QX11Info::isPlatformX11() && RulerSettings::self()->nativeMoving() ) {
    NETRootInfo wm_root( QX11Info::connection(), NET::WMMoveResize );
    wm_root.moveResizeRequest( winId(), inEvent->globalX(), inEvent->globalY(), NET::MoveResizeCancel );
  } else {
#endif
    if ( mDragging ) {
      mDragging = false;
      releaseMouse();
    }
#ifdef KRULER_HAVE_X11
  }
#endif

  showLabel();
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

      if ( mOrientation == North || mOrientation == South ) {
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
  switch ( mOrientation ) {
  case North:
  case South:
    longLen = w;
    painter.drawLine( 0, 0, 0, h - 1 );
    painter.drawLine( w - 1, h - 1, w - 1, 0 );
    break;

  case East:
  case West:
    longLen = h;
    painter.drawLine( 0, h - 1, w - 1, h - 1 );
    painter.drawLine( w - 1, 0, 0, 0 );
    break;
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

  switch ( mOrientation ) {
  case North:
  case South:
    painter.drawText( x - tw / 2, (h + th) / 2, text );
    break;

  case East:
  case West:
    painter.drawText( (w - tw) / 2, x + th / 2, text );
    break;
  }
}

void KLineal::drawScaleTick( QPainter &painter, int x, int len )
{
  int w = width();
  int h = height();
  switch( mOrientation ) {
  case North:
  case South:
    painter.drawLine( x, 0, x, len );
    painter.drawLine( x, h, x, h - len );
    break;
  case East:
  case West:
    painter.drawLine( 0, x, len, x );
    painter.drawLine( w, x, w - len, x );
    break;
  }
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
}
