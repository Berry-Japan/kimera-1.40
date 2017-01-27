#include "dicthiragana.h"
#include "inputmode.h"
#include "config.h"
#include "debug.h"
#include <qapplication.h>
#include <qdir.h>
#include <qfile.h>
#include <qmessagebox.h>
#include <qtextstream.h>
#include <qregexp.h>
using namespace Kimera;

const QKeyEvent DictHiragana::SEPARATOR_CHAR = QKeyEvent((QEvent::Type)QEvent::KeyPress, 0, 0, 0);

DictHiragana::DictHiragana() : _dicthira(QDict<QString>(17, FALSE)),
			       _dictkata(QDict<QString>()),
			       _dicthankana(QDict<QString>()),
			       _dictalphbt(QDict<QString>()),
			       _dictsymbol(QDict<QString>()),
			       _dictkanainp(QDict<QString>(17, FALSE)),
			       _dictdakuten(QDict<QString>()),
			       _reversedict(QDict<QString>())
{
  _dicthira.setAutoDelete(TRUE);
  _dictkata.setAutoDelete(TRUE);
  _dicthankana.setAutoDelete(TRUE);
  _dictalphbt.setAutoDelete(TRUE);
  _dictsymbol.setAutoDelete(TRUE);
  _dictkanainp.setAutoDelete(TRUE);
  _dictdakuten.setAutoDelete(TRUE);
  _reversedict.setAutoDelete(TRUE);
}


DictHiragana::~DictHiragana() { }


void
DictHiragana::init()
{
  DEBUG_TRACEFUNC();
  _dicthira.clear();
  _dictkata.clear();
  _dicthankana.clear();
  _dictalphbt.clear();
  _dictsymbol.clear();
  _dictkanainp.clear();
  _dictdakuten.clear();

  // Copy dictionary files
  copyDictFile("hiragana.dic");
  copyDictFile("katakana.dic");
  copyDictFile("hankakukana.dic");
  copyDictFile("zenkakualphabet.dic");
  copyDictFile("numeralsymbols.dic");
  copyDictFile("kanainput.dic");
  copyDictFile("dakuten.dic");

  QString dictdir = Config::dictionaryDirPath();
  initDict(_dicthira, dictdir + "/hiragana.dic");          // ひらがな辞書初期化
  initDict(_dictkata, dictdir + "/katakana.dic");          // カタカナ辞書初期化
  initDict(_dicthankana, dictdir + "/hankakukana.dic");    // 半角カナ辞書初期化
  initDict(_dictalphbt, dictdir + "/zenkakualphabet.dic"); // 全角英字辞書初期化
  initDict(_dictsymbol, dictdir + "/numeralsymbols.dic");  // 全角数字記号辞書初期化
  initDict(_dictkanainp, dictdir + "/kanainput.dic");      // かな入力辞書初期化
  initDict(_dictdakuten, dictdir + "/dakuten.dic");        // かな入力濁点辞書初期化

  // 句読点・記号・括弧
  _dictsymbol.insert(",", new QString(Config::readEntry("_cmbtouten", ","))); 
  _dictsymbol.insert(".", new QString(Config::readEntry("_cmbkuten", ".")));
  _dictsymbol.insert("/", new QString(Config::readEntry("_cmbsymbol", "/")));
  _dictsymbol.insert("[", new QString(Config::readEntry("_cmbbracket", "[").left(1)));
  _dictsymbol.insert("]", new QString(Config::readEntry("_cmbbracket", "]").right(1)));

  // 逆引き辞書初期化
  initDict(_reversedict, dictdir + "/hiragana.dic", TRUE);
  initDict(_reversedict, dictdir + "/zenkakualphabet.dic", TRUE);
  initDict(_reversedict, dictdir + "/numeralsymbols.dic", TRUE);
  initDict(_reversedict, dictdir + "/hankakukana.dic", TRUE);
  initDict(_reversedict, dictdir + "/katakana.dic", TRUE);
  _reversedict.replace(Config::readEntry("_cmbtouten", ","), new QString(","));
  _reversedict.replace(Config::readEntry("_cmbkuten", "."), new QString("."));
  _reversedict.replace(Config::readEntry("_cmbsymbol", "/"), new QString("/")); 
  _reversedict.replace(Config::readEntry("_cmbbracket", "[").left(1), new QString("["));
  _reversedict.replace(Config::readEntry("_cmbbracket", "]").right(1), new QString("]")); 
  _reversedict.replace(QObject::tr("゛"), new QString("@"));
  _reversedict.replace(QObject::tr("゜"), new QString("["));
  _reversedict.replace(QObject::tr("ﾞ"), new QString("@"));
  _reversedict.replace(QObject::tr("ﾟ"), new QString("["));
}


