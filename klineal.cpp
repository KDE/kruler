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

#include <QBitmap>
#include <QBrush>
#include <QClipboard>
#include <QPainter>
#include <QMouseEvent>
#include <QShortcut>

#include <KAction>
#include <KColorDialog>
#include <KConfig>
#include <KConfigDialog>
#include <KGlobalSettings>
#include <KHelpMenu>
#include <KInputDialog>
#include <KLocale>
#include <KMenu>
#include <KNotification>
#include <KStandardAction>
#include <KSystemTrayIcon>
#include <KToolInvocation>
#include <KWindowSystem>
#include <KApplication>

#include "kruler.h"
#include "qautosizelabel.h"

#include "ui_cfg_appearance.h"

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
    mLenMenu( 0 ),                // INFO This member could be eventually deleted
                                  // since if mFullScreenAction is initialized
                                  // mLenMenu should have been, too.
    mFullScreenAction( 0 ),
    mScaleDirectionAction( 0 ),
    mCenterOriginAction( 0 ),
    mOffsetAction( 0 ),
    mClicked( false )
{
  KWindowSystem::setType( winId(), NET::Override );   // or NET::Normal
  KWindowSystem::setState( winId(), NET::KeepAbove );

  setMinimumSize( 60, 60 );
  setMaximumSize( 8000, 8000 );
  setWhatsThis( i18n( "This is a tool to measure pixel distances and colors on the screen. "
                      "It is useful for working on layouts of dialogs, web pages etc." ) );
  setMouseTracking( true );

  QBitmap bim = QBitmap::fromData( QSize( 8, 48 ), cursorBits, QImage::Format_Mono );
  QMatrix m;
  m.rotate( 90.0 );
  mNorthCursor = QCursor( bim, bim, 3, 47 );
  bim = bim.transformed( m );
  mEastCursor = QCursor( bim, bim, 0, 3 );
  bim = bim.transformed( m );
  mSouthCursor = QCursor( bim, bim, 4, 0 );
  bim = bim.transformed( m );
  mWestCursor = QCursor( bim, bim, 47, 4 );

  mCurrentCursor = mNorthCursor;
  mColor = RulerSettings::self()->bgColor();
  mScaleFont = RulerSettings::self()->scaleFont();
  mLongEdgeLen = RulerSettings::self()->length();
  mOrientation = RulerSettings::self()->orientation();
  mLeftToRight = RulerSettings::self()->leftToRight();
  mOffset = RulerSettings::self()->offset();
  mRelativeScale = RulerSettings::self()->relativeScale();

  mLabel = new QAutoSizeLabel( this );
  mLabel->setGeometry( 0, height() - 12, 32, 12 );
  QFont labelFont( KGlobalSettings::generalFont().family(), 10 );
  labelFont.setPixelSize( 10 );
  mLabel->setFont( labelFont );
  mLabel->setWhatsThis( i18n( "This is the current distance measured in pixels." ) );
  mColorLabel = new QAutoSizeLabel( this );
  mColorLabel->setAutoFillBackground( true );
  mColorLabel->hide();
  QFont colorFont( KGlobalSettings::fixedFont().family(), 10 );
  colorFont.setPixelSize( 10 );
  mColorLabel->setFont( colorFont );
  mColorLabel->move( mLabel->pos() + QPoint(0, 20) );
  mColorLabel->setWhatsThis(i18n("This is the current color in hexadecimal rgb representation"
                                 " as you may use it in HTML or as a QColor name. "
                                 "The rectangles background shows the color of the pixel inside the "
                                 "little square at the end of the line cursor." ) );

  resize( QSize( mLongEdgeLen, mShortEdgeLen ) );

  mMenu = new KMenu( this );
  mMenu->addTitle( i18n( "KRuler" ) );
  KMenu *oriMenu = new KMenu( i18n( "&Orientation"), this );
  oriMenu->addAction( KIcon( "kruler-north" ),
                      i18nc( "Turn Kruler North", "&North" ),
                      this, SLOT( setNorth() ), Qt::Key_N );
  oriMenu->addAction( KIcon( "kruler-east" ),
                      i18nc( "Turn Kruler East", "&East" ),
                      this, SLOT( setEast() ), Qt::Key_E );
  oriMenu->addAction( KIcon( "kruler-south" ),
                      i18nc( "Turn Kruler South", "&South" ),
                      this, SLOT( setSouth() ), Qt::Key_S );
  oriMenu->addAction( KIcon( "kruler-west" ),
                      i18nc( "Turn Kruler West", "&West" ),
                      this, SLOT( setWest() ), Qt::Key_W );
  oriMenu->addAction( KIcon( "object-rotate-right" ),
                      i18n( "&Turn Right" ),
                      this, SLOT( turnRight() ), Qt::Key_R );
  oriMenu->addAction( KIcon( "object-rotate-left" ),
                      i18n( "Turn &Left" ),
                      this, SLOT( turnLeft() ), Qt::Key_L );

  new QShortcut( Qt::Key_N, this, SLOT( setNorth() ) );
  new QShortcut( Qt::Key_E, this, SLOT( setEast() ) );
  new QShortcut( Qt::Key_S, this, SLOT( setSouth() ) );
  new QShortcut( Qt::Key_W, this, SLOT( setWest() ) );
  new QShortcut( Qt::Key_R, this, SLOT( turnRight() ) );
  new QShortcut( Qt::Key_L, this, SLOT( turnLeft() ) );
  mMenu->addMenu( oriMenu );
  mLenMenu = new KMenu( i18n( "&Length" ), this );
  mLenMenu->addAction( i18nc( "Make Kruler Height Short", "&Short" ),
                       this, SLOT( setShortLength() ), Qt::CTRL + Qt::Key_S );
  mLenMenu->addAction( i18nc( "Make Kruler Height Medium", "&Medium" ),
                       this, SLOT( setMediumLength() ), Qt::CTRL + Qt::Key_M );
  mLenMenu->addAction( i18nc( "Make Kruler Height Tall", "&Tall" ),
                       this, SLOT( setTallLength() ), Qt::CTRL + Qt::Key_T );
  mFullScreenAction = mLenMenu->addAction( i18n("&Full Screen Width"),
                                           this, SLOT( setFullLength() ),
                                           Qt::CTRL + Qt::Key_F );
  new QShortcut( Qt::CTRL + Qt::Key_S, this, SLOT( setShortLength() ) );
  new QShortcut( Qt::CTRL + Qt::Key_M, this, SLOT( setMediumLength() ) );
  new QShortcut( Qt::CTRL + Qt::Key_T, this, SLOT( setTallLength() ) );
  new QShortcut( Qt::CTRL + Qt::Key_F, this, SLOT( setFullLength() ) );
  mLenMenu->addSeparator();
  mLenMenu->addAction( i18n( "Length..." ), this, SLOT( slotLength() ) );
  mMenu->addMenu( mLenMenu );

  KMenu* scaleMenu = new KMenu( i18n( "&Scale" ), this );
  mScaleDirectionAction = scaleMenu->addAction( i18n( "Right To Left" ), this, SLOT( switchDirection() ), Qt::Key_D );
  mCenterOriginAction = scaleMenu->addAction( i18n( "Center origin" ), this, SLOT( centerOrigin() ), Qt::Key_C );
  mCenterOriginAction->setEnabled( !mRelativeScale );
  mOffsetAction = scaleMenu->addAction( i18n( "Offset..." ), this, SLOT( slotOffset() ), Qt::Key_O );
  mOffsetAction->setEnabled( !mRelativeScale );
  scaleMenu->addSeparator();
  QAction *relativeScaleAction = scaleMenu->addAction( i18n( "Percentage" ) );
  relativeScaleAction->setCheckable( true );
  relativeScaleAction->setChecked( mRelativeScale );
  connect( relativeScaleAction, SIGNAL( toggled( bool ) ), this, SLOT( switchRelativeScale( bool ) ) );

  new QShortcut( Qt::Key_D, this, SLOT( switchDirection() ) );
  new QShortcut( Qt::Key_C, this, SLOT( centerOrigin() ) );
  new QShortcut( Qt::Key_O, this, SLOT( slotOffset() ) );
  mMenu->addMenu( scaleMenu );

  mMenu->addAction( KStandardAction::preferences( this, SLOT( slotPreferences() ), this ) );
  mMenu->addSeparator();
  KAction *copyColorAction = KStandardAction::copy( this, SLOT( copyColor() ), this );
  copyColorAction->setText( i18n( "Copy Color" ) );
  mMenu->addAction( copyColorAction );
  new QShortcut( copyColorAction->shortcut().primary(), this, SLOT( copyColor() ) );
  mMenu->addSeparator();
  mMenu->addMenu( ( new KHelpMenu( this, KGlobal::mainComponent().aboutData(), true ) )->menu() );
  mMenu->addSeparator();

  if ( RulerSettings::self()->trayIcon() ) {
    KAction *closeAction = KStandardAction::close( this, SLOT( slotClose() ), this );
    mMenu->addAction( closeAction );
    new QShortcut( closeAction->shortcut().primary(), this, SLOT( slotClose() ) );
  }

  KAction *quit = KStandardAction::quit( kapp, SLOT( quit() ), this );
  mMenu->addAction( quit );
  new QShortcut( quit->shortcut().primary(), this, SLOT(slotQuit() ) );

  mLastClickPos = geometry().topLeft() + QPoint( width() / 2, height() / 2 );

  setOrientation( mOrientation );

  if ( RulerSettings::self()->trayIcon() ) {
    KSystemTrayIcon *tray = new KSystemTrayIcon( KIcon( "kruler" ), this );
    tray->show();
  }
}

