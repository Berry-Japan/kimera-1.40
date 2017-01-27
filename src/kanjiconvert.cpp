#include "kanjiconvert.h"
#include "kimeraapp.h"
#include "kanjiengine.h"
#include "dicthiragana.h"
#include "preeditarea.h"
#include "inputmode.h"
#include "config.h"
#include "keyassigner.h"
#include "kimeraglobal.h"
#include "debug.h"
#include <qapplication.h>
#include <qlistbox.h>
#include <qkeysequence.h>
#include <qregexp.h>
#include <qmessagebox.h>
#include <qtimer.h>
#include <qclipboard.h>
#include <X11/keysym.h>
using namespace Kimera;

QIntDict<FuncPtr>  KanjiConvert::funcptrdict = QIntDict<FuncPtr>();

KanjiConvert::KanjiConvert() : _crntsegidx(0),
			       _minsegidx(0),
			       _inputkev(),
			       _yomiseglist(QStringList()),
			       _convtkanjilist(QStringList()),
			       _stat(NONE_YOMI),
			       _mode(), 
			       _dict(DictHiragana()),
			       _eng(0),
			       _engpdct(0),
			       _preedit(0),
			       _candlist(0),
			       _caretpos(0)
{  
  _inputkev.setAutoDelete(TRUE);
 
  // Create PreeditArea instance
  _preedit = new PreeditArea();

  // Create CandidateListBox instance
  _candlist = new CandidateListBox();

  connect(_candlist, SIGNAL(clicked(QListBoxItem*)), this, SLOT(slotReplaceSegment(QListBoxItem*)));
  connect(_preedit, SIGNAL(listPointChanged(const QPoint&)), _candlist, SLOT(setPos(const QPoint&)));

  QTimer::singleShot(0, this, SLOT(init()));
}


KanjiConvert::~KanjiConvert()
{
  if (_eng)
    _eng->cleanup();
  if (_engpdct)
    _engpdct->cleanup();
  delete  _preedit;
  delete  _candlist;
}


void 
KanjiConvert::init()
{
  DEBUG_TRACEFUNC();
  // Cleanup
  if (_eng) {
    _eng->cleanup();
    _eng = 0;
  }
  if ( _engpdct ) {
    disconnect(_engpdct, SIGNAL(predicted(const QString&)), 0, 0);
    if (_engpdct != _eng) {
      _engpdct->cleanup();
      _engpdct = 0;
    }
  }

  QString keng = Config::readEntry("_cmbkanjieng", "Canna");
  _eng = KanjiEngine::kanjiEngine(keng);
  if (!_eng) {
    QMessageBox::critical(0, "Kanji Engine Error", keng + ": No such kanji engine\nAssert!",
			  QMessageBox::Ok | QMessageBox::Default, 0);
    qApp->quit();
    return;
  }
  
  // initialize kanji engine
  bool ok = _eng->init();
  clear();
  if ( !ok ) {
    QMessageBox::critical(0, "Kanji Engine Error",
			  "Error Initializing " + keng + "!\nChange Kanji Engine.",
			  QMessageBox::Ok | QMessageBox::Default, 0);
  } else {
    if ( Config::readBoolEntry("_grppredict", FALSE) ) {
      // Initiates prediction engine
      QString pred = Config::readEntry("_cmbpredict", "");
      _engpdct = KanjiEngine::kanjiEngine(pred);
      if ( !_engpdct ) {
	QMessageBox::warning(0, "Prediction Engine Error", pred + ": No such prediction engine\n",
			     QMessageBox::Ok | QMessageBox::Default, 0);
      } else {
	connect(_engpdct, SIGNAL(predicted(const QString&)), this, SLOT(showCandidatePredicted(const QString&)));
	if (_engpdct != _eng) {
	  _engpdct->init();
	}
      }
    }
  }
  
  // Dictionary Initialization
  _dict.init();

  // Initiates preedit
  _preedit->init();

  // Initiates  dictionary of function pointer
  init_funcptrdict();
}


void 
KanjiConvert::init_funcptrdict()
{
  DEBUG_TRACEFUNC();
  if ( ! funcptrdict.isEmpty() ) 
    return;
  
  funcptrdict.setAutoDelete(TRUE);
  
  funcptrdict.insert(FID_NextCandidate,            new FuncPtr(&KanjiConvert::nextCandidate));
  funcptrdict.insert(FID_NextCandidateGroup,       new FuncPtr(&KanjiConvert::nextCandidateGroup));
  funcptrdict.insert(FID_PreviousCandidate,        new FuncPtr(&KanjiConvert::previousCandidate));
  funcptrdict.insert(FID_PreviousCandidateGroup,   new FuncPtr(&KanjiConvert::previousCandidateGroup));
  funcptrdict.insert(FID_FirstCandidate,           new FuncPtr(&KanjiConvert::firstCandidate));
  funcptrdict.insert(FID_LastCandidate,            new FuncPtr(&KanjiConvert::lastCandidate));
  funcptrdict.insert(FID_ConvertAllSegments,       new FuncPtr(&KanjiConvert::convertAllSegments));
  funcptrdict.insert(FID_ConvertCrntSegment,       new FuncPtr(&KanjiConvert::convertCrntSegment));
  funcptrdict.insert(FID_ConvertToOneSegment,      new FuncPtr(&KanjiConvert::convertToOneSegment));
  funcptrdict.insert(FID_DeleteForwardChar,        new FuncPtr(&KanjiConvert::deleteForwardChar));
  funcptrdict.insert(FID_DeleteBackwardChar,       new FuncPtr(&KanjiConvert::deleteBackwardChar));
  funcptrdict.insert(FID_DeleteAll,                new FuncPtr(&KanjiConvert::deleteAll));
  funcptrdict.insert(FID_DecideAllSegments,        new FuncPtr(&KanjiConvert::decideAllSegments));
  funcptrdict.insert(FID_DecideCrntSegment,        new FuncPtr(&KanjiConvert::decideCrntSegment));
  funcptrdict.insert(FID_DecidePredictedCandidate, new FuncPtr(&KanjiConvert::decidePredictedCandidate));
  funcptrdict.insert(FID_CancelConversion,         new FuncPtr(&KanjiConvert::cancelConversion));
  funcptrdict.insert(FID_LengthenSegment,          new FuncPtr(&KanjiConvert::lengthenSegment));
  funcptrdict.insert(FID_ShortenSegment,           new FuncPtr(&KanjiConvert::shortenSegment));
  funcptrdict.insert(FID_InsertSpace,              new FuncPtr(&KanjiConvert::insertSpace));
  funcptrdict.insert(FID_InsertAsciiSpace,         new FuncPtr(&KanjiConvert::insertAsciiSpace));
  funcptrdict.insert(FID_InsertMultibyteSpace,     new FuncPtr(&KanjiConvert::insertMultibyteSpace));
  funcptrdict.insert(FID_InsertOtherWidthSpace,    new FuncPtr(&KanjiConvert::insertOtherWidthSpace));
  funcptrdict.insert(FID_ConvertToHira,            new FuncPtr(&KanjiConvert::convertToHira));
  funcptrdict.insert(FID_ConvertToKana,            new FuncPtr(&KanjiConvert::convertToKata));
  funcptrdict.insert(FID_ConvertToHankakuKana,     new FuncPtr(&KanjiConvert::convertToHankakuKana));
  funcptrdict.insert(FID_ConvertToHankakuEisu,     new FuncPtr(&KanjiConvert::convertToHankakuEisu));
  funcptrdict.insert(FID_ConvertToZenkakuEisu,     new FuncPtr(&KanjiConvert::convertToZenkakuEisu));
  funcptrdict.insert(FID_ForwardSegment,           new FuncPtr(&KanjiConvert::forwardSegment));
  funcptrdict.insert(FID_BackwardSegment,          new FuncPtr(&KanjiConvert::backwardSegment));
  funcptrdict.insert(FID_ToFirstSegment,           new FuncPtr(&KanjiConvert::toFirstSegment));
  funcptrdict.insert(FID_ToLastSegment,            new FuncPtr(&KanjiConvert::toLastSegment));
  funcptrdict.insert(FID_CaretForward,             new FuncPtr(&KanjiConvert::caretForward));
  funcptrdict.insert(FID_CaretBackward,            new FuncPtr(&KanjiConvert::caretBackward));
  funcptrdict.insert(FID_CaretToFirst,             new FuncPtr(&KanjiConvert::caretToFirst));
  funcptrdict.insert(FID_CaretToLast,              new FuncPtr(&KanjiConvert::caretToLast));
  funcptrdict.insert(FID_SwitchZenkakuEisuMode,    new FuncPtr(&KanjiConvert::switchZenkakuEisuMode));
  funcptrdict.insert(FID_SetToHiraganaMode,        new FuncPtr(&KanjiConvert::setToHiraganaMode));
  funcptrdict.insert(FID_SetToKatakanaMode,        new FuncPtr(&KanjiConvert::setToKatakanaMode));
  funcptrdict.insert(FID_SetToHankakuKanaMode,     new FuncPtr(&KanjiConvert::setToHankakuKanaMode));
  funcptrdict.insert(FID_SetToZenkakuEisuMode,     new FuncPtr(&KanjiConvert::setToZenkakuEisuMode));
  funcptrdict.insert(FID_SwitchInputMethod,        new FuncPtr(&KanjiConvert::switchInputMethod));
  funcptrdict.insert(FID_ShowPropertyDialog,       new FuncPtr(&KanjiConvert::showPropertyDialog));
  funcptrdict.insert(FID_ExecDictTool,             new FuncPtr(&KanjiConvert::execDictTool));
  funcptrdict.insert(FID_ReconvertClipboardString, new FuncPtr(&KanjiConvert::reconvertClipboardString));
}