void
DictHiragana::initDict(QDict<QString>& dict, const QString& dictfile, bool reverse_dict)
{
  DEBUG_TRACEFUNC();
  QFile file(dictfile);
  if ( !file.open(IO_ReadOnly) ) {
    QMessageBox::critical(0, "File open error", "Cannot open file: " + dictfile,
    			  QMessageBox::Ok | QMessageBox::Default, 0);
    qApp->quit();
    return;
  }
    
  QTextStream  stream(&file);
  stream.setEncoding(QTextStream::Latin1);

  while ( !stream.eof() ) {
    //QString line = QString::fromLocal8Bit(stream.readLine());
    QString line = QObject::tr(stream.readLine());
    if ( !line.isEmpty() && !line.contains(QRegExp("^##")) ) {
      QString keystr, itemstr;
      int idx = line.find(QRegExp("[ \t]"));
      if (idx > 0) {
	keystr = line.left(idx);	
	line.remove(0, keystr.length());
	itemstr = line.replace(QRegExp("[ \t]"), "");
	if (!keystr.isEmpty() && !itemstr.isEmpty()) {
	  if ( !reverse_dict ) {
	    if ( !dict.find(keystr) )
	      dict.insert(keystr, new QString(itemstr));
	  } else {
	    // reverse-dictionary
	    if ( !dict.find(itemstr) )
	      dict.insert(itemstr, new QString(keystr));
	  }
	}
      }
    }
  }
  
  file.close();
}


// 平仮名へ変換する (濁点を前の文字に合わせて１文字にする 例:か゛-> が )
// parameter:    Roma string (only ASCII characters) or Hiragana string
// return value: Hiragana string
QString
DictHiragana::convertToHira(const QString& src) const
{
  DEBUG_TRACEFUNC("src: %s", src.local8Bit().data());
  // check parameter
  if ( src.isEmpty() )
    return QString::null;

  if (src.local8Bit().length() != src.length())
    return replaceDakutenChar(src);
  
  QString dest;
  uint index = 0;
  while (index < src.length()) {
    for (int len = 4; len > 0; --len) {
      QString*  pstr;
      // ひらがな検索
      pstr = _dicthira.find( src.mid(index, len) );
      if (pstr) {
	dest += *pstr;
	index += len;
	break;
      } 

      // 全角数字記号検索
      pstr = _dictsymbol.find( src.mid(index, len) );
      if (pstr) {
	dest += *pstr;
	index += len;
	break;
      }
      
      if (len == 1) {
	if (src.constref(index) == src.constref(index + 1)
	    && src.mid(index, 1).find( QRegExp("[^aiueo]", FALSE) ) == 0
	    && src.mid(index + 2, 1).find( QRegExp("[aiueoyhsw]", FALSE) ) == 0 ) {
	  // 'っ'に変換
	  pstr = _dicthira.find( src.mid(index + 1, 2) );
	  if (pstr) {
	    dest += QObject::tr("っ") + *pstr;
	    index += 3;
	  } else {
	    dest += QObject::tr("っ");
	    ++index;
	  }

	} else if ((src.constref(index) == "n" || src.constref(index) == "N")
		   && src.mid(index + 1, 1).find( QRegExp("[^aiueoy]", FALSE) ) == 0) {
	  // 'ん'に変換
	  dest += QObject::tr("ん");
	  ++index;
	  
	} else {
	  // 無変換
	  dest += src.constref(index);
	  ++index;
	}
	break;
      }
    }
  }

  return dest;
}


