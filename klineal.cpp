/***************************************************************************
                          klineal.cpp  -  description
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

#include <iostream>

#include <kconfig.h>
#include <kcursor.h>
#include <kdebug.h>
#include <kglobal.h>
#include <kglobalsettings.h>
#include <kiconloader.h>
#include <kimageeffect.h>
#include <klocale.h>
#include <kmainwindow.h>
#include <knotifyclient.h>
#include <kpopupmenu.h>
#include <kstandarddirs.h>
#include <kwin.h>
#include <kstdguiitem.h>

#include <qbitmap.h>
#include <qcursor.h>
#include <qdialog.h>
#include <qiconset.h>
#include <qimage.h>
#include <qpainter.h>
#include <qpixmap.h>
#include <qwhatsthis.h>

#include "klineal.h"

#define CFG_KEY_BGCOLOR "BgColor"
#define CFG_KEY_SCALE_FONT "ScaleFont"
#define CFG_KEY_LENGTH "Length"
#define CFG_GROUP_SETTINGS "StoredSettings"
#define DEFAULT_RULER_COLOR QColor(255, 200, 80)
#define FULLSCREENID 23
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
KLineal::KLineal(QWidget*parent,const char* name):KMainWindow(parent,name){
	if (!name) {
		name = "klineal";
	}
  mLenMenu=0;
  KWin::setType(winId(), NET::Override);   // or NET::Normal
  KWin::setState(winId(), NET::StaysOnTop);
  setPaletteBackgroundColor(black);
  QWhatsThis::add(this,
  i18n(
  	"This is a tool to measure pixel distances and colors on the screen. "
  	"It is useful for working on layouts of dialogs, web pages etc."
  ));
  QBitmap bim = QBitmap(QSize(8, 48), cursorBits);
  QWMatrix m;
  m.rotate(90.0);
  mNorthCursor = QCursor(bim, bim, 3, 47);
  bim = bim.xForm(m);
  mEastCursor = QCursor(bim, bim, 0, 3);
  bim = bim.xForm(m);
  mSouthCursor = QCursor(bim, bim, 4, 0);
  bim = bim.xForm(m);
  mWestCursor = QCursor(bim, bim, 47, 4);

  mCurrentCursor = mNorthCursor;
  setMinimumSize(60,60);
  setMaximumSize(8000,8000);
  KConfig *cfg = kapp->config();
  QColor defaultColor = DEFAULT_RULER_COLOR;
  QFont defaultFont(KGlobalSettings::generalFont().family(), 8);
  defaultFont.setPixelSize(8);
  if (cfg) {
      cfg->setGroup(CFG_GROUP_SETTINGS);
      mColor = cfg->readColorEntry(CFG_KEY_BGCOLOR, &defaultColor);
      mScaleFont = cfg->readFontEntry(CFG_KEY_SCALE_FONT, &defaultFont);
      mLongEdgeLen = cfg->readNumEntry(CFG_KEY_LENGTH, 600);
    } else {
  		mColor = defaultColor;
      mScaleFont = defaultFont;
      mLongEdgeLen = 400;
  }
  kdDebug() << "mLongEdgeLen=" << mLongEdgeLen << endl;
  mShortEdgeLen = 70;

  mLabel = new QLabel(this);
  mLabel->setGeometry(0,height()-12,32,12);
  mLabel->setBackgroundOrigin(ParentOrigin);
  QFont labelFont(KGlobalSettings::generalFont().family(), 10);
  labelFont.setPixelSize(10);
  mLabel->setFont(labelFont);
  QWhatsThis::add(mLabel,
  	i18n(
  		"This is the current distance measured in pixels."
  	));
	mColorLabel = new QLabel(this);
  mColorLabel->resize(45,12);
  mColorLabel->setPaletteBackgroundColor(mColor);
  mColorLabel->hide();
  QFont colorFont(KGlobalSettings::fixedFont().family(), 10);
  colorFont.setPixelSize(10);
  mColorLabel->setFont(colorFont);
  mColorLabel->move(mLabel->pos() + QPoint(0, 20));
  QWhatsThis::add(mColorLabel,
  	i18n(
  		"This is the current color in hexadecimal rgb representation as you may use it in HTML or as a QColor name. "
  		"The rectangles background shows the color of the pixel inside the "
  		"little square at the end of the line cursor."
  	));

  resize(QSize(mLongEdgeLen, mShortEdgeLen));
  setMouseTracking(TRUE);
  mDragging = FALSE;
  mOrientation = South;
  _clicked = false;
  setOrientation(South);
  // setMediumLength();
  mMenu = new KPopupMenu(this);
  mMenu->insertTitle(i18n("KRuler"));
  KPopupMenu *oriMenu = new KPopupMenu(this);
  oriMenu->insertItem(UserIconSet("kruler-north"), i18n("&North"), this, SLOT(setNorth()), Key_N);
  oriMenu->insertItem(UserIconSet("kruler-east"), i18n("&East"), this, SLOT(setEast()), Key_E);
  oriMenu->insertItem(UserIconSet("kruler-south"), i18n("&South"), this, SLOT(setSouth()), Key_S);
  oriMenu->insertItem(UserIconSet("kruler-west"), i18n("&West"), this, SLOT(setWest()), Key_W);
  oriMenu->insertItem(i18n("&Turn Right"), this, SLOT(turnRight()), Key_R);
  oriMenu->insertItem(i18n("Turn &Left"), this, SLOT(turnLeft()), Key_L);
  mMenu->insertItem(i18n("&Orientation"), oriMenu);
  mLenMenu = new KPopupMenu(this);
  mLenMenu->insertItem(i18n("&Short"), this, SLOT(setShortLength()), CTRL+Key_S);
  mLenMenu->insertItem(i18n("&Medium"), this, SLOT(setMediumLength()), CTRL+Key_M);
  mLenMenu->insertItem(i18n("&Tall"), this, SLOT(setTallLength()), CTRL+Key_T);
  mLenMenu->insertItem(i18n("&Full Screen Width"), this, SLOT(setFullLength()), CTRL+Key_F, FULLSCREENID);
  mMenu->insertItem(i18n("&Length"), mLenMenu);
  mMenu->insertItem(SmallIcon("colorscm"), i18n("&Choose Color..."), this, SLOT(choseColor()), CTRL+Key_C);
  mMenu->insertItem(SmallIcon("font"), i18n("Choose &Font..."), this, SLOT(choseFont()), Key_F);
  mMenu->insertSeparator();
  mMenu->insertItem(SmallIcon( "help" ), KStdGuiItem::help().text(), helpMenu());
  mMenu->insertSeparator();
  mMenu->insertItem(SmallIcon( "exit" ), KStdGuiItem::quit().text(), kapp, SLOT(quit()), CTRL+Key_Q);
  mLastClickPos = geometry().topLeft()+QPoint(width()/2, height()/2);
}

KLineal::~KLineal(){
}

void KLineal::move(int x, int y) {
  move(QPoint(x, y));
}

void KLineal::move(const QPoint &p) {
  setGeometry(QRect(p, size()));
}

QPoint KLineal::pos() {
  QRect r = frameGeometry();
  return r.topLeft();
}
int KLineal::x() {
  return pos().x();
}
int KLineal::y() {
  return pos().y();
}

static void rotateRect(QRect &r, QPoint center, int nineties) {
  static int sintab[4] = {0,1,0,-1};
  static int costab[4] = {1,0,-1,0};
  int i=0;
  int x1, y1, x2, y2;
  if (nineties < 0) {
    nineties = -nineties+4;
  }
  i = nineties % 4;
  r.moveBy(-center.x(), -center.y());
  r.coords(&x1, &y1, &x2, &y2);
  r.setCoords(
	      x1 * costab[i] + y1 * sintab[i],
	      -x1 * sintab[i] + y1 * costab[i],
	      x2 * costab[i] + y2 * sintab[i],
	      -x2 * sintab[i] + y2 * costab[i]
	      );
  r = r.normalize();
  r.moveBy(center.x(), center.y());
}

void KLineal::setupBackground() {
  QColor a, b, bg = mColor;
  KImageEffect::GradientType gradType = KImageEffect::HorizontalGradient;
  switch (mOrientation) {
    case North:
      a = bg.light(120);
      b = bg.dark(130);
      gradType = KImageEffect::VerticalGradient;
      break;
    case South:
      b = bg.light(120);
      a = bg.dark(130);
      gradType = KImageEffect::VerticalGradient;
      break;
    case West:
      a = bg.light(120);
      b = bg.dark(130);
      gradType = KImageEffect::HorizontalGradient;
      break;
    case East:
      b = bg.light(120);
      a = bg.dark(130);
      gradType = KImageEffect::HorizontalGradient;
      break;
  }
  QPixmap bgPixmap = QPixmap(KImageEffect::gradient(size(), a, b, gradType));
  setErasePixmap(bgPixmap);
  mLabel->setErasePixmap(bgPixmap);
}

void KLineal::setOrientation(int inOrientation) {
  QRect r = frameGeometry();
  int nineties = (int)inOrientation - (int)mOrientation;
  QPoint center = mLastClickPos;

  if (_clicked) {
    center = mLastClickPos;
    _clicked = false;
   } else {
     center = r.topLeft()+QPoint(width()/2, height()/2);
   }
   
  rotateRect(r, center, nineties);

  QRect desktop = KGlobalSettings::desktopGeometry(this);
  if (r.top() < desktop.top())
     r.moveTop( desktop.top() );
  if (r.bottom() > desktop.bottom())
     r.moveBottom( desktop.bottom() );
  if (r.left() < desktop.left())
     r.moveLeft( desktop.left() );
  if (r.right() > desktop.right())
     r.moveRight( desktop.right() );

  setGeometry(r);
  mOrientation = (inOrientation + 4) % 4;
  switch(mOrientation) {
  case North:
    mLabel->move(4, height()-mLabel->height()-4);
    mColorLabel->move(mLabel->pos() + QPoint(0, -20));
    mCurrentCursor = mNorthCursor;
    break;
  case South:
    mLabel->move(4, 4);
    mColorLabel->move(mLabel->pos() + QPoint(0, 20));
    mCurrentCursor = mSouthCursor;
    break;
  case East:
    mLabel->move(4, 4);
    mColorLabel->move(mLabel->pos() + QPoint(0, 20));
    mCurrentCursor = mEastCursor;
    break;
  case West:
    mLabel->move(width()-mLabel->width()-4, 4);
    mColorLabel->move(mLabel->pos() + QPoint(-5, 20));
    mCurrentCursor = mWestCursor;
    break;
  }
  if (mLenMenu)
    mLenMenu->changeItem(FULLSCREENID, mOrientation % 2 ? i18n("&Full Screen Height") : i18n("&Full Screen Width"));
  setCursor(mCurrentCursor);
  setupBackground();
  repaint();
}
void KLineal::setNorth() {
  setOrientation(North);
}
void KLineal::setEast() {
  setOrientation(East);
}
void KLineal::setSouth() {
  setOrientation(South);
}
void KLineal::setWest() {
  setOrientation(West);
}
void KLineal::turnRight() {
  setOrientation(mOrientation - 1);
}
void KLineal::turnLeft() {
  setOrientation(mOrientation + 1);
}
void KLineal::reLength(int percentOfScreen) {
  if (percentOfScreen < 10) {
    return;
  }
  QRect r = KGlobalSettings::desktopGeometry(this);

  if (mOrientation == North || mOrientation == South) {
    mLongEdgeLen = r.width() * percentOfScreen / 100;
    resize(mLongEdgeLen, height());
  } else {
    mLongEdgeLen = r.height() * percentOfScreen / 100;
    resize(width(), mLongEdgeLen);
  }
  if (x()+width() < 10) {
    move (10, y());
  }
  if (y()+height() < 10) {
    move (x(), 10);
  }
  saveSettings();
}
void KLineal::setShortLength() {
  reLength(30);
}
void KLineal::setMediumLength() {
  reLength(50);
}
void KLineal::setTallLength() {
  reLength(75);
}
void KLineal::setFullLength() {
  reLength(100);
}
void KLineal::choseColor() {
  QRect r = KGlobalSettings::desktopGeometry(this);

  QPoint pos = QCursor::pos();
  if (pos.x() + mColorSelector.width() > r.width()) {
    pos.setX(r.width() - mColorSelector.width());
  }
  if (pos.y() + mColorSelector.height() > r.height()) {
    pos.setY(r.height() - mColorSelector.height());
  }
  mStoredColor = mColor;
  mColorSelector.move(pos);
  mColorSelector.setColor(mColor);
  mColorSelector.setDefaultColor( DEFAULT_RULER_COLOR );
  mColorSelector.show();

  connect(&mColorSelector, SIGNAL(okClicked()), this, SLOT(setColor()));
  connect(&mColorSelector, SIGNAL(yesClicked()), this, SLOT(setColor()));
  connect(&mColorSelector, SIGNAL(closeClicked()), this, SLOT(setColor()));
  connect(&mColorSelector, SIGNAL(defaultClicked()), this, SLOT(setColor()));
  connect(&mColorSelector, SIGNAL(colorSelected(const QColor&)), this, SLOT(setColor(const QColor&)));
	/*
  connect(&mColorSelector, SIGNAL(cancelPressed()), this, SLOT(restoreColor()));
  connect(&mColorSelector, SIGNAL(okPressed()), this, SLOT(saveColor()));
  */
}

