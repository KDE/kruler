/*
    SPDX-FileCopyrightText: 2000 Till Krech <till@snafu.de>
    SPDX-FileCopyrightText: 2008 Mathias Soeken <msoeken@informatik.uni-bremen.de>
    SPDX-FileCopyrightText: 2017 Aurélien Gâteau <agateau@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "klineal.h"

#include <QAction>
#include <QApplication>
#include <QBrush>
#include <QInputDialog>
#include <QMenu>
#include <QMouseEvent>
#include <QPainter>
#include <QScreen>
#include <QSlider>
#include <QWidgetAction>
#include <QWindow>

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
#include <KX11Extras>
#include <netwm.h>
#include <private/qtx11extras_p.h>
#endif

#include "kruler.h"
#include "krulersystemtray.h"

#include "ui_cfg_advanced.h"
#include "ui_cfg_appearance.h"

static const int RESIZE_HANDLE_LENGTH = 7;
static const int RESIZE_HANDLE_THICKNESS = 24;
static const qreal RESIZE_HANDLE_OPACITY = 0.3;

static const int SMALL_TICK_SIZE = 6;
static const int MEDIUM1_TICK_SIZE = 10;
static const int MEDIUM2_TICK_SIZE = 15;
static const int LARGE_TICK_SIZE = 18;
static const qreal TICK_OPACITY = 0.3;

static const int THICKNESS = 70;
static const int RULER_MAX_LENGTH = 10000;

static const qreal OVERLAY_OPACITY = 0.1;
static const qreal OVERLAY_BORDER_OPACITY = 0.3;

static const int INDICATOR_MARGIN = 6;
static const int INDICATOR_RECT_RADIUS = 3;
static const qreal INDICATOR_RECT_OPACITY = 0.6;

static const int CURSOR_SIZE = 15; // Must be an odd number

/**
 * create the thingy with no borders and set up
 * its members
 */
