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
#include <QCommandLineParser>

#include "klineal.h"

#include "kruler_version.h"

int main(int argc, char *argv[])
{
  QApplication a( argc, argv );
  a.setAttribute(Qt::AA_UseHighDpiPixmaps, true);
  Kdelibs4ConfigMigrator migrate(QStringLiteral("kruler"));
  migrate.setConfigFiles(QStringList() << QStringLiteral("krulerrc") << QStringLiteral("kruler.notifyrc"));
  migrate.setUiFiles(QStringList() << QStringLiteral("krulerui.rc"));
  migrate.migrate();

  KAboutData aboutData( QStringLiteral("kruler"), i18n( "KDE Screen Ruler" ),
    QStringLiteral(KRULER_VERSION_STRING),
    i18n( "A screen ruler by KDE" ),
    KAboutLicense::GPL,
    i18n( "(c) 2000 - 2008, Till Krech\n(c) 2009, Mathias Soeken" ) );
  aboutData.addAuthor( i18n( "Mathias Soeken" ), i18n( "Maintainer" ), QStringLiteral("msoeken@tzi.de") );
  aboutData.addAuthor( i18n( "Till Krech" ), i18n( "Former Maintainer and Developer" ), QStringLiteral("till@snafu.de") );
  aboutData.addCredit( i18n( "Gunnstein Lye" ),i18n( "Initial port to KDE 2" ), QStringLiteral("gl@ez.no") );
  aboutData.setTranslator( i18nc( "NAME OF TRANSLATORS", "Your names" ), i18nc( "EMAIL OF TRANSLATORS", "Your emails" ) );
  KAboutData::setApplicationData(aboutData);

  QCommandLineParser parser;
  aboutData.setupCommandLine(&parser);
  parser.process(a);
  aboutData.processCommandLine(&parser);


  KLineal *ruler = new KLineal();
  ruler->show();
  int ret = a.exec();
  delete ruler;
  return ret;
}
