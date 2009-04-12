/***************************************************************************
                          klineal.h  -  description
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
#include <QMenu>


KRulerSystemTray::KRulerSystemTray( const KIcon& icon, QWidget * parent, KActionCollection *actions)
    : KSystemTrayIcon( icon, parent )
{
    QMenu * cm = contextMenu();
    cm->addAction( actions->action("preferences") );

}