KLineal::KLineal(QWidget *parent)
    : QWidget(parent)
{
    setAttribute(Qt::WA_TranslucentBackground);
#ifdef KRULER_HAVE_X11
    if (KWindowSystem::isPlatformX11()) {
        KX11Extras::setType(winId(), NET::Override); // or NET::Normal
    }
#endif

    setWindowTitle(i18nc("@title:window", "KRuler"));

    setWhatsThis(
        i18n("This is a tool to measure pixel distances on the screen. "
             "It is useful for working on layouts of dialogs, web pages etc."));
    setMouseTracking(true);

    mColor = RulerSettings::self()->bgColor();
    mScaleFont = RulerSettings::self()->scaleFont();
    int restoreLength = RulerSettings::self()->length();
    mHorizontal = RulerSettings::self()->horizontal();
    mLeftToRight = RulerSettings::self()->leftToRight();
    mOffset = RulerSettings::self()->offset();
    mRelativeScale = RulerSettings::self()->relativeScale();
    mAlwaysOnTopLayer = RulerSettings::self()->alwaysOnTop();

    createCrossCursor();

    // BEGIN setup menu and actions
    mActionCollection = new KActionCollection(this);
    mActionCollection->setConfigGroup(QStringLiteral("Actions"));

    mMenu = new QMenu(this);
    addAction(mMenu, QIcon::fromTheme(QStringLiteral("object-rotate-left")), i18n("Rotate"), this, SLOT(rotate()), Qt::Key_R, QStringLiteral("turn_right"));

    QMenu *scaleMenu = new QMenu(i18n("&Scale"), this);
    mScaleDirectionAction = addAction(scaleMenu, QIcon(), i18n("Right to Left"), this, SLOT(switchDirection()), Qt::Key_D, QStringLiteral("right_to_left"));
    mCenterOriginAction = addAction(scaleMenu, QIcon(), i18n("Center Origin"), this, SLOT(centerOrigin()), Qt::Key_C, QStringLiteral("center_origin"));
    mCenterOriginAction->setEnabled(!mRelativeScale);
    mOffsetAction = addAction(scaleMenu, QIcon(), i18n("Offset..."), this, SLOT(slotOffset()), Qt::Key_O, QStringLiteral("set_offset"));
    mOffsetAction->setEnabled(!mRelativeScale);
    scaleMenu->addSeparator();
    QAction *relativeScaleAction = addAction(scaleMenu, QIcon(), i18n("Percentage"), nullptr, nullptr, QKeySequence(), QStringLiteral("toggle_percentage"));
    relativeScaleAction->setCheckable(true);
    relativeScaleAction->setChecked(mRelativeScale);
    connect(relativeScaleAction, &QAction::toggled, this, &KLineal::switchRelativeScale);
    mMenu->addMenu(scaleMenu);

    mOpacity = RulerSettings::self()->opacity();
    QMenu *opacityMenu = new QMenu(i18n("O&pacity"), this);
    QWidgetAction *opacityAction = new QWidgetAction(this);
    QSlider *slider = new QSlider(this);
    slider->setMinimum(15);
    slider->setMaximum(255);
    slider->setSingleStep(1);
    slider->setOrientation(Qt::Horizontal);
    slider->setValue(RulerSettings::self()->opacity());
    // Show ticks so that the slider is a bit taller, thus easier to grab
    slider->setTickPosition(QSlider::TicksBothSides);
    slider->setTickInterval(30);
    slider->setMinimumWidth(200);
    connect(slider, &QSlider::valueChanged, this, &KLineal::slotOpacity);
    opacityAction->setDefaultWidget(slider);
    opacityMenu->addAction(opacityAction);
    mMenu->addMenu(opacityMenu);

    QAction *keyBindings = KStandardAction::keyBindings(this, &KLineal::slotKeyBindings, mActionCollection);
    mMenu->addAction(keyBindings);
    QAction *preferences = KStandardAction::preferences(this, &KLineal::slotPreferences, mActionCollection);
    mMenu->addAction(preferences);
    mMenu->addSeparator();
#if KXMLGUI_VERSION >= QT_VERSION_CHECK(6, 9, 0)
    auto helpMenu = new KHelpMenu(this, KAboutData::applicationData());
    helpMenu->setShowWhatsThis(true);
    mMenu->addMenu(helpMenu->menu());
#else
    mMenu->addMenu((new KHelpMenu(this, KAboutData::applicationData(), true))->menu());
#endif
    mMenu->addSeparator();
    if (RulerSettings::self()->trayIcon()) {
        createSystemTray();
    }

    QAction *quit = KStandardAction::quit(qApp, &QGuiApplication::quit, mActionCollection);
    mMenu->addAction(quit);

    mActionCollection->associateWidget(this);
    mActionCollection->readSettings();

    mLastClickGlobalPos = geometry().topLeft() + QPoint(width() / 2, height() / 2);

    setWindowFlags(mAlwaysOnTopLayer ? Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint : Qt::FramelessWindowHint);

    QSize restoreSize(restoreLength, THICKNESS);
    resize(mHorizontal ? restoreSize : restoreSize.transposed());
    setMinimumSize(THICKNESS, THICKNESS);
    setMaximumSize(RULER_MAX_LENGTH, RULER_MAX_LENGTH);
    setHorizontal(mHorizontal);
}

KLineal::~KLineal()
{
    saveSettings();
    delete mTrayIcon;
}

void KLineal::createCrossCursor()
{
    QPixmap pix(CURSOR_SIZE, CURSOR_SIZE);
    int halfSize = CURSOR_SIZE / 2;
    {
        pix.fill(Qt::transparent);
        QPainter painter(&pix);
        painter.setPen(Qt::red);
        painter.drawLine(0, halfSize, CURSOR_SIZE - 1, halfSize);
        painter.drawLine(halfSize, 0, halfSize, CURSOR_SIZE - 1);
    }
    mCrossCursor = QCursor(pix, halfSize, halfSize);
}

void KLineal::createSystemTray()
{
    mCloseAction = mActionCollection->addAction(KStandardAction::Close, this, SLOT(slotClose()));
    mMenu->addAction(mCloseAction);

    if (!mTrayIcon) {
        mTrayIcon = new KRulerSystemTray(QStringLiteral("kruler-symbolic"), this, mActionCollection);
        mTrayIcon->setCategory(KStatusNotifierItem::ApplicationStatus);
    }
}

QAction *KLineal::addAction(QMenu *menu,
                            const QIcon &icon,
                            const QString &text,
                            const QObject *receiver,
                            const char *member,
                            const QKeySequence &shortcut,
                            const QString &name)
{
    QAction *action = new QAction(icon, text, mActionCollection);
    mActionCollection->setDefaultShortcut(action, shortcut);
    if (receiver) {
        connect(action, SIGNAL(triggered()), receiver, member);
    }
    menu->addAction(action);
    mActionCollection->addAction(name, action);
    return action;
}

void KLineal::slotClose()
{
    hide();
}

void KLineal::move(const QPoint &p)
{
    setGeometry(QRect(p, size()));
}