void
KanjiConvert::setInputMode(const InputMode& mode)
{
  DEBUG_TRACEFUNC("mode: 0x%x", mode.id());

  clear();
  InputMode  old(_mode);
  _mode.merge(mode);
  if (_mode != old) {
    emit inputModeChanged(_mode.id() | (KimeraApp::isXIMInputtingEnabled() ? 0 : Mode_DirectInput));
  }
}


void    
KanjiConvert::clear()
{
  DEBUG_TRACEFUNC();
  _crntsegidx = 0;
  _minsegidx = 0;
  _inputkev.clear();
  _yomiseglist.clear();
  _convtkanjilist.clear();
  if (_eng)
    _eng->endConvert();
  _stat = NONE_YOMI;
  hideToolBox();
  _caretpos = 0;
}


void
KanjiConvert::setFont(QFont ft) 
{
  DEBUG_TRACEFUNC();
  _preedit->setFont(ft);        // Set font to preedit area
  _candlist->setFont(ft);       // Set font to candidate list box
}


bool
KanjiConvert::convertAllSegments()
{
  DEBUG_TRACEFUNC();
  if (!_eng)
    return FALSE;

  InputMode  m(_mode);
  switch (_stat) {
  case NONE_YOMI:
    // Do nothing
    return TRUE;
    break;

  case INPUTTING_YOMI:
  case CONVERTING_YOMI:
  case SHOWING_LIST:
  case CHANGING_LENGTH:
    _candlist->hide();
    if (_inputkev.getLast() && !_inputkev.getLast()->text().isEmpty()) {  // Check last KeyEvent
      _inputkev.append(new QKeyEvent(DictHiragana::SEPARATOR_CHAR));  // Insert SEPARATOR
    }
    m.merge(Mode_Hiragana);
    _eng->beginConvert(convertYomigana(m), _convtkanjilist, _yomiseglist);
    break;

  default:
    Q_ASSERT(0);
    return FALSE;
    break;
  }

  _stat = CONVERTING_YOMI;
  showPreeditArea();
  return TRUE;
}


bool
KanjiConvert::convertCrntSegment()
{
  DEBUG_TRACEFUNC();
  if (!_eng)
    return FALSE;

  QStringList convstr, yomiseg;
  switch (_stat) {
  case NONE_YOMI:
    // Do nothing
    return TRUE;
    break;

  case INPUTTING_YOMI:
    convertAllSegments();
    break;

  case CONVERTING_YOMI:
  case SHOWING_LIST:
    break;
    
  case CHANGING_LENGTH:
    _eng->resizeSegment(_crntsegidx, _convtkanjilist[_crntsegidx].length(), convstr, yomiseg);
    replaceSegments(_crntsegidx, _convtkanjilist.count() - _crntsegidx, convstr);
    setYomigana(_crntsegidx, yomiseg);
    break;    
    
  default:
    Q_ASSERT(0);
    return FALSE;
    break;
  }

  _stat = CONVERTING_YOMI;
  showPreeditArea();
  return TRUE;
}


bool 
KanjiConvert::convertToOneSegment()
{
  DEBUG_TRACEFUNC();
  if (!_eng)
    return FALSE;

  int cnt = 0;
  InputMode  m(_mode);
  switch (_stat) {
  case NONE_YOMI:
    // Do nothing
    return TRUE;
    break;

  case INPUTTING_YOMI:
  case CONVERTING_YOMI:
  case SHOWING_LIST:
  case CHANGING_LENGTH:
    _candlist->hide();
    if (_inputkev.getLast() && !_inputkev.getLast()->text().isEmpty()) {  // Check last KeyEvent
      _inputkev.append(new QKeyEvent(DictHiragana::SEPARATOR_CHAR));  // Insert SEPARATOR
    }
    m.merge(Mode_Hiragana);
    _convtkanjilist = convertYomigana(m);
    _eng->beginConvert(_convtkanjilist[0], _convtkanjilist, _yomiseglist);
    if (_yomiseglist.count() != 1)
      _eng->resizeSegment(0, _yomiseglist.join("").length(), _convtkanjilist, _yomiseglist);   // Resize max length
    // For Bug(?)
    while (_yomiseglist.count() != 1) {
      _eng->resizeSegment(0, KanjiEngine::LENGTHEN_1CHAR, _convtkanjilist, _yomiseglist);   // Resize max length
      if (++cnt > 10)
	break;
    }
    Q_ASSERT(_yomiseglist.count() == 1);
    break;

  default:
    Q_ASSERT(0);
    return FALSE;
    break;
  }

  _stat = CONVERTING_YOMI;
  showPreeditArea();
  return TRUE;
}


bool 
KanjiConvert::nextCandidate()
{
  DEBUG_TRACEFUNC();
  return  moveCrntItemCandidateList(1);
}


bool 
KanjiConvert::nextCandidateGroup()
{
  DEBUG_TRACEFUNC();
  return  moveCrntItemCandidateList( CandidateListBox::MAX_VISIBLE_ITEMS );
}


bool 
KanjiConvert::previousCandidate()
{
  DEBUG_TRACEFUNC();
  return  moveCrntItemCandidateList(-1);
}


bool 
KanjiConvert::previousCandidateGroup()
{
  DEBUG_TRACEFUNC();
  return  moveCrntItemCandidateList( -CandidateListBox::MAX_VISIBLE_ITEMS );
}


bool
KanjiConvert::firstCandidate()
{
  DEBUG_TRACEFUNC();
  if (!_eng)
    return FALSE;

  switch (_stat) {
  case NONE_YOMI:
  case INPUTTING_YOMI:
    // Do nothing
    break;
    
  case CONVERTING_YOMI:
  case SHOWING_LIST:
    if ( _candlist->isListVisible() ) {
      _candlist->selectCandidate(0);
      replaceSegments(_crntsegidx, 1, _candlist->currentText());  // Replace string of preeidt area
    } else if (_eng->isConverting()) {
      // Get Candidacies
      QStringList cand;
      _eng->getCandidate(_crntsegidx, cand);
      if (cand.isEmpty()) {
	cand << _convtkanjilist[_crntsegidx];
      }
      _candlist->showList(cand);
      replaceSegments(_crntsegidx, 1, _candlist->currentText());
    } else {
      break;
    }

    // show preedit
    _stat = SHOWING_LIST;
    showPreeditArea();  
    break;
    
  case CHANGING_LENGTH:
    // Do nothing
    break;

  default:
    Q_ASSERT(0);
    return FALSE;
    break;
  }

  return TRUE;
}


