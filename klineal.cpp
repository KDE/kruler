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

#include <iostream.h>
#include <kconfig.h>
#include <kwin.h>
#include <kcursor.h>
#include <kdebug.h>
#include <kglobal.h>
#include <kpopupmenu.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kmainwindow.h>
#include <qwhatsthis.h>
#include <kstddirs.h>
#include <qpixmap.h>
#include <qiconset.h>
#include <knotifyclient.h>

#include <qdialog.h>
#include <qpainter.h>
#include <qbitmap.h>

#include "klineal.h"

#define CFG_KEY_BGCOLOR "BgColor"
#define CFG_GROUP_SETTINGS "StoredSettings"
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
* load an icon from our data dir
*/
QIconSet KLineal::menuIcon(const char *name)
{
    QString path = KGlobal::dirs()->findResource("data", "kruler/" + QString(name) + ".png");
    // kdDebug() << "found: " << path << endl;
    return QIconSet(QPixmap(path));
}
/**
* create the thingy with no borders and set up
* its members
*/
KLineal::KLineal(QWidget*parent,const char* name):KMainWindow(parent,name){
	if (!name) {
		name = "klineal";
	}
  KWin::setType(winId(), NET::Override);   // or NET::Normal
  KWin::setState(winId(), NET::StaysOnTop);

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
  QColor defaultColor(255, 200, 80);
  if (cfg) {
  		cfg->setGroup(CFG_GROUP_SETTINGS);
      setBackgroundColor(cfg->readColorEntry(CFG_KEY_BGCOLOR, &defaultColor));
    } else {
  		setBackgroundColor(defaultColor);
  }
  mShortEdgeLen = 70;

  mLabel = new QLabel(this);
  mLabel->setGeometry(0,height()-12,32,12);
  mLabel->setBackgroundColor(backgroundColor());
  mLabel->setFont(QFont("Helvetica", 10));
  QWhatsThis::add(mLabel,
  	i18n(
  		"This is the current distance measured in pixels"
  	));
	mColorLabel = new QLabel(this);
  mColorLabel->resize(45,12);
  mColorLabel->setBackgroundColor(backgroundColor());
  mColorLabel->setFont(QFont("fixed", 10));
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
  setOrientation(South);
  setMediumLength();
  mMenu = new KPopupMenu(i18n("K-Ruler"));
  KPopupMenu *oriMenu = new KPopupMenu(this);
  oriMenu->insertItem(menuIcon("kruler-north"), i18n("North"), this, SLOT(setNorth()), Key_N);
  oriMenu->insertItem(menuIcon("kruler-east"), i18n("East"), this, SLOT(setEast()), Key_E);
  oriMenu->insertItem(menuIcon("kruler-south"), i18n("South"), this, SLOT(setSouth()), Key_S);
  oriMenu->insertItem(menuIcon("kruler-west"), i18n("West"), this, SLOT(setWest()), Key_W);
  oriMenu->insertItem(i18n("Turn Right"), this, SLOT(turnRight()), Key_R);
  oriMenu->insertItem(i18n("Turn Left"), this, SLOT(turnLeft()), Key_L);
  mMenu->insertItem(i18n("Orientation"), oriMenu);
  KPopupMenu *lenMenu = new KPopupMenu(this);
  lenMenu->insertItem(i18n("Short"), this, SLOT(setShortLength()), CTRL+Key_S);
  lenMenu->insertItem(i18n("Medium"), this, SLOT(setMediumLength()), CTRL+Key_M);
  lenMenu->insertItem(i18n("Tall"), this, SLOT(setTallLength()), CTRL+Key_T);
  lenMenu->insertItem(i18n("Full Screen Width"), this, SLOT(setFullLength()), CTRL+Key_F);
  mMenu->insertItem(i18n("Length"), lenMenu);
  mMenu->insertItem(i18n("Choose Color..."), this, SLOT(choseColor()), CTRL+Key_C);
  mMenu->insertSeparator();
  mMenu->insertItem(i18n("Help"), helpMenu());
  mMenu->insertSeparator();
  mMenu->insertItem(i18n("Quit"), kapp, SLOT(quit()), CTRL+Key_Q);
	mLastClickPos = geometry().topLeft()+QPoint(width()/2, height()/2);
	_clicked = false;
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
  QRect r = geometry();
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

void KLineal::setOrientation(int inOrientation) {
  QRect r = geometry();
  int nineties = (int)inOrientation - (int)mOrientation;
	QPoint center = mLastClickPos;
	
	if (_clicked) {
		center = mLastClickPos;
		_clicked = false;
	} else {
		center = r.topLeft()+QPoint(width()/2, height()/2);
	}
	
  rotateRect(r, center, nineties);
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
  setCursor(mCurrentCursor);
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
  mLongEdgeLen = QApplication::desktop()->width() * percentOfScreen / 100;
  if (mOrientation == North || mOrientation == South) {
    resize(mLongEdgeLen, height());
  } else {
    resize(width(), mLongEdgeLen);
  }
  if (x()+width() < 10) {
    move (10, y());
  }
  if (y()+height() < 10) {
    move (x(), 10);
  }
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
	QPoint pos = QCursor::pos();
	QSize screen(QApplication::desktop()->size());
	if (pos.x() + mColorSelector.width() > screen.width()) {
		pos.setX(screen.width() - mColorSelector.width());
	}
	if (pos.y() + mColorSelector.height() > screen.height()) {
		pos.setY(screen.height() - mColorSelector.height());
	}
  mStoredColor = backgroundColor();
  mColorSelector.move(pos);
  mColorSelector.show();
  mColorSelector.setColor(backgroundColor());


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
* set the ruler color to the previously selected color
*/
void KLineal::setColor() {
	setColor(mColorSelector.color());
	saveColor();
}

/**
* set the rulere color to some color
*/
void KLineal::setColor(const QColor &color) {
  setBackgroundColor(color);
  mLabel->setBackgroundColor(color);
}

/**
* save the ruler color to the config file
*/
void KLineal::saveColor() {
  KConfig *cfg = kapp->config(); // new KConfig(locateLocal("config", kapp->name()+"rc"));
  if (cfg) {
      QColor color = backgroundColor();
      cfg->setGroup(CFG_GROUP_SETTINGS);
    	cfg->writeEntry(QString(CFG_KEY_BGCOLOR), color);
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
	QSize screen(QApplication::desktop()->size());
  mMenu->move(pos);
  mMenu->show();
	if (pos.x() + mMenu->width() > screen.width()) {
		pos.setX(screen.width() - mMenu->width());
	}
	if (pos.y() + mMenu->height() > screen.height()) {
		pos.setY(screen.height() - mMenu->height());
	}
  mMenu->move(pos);
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
    s.sprintf("%d", cpos.x()-x());
    break;
  case East:
    s.sprintf("%d", cpos.y()-y());
    break;
  case West:
    s.sprintf("%d", cpos.y()-y());
    break;
  case South:
    s.sprintf("%d", cpos.x()-x());
    break;
  }
  mLabel->setText(s);
}
void KLineal::keyPressEvent(QKeyEvent *e) {
	QPoint dist(0,0);
	switch (e->key()) {
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
  KNotifyClient::event("cursormove");
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
  	mColorLabel->setBackgroundColor(color);
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
      grabMouse(KCursor::handCursor());
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
  painter.setFont(QFont("Helvetica", 8));
  QFontMetrics metrics = painter.fontMetrics();
  int longCoo;
  int longLen;
  int shortStart;
  switch (mOrientation) {
  case North:
  default:
    shortStart = 0;
    longLen = width();
    painter.drawLine(0, 0, 0, height()-1);
    painter.drawLine(0, height()-1, width()-1, height()-1);
    painter.drawLine(width()-1, height()-1, width()-1, 0);
    // painter.drawLine(width()-1, 0, 0, 0);
    break;
  case East:
    shortStart = width();
    longLen = height();
    painter.drawLine(0, 0, 0, height()-1);
    painter.drawLine(0, height()-1, width()-1, height()-1);
    // painter.drawLine(width()-1, height()-1, width()-1, 0);
    painter.drawLine(width()-1, 0, 0, 0);
    break;
  case South:
    shortStart = height();
    longLen = width();
    painter.drawLine(0, 0, 0, height()-1);
    // painter.drawLine(0, height()-1, width()-1, height()-1);
    painter.drawLine(width()-1, height()-1, width()-1, 0);
    painter.drawLine(width()-1, 0, 0, 0);
    break;
  case West:
    shortStart = 0;
    longLen = height();
    // painter.drawLine(0, 0, 0, height()-1);
    painter.drawLine(0, height()-1, width()-1, height()-1);
    painter.drawLine(width()-1, height()-1, width()-1, 0);
    painter.drawLine(width()-1, 0, 0, 0);
    break;
  }

  for (longCoo = 0; longCoo < longLen; longCoo+=2) {
    int len;
    if (!(longCoo % 10)) {
      if (!(longCoo % 20)) {
      	len = 15;
      	QString units;
      	units.sprintf("%d", longCoo);
      	QSize textSize = metrics.size(SingleLine, units);
      	switch (mOrientation) {
      	case North:
      	default:
      	  painter.drawText(longCoo - textSize.width()/2, shortStart + len + textSize.height(), units);
      	  break;
      	case South:
      	  painter.drawText(longCoo - textSize.width()/2, shortStart - len - 2, units);
      	  break;
      	case East:
      	  painter.drawText(shortStart - len - textSize.width() - 2, longCoo + textSize.height()/2 - 2, units);
      	  break;
      	case West:
      	  painter.drawText(shortStart + len + 2, longCoo + textSize.height()/2 - 2, units);
      	  break;
      	}
      } else {
       	len = 10;
      }
    } else {
      len = 6;
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