QPoint KLineal::pos() const
{
    return frameGeometry().topLeft();
}

void KLineal::drawBackground(QPainter &painter)
{
    QColor a, b, bg = mColor;
    QLinearGradient gradient;
    if (mHorizontal) {
        a = bg.lighter(120);
        b = bg.darker(130);
        gradient = QLinearGradient(1, 0, 1, height());
    } else {
        a = bg.lighter(120);
        b = bg.darker(130);
        gradient = QLinearGradient(0, 1, width(), 1);
    }
    a.setAlpha(mOpacity);
    b.setAlpha(mOpacity);
    gradient.setColorAt(0, a);
    gradient.setColorAt(1, b);
    painter.fillRect(rect(), QBrush(gradient));
}

void KLineal::setHorizontal(bool horizontal)
{
    QRect r = frameGeometry();
    if (mHorizontal != horizontal) {
        // relax restrictions before resizing
        setMaximumSize(RULER_MAX_LENGTH, RULER_MAX_LENGTH);
        r.setSize(r.size().transposed());
        if (horizontal) {
            setMaximumHeight(THICKNESS);
        } else {
            setMaximumWidth(THICKNESS);
        }
    }
    mHorizontal = horizontal;
    QPoint center = mLastClickGlobalPos;

    if (middleClicked) {
        center = mLastClickGlobalPos;
        middleClicked = false;
    } else {
        center = r.topLeft() + QPoint(width() / 2, height() / 2);
    }

    QPoint newTopLeft = QPoint(center.x() - height() / 2, center.y() - width() / 2);
    r.moveTo(newTopLeft);

    QRect desktop = QGuiApplication::primaryScreen()->geometry();

    if (r.width() > desktop.width()) {
        r.setWidth(desktop.width());
    }

    if (r.height() > desktop.height()) {
        r.setHeight(desktop.height());
    }

    if (r.top() < desktop.top()) {
        r.moveTop(desktop.top());
    }

    if (r.bottom() > desktop.bottom()) {
        r.moveBottom(desktop.bottom());
    }

    if (r.left() < desktop.left()) {
        r.moveLeft(desktop.left());
    }

    if (r.right() > desktop.right()) {
        r.moveRight(desktop.right());
    }

    setGeometry(r);

    updateScaleDirectionMenuItem();

    saveSettings();
}

void KLineal::rotate()
{
    setHorizontal(!mHorizontal);
}

void KLineal::updateScaleDirectionMenuItem()
{
    if (!mScaleDirectionAction)
        return;

    QString label;

    if (mHorizontal) {
        label = mLeftToRight ? i18n("Right to Left") : i18n("Left to Right");
    } else {
        label = mLeftToRight ? i18n("Bottom to Top") : i18n("Top to Bottom");
    }

    mScaleDirectionAction->setText(label);
}

QRect KLineal::beginRect() const
{
    int shortLen = RESIZE_HANDLE_THICKNESS;
    return mHorizontal ? QRect(0, (height() - shortLen) / 2 + 1, RESIZE_HANDLE_LENGTH, shortLen)
                       : QRect((width() - shortLen) / 2, 0, shortLen, RESIZE_HANDLE_LENGTH);
}

QRect KLineal::endRect() const
{
    int shortLen = RESIZE_HANDLE_THICKNESS;
    return mHorizontal ? QRect(width() - RESIZE_HANDLE_LENGTH, (height() - shortLen) / 2 + 1, RESIZE_HANDLE_LENGTH, shortLen)
                       : QRect((width() - shortLen) / 2, height() - RESIZE_HANDLE_LENGTH, shortLen, RESIZE_HANDLE_LENGTH);
}

Qt::CursorShape KLineal::resizeCursor() const
{
    return mHorizontal ? Qt::SizeHorCursor : Qt::SizeVerCursor;
}

void KLineal::switchDirection()
{
    mLeftToRight = !mLeftToRight;
    updateScaleDirectionMenuItem();
    repaint();
    saveSettings();
}

void KLineal::centerOrigin()
{
    mOffset = -qRound(length() * pixelRatio() / 2);
    repaint();
    saveSettings();
}

void KLineal::slotOffset()
{
    bool ok;
    int newOffset = QInputDialog::getInt(this, i18nc("@title:window", "Scale Offset"), i18n("Offset:"), mOffset, -2147483647, 2147483647, 1, &ok);

    if (ok) {
        mOffset = newOffset;
        repaint();
        saveSettings();
    }
}