KLineal::~KLineal()
{
}

void KLineal::slotClose()
{
  hide();
}

void KLineal::slotQuit()
{
   kapp->quit();
}

void KLineal::move( int x, int y )
{
  move( QPoint( x, y ) );
}

void KLineal::move(const QPoint &p)
{
  setGeometry( QRect( p, size() ) );
}

QPoint KLineal::pos()
{
  return frameGeometry().topLeft();
}

int KLineal::x()
{
  return pos().x();
}

int KLineal::y()
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

  QRect desktop = KGlobalSettings::desktopGeometry( this );

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
  switch( mOrientation ) {
  case North:
    mLabel->move( 4, height()-mLabel->height() - 4 );
    mColorLabel->move( mLabel->pos() + QPoint( 0, -20 ) );
    mCurrentCursor = mNorthCursor;
    break;

  case South:
    mLabel->move( 4, 4 );
    mColorLabel->move( mLabel->pos() + QPoint( 0, 20 ) );
    mCurrentCursor = mSouthCursor;
    break;

  case East:
    mLabel->move( 4, 4 );
    mColorLabel->move( mLabel->pos() + QPoint( 0, 20 ) );
    mCurrentCursor = mEastCursor;
    break;

  case West:
    mLabel->move( width()-mLabel->width() - 4, 4 );
    mColorLabel->move( mLabel->pos() + QPoint( -5, 20 ) );
    mCurrentCursor = mWestCursor;
    break;
  }

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

  QRect r = KGlobalSettings::desktopGeometry( this );

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

  saveSettings();
}

