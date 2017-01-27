#ifndef PRIMEENGINE_H
#define PRIMEENGINE_H

#include "kanjiengine.h"
#include <qstringlist.h>

class QProcess;

class PrimeEngine : public KanjiEngine {
  Q_OBJECT

public:
  PrimeEngine();
  ~PrimeEngine();
  
  QString name() const;
  bool  init();
  void  cleanup();
  bool  isKanjiConvEnabled() const { return FALSE; }
  bool  beginConvert(const QString& hira, QStringList& kanji, QStringList& yomigana);
  void  endConvert(const QStringList& kanji=QString::null);
  bool  getCandidate(int idx, QStringList& candidate);
  bool  resizeSegment(int idx, int len, QStringList& kanji, QStringList& yomigana);
  bool  isConverting() { return FALSE; }
  // Prediction
  bool  isPredictionEnabled() const { return TRUE; }
  void  predict(const QString& romaji, const QString& hira);   // Romaji and Hiragana
  void  learn(const QStringList& kanji, const QStringList& yomigana);
  
public slots:
  bool  reinit();

protected:
  enum CommandID {
    CMD_NONE = 0,
    CMD_VERSION,
    CMD_SESSION_START,
    CMD_EDIT_INSERT,
    CMD_CONV_CONVERT,
    CMD_CONV_SELECT,
    CMD_CONV_COMMIT,	
    CMD_EDIT_ERASE ,
    CMD_SESSION_END,
    CMD_CLOSE,
  };

  void writeCommand(CommandID id, const QString& cmd);

protected slots:
  void  readCandidates();
  void  readFromStdout();
  void  error();

private:
  QProcess*    _proc_predict;
  QProcess*    _proc_learn;
  QString      _ssnid;
  QValueList<CommandID>  _cmdsent;
  QStringList  _kanji_learn;
  QStringList  _yomi_learn;
  bool         _islearning;
  int          _reqcounter;
  QStringList  _resbuf;

};

#endif  // PRIMEENGINE_H
