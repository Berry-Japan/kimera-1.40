#include "primeengine.h"
#include "debug.h"
#include <qprocess.h>
#include <qtimer.h>
#include <qapplication.h>
#include <qcstring.h>
#include <qtextstream.h>
#include <qregexp.h>
#include <qtextcodec.h>

PrimeEngine::PrimeEngine()
  : _proc_predict(0), _proc_learn(0), _ssnid(), _cmdsent(), _kanji_learn(),
    _yomi_learn(), _islearning(FALSE), _reqcounter(0), _resbuf()
{
  _proc_predict = new QProcess(QString("prime"), this);
  _proc_learn = new QProcess(QString("prime"), this);
#ifndef KIMERA_NO_DEBUG
  _proc_predict->addArgument("--debug");
  _proc_learn->addArgument("--debug");
#endif

  connect(_proc_predict, SIGNAL(readyReadStdout()), this, SLOT(readCandidates()));
  connect(_proc_learn, SIGNAL(readyReadStdout()), this, SLOT(readFromStdout()));
}


PrimeEngine::~PrimeEngine()
{
}


QString
PrimeEngine::name() const
{
  return "PRIME";
}


bool
PrimeEngine::init()
{
  DEBUG_TRACEFUNC();

  // Data clear
  _ssnid = QString::null;
  _cmdsent.clear();
  _kanji_learn.clear();
  _yomi_learn.clear();
  _islearning = FALSE;
  _reqcounter = 0;
  _resbuf.clear();

  bool res = _proc_predict->start();
  if (res) {
    res = _proc_learn->start();
    if (res)
      writeCommand(CMD_VERSION, "version");
  }

  // Check res
  if (!res) {
    qWarning("Prime exec error");
    _proc_predict->kill();
    _proc_learn->kill();
    return FALSE;
  }

  connect(_proc_predict, SIGNAL(processExited()), this, SLOT(error())); 
  connect(_proc_learn, SIGNAL(processExited()), this, SLOT(error()));
  return TRUE;
}


void
PrimeEngine::cleanup()
{
  DEBUG_TRACEFUNC();
  disconnect(_proc_predict, SIGNAL(processExited()), this, SLOT(error())); 
  disconnect(_proc_learn, SIGNAL(processExited()), this, SLOT(error())); 
  _proc_predict->kill();
  _proc_learn->kill();
}


bool
PrimeEngine::reinit()
{
  DEBUG_TRACEFUNC();
  cleanup();
  return init();
}

bool
PrimeEngine::beginConvert(const QString&, QStringList&, QStringList&)
{
//   DEBUG_TRACEFUNC("hira:%s kanji:%s yomigana:%s", hira.local8Bit().data(), kanji.join(" ").local8Bit().data(),
// 		  yomigana.join(" ").local8Bit().data());
  return TRUE;
}


void
PrimeEngine::endConvert(const QStringList&)
{
//   DEBUG_TRACEFUNC("kanji: %s", kanji.join(" ").local8Bit().data());
}


bool
PrimeEngine::getCandidate(int, QStringList&)
{
//   DEBUG_TRACEFUNC("idx:%d candidate: %s", idx, candidate.join(" ").local8Bit().data());
  return TRUE;
}


bool
PrimeEngine::resizeSegment(int, int, QStringList&, QStringList&)
{
//   DEBUG_TRACEFUNC("idx:%d len:%d kanji:%s yomigana:%s", idx, len, kanji.join(" ").local8Bit().data(),
// 		  yomigana.join(" ").local8Bit().data());
  return TRUE;
}


void
PrimeEngine::writeCommand(CommandID id, const QString& cmd)
{
  DEBUG_TRACEFUNC("id: %d  cmd: %s", id, cmd.local8Bit().data());
  if (!_proc_learn->isRunning())
    return;

  QByteArray  ba;
  QTextStream  ts(ba, IO_WriteOnly);
  ts << QTextCodec::codecForTr()->fromUnicode(cmd) << '\n';
  _proc_learn->writeToStdin(ba);
  _cmdsent << id;
}