void KLineal::updateScaleDirectionMenuItem()
{
  if ( !mScaleDirectionAction ) return;

  QString label;

  if ( mOrientation == North || mOrientation == South ) {
    label = mLeftToRight ? i18n( "Right To Left" ) : i18n( "Left To Right" );
  } else {
    label = mLeftToRight ? i18n( "Bottom To Top" ) : i18n( "Top To Bottom" );
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
  int newOffset = KInputDialog::getInteger( i18n( "Scale offset" ),
                                            i18n( "Offset:" ), mOffset,
                                            -2147483647, 2147483647, 1, &ok, this );

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
  QRect r = KGlobalSettings::desktopGeometry( this );
  int width = ( ( mOrientation == North ) || ( mOrientation == South ) ) ? r.width() : r.height();
  int newLength = KInputDialog::getInteger( i18n( "Ruler Length" ),
                                            i18n( "Length:" ), mLongEdgeLen,
                                            0, width, 1, &ok, this );

  if ( ok ) {
    reLength( ( newLength * 100.f ) / width );
  }
}

void KLineal::slotPreferences()
{
  KConfigDialog *dialog = new KConfigDialog( this, "settings", RulerSettings::self() );

  Ui::ConfigAppearance appearanceConfig;
  QWidget *appearanceConfigWidget = new QWidget( dialog );
  appearanceConfig.setupUi( appearanceConfigWidget );
  dialog->addPage( appearanceConfigWidget, i18n( "Appearance" ), "preferences-desktop-default-applications" );

  dialog->exec();
  mColor = RulerSettings::self()->bgColor();
  mScaleFont = RulerSettings::self()->scaleFont();
  repaint();
  saveSettings();
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
  RulerSettings::self()->writeConfig();
}

void KLineal::copyColor()
{
  QApplication::clipboard()->setText( mColorLabel->text() );
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
  mColorLabel->show();
}

/**
 * hides the value label
 */
void KLineal::hideLabel()
{
  mLabel->hide();
  mColorLabel->hide();
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

void KLineal::keyPressEvent( QKeyEvent *e )
{
  QPoint dist;

  switch ( e->key() ) {
  case Qt::Key_F1:
    KToolInvocation::invokeHelp();
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
  KNotification::event( 0, "cursormove", QString() );
}

/**
 * overwritten to handle the line cursor which is a separate widget outside the main
 * window. Also used for dragging.
 */
void KLineal::mouseMoveEvent( QMouseEvent *inEvent )
{
  Q_UNUSED( inEvent );

  if ( mDragging && this == mouseGrabber() ) {
    move( QCursor::pos() - mDragOffset );
  } else {
    QPoint p = QCursor::pos();

    switch (mOrientation) {
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

    QColor color = KColorDialog::grabColor( p );
    int h, s, v;
    color.getHsv( &h, &s, &v );
    mColorLabel->setText( color.name().toUpper() );
    QPalette palette = mColorLabel->palette();
    palette.setColor( mColorLabel->backgroundRole(), color );
    if ( v < 255 / 2 ) {
      v = 255;
    } else {
      v = 0;
    }
    color.setHsv( h, s, v );
    palette.setColor( mColorLabel->foregroundRole(), color );
    mColorLabel->setPalette( palette );
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
    if ( !mDragging ) {
      grabMouse( Qt::SizeAllCursor );
      mDragging = true;
    }
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

  if ( mDragging ) {
    mDragging = false;
    releaseMouse();
  }

  showLabel();
}

void KLineal::wheelEvent( QWheelEvent *e )
{
  if ( !mRelativeScale ) {
    int numDegrees = e->delta() / 8;
    int numSteps = numDegrees / 15;

    mOffset += numSteps;

    repaint();
    adjustLabel();
    saveSettings();
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
  QFontMetrics metrics = painter.fontMetrics();
  int longLen;
  int shortStart;
  int w = width();
  int h = height();

  // draw a frame around the whole thing
  // (for some unknown reason, this doesn't show up anymore)
  switch ( mOrientation ) {
  case North:
  default:
    shortStart = 0;
    longLen = w;
    painter.drawLine( 0, 0, 0, h - 1 );
    painter.drawLine( 0, h - 1, w - 1, h - 1 );
    painter.drawLine( w - 1, h - 1, w - 1, 0 );
    break;

  case East:
    shortStart = w;
    longLen = h;
    painter.drawLine( 0, 0, 0, h - 1 );
    painter.drawLine( 0, h - 1, w - 1, h - 1 );
    painter.drawLine( w - 1, 0, 0, 0 );
    break;

  case South:
    shortStart = h;
    longLen = w;
    painter.drawLine( 0, 0, 0, h - 1 );
    painter.drawLine( w - 1, h - 1, w - 1, 0 );
    painter.drawLine( w - 1, 0, 0, 0 );
    break;

  case West:
    shortStart = 0;
    longLen = h;
    painter.drawLine( 0, h - 1, w - 1, h - 1 );
    painter.drawLine( w - 1, h - 1, w - 1, 0 );
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

      if ( digit % 20 == 0 ) {
        font.setBold( digit % 100 == 0 );
        painter.setFont( font );
        QString units;
        units.sprintf( "%d", digit );
        QSize textSize = metrics.size( Qt::TextSingleLine, units );
        int tw = textSize.width();
        int th = textSize.height();

        switch ( mOrientation ) {
        case North:
          painter.drawText( x - tw / 2, shortStart + len + th, units );
          break;

        case South:
          painter.drawText( x - tw / 2, shortStart - len - 2, units );
          break;

        case East:
          painter.drawText( shortStart - len - tw - 2, x + th / 2 - 2, units );
          break;

        case West:
          painter.drawText( shortStart + len + 2, x + th / 2 - 2, units );
          break;
        }
      }

      switch( mOrientation ) {
      case North:
        painter.drawLine( x, shortStart, x, shortStart + len );
        break;
      case South:
        painter.drawLine( x, shortStart, x, shortStart - len );
        break;
      case East:
        painter.drawLine( shortStart, x, shortStart - len, x );
        break;
      case West:
        painter.drawLine( shortStart, x, shortStart + len, x );
        break;
      }
    }
  } else {
    float step = longLen / 100.f;
    int len;

    font.setBold( true );
    painter.setFont( font );

    for ( int i = 0; i <= 100; ++i ) {
      int x = (int)( i * step );
      len = ( i % 10 ) ? 6 : 15;

      if ( i % 10 == 0 ) {
        QString units;
        int value = mLeftToRight ? i : ( 100 - i );
        units.sprintf( "%d%%", value );
        QSize textSize = metrics.size( Qt::TextSingleLine, units );
        int tw = textSize.width();
        int th = textSize.height();

        switch ( mOrientation ) {
        case North:
          painter.drawText( x - tw / 2, shortStart + len + th, units );
          break;

        case South:
          painter.drawText( x - tw / 2, shortStart - len - 2, units );
          break;

        case East:
          painter.drawText( shortStart - len - tw - 2, x + th / 2 - 2, units );
          break;

        case West:
          painter.drawText( shortStart + len + 2, x + th / 2 - 2, units );
          break;
        }
      }

      switch( mOrientation ) {
      case North:
        painter.drawLine( x, shortStart, x, shortStart + len );
        break;
      case South:
        painter.drawLine( x, shortStart, x, shortStart - len );
        break;
      case East:
        painter.drawLine( shortStart, x, shortStart - len, x );
        break;
      case West:
        painter.drawLine( shortStart, x, shortStart + len, x );
        break;
      }
    }
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

#include "klineal.moc"