/**
* slot to choose a font
*/
void KLineal::choseFont() {
  QFont font = mScaleFont;
  int result = KFontDialog::getFont(font, false, this);
  if (result == KFontDialog::Accepted) {
    setFont(font);
  }
}

/**
* set the ruler color to the previously selected color
*/
void KLineal::setFont(QFont &font) {
	mScaleFont = font;
	saveSettings();
  repaint();
}


/**
* set the ruler color to the previously selected color
*/
void KLineal::setColor() {
	setColor(mColorSelector.color());
	saveSettings();
}

/**
* set the ruler color to some color
*/
void KLineal::setColor(const QColor &color) {
  mColor = color;
  if ( !mColor.isValid() )
    mColor = DEFAULT_RULER_COLOR;
  setupBackground();
}

/**
* save the ruler color to the config file
*/
void KLineal::saveSettings() {
  KConfig *cfg = kapp->config(); // new KConfig(locateLocal("config", kapp->name()+"rc"));
  if (cfg) {
      QColor color = mColor;
      cfg->setGroup(CFG_GROUP_SETTINGS);
    	cfg->writeEntry(QString(CFG_KEY_BGCOLOR), color);
    	cfg->writeEntry(QString(CFG_KEY_SCALE_FONT), mScaleFont);
    	cfg->writeEntry(QString(CFG_KEY_LENGTH), mLongEdgeLen);
      cfg->sync();
  }
}