bool
KanjiConvert::lastCandidate()
{
  DEBUG_TRACEFUNC();
  if (!_eng)
    return FALSE;
  
  switch (_stat) {
  case NONE_YOMI:
  case INPUTTING_YOMI:
    // Do nothing
    break;
    
  case CONVERTING_YOMI:
  case SHOWING_LIST:
    if ( _candlist->isListVisible() ) {
      _candlist->selectCandidate(_candlist->count() - 1);
      replaceSegments(_crntsegidx, 1, _candlist->currentText());  // Replace string of preeidt area
    } else if (_eng->isConverting()) {
      // Get Candidacies
      QStringList cand;
      _eng->getCandidate(_crntsegidx, cand);
      if (cand.isEmpty()) {
	cand << _convtkanjilist[_crntsegidx];
      }
      _candlist->showList(cand, cand.count() - 1);
      replaceSegments(_crntsegidx, 1, _candlist->currentText());
    } else {
      break;
    }

    // show preedit
    _stat = SHOWING_LIST;
    showPreeditArea();  
    break;
    
  case CHANGING_LENGTH:
    // Do nothing
    break;

  default:
    Q_ASSERT(0);
    return FALSE;
    break;
  }

  return TRUE;
}


bool
KanjiConvert::moveCrntItemCandidateList(int d)
{
  DEBUG_TRACEFUNC("d: %d", d);
  if (!_eng)
    return FALSE;

  switch (_stat) {
  case NONE_YOMI:
  case INPUTTING_YOMI:
    // Do nothing
    return TRUE;
    break;
    
  case CONVERTING_YOMI:
  case SHOWING_LIST:
    if ( _candlist->isListVisible() ) {
      // It means CandidateListBox shows
      if (d > 0) {
	_candlist->selectNextCandidate(d);
      } else if (d < 0) {
	_candlist->selectPreviousCandidate(-d);
      }

      // Replace string of preeidt area
      replaceSegments(_crntsegidx, 1, _candlist->currentText());

    } else if (_eng->isConverting()) {
      // Get Candidacies
      QStringList cand;
      if ( !_eng->getCandidate(_crntsegidx, cand) ) {
	break;
      }
      if (cand.isEmpty()) {
	cand << _convtkanjilist[_crntsegidx];
      }      
      if (cand.count() > 1 && cand.first() == _convtkanjilist[_crntsegidx]) {
	_candlist->showList(cand, 1);
      } else {
	_candlist->showList(cand);
      }
      // Replace string of converted kanji string list
      replaceSegments(_crntsegidx, 1, _candlist->currentText());

    } else {
      convertAllSegments();
    }
    break;
    
  case CHANGING_LENGTH:
    // Do nothing
    return TRUE;
    break;

  default:
    Q_ASSERT(0);
    return FALSE;
    break;
  }

  // show preedit
  _stat = SHOWING_LIST;
  showPreeditArea();  
  return TRUE;
}


// Inserts a character to roma string
bool
KanjiConvert::insertChar(const QKeyEvent& ke)
{
  DEBUG_TRACEFUNC();
  if (ke.count() != 1 || ke.text().isEmpty()) {
    clear();
    return FALSE;
  }  

  switch (_stat) {
  case CONVERTING_YOMI:
  case SHOWING_LIST:
  case CHANGING_LENGTH:  
    decideAllSegments();
    // no break

  case NONE_YOMI:
  case INPUTTING_YOMI:
    if (_caretpos == _yomiseglist[0].length()) {
      // Appends char
      _inputkev.append( new QKeyEvent(ke.type(), ke.key(), ke.ascii(), ke.state(), ke.text(),
				      ke.isAutoRepeat(), ke.count()) );
      _yomiseglist = convertYomigana(_mode);
      _caretpos = _yomiseglist[0].length();
    } else {
      // Inserts char
      int i = findIndexKeyEventList(_caretpos);
      if (i < 0) {
	createKeyEventList(_yomiseglist.join(""), _inputkev);
	i = findIndexKeyEventList(_caretpos);
      }

      QKeyEvent* pke = _inputkev.at(i);
      if (pke && !pke->text().isEmpty()) {  // Check SEPARATOR
	_inputkev.insert(i, new QKeyEvent(DictHiragana::SEPARATOR_CHAR));  // Insert SEPARATOR
      }
      _inputkev.insert(i, new QKeyEvent(ke.type(), ke.key(), ke.ascii(), ke.state(),
					ke.text(), ke.isAutoRepeat(), ke.count()) );
      int d = _yomiseglist[0].length() - _caretpos;
      _yomiseglist = convertYomigana(_mode);
      _caretpos = _yomiseglist[0].length() - d;
    }

    _stat = INPUTTING_YOMI;
    predict();
    showPreeditArea();
    break;

  default:
    Q_ASSERT(0);
    return FALSE;
    break;
  }

  return TRUE;
}


bool
KanjiConvert::deleteForwardChar()
{
  DEBUG_TRACEFUNC();
  qDebug("current Yomigana:%s", _yomiseglist.join(" ").local8Bit().data());

  QString str;
  switch (_stat) {
  case NONE_YOMI:
    // Do noting
    break;
    
  case INPUTTING_YOMI:
    if (_caretpos < _yomiseglist[0].length()) {
      int idx = findIndexKeyEventList(_caretpos);
      if (idx < 0) {
	createKeyEventList(_yomiseglist.join(""), _inputkev);
	idx = findIndexKeyEventList(_caretpos);
      }
      str = convertYomigana(_mode);
      str.remove(_caretpos, 1);
      
      QPtrList<QKeyEvent>  klst;
      klst.setAutoDelete( TRUE );
      int n = _inputkev.count() - idx;
      for (int i = 0; i < n; ++i)
	klst.prepend(_inputkev.take(_inputkev.count() - 1));
      
      syncKeyEventListForward(str.right(str.length() - _caretpos), klst);
      
      while (klst.count() > 0)
	_inputkev.append(klst.take(0));
      
      _yomiseglist = convertYomigana(_mode);
      Q_ASSERT(_yomiseglist[0].isEmpty() || _yomiseglist[0] == str);
      qDebug("%s: %s", __func__, _yomiseglist[0].local8Bit().data());
      
      // Sets status
      if ( _yomiseglist.join("").isEmpty() ) {
	clear();
      } else {
	_stat = INPUTTING_YOMI;
	// Prediction
	predict();
      }
      showPreeditArea();
    }
    break;
    
  case CONVERTING_YOMI:
  case SHOWING_LIST:
  case CHANGING_LENGTH:
    // Do nothing
    break;
    
  default:
    Q_ASSERT(0);
    return FALSE;
    break;
  }
  
  return TRUE;
}


bool
KanjiConvert::deleteBackwardChar()
{
  DEBUG_TRACEFUNC();
  qDebug("current Yomigana:%s", _yomiseglist.join(" ").local8Bit().data());

  QString str;
  switch (_stat) {
  case NONE_YOMI:
    // Do noting
    break;
    
  case INPUTTING_YOMI:
    if (_caretpos > 0) {
      int idx = findIndexKeyEventList(_caretpos);
      if (idx < 0) {
	createKeyEventList(_yomiseglist.join(""), _inputkev);
	idx = findIndexKeyEventList(_caretpos);
      }
      str = convertYomigana(_mode);
      str.remove(_caretpos - 1, 1);
      --_caretpos;
      
      QPtrList<QKeyEvent>  klst;
      klst.setAutoDelete( TRUE );
      for (int i = 0; i < idx; ++i)
	klst.append(_inputkev.take(0));
      
      syncKeyEventListBackward(str.left(_caretpos), klst);
      
      while (klst.count() > 0)
	_inputkev.prepend(klst.take(klst.count() - 1));
      
      _yomiseglist = convertYomigana(_mode);
      Q_ASSERT(_yomiseglist[0].isEmpty() || _yomiseglist[0] == str);
      qDebug("%s: %s", __func__, _yomiseglist[0].local8Bit().data());
      
      // Sets status
      if ( _yomiseglist.join("").isEmpty() ) {
	clear();
      } else {
	_stat = INPUTTING_YOMI;
	// Prediction
	predict();
	showPreeditArea();
      }
    }
    qDebug("current Yomigana:%s", _yomiseglist.join("").local8Bit().data());
    break;

  case CONVERTING_YOMI:
  case SHOWING_LIST:
  case CHANGING_LENGTH:
    // Do nothing
    break;
    
  default:
    Q_ASSERT(0);
    return FALSE;
    break;
  }
  
  return TRUE;
}


