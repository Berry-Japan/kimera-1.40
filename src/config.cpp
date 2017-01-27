#include "config.h"
#include "kimeraglobal.h"
#include <qapplication.h>
#include <qdir.h>
#include <qsettings.h>
using namespace Kimera;

static Config* conf = 0;
static QSettings settings(QSettings::Ini);

Config::Config() : QObject(qApp), _timerid(0)
{
}

QString
Config::readEntry(const QString& key, const QString& def, bool* ok)
{
  init();
  return settings.readEntry(KIMERACONF + key, def, ok);
}


bool
Config::readBoolEntry(const QString& key, bool def, bool* ok)
{
  init();
  return settings.readBoolEntry(KIMERACONF + key, def, ok);
}


int
Config::readNumEntry(const QString& key, int def, bool* ok)
{
  init();
  return settings.readNumEntry(KIMERACONF + key, def, ok);
}


bool 
Config::writeEntry(const QString& key, const QString& value, bool override)
{
  init();
  bool res = TRUE;
  bool ok;
  settings.readEntry(KIMERACONF + key, QString::null, &ok);
  if (override || !ok) {
    res = settings.writeEntry(KIMERACONF + key, value);
  }

  startSyncTimer();
  return res;
}


bool 
Config::writeEntry(const QString& key, bool value, bool override)
{
  init();
  bool res = TRUE;
  bool ok;
  settings.readBoolEntry(KIMERACONF + key, FALSE, &ok);
  if (override || !ok) {
    res = settings.writeEntry(KIMERACONF + key, value);
  }

  startSyncTimer();
  return res;
}


bool 
Config::writeEntry(const QString& key, int value, bool override)
{
  init();
  bool res = TRUE;
  bool ok;
  settings.readNumEntry(KIMERACONF + key, 0, &ok);
  if (override || !ok) {
    res = settings.writeEntry(KIMERACONF + key, value);
  }

  startSyncTimer();
  return res;
}


void
Config::init()
{
  if ( !conf ) {
    conf = new Config();

    QDir confdir(QDir::homeDirPath() + "/.kimera");
    if ( !confdir.exists() ) {
      confdir.mkdir(confdir.path());
    }
    settings.setPath("localhost", "kimera", QSettings::User);
    settings.insertSearchPath(QSettings::Unix, confdir.path());
  }
}


QString
Config::localEnvDirPath()
{
  init();
  return QDir::homeDirPath() + "/.kimera";
}


QString
Config::dictionaryDirPath()
{
  QDir  dicdir(localEnvDirPath() + "/dic-" KIMERA_VERSION);
  if ( !dicdir.exists() ) {
    dicdir.mkdir("", FALSE);
  }

  return dicdir.path();
}


QString
Config::simpleConvDictDBFilePath()
{
  return localEnvDirPath() + "/sconvdict.dbf";
}


QString
Config::additionalConvDictPath()
{
  init();
  return QString(INSTALL_PATH "/symbol.cdic");
}


void
Config::startSyncTimer()
{
  if (conf->_timerid > 0)
    conf->killTimer(conf->_timerid);
  conf->_timerid = conf->startTimer( 100 );
}


void
Config::timerEvent(QTimerEvent* e)
{
  if (e->timerId() == conf->_timerid) {
    settings.sync();    // Sync data
    conf->killTimer(conf->_timerid);
    conf->_timerid = 0;
  }
}