void KLineal::slotOpacity(int value)
{
    mOpacity = value;
    repaint();
    RulerSettings::self()->setOpacity(value);
    RulerSettings::self()->save();
}

void KLineal::slotKeyBindings()
{
    KShortcutsDialog::showDialog(mActionCollection, KShortcutsEditor::LetterShortcutsAllowed, this);
}

void KLineal::slotPreferences()
{
    KConfigDialog *dialog = new KConfigDialog(this, QStringLiteral("settings"), RulerSettings::self());

    Ui::ConfigAppearance appearanceConfig;
    QWidget *appearanceConfigWidget = new QWidget(dialog);
    appearanceConfig.setupUi(appearanceConfigWidget);
    dialog->addPage(appearanceConfigWidget, i18n("Appearance"), QStringLiteral("preferences-desktop-default-applications"));

#ifdef KRULER_HAVE_X11
    // Advanced page only contains the "Native moving" and "Always on top" settings, disable when not running on X11
    if (QX11Info::isPlatformX11()) {
        Ui::ConfigAdvanced advancedConfig;
        QWidget *advancedConfigWidget = new QWidget(dialog);
        advancedConfig.setupUi(advancedConfigWidget);
        dialog->addPage(advancedConfigWidget, i18n("Advanced"), QStringLiteral("preferences-other"));
        dialog->setFaceType(KConfigDialog::List);
    } else {
        dialog->setFaceType(KConfigDialog::Plain);
    }
#else
    dialog->setFaceType(KConfigDialog::Plain);
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

    setWindowFlags(mAlwaysOnTopLayer ? Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint : Qt::FramelessWindowHint);

    if (RulerSettings::self()->trayIcon()) {
        if (!mTrayIcon) {
            createSystemTray();
        }
    } else {
        delete mTrayIcon;
        mTrayIcon = nullptr;

        if (mCloseAction) {
            mCloseAction->setVisible(false);
        }
    }
    show();
    repaint();
}

void KLineal::switchRelativeScale(bool checked)
{
    mRelativeScale = checked;

    mCenterOriginAction->setEnabled(!mRelativeScale);
    mOffsetAction->setEnabled(!mRelativeScale);

    repaint();
    saveSettings();
}

/**
 * save the ruler color to the config file
 */
void KLineal::saveSettings()
{
    RulerSettings::self()->setBgColor(mColor);
    RulerSettings::self()->setScaleFont(mScaleFont);
    RulerSettings::self()->setLength(length());
    RulerSettings::self()->setHorizontal(mHorizontal);
    RulerSettings::self()->setLeftToRight(mLeftToRight);
    RulerSettings::self()->setOffset(mOffset);
    RulerSettings::self()->setRelativeScale(mRelativeScale);
    RulerSettings::self()->setAlwaysOnTop(mAlwaysOnTopLayer);
    RulerSettings::self()->save();
}

/**
 * lets the context menu appear at current cursor position
 */
void KLineal::showMenu()
{
    QPoint pos = QCursor::pos();
    mMenu->popup(pos);
}

int KLineal::length() const
{
    return mHorizontal ? width() : height();
}

inline qreal KLineal::pixelRatio() const
{
    if (!windowHandle())
        return 1;
    return windowHandle()->devicePixelRatio();
}

QString KLineal::indicatorText(int xy) const
{
    if (!mRelativeScale) {
        int len = mLeftToRight ? xy + 1 : length() - xy;
        return i18n("%1 px", qRound(len * pixelRatio()));
    } else {
        int len = (xy * 100) / length();

        if (!mLeftToRight) {
            len = 100 - len;
        }
        return i18n("%1%", len);
    }
}

void KLineal::keyPressEvent(QKeyEvent *e)
{
    QPoint dist;

    switch (e->key()) {
    case Qt::Key_F1:
        KHelpClient::invokeHelp();
        return;

    case Qt::Key_Left:
        dist.setX(-1);
        break;

    case Qt::Key_Right:
        dist.setX(1);
        break;

    case Qt::Key_Up:
        dist.setY(-1);
        break;

    case Qt::Key_Down:
        dist.setY(1);
        break;

    default:
        QWidget::keyPressEvent(e);
        return;
    }

    if (e->modifiers() & Qt::ShiftModifier) {
        dist *= 10;
    }

    if (e->modifiers() & Qt::AltModifier) {
        QCursor::setPos(QCursor::pos() + dist);
    } else {
        move(pos() + dist);
        update();
    }
    KNotification::event(QString(), QStringLiteral("cursormove"), QString());
}

