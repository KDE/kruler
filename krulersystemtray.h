/***************************************************************************
                          krulersystemtray.h  -  description
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

#ifndef KRULERSYSTEMTRAY_H
#define KRULERSYSTEMTRAY_H

#include <kactioncollection.h>
#include <kicon.h>
#include <kstatusnotifieritem.h>

class KRulerSystemTray : public KStatusNotifierItem
{
public:
  KRulerSystemTray( const QString &iconName, QWidget *parent, KActionCollection *actions );
};

#endif
