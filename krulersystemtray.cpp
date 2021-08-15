/*
    SPDX-FileCopyrightText: 2009 Montel Laurent <montel@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "krulersystemtray.h"

#include <QMenu>

#include <KActionCollection>
#include <KLocalizedString>

KRulerSystemTray::KRulerSystemTray( const QString& iconName, QWidget *parent, KActionCollection *actions)
  : KStatusNotifierItem( parent )
{
  setIconByName( iconName );
  setStatus(KStatusNotifierItem::Active);
  setToolTip( iconName, i18n( "KDE Screen Ruler" ), QString() );
  QMenu *cm = contextMenu();
  cm->addAction( actions->action( QStringLiteral( "preferences" ) ) );
}