/**
* restores the color
*/
void KLineal::restoreColor() {
  setColor(mStoredColor);
}
/**
* lets the context menu appear at current cursor position
*/
void KLineal::showMenu() {
	QPoint pos = QCursor::pos();
	mMenu->popup(pos);
}

/**
* overwritten to switch the value label and line cursor on
*/
void KLineal::enterEvent(QEvent * /*inEvent*/) {
  if (!mDragging) showLabel();
}
/**
* overwritten to switch the value label and line cursor off
*/
void KLineal::leaveEvent(QEvent * /*inEvent*/) {
  if (!geometry().contains(QCursor::pos())) {
    hideLabel();
  }
}
/**
* shows the value lable
*/
void KLineal::showLabel() {
  adjustLabel();
  mLabel->show();
  mColorLabel->show();
}
/**
* hides the value label
*/
void KLineal::hideLabel() {
  mLabel->hide();
  mColorLabel->hide();
}
/**
* updates the current value label
*/
void KLineal::adjustLabel() {
  QString s;
  QPoint cpos = QCursor::pos();
  switch (mOrientation) {
  case North:
    s.sprintf("%d px", cpos.x()-x());
    break;
  case East:
    s.sprintf("%d px", cpos.y()-y());
    break;
  case West:
    s.sprintf("%d px", cpos.y()-y());
    break;
  case South:
    s.sprintf("%d px", cpos.x()-x());
    break;
  }
  mLabel->setText(s);
}
void KLineal::keyPressEvent(QKeyEvent *e) {
	QPoint dist(0,0);
	switch (e->key()) {
	case Key_F1:
    	kapp->invokeHelp();
 			break;
  	case Key_Left:
    	dist.setX(-1);
 			break;
  	case Key_Right:
    	dist.setX(1);
 			break;
  	case Key_Up:
    	dist.setY(-1);
 			break;
  	case Key_Down:
    	dist.setY(1);
 			break;
  	default:
    	KMainWindow::keyPressEvent(e);
      return;
 	}
	if (e->state() & ShiftButton) {
  	dist *= 10;
  }
  move(pos()+dist);
  KNotifyClient::event(0, "cursormove",  QString::null);
}
/**
* overwritten to handle the line cursor which is a seperate widget outside the main
* window. Also used for dragging.
*/
void KLineal::mouseMoveEvent(QMouseEvent * /*inEvent*/) {
  if (mDragging && this == mouseGrabber()) {
    move(QCursor::pos() - mDragOffset);
  } else {
  	QPoint p = QCursor::pos();
  	switch (mOrientation) {
    case North:
    	p.setY(p.y()-46);
      break;
    case East:
    	p.setX(p.x()+46);
      break;
    case West:
    	p.setX(p.x()-46);
      break;
    case South:
    	p.setY(p.y()+46);
    	break;
    }
    // cerr << p.x()-x() << "," << p.y()-y() << ": " << KColorDialog::grabColor(p).name() << endl;
    QColor color = KColorDialog::grabColor(p);
    int h, s, v;
    color.hsv(&h, &s, &v);
 		mColorLabel->setText(color.name().upper());
  	mColorLabel->setPaletteBackgroundColor(color);
  	if (v < 255/2) {
  		v = 255;
  	} else {
  		v = 0;
   	}
   	color.setHsv(h, s, v);
  	QPalette palette = mColorLabel->palette();
  	palette.setColor(QColorGroup::Foreground, color);
  	mColorLabel->setPalette(palette);
    adjustLabel();
  }
}

