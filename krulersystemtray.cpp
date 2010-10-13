/***************************************************************************
                          krulersystemtray.cpp  -  description
                             -------------------
    Copyright            : (C) 2009        by Montel Laurent <montel@kde.org>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "krulersystemtray.h"

#include <KLocale>
#include <KMenu>

KRulerSystemTray::KRulerSystemTray( const QString& iconName, QWidget *parent, KActionCollection *actions)
  : KStatusNotifierItem( parent )
{
  setIconByName( iconName );
  setStatus(KStatusNotifierItem::Active);
  setToolTip( iconName, i18n( "KDE Screen Ruler" ), QString() );
  KMenu *cm = contextMenu();
  cm->addAction( actions->action( QLatin1String( "preferences" ) ) );
}