bool
KanjiConvert::deleteAll()
{
  DEBUG_TRACEFUNC();
  switch (_stat) {
  case NONE_YOMI:
  case INPUTTING_YOMI:
  case CONVERTING_YOMI:
  case SHOWING_LIST:
  case CHANGING_LENGTH:
    clear();
    break;
    
  default:
    Q_ASSERT(0);
    return FALSE;
    break;
  }
  
  return TRUE;
}


bool
KanjiConvert::decideCrntSegment()
{
  DEBUG_TRACEFUNC();
 
  QString str;
  switch (_stat) {
  case NONE_YOMI:
  case INPUTTING_YOMI:
    // Do nothing
    return TRUE;
    break;
    
  case CONVERTING_YOMI:
  case SHOWING_LIST:
  case CHANGING_LENGTH:
    // Emit segment decided
    emit  decideSegments(_preedit->backwardText() + _preedit->attentionText());
    _minsegidx = ++_crntsegidx;
    if ( _convtkanjilist[_crntsegidx].isEmpty() ) {
      qDebug("no more kanji to to converted");
      if (_eng)
	_eng->endConvert(_convtkanjilist);
      learn(_convtkanjilist);
      clear();
      return TRUE;
    }

    qDebug("Segment minimum index: %d", _minsegidx);
    qDebug("Number of segments remaining: %d : %s", _convtkanjilist.count() - _minsegidx,
	   _convtkanjilist.join(" ").local8Bit().data());
    
    // Sync input-keys
    for (int i = _crntsegidx; i < (int)_yomiseglist.count(); ++i) {
      str += _yomiseglist[i];
    }
    syncKeyEventListForward(str, _inputkev);
    break;

  default:
    Q_ASSERT(0);
    return FALSE;
    break;
  }

  _candlist->hide();
  _stat = CONVERTING_YOMI;
  showPreeditArea();
  return TRUE;
}


bool
KanjiConvert::decideAllSegments()
{
  DEBUG_TRACEFUNC();
  emit decideSegments(_preedit->text());
  QStringList sl = (_stat == INPUTTING_YOMI) ? _yomiseglist.join("") : _convtkanjilist;
  if (_eng)
    _eng->endConvert(sl);
  learn(sl);
  clear();
  return TRUE;
}


bool
KanjiConvert::decideCandidateList(int no)
{
  DEBUG_TRACEFUNC("no: %d", no);
  if (_stat != SHOWING_LIST) {
    Q_ASSERT(0);
    return FALSE;
  }
  if (no < 1 && no > 9)
    return FALSE;

  // Replace string
  QString str = _candlist->text(_candlist->topItem() + no - 1);
  if (!str.isEmpty())
    replaceSegments(_crntsegidx, 1, str);
  _candlist->hide();
  _stat = CONVERTING_YOMI;
  showPreeditArea();
  return TRUE;
}

bool
KanjiConvert::decidePredictedCandidate()
{
  DEBUG_TRACEFUNC();

  switch (_stat) {
  case NONE_YOMI:
    // Do nothing
    break;

  case INPUTTING_YOMI:
    if (!_candlist->candidatePredicted().isEmpty()) {
      emit decideSegments(_candlist->candidatePredicted());
      // Learning
      learnPrediction(_candlist->candidatePredicted());
      clear();
    }
    break;

  case CONVERTING_YOMI:
  case SHOWING_LIST:
  case CHANGING_LENGTH:
    // Do nothing
    break;

  default:
    Q_ASSERT(0);
    return FALSE;
    break;
  }

  return TRUE;
}


bool
KanjiConvert::cancelConversion()
{
  DEBUG_TRACEFUNC();

  QString yomi;  
  switch (_stat) {
  case NONE_YOMI:
  case INPUTTING_YOMI:
    // Do nothing
    break;

  case CONVERTING_YOMI:
  case SHOWING_LIST:
  case CHANGING_LENGTH:
    if (_minsegidx > 0) {
      for (int i = 0; i < (int)_minsegidx; ++i)
	_yomiseglist.pop_front();
      syncKeyEventListForward(_yomiseglist.join(""), _inputkev);
    }
    if (_inputkev.getLast() && _inputkev.getLast()->text().isEmpty()) {  // Removes SEPARATOR_CHAR
      _inputkev.remove(_inputkev.count() - 1);
    }
    _yomiseglist = convertYomigana(_mode);

    // send End Convert message
    if (_eng)
      _eng->endConvert(); 
    _candlist->hide();
    _crntsegidx = _minsegidx = 0;
    _caretpos = _yomiseglist[0].length();
    _convtkanjilist.clear();
    _stat = INPUTTING_YOMI;
    showPreeditArea();  
    break;
    
  default:
    Q_ASSERT(0);
    return FALSE;
    break;
  }
  
  return TRUE;
}


bool    
KanjiConvert::lengthenSegment()
{
  DEBUG_TRACEFUNC();
  if (!_eng)
    return FALSE;

  switch (_stat) {
  case NONE_YOMI:
  case INPUTTING_YOMI:
    // Do nothing
    return TRUE;
    break;

  case CONVERTING_YOMI:
  case SHOWING_LIST:
  case CHANGING_LENGTH:
    if (_yomiseglist[_crntsegidx + 1].isEmpty()) {
      // Do nothing
      return TRUE;

    } else {
      QStringList convstr, yomiseg;
      _eng->resizeSegment(_crntsegidx, KanjiEngine::LENGTHEN_1CHAR, convstr, yomiseg);
      replaceSegments(_crntsegidx, _convtkanjilist.count() - _crntsegidx, yomiseg);
      setYomigana(_crntsegidx, yomiseg);
    }
    break;

  default:
    Q_ASSERT(0);
    return FALSE;
    break;
  }
  
  _candlist->hide();
  _stat = CHANGING_LENGTH;
  showPreeditArea();
  return TRUE;
}


bool    
KanjiConvert::shortenSegment()
{
  DEBUG_TRACEFUNC();
  if (!_eng)
    return FALSE;

  switch (_stat) {
  case NONE_YOMI:
  case INPUTTING_YOMI:
    // Do nothing
    return TRUE;
    break;

  case CONVERTING_YOMI:
  case SHOWING_LIST:
  case CHANGING_LENGTH:
    if (_yomiseglist[_crntsegidx].length() <= 1) {
      // Do nothing
      return TRUE;

    } else {
      QStringList convstr, yomiseg;
      _eng->resizeSegment(_crntsegidx, KanjiEngine::SHORTEN_1CHAR, convstr, yomiseg);
      replaceSegments(_crntsegidx, _convtkanjilist.count() - _crntsegidx, yomiseg);
      setYomigana(_crntsegidx, yomiseg);
    }
    break;
    
  default:
    Q_ASSERT(0);
    return FALSE;
    break;
  }

  _candlist->hide();
  _stat = CHANGING_LENGTH;
  showPreeditArea();
  return TRUE;
}


bool
KanjiConvert::forwardSegment()
{
  DEBUG_TRACEFUNC();
  switch (_stat) {
  case NONE_YOMI:
  case INPUTTING_YOMI:
    // Do nothing
    return TRUE;
    break;
    
  case CONVERTING_YOMI:
  case SHOWING_LIST:
  case CHANGING_LENGTH:
    if (_crntsegidx < _convtkanjilist.count() - 1)
      _crntsegidx++;
    break;
    
  default:
    Q_ASSERT(0);
    return FALSE;
    break;
  }

  _candlist->hide();
  _stat = CONVERTING_YOMI;
  showPreeditArea();  
  return TRUE;
}


bool
KanjiConvert::backwardSegment()
{
  DEBUG_TRACEFUNC();
  switch (_stat) {
  case NONE_YOMI:
  case INPUTTING_YOMI:
    // Do nothing
    return TRUE;
    break;
    
  case CONVERTING_YOMI:
  case SHOWING_LIST:
  case CHANGING_LENGTH:
    if (_crntsegidx > _minsegidx)
      _crntsegidx--;
    break;
    
  default:
    Q_ASSERT(0);
    return FALSE;
    break;
  }

  _candlist->hide();
  _stat = CONVERTING_YOMI;
  showPreeditArea();  
  return TRUE;
}


bool
KanjiConvert::toFirstSegment()
{
  DEBUG_TRACEFUNC();
  switch (_stat) {
  case NONE_YOMI:
  case INPUTTING_YOMI:
    // Do nothing
    break;
    
  case CONVERTING_YOMI:
  case SHOWING_LIST:
  case CHANGING_LENGTH:
    _crntsegidx = _minsegidx;
    break;

  default:
    Q_ASSERT(0);
    return FALSE;
    break;
  }

  _candlist->hide();
  _stat = CONVERTING_YOMI;
  showPreeditArea();  
  return TRUE;
}


