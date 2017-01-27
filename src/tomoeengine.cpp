#include "tomoeengine.h"
#include "debug.h"
#include <qprocess.h>
#include <qdir.h>

TomoeEngine::TomoeEngine()
  : _tomoe(0)
{
  _tomoe = new QProcess(QString("kimera-tomoe-gtk"), this);
  connect(_tomoe, SIGNAL(readyReadStdout()), this, SLOT(readFromStdout()));
}


TomoeEngine::~TomoeEngine()
{}


QString
TomoeEngine::name() const
{
  return "Tomoe";
}


bool
TomoeEngine::init()
{
  DEBUG_TRACEFUNC();

  if (_tomoe->isRunning())
    return TRUE;

  _tomoe->setWorkingDirectory(QDir::homeDirPath());
  return _tomoe->start();
}


void
TomoeEngine::cleanup()
{
  DEBUG_TRACEFUNC();
  _tomoe->kill();
}


bool
TomoeEngine::beginConvert(const QString&, QStringList&, QStringList&)
{
  return TRUE;
}


void
TomoeEngine::endConvert(const QStringList&)
{}


bool
TomoeEngine::getCandidate(int, QStringList&)
{
  return TRUE;
}


bool
TomoeEngine::resizeSegment(int, int, QStringList&, QStringList&)
{
  return TRUE;
}


void
TomoeEngine::readFromStdout()
{ 
  DEBUG_TRACEFUNC();
  const QString prefix("commit_string:\t");

  while (_tomoe->canReadLineStdout()) {
    QByteArray str = _tomoe->readStdout();
    QString line = QString::fromUtf8(str.data(), str.size()).section('\n', -2, -2);
    if ( line.startsWith(prefix, FALSE) ) {
      qDebug("String commited(%d): %s", line.length(), line.mid(prefix.length()).local8Bit().data() );
      emit decided(line.mid(prefix.length()));
    }
  }
  

//   while (_tomoe->canReadLineStdout()) {
//     QString str = _tomoe->readLineStdout();
//     if ( str.startsWith(prefix, FALSE) ) {
//       qDebug("String commited: %s", str.mid(prefix.length()).local8Bit().data() );
//       emit decided(  str.mid(prefix.length()));
//     };
//   }
}


EXPORT_KANJIENGINE(TomoeEngine)