void KLineal::leaveEvent(QEvent *e)
{
    Q_UNUSED(e);
    update();
}

/**
 * overwritten for dragging and context menu
 */
void KLineal::mousePressEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton) {
        if (beginRect().contains(mCursorPos)) {
            windowHandle()->startSystemResize(mHorizontal ? Qt::LeftEdge : Qt::TopEdge);
        } else if (endRect().contains(mCursorPos)) {
            windowHandle()->startSystemResize(mHorizontal ? Qt::RightEdge : Qt::BottomEdge);
        } else {
            windowHandle()->startSystemMove();
        }
    } else if (e->button() == Qt::MiddleButton) {
        middleClicked = true;
        mLastClickGlobalPos = QCursor::pos();
        rotate();
    } else if (e->button() == Qt::RightButton) {
        showMenu();
    }
}

void KLineal::mouseMoveEvent(QMouseEvent *e)
{
    mCursorPos = e->position().toPoint();
    if (beginRect().contains(mCursorPos) || endRect().contains(mCursorPos)) {
        setCursor(resizeCursor());
    } else {
        setCursor(mCrossCursor);
    }
    update();
}

void KLineal::wheelEvent(QWheelEvent *e)
{
    int numDegrees = e->angleDelta().y() / 8;
    int numSteps = numDegrees / 15;

    // changing offset
    if (e->buttons() == Qt::LeftButton) {
        if (!mRelativeScale) {
            mOffset += numSteps;

            repaint();
            saveSettings();
        }
    }

    QWidget::wheelEvent(e);
}

/**
 * draws the scale according to the orientation
 */
void KLineal::drawScale(QPainter &painter)
{
    painter.setPen(QPen(Qt::black, 1 / pixelRatio()));
    QFont font = mScaleFont;
    painter.setFont(font);
    int longLen = length() * pixelRatio();

    if (!mRelativeScale) {
        int digit;
        int len;
        // Draw from -1 to longLen rather than from 0 to longLen - 1 to take into
        // account the offset applied in drawScaleTick
        for (int x = -1; x <= longLen; ++x) {
            if (mLeftToRight) {
                digit = x + mOffset;
            } else {
                digit = longLen - x + mOffset;
            }

            if (digit % 2)
                continue;

            if (digit % 100 == 0) {
                len = LARGE_TICK_SIZE;
            } else if (digit % 20 == 0) {
                len = MEDIUM2_TICK_SIZE;
            } else if (digit % 10 == 0) {
                len = MEDIUM1_TICK_SIZE;
            } else {
                len = SMALL_TICK_SIZE;
            }

            if (digit % 100 == 0 && digit != 0) {
                QString units = QStringLiteral("%1").arg(digit);
                drawScaleText(painter, x, units);
            }

            drawScaleTick(painter, x, len);
        }
    } else {
        float step = longLen / 100.f;
        int len;

        for (int i = 0; i <= 100; ++i) {
            int x = (int)(i * step);

            if (i % 10 == 0 && i != 0 && i != 100) {
                int value = mLeftToRight ? i : (100 - i);
                const QString units = QString::asprintf("%d%%", value);
                drawScaleText(painter, x, units);
                len = MEDIUM2_TICK_SIZE;
            } else {
                len = SMALL_TICK_SIZE;
            }

            drawScaleTick(painter, x, len);
        }
    }
}

void KLineal::drawScaleText(QPainter &painter, int x, const QString &text)
{
    QFontMetrics metrics = painter.fontMetrics();
    QSize textSize = metrics.size(Qt::TextSingleLine, text);
    qreal w = width();
    qreal h = height();
    qreal tw = textSize.width();
    qreal th = metrics.ascent();
    qreal lx = x / pixelRatio();

    if (mHorizontal) {
        painter.drawText(QPointF(lx - tw / 2, (h + th) / 2), text);
    } else {
        painter.drawText(QPointF((w - tw) / 2, lx + th / 2), text);
    }
}

void KLineal::drawScaleTick(QPainter &painter, int x, int len)
{
    painter.setOpacity(TICK_OPACITY);
    qreal w = width();
    qreal h = height();
    // Offset by one because we are measuring lengths, not positions, so when the
    // indicator is at position 0 it measures a length of 1 pixel.
    if (mLeftToRight) {
        --x;
    } else {
        ++x;
    }

    // The value is in physical coords, but Qt is drawing in logical coords.
    qreal lx = x / pixelRatio();

    if (mHorizontal) {
        painter.drawLine(QLineF(lx, 0, lx, len));
        painter.drawLine(QLineF(lx, h, lx, h - len));
    } else {
        painter.drawLine(QLineF(0, lx, len, lx));
        painter.drawLine(QLineF(w, lx, w - len, lx));
    }
    painter.setOpacity(1);
}

