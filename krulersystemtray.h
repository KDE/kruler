/*
    SPDX-FileCopyrightText: 2009 Montel Laurent <montel@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KRULERSYSTEMTRAY_H
#define KRULERSYSTEMTRAY_H

#include <KStatusNotifierItem>

class KActionCollection;

class KRulerSystemTray : public KStatusNotifierItem
{
public:
  KRulerSystemTray( const QString &iconName, QWidget *parent, KActionCollection *actions );
};

#endif
