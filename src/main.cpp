#include "kimeraapp.h"
#include "mainform.h"
#include <qtextcodec.h>
#include <qapplication.h>
#include <qstylefactory.h>
#include <qdir.h>
#include <qfile.h>
#include <sys/types.h>
#include <unistd.h>
#include <pwd.h>

void 
outputMessage(QtMsgType type, const char *msg)
{
  switch ( type ) {
#ifndef KIMERA_NO_DEBUG
  case QtDebugMsg:
    fprintf(stderr, "%s\n", msg);
    break;
    
  case QtWarningMsg:
    fprintf(stderr, "Warning: %s\n", msg);
    break;
#endif

  case QtFatalMsg:
    fprintf(stderr, "Fatal: %s\n", msg);
#ifndef KIMERA_NO_DEBUG
    abort();          // dump core on purpose
#endif
    break;

  default:
    break;
  }
}


int 
main(int argc, char* argv[])
{
  qInstallMsgHandler(outputMessage);
  KimeraApp app(argc, argv);

  // Check lock file
  struct passwd* pass = getpwuid( getuid() );
  QDir::setCurrent("/tmp");
  QFile lockfile(QString(".kimera_") + pass->pw_name + ".lock");
  if ( !lockfile.exists() ) {
    qFatal("No such lock file: %s/%s", QDir::current().path().ascii(), lockfile.name().ascii());
    return -1;
  } else {
    lockfile.remove();
  }

  // Sets style
  qDebug("QStyle keys=%s", QStyleFactory::keys().join(":").ascii());
  QStyle* style = QStyleFactory::create( "Plastik" );
  if ( style ) {
    QApplication::setStyle( style );
  }
  QColor color(QWidget().paletteBackgroundColor());
  QApplication::setPalette(QPalette(color, color)); // Set default background color

  // Sets codec, eucJP.
  QTextCodec*  codec = QTextCodec::codecForName("eucJP");  
  Q_CHECK_PTR( codec );
  qDebug("codec: %s", codec->name());
  QTextCodec::setCodecForTr( codec );

  // Shows main widget
  MainForm* mainwidget = new MainForm();
  app.setMainWidget(mainwidget);
  mainwidget->move( mainwidget->loadPos() );
  mainwidget->show();    // Need to show at least once for initiation
  return app.exec();
}
