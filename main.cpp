
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <kcmdlineargs.h>
#include <kaboutdata.h>
#include <klocale.h>

#include "klineal.h"

static const char homePageURL[] =
	"http://www.snafu.de/~till/";
static const char freeFormText[] =
	"\"May the source be with you.\"";



static KCmdLineOptions options[] =
{
  KCmdLineLastOption
  // INSERT YOUR COMMANDLINE OPTIONS HERE
};

int main(int argc, char *argv[])
{


  KAboutData aboutData( "kruler", I18N_NOOP("KDE Screen Ruler"),
    VERSION,
		I18N_NOOP("A screen ruler for the K Desktop Environment"),
		KAboutData::License_GPL,
    "(c) 2000, Till Krech",
		freeFormText,
		homePageURL);
  aboutData.addAuthor("Till Krech",I18N_NOOP("Programming"), "till@snafu.de");
  aboutData.addCredit("Gunnstein Lye",I18N_NOOP("Initial port to KDE 2"), "gl@ez.no");
  KCmdLineArgs::init( argc, argv, &aboutData );
  KCmdLineArgs::addCmdLineOptions( options ); // Add our own options.

	KApplication a;

  KLineal *ruler = new KLineal();
  a.setMainWidget(ruler);
  ruler->show();

  return a.exec();
}