bool
KanjiConvert::toLastSegment()
{
  DEBUG_TRACEFUNC();
  switch (_stat) {
  case NONE_YOMI:
  case INPUTTING_YOMI:
    // Do nothing
    return TRUE;
    break;
    
  case CONVERTING_YOMI:
  case SHOWING_LIST:
  case CHANGING_LENGTH:
    _crntsegidx = _yomiseglist.count() - 1;
    break;
    
  default:
    Q_ASSERT(0);
    return FALSE;
    break;
  }

  _candlist->hide();
  _stat = CONVERTING_YOMI;
  showPreeditArea();  
  return TRUE;
}


bool
KanjiConvert::insertSpace()
{
  DEBUG_TRACEFUNC();

  bool res = TRUE;
  QString s;
  switch (_stat) {
  case NONE_YOMI:
    if (currentSettingSpaceChar() == " ") {
      res = FALSE;        // Forward key-event for xemacs bug?
    } else {
      emit decideSegments(tr("　"));  // Multibyte space
    }
    break;

  case INPUTTING_YOMI:    
  case CONVERTING_YOMI:
  case SHOWING_LIST:
  case CHANGING_LENGTH:
    // Do nothing
    break;
    
  default:
    Q_ASSERT(0);
    return FALSE;
    break;
  }
  
  return res;
}


bool
KanjiConvert::insertAsciiSpace()
{
  DEBUG_TRACEFUNC();
  
  switch (_stat) {
  case NONE_YOMI:
    emit decideSegments(" ");
    break;

  case INPUTTING_YOMI:    
  case CONVERTING_YOMI:
  case SHOWING_LIST:
  case CHANGING_LENGTH:
    // Do nothing
    break;
    
  default:
    Q_ASSERT(0);
    return FALSE;
    break;
  }
  
  return TRUE;
}


bool
KanjiConvert::insertMultibyteSpace()
{
  DEBUG_TRACEFUNC();

  switch (_stat) {
  case NONE_YOMI:
    emit decideSegments(tr("　"));  // Multibyte space
    break;

  case INPUTTING_YOMI:    
  case CONVERTING_YOMI:
  case SHOWING_LIST:
  case CHANGING_LENGTH:
    // Do nothing
    break;
    
  default:
    Q_ASSERT(0);
    return FALSE;
    break;
  }
  
  return TRUE;
}


bool
KanjiConvert::insertOtherWidthSpace()
{
  DEBUG_TRACEFUNC();

  QString s;
  switch (_stat) {
  case NONE_YOMI:
    if (currentSettingSpaceChar() == " ") {
      emit decideSegments(tr("　"));  // Multibyte space
    } else {
      emit decideSegments(" ");  // Ascii space
    }
    break;

  case INPUTTING_YOMI:    
  case CONVERTING_YOMI:
  case SHOWING_LIST:
  case CHANGING_LENGTH:
    // Do nothing
    break;
    
  default:
    Q_ASSERT(0);
    return FALSE;
    break;
  }
  
  return TRUE;
}


bool
KanjiConvert::convertToHira()
{
  DEBUG_TRACEFUNC();
  if (!_eng)
    return FALSE;

  InputMode  m(_mode);
  QStringList dummy;
  switch (_stat) {
  case NONE_YOMI:
    return TRUE;
    break;

  case INPUTTING_YOMI:
    if (_inputkev.getLast() && !_inputkev.getLast()->text().isEmpty()) {  // Check last KeyEvent
      _inputkev.append(new QKeyEvent(DictHiragana::SEPARATOR_CHAR));  // Insert SEPARATOR
    }
    m.merge(Mode_Hiragana);
    _convtkanjilist = convertYomigana(m);
    _eng->beginConvert(_convtkanjilist[0], dummy, _yomiseglist);
    if (_yomiseglist.count() != 1)
      _eng->resizeSegment(0, _yomiseglist.join("").length(), dummy, _yomiseglist);   // Resize max length
    Q_ASSERT(_yomiseglist.count() == 1);
    break;
    
  case CONVERTING_YOMI:
  case SHOWING_LIST:
  case CHANGING_LENGTH:
    // convert first segment to Hiragara
    Q_ASSERT(!_yomiseglist[_crntsegidx].isEmpty());
    Q_ASSERT(!_convtkanjilist[_crntsegidx].isEmpty());
    // Replace string
    m.merge(Mode_Hiragana);
    convertSegment(_crntsegidx, m);
    break;

  default:
    Q_ASSERT(0);
    return FALSE;
    break;
  }

  _candlist->hide();
  _stat = CONVERTING_YOMI;
  showPreeditArea();  
  return TRUE;
}


bool
KanjiConvert::convertToKata()
{
  DEBUG_TRACEFUNC();
  if (!_eng)
    return FALSE;

  InputMode  m(_mode);
  QStringList dummy;
  switch (_stat) {
  case NONE_YOMI:
    return TRUE;
    break;

  case INPUTTING_YOMI:
    if (_inputkev.getLast() && !_inputkev.getLast()->text().isEmpty()) {  // Check last KeyEvent
      _inputkev.append(new QKeyEvent(DictHiragana::SEPARATOR_CHAR));  // Insert SEPARATOR
    }
    m.merge(Mode_Katakana);
    _convtkanjilist = convertYomigana(m);
    m.merge(Mode_Hiragana);
    _eng->beginConvert(convertYomigana(m), dummy, _yomiseglist);
    if (_yomiseglist.count() != 1)
      _eng->resizeSegment(0, _yomiseglist.join("").length(), dummy, _yomiseglist);   // Resize max length
    Q_ASSERT(_yomiseglist.count() == 1);
    break;
    
  case CONVERTING_YOMI:
  case SHOWING_LIST:
  case CHANGING_LENGTH:
    // convert first segment to Katakana
    Q_ASSERT(!_yomiseglist[_crntsegidx].isEmpty());
    Q_ASSERT(!_convtkanjilist[_crntsegidx].isEmpty());
    // Replace string
    m.merge(Mode_Katakana);
    convertSegment(_crntsegidx, m);
    break;

  default:
    Q_ASSERT(0);
    return FALSE;
    break;
  }

  _candlist->hide();
  _stat = CONVERTING_YOMI;
  showPreeditArea();
  return TRUE;
}


bool
KanjiConvert::convertToHankakuKana()
{
  DEBUG_TRACEFUNC();
  if (!_eng)
    return FALSE;

  InputMode  m(_mode);
  QStringList dummy;
  switch (_stat) {
  case NONE_YOMI:
    return TRUE;
    break;

  case INPUTTING_YOMI:
    if (_inputkev.getLast() && !_inputkev.getLast()->text().isEmpty()) {  // Check last KeyEvent
      _inputkev.append(new QKeyEvent(DictHiragana::SEPARATOR_CHAR));  // Insert SEPARATOR
    }
    m.merge(Mode_HankakuKana);
    _convtkanjilist = convertYomigana(m);
    m.merge(Mode_Hiragana);
    _eng->beginConvert(convertYomigana(m), dummy, _yomiseglist);
    if (_yomiseglist.count() != 1)
      _eng->resizeSegment(0, _yomiseglist.join("").length(), dummy, _yomiseglist);   // Resize max length
    Q_ASSERT(_yomiseglist.count() == 1);
    break;
    
  case CONVERTING_YOMI:
  case SHOWING_LIST:
  case CHANGING_LENGTH:
    // convert first segment to Katakana
    Q_ASSERT(!_yomiseglist[_crntsegidx].isEmpty());
    Q_ASSERT(!_convtkanjilist[_crntsegidx].isEmpty());
    // Replace string
    m.merge(Mode_HankakuKana);
    convertSegment(_crntsegidx, m);
    break;

  default:
    Q_ASSERT(0);
    return FALSE;
    break;
  }

  _candlist->hide();
  _stat = CONVERTING_YOMI;
  showPreeditArea();
  return TRUE;
}


