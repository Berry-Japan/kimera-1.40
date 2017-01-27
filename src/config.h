#ifndef CONFIG_H
#define CONFIG_H

#include <qobject.h>

class Config : QObject {
public:
  static QString readEntry(const QString& key, const QString& def=QString::null, bool* ok=0);
  static bool readBoolEntry(const QString& key, bool def=FALSE, bool* ok=0);
  static int  readNumEntry(const QString& key, int def=0, bool* ok=0);
  static bool writeEntry(const QString& key, const QString& value, bool override=TRUE);
  static bool writeEntry(const QString& key, bool value, bool override=TRUE);
  static bool writeEntry(const QString& key, int value, bool override=TRUE);

  static QString localEnvDirPath();
  static QString dictionaryDirPath();
  static QString simpleConvDictDBFilePath();
  static QString additionalConvDictPath();
  
protected:
  static void init();
  static void startSyncTimer();
  void  timerEvent(QTimerEvent *);

private:
  Config();
  int  _timerid;
};

#endif