// parameter: Roma string (only ASCII characters) or Hiragana string
QString
DictHiragana::convertToKata(const QString& src) const
{
  DEBUG_TRACEFUNC("src: %s", src.local8Bit().data());
  Q_ASSERT(!_dictkata.isEmpty());

  QString dest;
  // check parameter
  if (src.local8Bit().length() == src.length()) {
    // Case of Romaji
    QString hira = convertToHira(src);
    for (int j = 0; j < (int)hira.length(); j++) {
      QString* pstr = _dictkata.find( hira.constref(j) );
      if ( pstr ) {
	dest += *pstr;
      } else {
	dest += hira.constref(j);
      }
    }
  } else {
    // Hiragana -> Katakana
    QString str = replaceDakutenChar(src);
    for (int i = 0; i < (int)str.length(); i++) {
      QString* pstr = _dictkata.find( str.constref(i) );
      if ( pstr ) {
	dest += *pstr;
      } else {
	dest += str.constref(i);
      }
    }
  }
  
  return  dest;
}


// parameter: Roma string (only ASCII characters) or Hiragana string
QString
DictHiragana::convertToHankakuKana(const QString& src) const
{
  DEBUG_TRACEFUNC();
  Q_ASSERT(!_dicthankana.isEmpty());

  QString dest;
  // check parameter
  if (src.local8Bit().length() == src.length()) {
    QString hira = convertToHira(src);
    for (int j = 0; j < (int)hira.length(); j++) {
      QString* pstr = _dicthankana.find( hira.constref(j) );
      if ( pstr ) {
	dest += *pstr;
      } else {
	dest += hira.constref(j);
      }
    }
    
  } else {
    // Hiragana -> HankakuKana
    for (int i = 0; i < (int)src.length(); i++) {
      QString* pstr = _dicthankana.find( src.constref(i) );
      if ( pstr ) {
	dest += *pstr;
      } else {
	dest += src.constref(i);
      }
    }
  }
  
  return  dest;
}


// parameter: Roma string (only ASCII characters)
QString
DictHiragana::convertToZenkakuEisu(const QString& src) const
{
  DEBUG_TRACEFUNC();
  Q_ASSERT(!_dictalphbt.isEmpty());
  Q_ASSERT(!_dictsymbol.isEmpty());

  QString dest;
  for (int i = 0; i < (int)src.length(); i++) {
    QString* pstr;
    if ( (pstr = _dictalphbt.find(src.constref(i))) ) {  // if the key exists
      dest += *pstr;
      
    } else if ( (pstr = _dictsymbol.find(src.constref(i))) ) {  // if the key exists 
      dest += *pstr;
      
    } else {
      dest += src.constref(i);
    }
  }
  
  return dest;
}


QString
DictHiragana::convertString(const QString& hira, const InputMode& mode) const
{
  DEBUG_TRACEFUNC("hira: %s  mode:0x%x", hira.local8Bit().data(), mode.id());
  if (hira.isEmpty())
    return QString::null;

  QString res;
  switch (mode.id() & Mode_ModeMask) {
  case Mode_Hiragana:
    res = convertToHira(hira);
    break;
    
  case Mode_Katakana:
    res = convertToKata(hira);
    break;
    
  case Mode_HankakuKana:
    res = convertToHankakuKana(hira);
    break;

  case Mode_ZenkakuEisu:
    if (hira.local8Bit().length() != hira.length()) {
      // Multibyte chars
      res = convertToZenkakuEisu( reverseConvt(hira) );
    } else {
      res = convertToZenkakuEisu(hira);
    }
    break;
    
  case Mode_HankakuEisu:
    if (hira.local8Bit().length() != hira.length()) {
      res = reverseConvt(hira);
    } else {
      res = hira;
    }
    break;

  default:
    Q_ASSERT(0);
    break;
  }
  
  return res;
}


