/*
    SPDX-FileCopyrightText: 2000-2008 Till Krech <till@snafu.de>
    SPDX-FileCopyrightText: 2009 Mathias Soeken <msoeken@tzi.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include <QApplication>

#include <KAboutData>
#include <KLocalizedString>
#include <Kdelibs4ConfigMigrator>
#include <QCommandLineParser>

#include "klineal.h"

#include "kruler_version.h"

int main(int argc, char *argv[])
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
  QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
#endif
  QApplication app( argc, argv );
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
  Kdelibs4ConfigMigrator migrate(QStringLiteral("kruler"));
  migrate.setConfigFiles(QStringList() << QStringLiteral("krulerrc") << QStringLiteral("kruler.notifyrc"));
  migrate.setUiFiles(QStringList() << QStringLiteral("krulerui.rc"));
  migrate.migrate();
#endif

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
  parser.process(app);
  aboutData.processCommandLine(&parser);

  KLineal ruler;
  ruler.show();
  return app.exec();
}