void
PrimeEngine::readFromStdout()
{ 
  DEBUG_TRACEFUNC();

  for (;;) {
    // Read and process the data.
    while (_proc_learn->canReadLineStdout()) {
      QByteArray cstr(_proc_learn->readStdout());
      _resbuf += QStringList::split(QRegExp("\n+"), tr(cstr.data()), TRUE);
    }
  
    if (_resbuf.isEmpty()) {
      break;
    }
    int idx = _resbuf.findIndex(QString::null);
    if (idx < 0) {
      break;
    }
    if (_resbuf[0] != "ok") {
      qWarning("Prime Error: %s", _resbuf[0].latin1());
      QTimer::singleShot(0, this, SLOT(reinit()));
      break;
    }
    
    int index = 0;
    QStringList::Iterator it;
    switch (_cmdsent.front()) {
    case CMD_VERSION:
      qDebug("Prime version: %s", _resbuf[1].latin1());
      writeCommand(CMD_SESSION_START, "session_start\tJapanese");
      break;
      
    case CMD_SESSION_START:
      _ssnid = _resbuf[1];
      qDebug("Prime Session id: %s", _ssnid.latin1());
      break;
      
    case CMD_EDIT_INSERT:
      writeCommand(CMD_CONV_CONVERT, QString("conv_convert\t") + _ssnid);
      break;

    case CMD_CONV_CONVERT:
      it = _resbuf.begin();
      ++it;
      while (!(*(++it)).isEmpty()) {
	if (*it == _kanji_learn[0])
	  break;
	else
	  ++index;
      }

      if (!(*it).isEmpty()) {
	writeCommand(CMD_CONV_SELECT, QString("conv_select\t") + _ssnid + "\t" + QString::number(index));
      } else {
	writeCommand(CMD_EDIT_ERASE, QString("edit_erase\t") + _ssnid);
	_kanji_learn.pop_front();
	_yomi_learn.pop_front();
      }
      break;

    case CMD_CONV_SELECT:
      writeCommand(CMD_CONV_COMMIT, QString("conv_commit\t") + _ssnid);
      break;

    case CMD_CONV_COMMIT:
      writeCommand(CMD_EDIT_ERASE, QString("edit_erase\t") + _ssnid);
      _kanji_learn.pop_front();
      _yomi_learn.pop_front();
      break;

    case CMD_EDIT_ERASE:
      // Next learning
      if (!_yomi_learn.isEmpty()) {
	writeCommand(CMD_EDIT_INSERT, QString("edit_insert\t") + _ssnid + "\t" + _yomi_learn[0]);
      } else {
	_islearning = FALSE;
      }      
      break;

    case CMD_SESSION_END:
      break;

    case CMD_CLOSE:
      break;

    default:
      break;
    }

    _cmdsent.pop_front();
    for (int i = 0; i < idx + 1; ++i)
      _resbuf.pop_front();
  }
}


void
PrimeEngine::learn(const QStringList& kanji, const QStringList& yomigana)
{
  DEBUG_TRACEFUNC();
  if (!_proc_learn->isRunning() || _ssnid.isEmpty())
    return;

  if (kanji.count() > 0 && kanji.count() == yomigana.count()) {
    _kanji_learn += kanji;
    _yomi_learn += yomigana;
  }

  if (!_islearning && !_yomi_learn.isEmpty()) {
    writeCommand(CMD_EDIT_INSERT, QString("edit_insert\t") + _ssnid + "\t" + _yomi_learn[0]);
    _islearning = TRUE;
  }
}


void
PrimeEngine::predict(const QString& romaji, const QString&)
{
  DEBUG_TRACEFUNC();
  if (!_proc_predict->isRunning())
    return;

  QByteArray  cmd;
  QTextStream  ts(cmd, IO_WriteOnly);
  ts << "lookup\t" << QTextCodec::codecForTr()->fromUnicode(romaji) << '\n';
  _proc_predict->writeToStdin(cmd);
  ++_reqcounter;
}


void
PrimeEngine::readCandidates()
{
  DEBUG_TRACEFUNC();
  --_reqcounter;
  QStringList buf;
  while (_proc_predict->canReadLineStdout()) {
    QByteArray cstr(_proc_predict->readStdout());
    buf += QStringList::split(QRegExp("\n+"), tr(cstr.data()), TRUE);
  }
  
  if (buf[0] == "ok") {
    QStringList cand;
    for (int i = 1; i < (int)buf.count(); ++i) {
      QString s = buf[i].section('\t', 1, 1);
      if (!s.isEmpty())
	cand << s;
    }

    emit predicted(cand.first());   // Only first condidate
  }
}


void
PrimeEngine::error()
{
  DEBUG_TRACEFUNC();
  qDebug("Prime Error");
  QTimer::singleShot(0, this, SLOT(reinit()));
}


EXPORT_KANJIENGINE(PrimeEngine)