bool
KanjiConvert::convertToHankakuEisu()
{
  DEBUG_TRACEFUNC();
  if (!_eng)
    return FALSE;

  InputMode  m(_mode);
  QStringList dummy;
  switch (_stat) {
  case NONE_YOMI:
    return TRUE;
    break;

  case INPUTTING_YOMI:
    if (_inputkev.getLast() && !_inputkev.getLast()->text().isEmpty()) {  // Check last KeyEvent
      _inputkev.append(new QKeyEvent(DictHiragana::SEPARATOR_CHAR));  // Insert SEPARATOR
    }
    m.merge(Mode_HankakuEisu);
    _convtkanjilist = convertYomigana(m);
    m.merge(Mode_Hiragana);
    _eng->beginConvert(convertYomigana(m), dummy, _yomiseglist);
    if (_yomiseglist.count() != 1)
      _eng->resizeSegment(0, _yomiseglist.join("").length(), dummy, _yomiseglist);   // Resize max length
    Q_ASSERT(_yomiseglist.count() == 1);
    break;
    
  case CONVERTING_YOMI:
  case SHOWING_LIST:
  case CHANGING_LENGTH:
    Q_ASSERT(!_yomiseglist[_crntsegidx].isEmpty());
    Q_ASSERT(!_convtkanjilist[_crntsegidx].isEmpty());
    m.merge(Mode_HankakuEisu);
    convertSegment(_crntsegidx, m);
    break;

  default:
    Q_ASSERT(0);
    return FALSE;
    break;
  }

  _candlist->hide();
  _stat = CONVERTING_YOMI;
  showPreeditArea();  
  return TRUE;
}


bool
KanjiConvert::convertToZenkakuEisu()
{
  DEBUG_TRACEFUNC();
  if (!_eng)
    return FALSE;

  InputMode  m(_mode);
  QStringList dummy;
  switch (_stat) {
  case NONE_YOMI:
    return TRUE;
    break;
    
  case INPUTTING_YOMI:
    if (_inputkev.getLast() && !_inputkev.getLast()->text().isEmpty()) {  // Check last KeyEvent
      _inputkev.append(new QKeyEvent(DictHiragana::SEPARATOR_CHAR));  // Insert SEPARATOR
    }
    m.merge(Mode_ZenkakuEisu);
    _convtkanjilist = convertYomigana(m);
    m.merge(Mode_Hiragana);
    _eng->beginConvert(convertYomigana(m), dummy, _yomiseglist);
    if (_yomiseglist.count() != 1)
      _eng->resizeSegment(0, _yomiseglist.join("").length(), dummy, _yomiseglist);   // Resize max length
    Q_ASSERT(_yomiseglist.count() == 1);
    break;
    
  case CONVERTING_YOMI:
  case SHOWING_LIST:
  case CHANGING_LENGTH:
    Q_ASSERT(!_yomiseglist[_crntsegidx].isEmpty());
    m.merge(Mode_ZenkakuEisu);
    convertSegment(_crntsegidx, m);
    break;

  default:
    Q_ASSERT(0);
    return FALSE;
    break;
  }

  _candlist->hide();
  _stat = CONVERTING_YOMI; 
  showPreeditArea();  
  return TRUE;
}


bool
KanjiConvert::switchZenkakuEisuMode()
{
  DEBUG_TRACEFUNC();
  return TRUE;
}


bool
KanjiConvert::setToHiraganaMode()
{
  DEBUG_TRACEFUNC();

  switch (_stat) {
  case NONE_YOMI:
    setInputMode(Kimera::Mode_Hiragana);
    break;

  case INPUTTING_YOMI:
  case CONVERTING_YOMI:
  case SHOWING_LIST:
  case CHANGING_LENGTH:
    // Do nothing
    break;

  default:
    Q_ASSERT(0);
    return FALSE;
    break;
  }

  return TRUE;
}


bool
KanjiConvert::setToKatakanaMode()
{
  DEBUG_TRACEFUNC();

  switch (_stat) {
  case NONE_YOMI:
    setInputMode(Kimera::Mode_Katakana);
    break;

  case INPUTTING_YOMI:
  case CONVERTING_YOMI:
  case SHOWING_LIST:
  case CHANGING_LENGTH:
    // Do nothing
    break;

  default:
    Q_ASSERT(0);
    return FALSE;
    break;
  }

  return TRUE;
}


bool
KanjiConvert::setToHankakuKanaMode()
{
  DEBUG_TRACEFUNC();

  switch (_stat) {
  case NONE_YOMI:
    setInputMode(Kimera::Mode_HankakuKana);
    break;

  case INPUTTING_YOMI:
  case CONVERTING_YOMI:
  case SHOWING_LIST:
  case CHANGING_LENGTH:
    // Do nothing
    break;

  default:
    Q_ASSERT(0);
    return FALSE;
    break;
  }

  return TRUE;
}


bool
KanjiConvert::setToZenkakuEisuMode()
{
  DEBUG_TRACEFUNC();

  switch (_stat) {
  case NONE_YOMI:
    setInputMode(Kimera::Mode_ZenkakuEisu);
    break;

  case INPUTTING_YOMI:
  case CONVERTING_YOMI:
  case SHOWING_LIST:
  case CHANGING_LENGTH:
    // Do nothing
    break;

  default:
    Q_ASSERT(0);
    return FALSE;
    break;
  }

  return TRUE;
}


bool
KanjiConvert::switchInputMethod()
{
  DEBUG_TRACEFUNC();

  switch (_stat) {
  case NONE_YOMI:
    setInputMode((_mode.id() & Kimera::Mode_RomaInput) ? Kimera::Mode_KanaInput : Kimera::Mode_RomaInput);
    break;

  case INPUTTING_YOMI:
  case CONVERTING_YOMI:
  case SHOWING_LIST:
  case CHANGING_LENGTH:
    // Do nothing
    break;

  default:
    Q_ASSERT(0);
    return FALSE;
    break;
  }

  return TRUE;
}


bool
KanjiConvert::showPropertyDialog()
{
  DEBUG_TRACEFUNC();

  switch (_stat) {
  case NONE_YOMI:
    emit propertyDialogActivated();
    break;

  case INPUTTING_YOMI:
  case CONVERTING_YOMI:
  case SHOWING_LIST:
  case CHANGING_LENGTH:
    // Do nothing
    break;

  default:
    Q_ASSERT(0);
    return FALSE;
    break;
  }

  return TRUE;
}


bool
KanjiConvert::execDictTool()
{
  DEBUG_TRACEFUNC();

  switch (_stat) {
  case NONE_YOMI:
  case INPUTTING_YOMI:
  case CONVERTING_YOMI:
  case SHOWING_LIST:
  case CHANGING_LENGTH:
    emit dictToolActivated();
    break;

  default:
    Q_ASSERT(0);
    return FALSE;
    break;
  }

  return TRUE;
}


bool
KanjiConvert::reconvertClipboardString()
{
  DEBUG_TRACEFUNC();

  QString str;
  switch (_stat) {
  case NONE_YOMI: {
    str = QApplication::clipboard()->text(QClipboard::Selection).left(1024);
    int idx = str.find('\n');
    if (idx > 0) {
      str = str.left(idx);
    }

    if (!str.isEmpty()) {
      qDebug("Mouse selection string: %s", str.local8Bit().data());
      _eng->beginConvert(str, _convtkanjilist, _yomiseglist);
      qDebug("kanjilist:%s   yomi:%s", _convtkanjilist.join(" ").local8Bit().data(),
	     _yomiseglist.join(" ").local8Bit().data());
      
      // Creates KeyEvent list
      createKeyEventList(_yomiseglist.join(""), _inputkev);
    }
    QApplication::clipboard()->clear(QClipboard::Selection);
    break; }

  case INPUTTING_YOMI:
  case CONVERTING_YOMI:
  case SHOWING_LIST:
  case CHANGING_LENGTH:
    // Do nothing
    break;
    
  default:
    Q_ASSERT(0);
    return FALSE;
    break;
  }
  
  _stat = CONVERTING_YOMI;
  showPreeditArea();
  return TRUE;
}


bool
KanjiConvert::caretForward()
{
  DEBUG_TRACEFUNC();
  
  switch (_stat) {
  case NONE_YOMI:
    // Do nothing
    break;

  case INPUTTING_YOMI:
    if (_caretpos < _yomiseglist[0].length())
      ++_caretpos;
    showPreeditArea();
    break;

  case CONVERTING_YOMI:
  case SHOWING_LIST:
  case CHANGING_LENGTH:
    // Do nothing
    break;

  default:
    Q_ASSERT(0);
    return FALSE;
    break;
  }

  return TRUE;
}


