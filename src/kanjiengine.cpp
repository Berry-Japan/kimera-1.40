#include "kanjiengine.h"
#include <qptrlist.h>

static QPtrList<KanjiEngine>* enginelist = 0;


class KanjiEngineCleanup {
public:
  ~KanjiEngineCleanup();
};


KanjiEngineCleanup::~KanjiEngineCleanup()
{
  if (enginelist)
    delete enginelist;

  enginelist = 0;
}
static KanjiEngineCleanup kanjienginecleanup;


KanjiEngine::KanjiEngine()
{
  if (!enginelist) {
    enginelist = new QPtrList<KanjiEngine>;
    enginelist->setAutoDelete(FALSE);
  }
  enginelist->append(this);
}


bool
KanjiEngine::init()
{
  return TRUE; 
}


void
KanjiEngine::cleanup()
{
}


bool
KanjiEngine::isKanjiConvEnabled() const
{
  return TRUE;
}


bool
KanjiEngine::isConverting()
{
  Q_ASSERT(0);
  return FALSE;
}


bool
KanjiEngine::isTCPConnectionSupported() const
{
  return FALSE;
}


KanjiEngine*
KanjiEngine::kanjiEngine(const QString& name)
{
  Q_ASSERT(enginelist);
  KanjiEngine* eng;
  for (eng = enginelist->first(); eng; eng = enginelist->next()) {
    if (eng->name().lower() == name.lower())
      break;
  }
  
  return eng;
}


QStringList
KanjiEngine::kanjiEngineList()
{
  QStringList lst;
  for (KanjiEngine* eng = enginelist->first(); eng; eng = enginelist->next()) {
    if (!eng->name().isEmpty())
      lst << eng->name();
  }
  return lst;
}


bool
KanjiEngine::isPredictionEnabled() const
{
  return FALSE;
}


void
KanjiEngine::predict(const QString&, const QString&)
{
}


void
KanjiEngine::learn(const QStringList&, const QStringList&)
{
}