//
// 最後の文字が ('n' + SEPARATOR_CHAR) だったら, 'ん' に変換
//
QString
DictHiragana::convert(const QPtrList<QKeyEvent>& kevlst, const InputMode& mode, bool tenkey_hankaku) const
{
  DEBUG_TRACEFUNC("kevlst.count: %u  mode: 0x%x  tenkey_hankaku: %d", kevlst.count(), mode.id(), tenkey_hankaku);
  if ( !kevlst.count() )
    return QString::null;
  
  QString res;
  QPtrListIterator<QKeyEvent> it(kevlst);
  QKeyEvent* kev;  

  if ((mode.id() & Mode_KanaInput) && !(mode.id() & (Mode_ZenkakuEisu | Mode_HankakuEisu))) {
    QString str;
    while ( (kev = it.current()) ) {
      ++it;
      if (tenkey_hankaku && (kev->state() & Qt::Keypad)) {
	res += convertString(str, mode);   // 濁点変換
	str = QString::null;
	res += kev->text();
      } else {
	str += convertKey(*kev, mode);
      }
    }
    res += convertString(str, mode);

  } else if (mode.id() & (Mode_ZenkakuEisu | Mode_HankakuEisu)) {
    QString cstr;
    while ( (kev = it.current()) ) {
      ++it;
      cstr += kev->text();
    }
    res = convertString(cstr, mode);
    
  } else {
    QString cstr;
    while ( (kev = it.current()) ) {
      ++it;
      if (tenkey_hankaku && (kev->state() & Qt::Keypad)) {
	res += convertString(cstr, mode);
	cstr = QString::null;
	res += kev->text();
      } else if (kev->state() & Qt::MetaButton) {
	// Couples dakuten-char
	QString s = convertString(cstr, mode);
	if (s.length() > 1) {
	  res += s.left(s.length() - 1);
	}
	res += convertString(s.right(1) + convertKey(*kev, mode), mode);
	cstr = QString::null;
      } else {
	if ( !kev->text().isEmpty() ) {   // means it's not SEPARATOR_CHAR
	  cstr += kev->text();
	} else {
	  res += convertString(cstr, mode); // Check next char
	  cstr = QString::null;
	}
      }
    }
    res += convertString(cstr, mode);

    // Check last KeyEvent
    if (kevlst.getLast()->text().isEmpty() && (res.right(1) == "n" || res.right(1) == "N")) {
      res.truncate(res.length() - 1);
      res += convertString("nn", mode);
    }
  }
  
  return res;
}


//  かな入力変換
//  params: QKeyEvent
//  params: Input mode
//  return: Hiragana string
QString
DictHiragana::convertKey(const QKeyEvent& key, const InputMode& mode) const
{
  DEBUG_TRACEFUNC();
  if (mode.id() & Mode_ZenkakuEisu) {
    return convertToZenkakuEisu(key.text());
  }
  if (mode.id() & Mode_HankakuEisu) {
    return key.text();
  }
  if ((mode.id() & Mode_RomaInput) && !(key.state() & Qt::MetaButton)) {
    return QString::null;
  }
  
  QString keystr;
  if (key.state() == Qt::NoButton) {
    keystr = key.text();
  } else if (key.state() & Qt::ShiftButton) { 
    keystr = QString("Shift+") + key.text();
  } else if (key.state() & Qt::Keypad) {
    keystr = QString("Keypad+") + key.text();
  } else if (key.state() & Qt::MetaButton) {
    keystr = QString("Meta+") + key.text();
  } else {
    return QString::null;
  }

  QString* pres = _dictkanainp[keystr];
  if ( !pres )
    return QString::null;

  QString res;
  switch (mode.id() & Mode_ModeMask) {
  case Mode_Hiragana:
    res = *pres;
    break;
    
  case Mode_Katakana:
    res = convertToKata(*pres);
    break;

  case Mode_HankakuKana:
    res = convertToHankakuKana(*pres);
    break;
    
  default:
    Q_ASSERT(0);
    break;
  }

  return res;
}


