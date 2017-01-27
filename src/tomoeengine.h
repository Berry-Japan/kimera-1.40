#ifndef TOMOEENGINE_H
#define TOMOEENGINE_H

#include "kanjiengine.h"

class QProcess;

class TomoeEngine : public KanjiEngine {
  Q_OBJECT

public:
  TomoeEngine();
  ~TomoeEngine();
  
  QString name() const;
  bool isKanjiConvEnabled() const { return FALSE; }

  bool  init();
  void  cleanup();
  bool  beginConvert(const QString& hira, QStringList& kanji, QStringList& yomigana);
  void  endConvert(const QStringList& kanji=QString::null);
  bool  getCandidate(int idx, QStringList& candidate);
  bool  resizeSegment(int idx, int len, QStringList& kanji, QStringList& yomigana);
  bool  isConverting() { return FALSE; }

protected slots:
  void  readFromStdout();

private:
  QProcess*    _tomoe;
};

#endif  // TOMOEENGINE_H