bool
KanjiConvert::caretBackward()
{
  DEBUG_TRACEFUNC();
  
  switch (_stat) {
  case NONE_YOMI:
    // Do nothing
    break;
 
  case INPUTTING_YOMI:
    if (_caretpos > 0)
      --_caretpos;
    showPreeditArea();
    break;

  case CONVERTING_YOMI:
  case SHOWING_LIST:
  case CHANGING_LENGTH:
    // Do nothing
    break;

  default:
    Q_ASSERT(0);
    return FALSE;
    break;
  }

  return TRUE;
}


bool
KanjiConvert::caretToFirst()
{
  DEBUG_TRACEFUNC();
  
  switch (_stat) {
  case NONE_YOMI:
    // Do nothing
    break;
 
  case INPUTTING_YOMI:
    _caretpos = 0;
    showPreeditArea();
    break;

  case CONVERTING_YOMI:
  case SHOWING_LIST:
  case CHANGING_LENGTH:
    // Do nothing
    break;

  default:
    Q_ASSERT(0);
    return FALSE;
    break;
  }

  return TRUE;
}


bool
KanjiConvert::caretToLast()
{
  DEBUG_TRACEFUNC();
  
  switch (_stat) {
  case NONE_YOMI:
    // Do nothing
    break;
 
  case INPUTTING_YOMI:
    _caretpos = _yomiseglist[0].length();
    showPreeditArea();
    break;

  case CONVERTING_YOMI:
  case SHOWING_LIST:
  case CHANGING_LENGTH:
    // Do nothing
    break;

  default:
    Q_ASSERT(0);
    return FALSE;
    break;
  }

  return TRUE;
}


void
KanjiConvert::replaceSegments(uint idx, uint num, const QStringList& convseg)
{
  DEBUG_TRACEFUNC("list: %s  idx: %d", convseg.join(" ").local8Bit().data(), idx);
  CHECK_NOCHANGE(_stat, int);

  qDebug("candidacies(%d) :%s  (crnt:%d  min:%d)", convseg.count(), convseg.join(" ").local8Bit().data(), _crntsegidx, _minsegidx);  
  if (idx + num > _convtkanjilist.count()) {
    Q_ASSERT(0);
    return;
  }

  // Sets converted kanji list
  QStringList::iterator it = _convtkanjilist.begin();
  it += idx;
  QStringList::iterator itend = it;
  itend += num;
  //_convtkanjilist.erase(it, _convtkanjilist.end());  // Erase
  it = _convtkanjilist.erase(it, itend);  // Erase
  for (int i = 0; i < (int)convseg.count(); ++i) {
    if ( !convseg[i].isEmpty() ) {  // Check str
      it = _convtkanjilist.insert(it, convseg[i]);   // insert str
      ++it;
    }
  }
}


void
KanjiConvert::setYomigana(int idx, const QStringList& yomiseg)
{
  DEBUG_TRACEFUNC("yomiseg:%s  idx:%d", yomiseg.join(" ").local8Bit().data(), idx);
  CHECK_NOCHANGE(_crntsegidx, int);
  CHECK_NOCHANGE(_convtkanjilist, QStringList);

  if (yomiseg.isEmpty() || idx < 0 || idx >= (int)_yomiseglist.count()) {
    Q_ASSERT(0);
    return;
  }

  // Sets converted kanji list
  QStringList::iterator it = _yomiseglist.begin();
  it += idx;
  _yomiseglist.erase(it, _yomiseglist.end());  // Erase
  for (int i = 0; i < (int)yomiseg.count(); ++i)
    if ( !yomiseg[i].isEmpty() )
      _yomiseglist << yomiseg[i];    // append str
  
  // Show segments
  qDebug("yomi: %s  convt:%s (crnt:%d min:%d)", _yomiseglist.join(" ").local8Bit().data(),
	 _convtkanjilist.join(" ").local8Bit().data(), _crntsegidx, _minsegidx);
}


void 
KanjiConvert::slotReplaceSegment(QListBoxItem* item)
{
  Q_CHECK_PTR( item );
  DEBUG_TRACEFUNC("item: %s", item->text().local8Bit().data());
  CHECK_NOCHANGE(_yomiseglist, QStringList);

  if (_stat != SHOWING_LIST)
    return;

  // Replace string
  replaceSegments(_crntsegidx, 1, item->text());
  _candlist->hide();
  _stat = CONVERTING_YOMI;
  showPreeditArea();
}


void
KanjiConvert::hideToolBox()
{
  DEBUG_TRACEFUNC();
  _preedit->hide();
  _candlist->hide();
}


bool
KanjiConvert::processKeyEvent(const QKeyEvent& key)
{
  DEBUG_TRACEFUNC("The key code: 0x%x   key count: %d  ascii: %s", key.key(), key.count(), key.text().data());

  int k = key.key();
  k |= (key.state() & Qt::ControlButton) ? Qt::CTRL : 0;
  k |= (key.state() & Qt::ShiftButton) ? Qt::SHIFT : 0;
  k |= (key.state() & Qt::AltButton) ? Qt::ALT : 0;

  bool res = TRUE;
  FuncID  fid = KeyAssigner::functionID( k, _stat );
  switch ( fid ) {
  case FID_None:
    res = (_stat != NONE_YOMI);
    break;

  case FID_InsertChar:
    if (_stat == SHOWING_LIST && k >= Qt::Key_1 && k <= Qt::Key_9) {
      res = decideCandidateList(k - Qt::Key_0);
    } else {
      res = insertChar( key );
    }
    break;    

  default:
    pmfunc  pf = funcptrdict[fid]->ptr();  // get pointer
    if ( !pf ) {
      qFatal("Incorrect processing  %s:%d", __FILE__, __LINE__);
      clear();
      res = FALSE;
    } else {
      qDebug("FuncName: %s", KeyAssigner::functionName( fid ).local8Bit().data());
      res = (this->*pf)();      // execute function
    }
    break;
  }
  
  return res;
}


void 
KanjiConvert::setPreeditPoint(QPoint p)
{
  DEBUG_TRACEFUNC("x:%d y:%d", p.x(), p.y());
  _preedit->movePreeditPos( p );
}


void
KanjiConvert::showPreeditArea()
{
  DEBUG_TRACEFUNC();
  CHECK_NOCHANGE(_convtkanjilist, QStringList);
  CHECK_NOCHANGE(_yomiseglist, QStringList);

  QStringList list;
  switch (_stat) {
  case NONE_YOMI:
    clear();
    break;

  case INPUTTING_YOMI:
    _preedit->showInputingString(_yomiseglist.join(""), _caretpos);
    qDebug("showInputingString caret:%d  Yomigana: %s", _caretpos, _yomiseglist.join("").local8Bit().data());
    break;

  case CONVERTING_YOMI:
  case SHOWING_LIST:
    if ( !_convtkanjilist.count() ) {
      qDebug("no converted kanji list");
      return;
    } else if (_minsegidx >= _convtkanjilist.count()) {
      qFatal("Bad parameter  _minsegidx:%d  _convtkanjilist.count():%d  %s:%d",
	     _minsegidx, _convtkanjilist.count(), __FILE__, __LINE__);
      return;
    }    

    for (int i = _minsegidx; i < (int)_convtkanjilist.count(); i++)
	list << _convtkanjilist[i];

    qDebug("showConvertingSegments : %s  (%d:%d)", list.join(" ").local8Bit().data(), _crntsegidx, _minsegidx);
    _preedit->showConvertingSegments(list, _crntsegidx - _minsegidx);
    break;

  case CHANGING_LENGTH:
    if ( !_convtkanjilist.count() ) {
      qDebug("no converted kanji list");
      return;
    } else if (_minsegidx >= _yomiseglist.count()) {
      qFatal("Bad parameter  _minsegidx:%d  _convtkanjilist.count():%d  %s:%d",
	     _minsegidx, _convtkanjilist.count(), __FILE__, __LINE__);
      return;
    }
    
    for (int i = _minsegidx; i < (int)_crntsegidx; i++)
      list << _convtkanjilist[i];
    
    for (int i = _crntsegidx; i < (int)_yomiseglist.count(); i++)
      list << _yomiseglist[i];
    
    qDebug("showChangingSegmentLength : %s  (%d:%d)", list.join(" ").local8Bit().data(), _crntsegidx, _minsegidx);
    _preedit->showChangingSegmentLength(list, _crntsegidx - _minsegidx);
    break;
    
  default:
    Q_ASSERT(0);
    break;
  }
}