QString
DictHiragana::reverseConvt(const QString& hira) const
{
  DEBUG_TRACEFUNC();
  QString res;
  for (int i = 0; i < (int)hira.length(); ++i) {
    QChar c = hira[i];
    if ( c.latin1() ) {   // Check ascii or multibyte-char
      // Ascii char
      res += c;
    } else {
      // Multibyte char
      QString* pstr = _reversedict.find(hira.mid(i, 1));
      res += pstr ? *pstr : c;
    }
  } 
  return (res == hira) ? res : reverseConvt(res);
}


//
// Create new QKeyEventList
//   Auto-deletion of QPtrList class (kevlst) must be enabled
void
DictHiragana::reverse(const QString& yomi, const InputMode& mode, bool tenkey_hankaku, QPtrList<QKeyEvent>& kevlst) const
{
  DEBUG_TRACEFUNC();

  kevlst.clear();
  if (yomi.isEmpty())
    return;

  if ((mode.id() & Mode_KanaInput) && (mode.id() & (Mode_Hiragana | Mode_Katakana | Mode_HankakuKana))) {
    Q_ASSERT(0);
    return;
  }

  QRegExp rx("[0-9+*/.-]");
  for (int i = 0; i < (int)yomi.length(); ++i) {
    if (tenkey_hankaku && rx.exactMatch(yomi.mid(i, 1))) {
      // Tenkey 
      kevlst.append(new QKeyEvent(QEvent::KeyPress, 0, 0, Qt::Keypad, yomi.mid(i, 1)));
    } else if ((mode.id() & Mode_HankakuKana) && (yomi.mid(i, 1) == QObject::tr("ﾞ") || yomi.mid(i, 1) == QObject::tr("ﾟ"))) {
      // Dakuten or Handakuten
      if (kevlst.count() > 1 && kevlst.last()->text().isEmpty()) {  // SEPARATOR_CHAR
	kevlst.remove(kevlst.count() - 1);
      }
      QString cstr = convertString(yomi.mid(i, 1), Mode_HankakuEisu);
      kevlst.append(new QKeyEvent(QEvent::KeyPress, 0, 0, Qt::MetaButton, cstr));  // Exceptional key event
    } else {
      QString cstr = convertString(yomi.mid(i, 1), Mode_HankakuEisu);
      for (int j = 0; j < (int)cstr.length(); ++j) {
	kevlst.append(new QKeyEvent(QEvent::KeyPress, 0, 0, 0, cstr.mid(j, 1)));
      }
    }
    
    if (i != (int)yomi.length() - 1)
      kevlst.append(new QKeyEvent(DictHiragana::SEPARATOR_CHAR));  // Insert SEPARATOR_CHAR
  }
}


void
DictHiragana::copyDictFile(const QString dicname)
{
  DEBUG_TRACEFUNC();
  QFile src(QString(INSTALL_PATH "/dic/") + dicname);
  QDir  dir(Config::dictionaryDirPath());
  QFile dst( dir.filePath(dicname) );

  if( !dst.exists() ) {  
    if ( !src.exists() || !src.open(IO_ReadOnly) ) {
      qFatal("can not open read-mode file: %s", dicname.ascii());
      return;
    }
    
    if ( !dst.open(IO_WriteOnly) ) {
      qFatal("can not open write-mode file: %s", dicname.ascii());
      return;
    }
    
    QTextStream tssrc(&src);
    QTextStream tsdst(&dst);
    tssrc.setEncoding(QTextStream::Latin1);
    tsdst.setEncoding(QTextStream::Latin1);
    tsdst << tssrc.read();
  }
}


QString
DictHiragana::replaceDakutenChar(const QString& str) const
{
  if (str.length() <= 1) 
    return str;
  
  QString dest;
  uint i = 0;
  while (i < str.length()) {
    QString* pstr;
    pstr = _dictdakuten.find( str.mid(i, 2) );
    
    if ( pstr ) {
      dest += *pstr;
      ++i;
    } else {
      dest += str.constref(i);
    }
    ++i;
  }
  
  return dest;
}