/**
* overwritten for dragging and contect menu
*/
void KLineal::mousePressEvent(QMouseEvent *inEvent) {
  mLastClickPos = QCursor::pos();
  hideLabel();

  QRect gr = geometry();
  mDragOffset = mLastClickPos - QPoint(gr.left(), gr.top());
  if (inEvent->button() == LeftButton) {
    if (!mDragging) {
      grabMouse(KCursor::sizeAllCursor());
      mDragging = TRUE;
    }
  } else if (inEvent->button() == MidButton) {
		_clicked = true;
    turnLeft();
  } else if (inEvent->button() == RightButton) {
    showMenu();
  }
}
/**
* overwritten for dragging
*/
void KLineal::mouseReleaseEvent(QMouseEvent * /*inEvent*/) {
  if (mDragging) {
    mDragging = FALSE;
    releaseMouse();
  }
  showLabel();
}
/**
* draws the scale according to the orientation
*/
void KLineal::drawScale(QPainter &painter) {
  painter.setPen(black);
  QFont font = mScaleFont;
  // font.setPixelSize(9);
  painter.setFont(font);
  QFontMetrics metrics = painter.fontMetrics();
  int longCoo;
  int longLen;
  int shortStart;
  int w = width();
  int h = height();

  // draw a frame around the whole thing
  // (for some unknown reason, this doesn't show up anymore)
  switch (mOrientation) {
  case North:
  default:
    shortStart = 0;
    longLen = w;
    painter.drawLine(0, 0, 0, h-1);
    painter.drawLine(0, h-1, w-1, h-1);
    painter.drawLine(w-1, h-1, w-1, 0);
    // painter.drawLine(width()-1, 0, 0, 0);
    break;
  case East:
    shortStart = w;
    longLen = h;
    painter.drawLine(0, 0, 0, h-1);
    painter.drawLine(0, h-1, w-1, h-1);
    // painter.drawLine(width()-1, height()-1, width()-1, 0);
    painter.drawLine(w-1, 0, 0, 0);
    break;
  case South:
    shortStart = h;
    longLen = w;
    painter.drawLine(0, 0, 0, h-1);
    // painter.drawLine(0, height()-1, width()-1, height()-1);
    painter.drawLine(w-1, h-1, w-1, 0);
    painter.drawLine(w-1, 0, 0, 0);
    break;
  case West:
    shortStart = 0;
    longLen = h;
    // painter.drawLine(0, 0, 0, height()-1);
    painter.drawLine(0, h-1, w-1, h-1);
    painter.drawLine(w-1, h-1, w-1, 0);
    painter.drawLine(w-1, 0, 0, 0);
    break;
  }
  int ten = 10, twenty = 20, fourty = 40, hundred = 100;
  for (longCoo = 0; longCoo < longLen; longCoo+=2) {
    int len = 6;
    if (ten == 10) {
      if (twenty == 20) {
        /**/
        if (hundred == 100) {
          font.setBold(true);
          painter.setFont(font);
          len = 18;
        } else {
      	  len = 15;
        }
      	QString units;
        int digits;
        if (hundred == 100 || mOrientation == West || mOrientation == East) {
          digits = longCoo;
        } else {
          digits = longCoo % 100;
        }
      	units.sprintf("%d", digits);
      	QSize textSize = metrics.size(SingleLine, units);
        int tw = textSize.width();
        int th = textSize.height();
       	switch (mOrientation) {
       	case North:
           if (digits < 1000  || fourty == 40 || hundred == 100) {
            if (longCoo != 0) {
       	      painter.drawText(longCoo - tw/2, shortStart + len + th, units);
            } else {
              painter.drawText(1, shortStart + len + th, units);
            }
           }
       	  break;
       	case South:
           if (digits < 1000  || fourty == 40 || hundred == 100) {
            if (longCoo != 0) {
       	      painter.drawText(longCoo - tw/2, shortStart - len - 2, units);
            } else {
       	      painter.drawText(1, shortStart - len - 2, units);
            }
           }
       	  break;
       	case East:
          if (longCoo != 0) {
       	    painter.drawText(shortStart - len - tw - 2, longCoo + th/2 - 2, units);
          } else {
       	    painter.drawText(shortStart - len - tw - 2, th-2, units);
          }
       	  break;
       	case West:
          if (longCoo != 0) {
       	    painter.drawText(shortStart + len + 2, longCoo + th/2 - 2, units);
          } else {
       	    painter.drawText(shortStart + len + 2, th-2, units);
          }
       	  break;
       	}
      } else {
       	len = 10;
      }
    }
    switch(mOrientation) {
    case North:
      painter.drawLine(longCoo, shortStart, longCoo, shortStart+len);
      break;
    case South:
      painter.drawLine(longCoo, shortStart, longCoo, shortStart-len);
      break;
    case East:
      painter.drawLine(shortStart, longCoo, shortStart-len, longCoo);
      break;
    case West:
      painter.drawLine(shortStart, longCoo, shortStart+len, longCoo);
      break;
    }
    ten = (ten == 10) ? 2: ten + 2;
    twenty = (twenty == 20) ? 2: twenty + 2;
    fourty = (fourty == 40) ? 2: fourty + 2;
    if (hundred == 100) {
      hundred = 2;
      font.setBold(false);
      painter.setFont(font);
    } else {
      hundred += 2;
    }
  }
}
/**
* actually draws the ruler
*/
void KLineal::paintEvent(QPaintEvent * /*inEvent*/) {
  QPainter painter;
  painter.begin(this);
  drawScale(painter);
  painter.end();
}

#include "klineal.moc"
