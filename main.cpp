/***************************************************************************
                          main.cpp  -  description
                             -------------------
    Copyright            : (C) 2000 - 2008 by Till Krech <till@snafu.de>
                           (C) 2009        by Mathias Soeken <msoeken@tzi.de>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QApplication>

#include <KAboutData>
#include <KLocalizedString>
#include <Kdelibs4ConfigMigrator>

#include "klineal.h"

int main(int argc, char *argv[])
{
  QApplication a( argc, argv );

  Kdelibs4ConfigMigrator migrate(QLatin1String("kruler"));
  migrate.setConfigFiles(QStringList() << QLatin1String("krulerrc") << QLatin1String("kruler.notifyrc"));
  migrate.setUiFiles(QStringList() << QLatin1String("krulerui.rc"));
  migrate.migrate();

  KAboutData aboutData( "kruler", i18n( "KDE Screen Ruler" ),
    "5.0", // version string
    i18n( "A screen ruler for KDE" ),
    KAboutLicense::GPL,
    i18n( "(c) 2000 - 2008, Till Krech\n(c) 2009, Mathias Soeken" ) );
  aboutData.addAuthor( i18n( "Mathias Soeken" ), i18n( "Maintainer" ), "msoeken@tzi.de" );
  aboutData.addAuthor( i18n( "Till Krech" ), i18n( "Former Maintainer and Developer" ), "till@snafu.de" );
  aboutData.addCredit( i18n( "Gunnstein Lye" ),i18n( "Initial port to KDE 2" ), "gl@ez.no" );

  KAboutData::setApplicationData(aboutData);

  KLineal *ruler = new KLineal();
  ruler->show();
  int ret = a.exec();
  delete ruler;
  return ret;
}