//
//  Auto-deletion of QPtrList class (kevlst) must be enabled
//
void
KanjiConvert::syncKeyEventListForward(const QString& str, QPtrList<QKeyEvent>& kevlst) const
{
    DEBUG_TRACEFUNC("str: %s", str.local8Bit().data());

  if (kevlst.isEmpty()) {
    return;
  }
  if (str.isEmpty()) {
    kevlst.clear();
    return;
  }

  bool ok = FALSE;
  bool hankaku = isTenkeyHankaku();
  int n = QMIN(kevlst.count(), 5);
  for (int i = 0; i < n; ++i) {
    kevlst.first();
    kevlst.remove();
    if (_dict.convert(kevlst, _mode, hankaku) == str) {
      ok = TRUE;
      break;
    }
  }

  // Create new QKeyEventList
  if (!ok) {
    createKeyEventList(str, kevlst);
  }
  qDebug("%s  str:%s", __func__, str.local8Bit().data());
}


//
//  Auto-deletion of QPtrList class (kevlst) must be enabled
//
void
KanjiConvert::syncKeyEventListBackward(const QString& str, QPtrList<QKeyEvent>& kevlst) const
{
  DEBUG_TRACEFUNC("str: %s", str.local8Bit().data());
 
  if (kevlst.isEmpty()) {
    return;
  }
  if (str.isEmpty()) {
    kevlst.clear();
    return;
  }

  bool ok = FALSE;
  bool hankaku = isTenkeyHankaku();
  int n = QMIN(kevlst.count(), 5);
  for (int i = 0; i < n; ++i) {
    kevlst.last();
    kevlst.remove();
    if (_dict.convert(kevlst, _mode, hankaku) == str) {
      ok = TRUE;
      break;
    }
  }

  // Create new QKeyEventList
  if ( !ok ) {
    createKeyEventList(str, kevlst);
  }
}


QString
KanjiConvert::convertYomigana(const InputMode& m)
{
  DEBUG_TRACEFUNC();
 
  Q_ASSERT((m.id() & Kimera::Mode_ModeMask) && (m.id() & Kimera::Mode_InputMask));
  Q_ASSERT( !_inputkev.isEmpty() );
  return _dict.convert(_inputkev, m, isTenkeyHankaku());
}


void
KanjiConvert::predict()
{
  DEBUG_TRACEFUNC();
  if (_engpdct && _mode.id() & (Mode_Hiragana | Mode_Katakana)) {
    InputMode m(_mode);
    m.merge( (_mode.id() & Mode_RomaInput) ? Mode_HankakuEisu : Mode_Hiragana );
    _engpdct->predict(convertYomigana(m), convertYomigana(_mode));
  }
}


void
KanjiConvert::showCandidatePredicted(const QString& cand)
{
  DEBUG_TRACEFUNC();
  if (_stat == INPUTTING_YOMI && cand != _preedit->text()) {
    _candlist->showCandidatePredicted(cand);
  } else {
    _candlist->showCandidatePredicted(QString::null);   // Hides prediction box
  }
}

void
KanjiConvert::learn(const QStringList& kanji)
{
  DEBUG_TRACEFUNC();
  if (!_engpdct)
    return;

  // Roma-input
  InputMode  m(_mode);
  switch (_stat) {
  case NONE_YOMI:
    // Do nothing
    break;
    
  case INPUTTING_YOMI:
    m.merge( (_mode.id() & Mode_RomaInput) ? Mode_HankakuEisu : Mode_Hiragana );
    _engpdct->learn(kanji, convertYomigana(m));
    break;

  case CONVERTING_YOMI:
  case SHOWING_LIST:
  case CHANGING_LENGTH:
    if (_yomiseglist.count() == 1 && (_mode.id() & Mode_RomaInput)
	&& (_mode.id() & (Mode_Hiragana | Mode_Katakana))) {
      m.merge( Mode_HankakuEisu );
      _engpdct->learn(kanji, convertYomigana(m));
    } else {
      _engpdct->learn(kanji, _yomiseglist);
    }
    break;

  default:
    Q_ASSERT(0);
    return;
    break;
  }
}


void
KanjiConvert::learnPrediction(const QString &)
{
  DEBUG_TRACEFUNC();
  //_engpdct->learnPrediction(kanji, _inputkeys);
}


int
KanjiConvert::findIndexKeyEventList(uint caret_pos) const
{
  DEBUG_TRACEFUNC("caret_pos: %d", caret_pos);
  
  if (_inputkev.isEmpty())
    return -1;

  if (caret_pos >= _yomiseglist.join("").length())
    return _inputkev.count();
  
  int idx = 0;
  QPtrListIterator<QKeyEvent> it(_inputkev);
  if (caret_pos > 0) {
    bool hankaku = isTenkeyHankaku();
    QPtrList<QKeyEvent>  klst;
    klst.setAutoDelete( FALSE );
    QString target = _dict.convertString(_yomiseglist.join("").left(caret_pos), _mode);
    //QString target = _yomiseglist.join("").left(caret_pos);

    QKeyEvent* kev;  
    while ( (kev = it.current()) ) {
      ++it;
      ++idx;
      klst.append( kev );
      QString s = _dict.convert(klst, _mode, hankaku);
      qDebug("s:%s  target:%s", s.local8Bit().data(), target.local8Bit().data());
      if (s == target) {
	break;
      }
      if (s.find(target) == 0) {
	idx = -1;
	break;
      }
    }
  }

  Q_ASSERT(idx < (int)_inputkev.count());
  qDebug("%s: idx:%d  _inputkev.count():%d", __func__, idx, _inputkev.count());
  return idx;
}


//
// Create new QKeyEventList
//   Auto-deletion of QPtrList class (kevlst) must be enabled
//
void
KanjiConvert::createKeyEventList(const QString& yomi, QPtrList<QKeyEvent>& kevlst) const
{
  DEBUG_TRACEFUNC("yomi: %s  (_mode.id:0x%x)", yomi.local8Bit().data(), _mode.id());
  
  if ((_mode.id() & Mode_KanaInput) && (_mode.id() & (Mode_Hiragana | Mode_Katakana | Mode_HankakuKana))) {
    Q_ASSERT(0);
    return;
  }
  
  _dict.reverse(yomi, _mode, isTenkeyHankaku(), kevlst);
  qDebug("kevlst.count():%d", kevlst.count());
}


void
KanjiConvert::convertSegment(uint segment_idx, const InputMode& mode)
{
  DEBUG_TRACEFUNC("segment_idx: %d", segment_idx);

  int bcrt = 0;
  for (int i = 0; i < (int)segment_idx; ++i)
    bcrt += _yomiseglist[i].length();
  
  int bgn = findIndexKeyEventList(bcrt);
  if (bgn < 0) {
    createKeyEventList(_yomiseglist.join(""), _inputkev);
    bgn = findIndexKeyEventList(bcrt);
  }
  int end = findIndexKeyEventList(bcrt + _yomiseglist[_crntsegidx].length()); 
  QPtrList<QKeyEvent>  klst;
  klst.setAutoDelete( FALSE );
  for (int i = bgn; i < end; ++i)
    klst.append( _inputkev.at(i) );
  
  replaceSegments(segment_idx, 1, _dict.convert(klst, mode));
}


QString
KanjiConvert::currentSettingSpaceChar() const
{
  DEBUG_TRACEFUNC();
  QString str;
  QString cfg = Config::readEntry("_cmbspacekey", "");
  if (cfg == tr("半角") || cfg == tr("入力モードに従う") && (_mode.id() & (Mode_HankakuKana | Mode_HankakuEisu)))
    str = " ";
  else
    str = tr("　");

  return str;
}


bool
KanjiConvert::isTenkeyHankaku() const
{
  QString cfg = Config::readEntry("_cmbtenkey", "");
  return (cfg == tr("半角") || cfg == tr("入力モードに従う") && (_mode.id() & (Mode_HankakuKana | Mode_HankakuEisu)));
}