void KLineal::drawResizeHandle(QPainter &painter, Qt::Edge edge)
{
    QRect rect;
    switch (edge) {
    case Qt::LeftEdge:
    case Qt::TopEdge:
        rect = beginRect();
        break;
    case Qt::RightEdge:
    case Qt::BottomEdge:
        rect = endRect();
        break;
    }
    painter.setOpacity(RESIZE_HANDLE_OPACITY);
    if (mHorizontal) {
        int y1 = (THICKNESS - RESIZE_HANDLE_THICKNESS) / 2;
        int y2 = y1 + RESIZE_HANDLE_THICKNESS - 1;
        for (int x = rect.left() + 1; x < rect.right(); x += 2) {
            painter.drawLine(x, y1, x, y2);
        }
    } else {
        int x1 = (THICKNESS - RESIZE_HANDLE_THICKNESS) / 2;
        int x2 = x1 + RESIZE_HANDLE_THICKNESS - 1;
        for (int y = rect.top() + 1; y < rect.bottom(); y += 2) {
            painter.drawLine(x1, y, x2, y);
        }
    }
    painter.setOpacity(1);
}

void KLineal::drawIndicatorOverlay(QPainter &painter, int xy)
{
    painter.setPen(Qt::red);
    painter.setOpacity(OVERLAY_OPACITY);
    if (mHorizontal) {
        QPointF p1(mLeftToRight ? 0 : width(), 0);
        QPointF p2(xy, THICKNESS);
        QRectF rect(p1, p2);
        painter.fillRect(rect, Qt::red);

        painter.setOpacity(OVERLAY_BORDER_OPACITY);
        painter.drawLine(xy, 0, xy, THICKNESS);
    } else {
        QPointF p1(0, mLeftToRight ? 0 : height());
        QPointF p2(THICKNESS, xy);
        QRectF rect(p1, p2);
        painter.fillRect(rect, Qt::red);

        painter.setOpacity(OVERLAY_BORDER_OPACITY);
        painter.drawLine(0, xy, THICKNESS, xy);
    }
}

void KLineal::drawIndicatorText(QPainter &painter, int xy)
{
    QString text = indicatorText(xy);
    painter.setFont(font());
    QFontMetrics fm = QFontMetrics(font());
    int tx, ty;
    int tw = fm.boundingRect(text).width();
    if (mHorizontal) {
        tx = xy + INDICATOR_MARGIN;
        if (tx + tw > width()) {
            tx = xy - tw - INDICATOR_MARGIN;
        }
        ty = height() - SMALL_TICK_SIZE - INDICATOR_RECT_RADIUS;
    } else {
        tx = (width() - tw) / 2;
        ty = xy + fm.ascent() + INDICATOR_MARGIN;
        if (ty > height()) {
            ty = xy - INDICATOR_MARGIN;
        }
    }

    // Draw background rect
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setOpacity(INDICATOR_RECT_OPACITY);
    painter.setBrush(Qt::red);
    QRectF bgRect(tx, ty - fm.ascent() + 1, tw, fm.ascent());
    bgRect.adjust(-INDICATOR_RECT_RADIUS, -INDICATOR_RECT_RADIUS, INDICATOR_RECT_RADIUS, INDICATOR_RECT_RADIUS);
    bgRect.translate(0.5, 0.5);
    painter.drawRoundedRect(bgRect, INDICATOR_RECT_RADIUS, INDICATOR_RECT_RADIUS);

    // Draw text
    painter.setOpacity(1);
    painter.setPen(Qt::white);
    painter.drawText(tx, ty, text);
}

/**
 * actually draws the ruler
 */
void KLineal::paintEvent(QPaintEvent *e)
{
    Q_UNUSED(e);

    QPainter painter(this);
    drawBackground(painter);
    drawScale(painter);

    drawResizeHandle(painter, mHorizontal ? Qt::LeftEdge : Qt::TopEdge);
    drawResizeHandle(painter, mHorizontal ? Qt::RightEdge : Qt::BottomEdge);
    if (underMouse()) {
        int xy = mHorizontal ? mCursorPos.x() : mCursorPos.y();
        drawIndicatorOverlay(painter, xy);
        drawIndicatorText(painter, xy);
    }
}

#include "moc_klineal.cpp"
