#include "cannaengine.h"
#include "kimeraglobal.h"
#include "config.h"
#include "debug.h"
#include <qapplication.h>
#include <qeventloop.h>
#include <qmessagebox.h>
#include <qcstring.h>
#include <qdatastream.h>
#include <qtextcodec.h>
#include <qsocket.h>
#include <qdatetime.h>
#include <qstringlist.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <stdlib.h>
#include <pwd.h>
#include <grp.h>

static QStringList CANNA_AF_UNIX_FILES;

CannaEngine::CannaEngine()
  : _contxt(0), _convertflag(FALSE), _connectimer(0)
{
  CANNA_AF_UNIX_FILES << "/tmp/.iroha_unix/IROHA" << "/var/run/.iroha_unix/IROHA";
  _sock = new QSocket();
  connect(_sock, SIGNAL(connected()), this, SLOT(slotConnected()));
  connect(_sock, SIGNAL(error(int)), this, SLOT(slotDetectError()));
  connect(_sock, SIGNAL(connectionClosed()), this, SLOT(slotConnectionClosed()));
}


CannaEngine::~CannaEngine()
{
  delete _sock;
}


QString
CannaEngine::name() const
{
  return QString("Canna");
}


bool
CannaEngine::isTCPConnectionSupported() const
{
  return TRUE;
}


// connect to Kanji server
bool
CannaEngine::init()
{
  DEBUG_TRACEFUNC();
  _sock->close();

  if ( Config::readBoolEntry("_grpremote", FALSE) ) {
    // INET Domain
    qDebug("INET Domain socket");
    _sock->connectToHost(Config::readEntry("_edtsvrname", "localhost"),
			 Config::readEntry("_edtport", "0").toUInt());
    _connectimer = startTimer(3000);

  } else {
    // UNIX Domain
    qDebug("UNIX Domain socket");

    int  fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (fd < 0) {
      QMessageBox::critical(0, "Kanji Server Error",
			    "Socket Create Error",
			    QMessageBox::Ok | QMessageBox::Default, 0);
      return FALSE;
    }

    int i;
    for (i = 0; i < (int)CANNA_AF_UNIX_FILES.count(); ++i) {
      struct sockaddr_un  addr;
      memset(&addr, 0, sizeof(addr));
      addr.sun_family = AF_UNIX;
      strncpy(addr.sun_path, CANNA_AF_UNIX_FILES[i].latin1(), sizeof(addr.sun_path) - 1);

      // connect
      if ( !::connect(fd, (sockaddr *)&addr, sizeof(addr)) )
	break;
    }

    if (i == (int)CANNA_AF_UNIX_FILES.count()) {
      QMessageBox::critical(0, "Kanji Server Error",
			    "Socket Connection Error",
			    QMessageBox::Ok | QMessageBox::Default, 0);
      ::close(fd);
      return FALSE;
    }

    _sock->setSocket(fd);  // UNIX Domain socket set
    slotConnected();
  }

  return TRUE;
}


void
CannaEngine::cleanup()
{
  DEBUG_TRACEFUNC();
  _sock->close();
  _contxt = 0;
  _convertflag = FALSE;
  if (_connectimer > 0) {
    killTimer(_connectimer);
    _connectimer = 0;
 }
}


void
CannaEngine::slotConnected()
{
  DEBUG_TRACEFUNC();
  qDebug("socket: %d", _sock->socket());
  sendInitialize();
}


void
CannaEngine::slotDetectError()
{
  DEBUG_TRACEFUNC();
  cleanup();
  Config::writeEntry("_grpremote", FALSE);

  QMessageBox::critical(0, "Kanji Server Error",
			tr("Canna との通信でエラーが発生しました。\n"
			   "かな漢字変換を停止します。"),
			QMessageBox::Ok | QMessageBox::Default, 0);
}


void
CannaEngine::slotConnectionClosed()
{
  DEBUG_TRACEFUNC();
  cleanup();
  Config::writeEntry("_grpremote", FALSE);

  QMessageBox::warning(0, "Kanji Server Connection Closed",
		       tr("Canna とのセッションが切断されました。\n"
			  "かな漢字変換を停止します。"),
		       QMessageBox::Ok | QMessageBox::Default, 0);
}


void
CannaEngine::sendInitialize()
{
  DEBUG_TRACEFUNC();
  QByteArray  pack;
  QDataStream ds(pack, IO_WriteOnly);
  ds.setByteOrder(QDataStream::BigEndian);

  QCString user("3.3:");
  struct passwd* pass = getpwuid( getuid() );
  user += QCString(pass->pw_name);
  ds << (int)RID_INITIALIZE << (int)user.length() + 1;
  ds.writeRawBytes(user.data(), user.length() + 1);
  _sock->writeBlock(pack.data(), pack.size());

  // wait for replay
  QTime t;
  t.start();
  while (_sock->bytesAvailable() != 4) {
    qApp->eventLoop()->processEvents(QEventLoop::ExcludeUserInput);
    if (t.elapsed() > 2000) {
      qWarning("No response!");
      return;
    }
  }

  recvInitializeReply();
}


void
CannaEngine::recvInitializeReply()
{
  DEBUG_TRACEFUNC();
  uint len;
  if ((len = _sock->size()) != 4) {
    qWarning("Server response incorrect.  %s:%d", __FILE__, __LINE__);
  }

  QByteArray respack(len);
  QDataStream ds(respack, IO_ReadOnly);
  ds.setByteOrder(QDataStream::BigEndian);

  // response packet recv
  if (_sock->readBlock(respack.data(), len) < 0) {
    qFatal("Recv error.  %s:%d", __FILE__, __LINE__);
    return;
  }

  int res;
  ds >> res;
  if (res == -1 || res == -2) {
    cleanup();
    QMessageBox::critical(0, "Protocol version error",
			  "Canna protocol version error",
			  QMessageBox::Ok | QMessageBox::Default, 0);
    return;
  }

  QDataStream dst(respack, IO_ReadOnly);
  dst.setByteOrder(QDataStream::BigEndian);
  u_int16_t  minor;
  dst >> minor >> _contxt;
  qDebug("minor version: %d  context: %d", minor, _contxt);

  noticeGroupName();
}


void
CannaEngine::noticeGroupName()
{
  DEBUG_TRACEFUNC();
  QByteArray  pack;
  QDataStream ds(pack, IO_WriteOnly);
  ds.setByteOrder(QDataStream::BigEndian);

  char* gname = "users";
  struct passwd* pass = getpwuid( getuid() );
  if ( pass ) {
    struct group* grp = getgrgid(pass->pw_gid);
    if ( grp ) {
      gname = grp->gr_name;
    }
  }

  u_int16_t glen = strlen(gname) + 1;
  ds << (u_int8_t)RID_NOTICE_GROUP_NAME << (u_int8_t)0 << (u_int16_t)(glen + 6)
     << (int)0 << _contxt;
  ds.writeRawBytes(gname, glen);
  _sock->writeBlock(pack.data(), pack.size());

  if ( !waitForReply(RID_NOTICE_GROUP_NAME) ) {
    slotDetectError();
    return;
  }

  recvNoticeGroupName();
}


void
CannaEngine::recvNoticeGroupName()
{
  DEBUG_TRACEFUNC();
  uint datalen = _sock->size();
  QByteArray data(datalen);
  if (_sock->readBlock(data.data(), datalen) < 0) {
    qFatal("Recv error.  %s:%d", __FILE__, __LINE__);
    return;
  }

  QDataStream ds(data, IO_ReadOnly);
  ds.setByteOrder(QDataStream::BigEndian);

  u_int16_t len;
  ds >> len;

  if (len != datalen - 2) {
    qWarning("Server response incorrect.  %s", __func__);
  }

  int8_t  stat;
  ds >> stat;
  if (stat) {
    cleanup();
    QMessageBox::critical(0, "mount dictionary error",
			  "Canna mount dictionary error",
			  QMessageBox::Ok | QMessageBox::Default, 0);
  } else {
    qDebug("notice group name successful.");
  }

  getDictionaryList();
}


void
CannaEngine::getDictionaryList()
{
  DEBUG_TRACEFUNC();
  QByteArray  pack;
  QDataStream ds(pack, IO_ReadWrite);
  ds.setByteOrder(QDataStream::BigEndian);

  ds << (u_int8_t)RID_GET_DICTIONARY_LIST << (u_int8_t)0 << (u_int16_t)4 << _contxt << (u_int16_t)4096;
  _sock->writeBlock(pack.data(), pack.size());

  if ( !waitForReply(RID_GET_DICTIONARY_LIST) ) {
    slotDetectError();
    return;
  }

  recvGetDictionaryListReply();
}


void
CannaEngine::recvGetDictionaryListReply()
{
  DEBUG_TRACEFUNC();
  uint datalen = _sock->size();
  QByteArray data(datalen);
  if (_sock->readBlock(data.data(), datalen) < 0) {
    qFatal("Recv error.  %s:%d", __FILE__, __LINE__);
    return;
  }

  QDataStream ds(data, IO_ReadOnly);
  ds.setByteOrder(QDataStream::BigEndian);

  int16_t len, ndic;
  ds >> len >> ndic;

  if (len != (int)datalen - 2) {
    qWarning("Server response incorrect.  %s", __func__);
  }

  qDebug("number of dictionary: %d", ndic);
  QString dic;
  QStringList diclist;
  for (int i = 0; i < ndic; i++) {
    dic = QString();
    int8_t s;
    while (1) {
      ds >> s;
      if ( !s ) break;
      dic += s;
    }

    if (!dic.isEmpty())
      diclist << dic;
  }

  // Mounts dictionaries
  for (QStringList::Iterator it = diclist.begin(); it != diclist.end(); ++it) {
    if (*it != "pub") {    // Dosen't mount "pub" dictionary
      if ( !mountDictionary(*it) )
	break;
    }
  }
}


bool
CannaEngine::mountDictionary(QString dic)
{
  DEBUG_TRACEFUNC();

  QByteArray  pack;
  QDataStream ds(pack, IO_ReadWrite);
  ds.setByteOrder(QDataStream::BigEndian);

  qDebug("request mount dictionary name: %s", dic.data());
  ds << (u_int8_t)RID_MOUNT_DICTIONARY << (u_int8_t)0 << (u_int16_t)(dic.length()+7) << (int)0x0200 << _contxt;
  ds.writeRawBytes(dic.latin1(), dic.length() + 1);
  _sock->writeBlock(pack.data(), pack.size());

  if ( !waitForReply(RID_MOUNT_DICTIONARY) ) {
    slotDetectError();
    return FALSE;
  }

  return recvMountDictionaryReply();
}


bool
CannaEngine::recvMountDictionaryReply()
{
  DEBUG_TRACEFUNC();
  uint datalen = _sock->size();
  QByteArray data(datalen);
  if (_sock->readBlock(data.data(), datalen) < 0) {
    qFatal("Recv error.  %s:%d", __FILE__, __LINE__);
    return FALSE;
  }

  QDataStream ds(data, IO_ReadOnly);
  ds.setByteOrder(QDataStream::BigEndian);

  int16_t len;
  ds >> len;

  if (len != (int)datalen - 2) {
    qWarning("Server response incorrect.  %s", __func__);
    return FALSE;
  }

  int8_t  stat;
  ds >> stat;
  if (stat) {
    cleanup();
    QMessageBox::critical(0, "mount dictionary error",
			  "Canna mount dictionary error",
			  QMessageBox::Ok | QMessageBox::Default, 0);
    return FALSE;
  }
  qDebug("mount dictionary successful.");
  return TRUE;
}


bool
CannaEngine::beginConvert(const QString& hira, QStringList& kanji, QStringList& yomigana)
{
  DEBUG_TRACEFUNC();
  if ( _sock->state() != QSocket::Connected )
    return FALSE;

  if ( _convertflag ) {
    qWarning("%s:_convertflag incorrect", __func__);
    return FALSE;
  }

  QByteArray  pack;
  QDataStream ds(pack, IO_WriteOnly);
  ds.setByteOrder(QDataStream::BigEndian);

  qDebug("Yomigana size(QCString): %d", hira.local8Bit().length());
  //QByteArray str = eucToUint16(hira.local8Bit());
  QByteArray str = eucToUint16(QTextCodec::codecForTr()->fromUnicode(hira));
  qDebug("Yomigana size(uint16 array): %d", str.size());

  ds << (u_int8_t)RID_BEGIN_CONVERT << (u_int8_t)0 << (u_int16_t)(str.size()+6) << 0L << _contxt;
  ds.writeRawBytes(str.data(), str.size());
  _sock->writeBlock(pack.data(), pack.size());

  _convertflag = TRUE;  // start converting
  qDebug("beginConvert in progress ...");

  if ( !waitForReply(RID_BEGIN_CONVERT) ) {
    slotDetectError();
    return FALSE;
  }

  kanji = recvBeginConvertReply();
  yomigana.clear();
  for (int i = 0; i < (int)kanji.count(); ++i) {
    QString str = getYomi(i);
    if (str.isEmpty())
      return FALSE;
    yomigana << str;
  }

  Q_ASSERT(kanji.count() == yomigana.count());
  return TRUE;
}


QStringList
CannaEngine::recvBeginConvertReply()
{
  DEBUG_TRACEFUNC();

  uint datalen = _sock->size();
  QByteArray data(datalen);
  if (_sock->readBlock(data.data(), datalen) < 0) {
    qFatal("Recv error.  %s:%d", __FILE__, __LINE__);
    return QStringList();
  }

  QDataStream ds(data, IO_ReadOnly);
  ds.setByteOrder(QDataStream::BigEndian);

  int16_t len;
  ds >> len;

  if (len != (int)datalen - 2) {
    qWarning("beginConvert: server response incorrect. (%s)", __func__);
  }

  int16_t nbunsetu;
  ds >> nbunsetu;
  if (nbunsetu <= 0) {
    qDebug("beginConvert: convert error.  %s", __func__);
    return QStringList();
  }

  QStringList strlist;
  for (int i = 0; i < nbunsetu; i++) {
    QByteArray  ba;
    QDataStream dsba(ba, IO_WriteOnly);
    dsba.setByteOrder(QDataStream::BigEndian);
    u_int16_t us;

    ds >> us;
    while (us) {
      dsba << us;
      ds >> us;
    }

    QString str = (tr(uint16ToEuc(ba)));
    if (!str.isEmpty())
      strlist << str;
  }

  qDebug("beginConvert successful");
  qDebug("[%s]  %s", __func__, strlist.join(" ").local8Bit().data());
  return strlist;
}


void
CannaEngine::endConvert(const QStringList &kanji)
{
  DEBUG_TRACEFUNC();
  if ( _sock->state() != QSocket::Connected )
    return;

  if ( !_convertflag ) {
    qDebug("%s: unnecessary to end convert", __func__);
    return;
  }

  QByteArray  pack;
  QDataStream ds(pack, IO_WriteOnly);
  ds.setByteOrder(QDataStream::BigEndian);

  // 学習機能追加のため
  ds << (u_int8_t)RID_END_CONVERT << (u_int8_t)0 << (u_int16_t)(8+kanji.count()*2)/*データ長*/ << _contxt
     << (int16_t)kanji.count()/*文節数*/ << 1L/*モード*/;
  for (int i=0; i < (int)kanji.count(); i++) {
	QStringList cand;
	if (getCandidate(i, cand)) {
	  int n = cand.findIndex(kanji[i]);
	  ds << (u_int16_t)QMAX(n, 0);
	}
  }
  /*ds << (u_int8_t)RID_END_CONVERT << (u_int8_t)0 << (u_int16_t)8 << _contxt
     << (int16_t)0 << 0L;*/
  _sock->writeBlock(pack.data(), pack.size());

  _convertflag = FALSE;  // conversion done
  qDebug("%s: done", __func__);

  if ( !waitForReply(RID_END_CONVERT) ) {
    slotDetectError();
    return;
  }

  recvEndConvertReply();
}


void
CannaEngine::recvEndConvertReply() const
{
  DEBUG_TRACEFUNC();
  uint datalen = _sock->size();
  QByteArray data(datalen);
  if (_sock->readBlock(data.data(), datalen) < 0) {
    qFatal("Recv error.  %s:%d", __FILE__, __LINE__);
    return;
  }

  QDataStream ds(data, IO_ReadOnly);
  ds.setByteOrder(QDataStream::BigEndian);

  int16_t len;
  ds >> len;

  if (len != (int)datalen - 2) {
    qWarning("Server response incorrect.  %s", __func__);
    return;
  }

  int8_t res;
  ds >> res;
  if ( res ) {
    qFatal("End Convert error.  %s:%d", __FILE__, __LINE__);
    return;
  }

  qDebug("endConvert successful");
}


bool
CannaEngine::getCandidate(int idx, QStringList& candidate)
{
  DEBUG_TRACEFUNC("idx: %d", idx);
  if ( _sock->state() != QSocket::Connected )
    return FALSE;

  if ( !_convertflag )
    return FALSE;

  QByteArray  pack;
  QDataStream ds(pack, IO_WriteOnly);
  ds.setByteOrder(QDataStream::BigEndian);

  ds << (u_int8_t)RID_GET_CANDIDATE_LIST << (u_int8_t)0 << (u_int16_t)6 << _contxt
     << (int16_t)idx << (u_int16_t)4096;
  _sock->writeBlock(pack.data(), pack.size());

  if ( !waitForReply(RID_GET_CANDIDATE_LIST) ) {
    slotDetectError();
    return FALSE;
  }

  candidate = recvGetCandidateListReply();
  return TRUE;
}


QStringList
CannaEngine::recvGetCandidateListReply()
{
  DEBUG_TRACEFUNC();
  uint datalen = _sock->size();
  QByteArray data(datalen);
  if (_sock->readBlock(data.data(), datalen) < 0) {
    qFatal("Recv error.  %s:%d", __FILE__, __LINE__);
    return QStringList();
  }

  QDataStream ds(data, IO_ReadOnly);
  ds.setByteOrder(QDataStream::BigEndian);

  int16_t len;
  ds >> len;

  if (len != (int)datalen - 2) {
    qWarning("Server response incorrect.  %s", __func__);
    return QStringList();
  }

  int16_t ncand;
  ds >> ncand;
  if (ncand < 0) {
    qFatal("GetCandidateList error.  %s:%d", __FILE__, __LINE__);
    return QStringList();
  }
  qDebug("Number of Candidates: %d", ncand);

  if ( !ncand ) {
    return QStringList();
  }

  QStringList  listcand;
  for (int i = 0; i < ncand; i++) {
    QByteArray  ba;
    QDataStream dsba(ba, IO_WriteOnly);
    dsba.setByteOrder(QDataStream::BigEndian);
    u_int16_t us;

    ds >> us;
    while (us) {
      dsba << us;
      ds >> us;
    }

    QString str = tr(uint16ToEuc(ba));
    if ( !str.isEmpty() && listcand.find(str) == listcand.end()) {
      listcand << str;
    }
  }

  qDebug("[%s]  %s", __func__, listcand.join(" ").local8Bit().data());
  return listcand;
}


QString
CannaEngine::getYomi(int idx)
{
  DEBUG_TRACEFUNC();
  if ( !_convertflag )
    return QString::null;

  QByteArray  pack;
  QDataStream ds(pack, IO_WriteOnly);
  ds.setByteOrder(QDataStream::BigEndian);

  ds << (u_int8_t)RID_GET_YOMI << (u_int8_t)0 << (u_int16_t)6 << _contxt
     << (int16_t)idx << (u_int16_t)4096;
  _sock->writeBlock(pack.data(), pack.size());

  if ( !waitForReply(RID_GET_YOMI) ) {
    slotDetectError();
    return QString::null;
  }

  return recvGetYomiReply();
}


QString
CannaEngine::recvGetYomiReply()
{
  DEBUG_TRACEFUNC();
  uint datalen = _sock->size();
  QByteArray data(datalen);
  if (_sock->readBlock(data.data(), datalen) < 0) {
    qFatal("Recv error.  %s:%d", __FILE__, __LINE__);
    return QString::null;
  }

  QDataStream ds(data, IO_ReadOnly);
  ds.setByteOrder(QDataStream::BigEndian);

  int16_t len;
  ds >> len;

  if (len != (int)datalen - 2) {
    qWarning("Server response incorrect.  %s", __func__);
    return QString::null;
  }

  int16_t n;
  ds >> n;

  QByteArray  ba;
  QDataStream dsba(ba, IO_WriteOnly);
  dsba.setByteOrder(QDataStream::BigEndian);
  u_int16_t us;

  ds >> us;
  while (us) {
    dsba << us;
    ds >> us;
  }

  return tr(uint16ToEuc(ba));
}


bool
CannaEngine::resizeSegment(int idx, int len, QStringList& kanji, QStringList& yomigana)
{
  DEBUG_TRACEFUNC("idex: %d  len: %d", idx, len);
  if ( _sock->state() != QSocket::Connected )
    return FALSE;

  if ( !_convertflag ) {
    Q_ASSERT(0);
    return FALSE;
  }

  if (len < 0) {
    int length = getYomi(0).length();
    len = (len == LENGTHEN_1CHAR) ? length + 1 : length - 1;
  }

  QByteArray  pack;
  QDataStream ds(pack, IO_WriteOnly);
  ds.setByteOrder(QDataStream::BigEndian);

  ds << (u_int8_t)RID_RESIZE_PAUSE << (u_int8_t)0
     << (u_int16_t)6 << _contxt
     << (int16_t)idx << (u_int16_t)len;
  _sock->writeBlock(pack.data(), pack.size());

  if ( !waitForReply(RID_RESIZE_PAUSE) ) {
    slotDetectError();
    return FALSE;
  }

  kanji = recvResizePause();
  yomigana.clear();
  for (int i = 0; i < (int)kanji.count(); ++i) {
    QString str = getYomi(idx + i);
    if (str.isEmpty())
      return FALSE;
    yomigana << str;
  }

  Q_ASSERT(kanji.count() == yomigana.count());
  return TRUE;
}


QStringList
CannaEngine::recvResizePause()
{
  DEBUG_TRACEFUNC();
  uint datalen = _sock->size();
  QByteArray data(datalen);
  if (_sock->readBlock(data.data(), datalen) < 0) {
    qFatal("Recv error.  %s:%d", __FILE__, __LINE__);
    return QStringList();
  }

  QDataStream ds(data, IO_ReadOnly);
  ds.setByteOrder(QDataStream::BigEndian);

  int16_t len;
  ds >> len;

  if (len != (int)datalen - 2) {
    qWarning("Server response incorrect.  %s", __func__);
    return QStringList();
  }

  int16_t nbunsetu;
  ds >> nbunsetu;
  if (nbunsetu <= 0) {
    qDebug("Convert error.  %s", __func__);
    return QStringList();
  }

  qDebug("%s Number of Bunsetu: %d", __func__, nbunsetu);

  QStringList strlist;
  for (int i = 0; i < nbunsetu; i++) {
    QByteArray  ba;
    QDataStream dsba(ba, IO_WriteOnly);
    dsba.setByteOrder(QDataStream::BigEndian);
    u_int16_t us;

    ds >> us;
    while (us && !ds.atEnd()) {
      dsba << us;
      ds >> us;
    }

    QString str = tr(uint16ToEuc(ba));
    if ( !str.isEmpty() )
      strlist << str;
  }

  qDebug("[%s]  %s", __func__, strlist.join(" ").local8Bit().data());
  return strlist;
}


bool
CannaEngine::waitForReply(int msgid, int msecs)
{
  DEBUG_TRACEFUNC("msgid:%d msecs:%d", msgid, msecs);

  _sock->flush();
  bool timeout;
  Q_LONG size = _sock->waitForMore(msecs, &timeout);
  if (!size || timeout) {
    qDebug("Timeout!  size:%ld", size);
    return FALSE;
  }

  char buf[2];
  size = _sock->readBlock(buf, 2);
  if (size != 2) {
    qFatal("Recv error %ld  %s:%d", size, __FILE__, __LINE__);
    return FALSE;
  }
  if (buf[0] != msgid) {
    qWarning("Bad message ID : req:0x%x  recv:0x%x", (u_int8_t)msgid, (u_int8_t)buf[0]);
    return FALSE;
  }

  return TRUE;
}


// 返却値はネットワークバイトオーダ変換済なので，直ちに送信可能
// 末尾に (u_int16_t)0 が追加
QByteArray
CannaEngine::eucToUint16(const QCString& src)
{
  QByteArray dest;
  QDataStream dsdest(dest, IO_ReadWrite);
  dsdest.setByteOrder(QDataStream::BigEndian);

  QCString::ConstIterator it = src.begin();
  while (it != src.end()) {

    if (!(*it & 0x80)) {
      // ASCII
      dsdest << (u_int16_t)*it;

    } else if (it + 1 != src.end()) {
      switch ((u_int8_t)*it) {
      case 0x8e:   // 半角カナ
	dsdest << (u_int16_t)(0x0080 | (*(++it) & 0x7f));
	break;

      case 0x8f:   // 外字
	if (it + 2 == src.end())
	  return 0;

	dsdest << (u_int16_t)(0x8000 | ((*(++it) & 0x7f) << 8) | (*(++it) & 0x7f));
	break;

      default:  // 全角かな
	dsdest << (u_int16_t)(0x8080 | ((*it & 0x7f) << 8) | (*(++it) & 0x7f));
	break;
      }

    } else {
      // error
      return 0;
    }

    ++it;
  }

  return dest;
}


// 引数はネットワークバイトオーダの配列を指定
QCString
CannaEngine::uint16ToEuc(const QByteArray& src)
{
  QDataStream dssrc(src, IO_ReadOnly);
  dssrc.setByteOrder(QDataStream::BigEndian);
  QCString dst;
  QDataStream  dsdst(dst, IO_WriteOnly);
  dsdst.setByteOrder(QDataStream::BigEndian);

  u_int8_t b;
  QCString str;
  while (!dssrc.atEnd()) {
    u_int16_t us;
    dssrc >> us;

    switch (us & 0x8080) {
    case 0:  // ASCII
      dsdst << (u_int8_t)(us & 0x007f);
      break;

    case 0x0080:  // 半角カナ
      dsdst << (0x8e80 | (us & 0x007f));
      break;

    case 0x8000:  // 外字
      dsdst << (u_int8_t)0x8f;
      dsdst << us;
      break;

    case 0x8080:  // 全角かな
      b = (us & 0xff00) >> 8;
      if (b >= 0xa1 && b <= 0xa8 || b >= 0xb0 && b <= 0xf4) {  // EUC code
	dsdst << us;

      } else if (us >= 0xada1 && us <= 0xadb4) {
	str.sprintf("(%u)", us - 0xada0);
	dst += str;

      } else {
	qWarning("No such EUC code!!!!!  us:0x%x", us);
      }
      break;

    default:
      Q_ASSERT(0);
      break;
    }
  }

  dsdst << (u_int8_t)0;
  return dst;
}


void
CannaEngine::timerEvent(QTimerEvent* e)
{
  DEBUG_TRACEFUNC();
  if (_connectimer > 0 && e->timerId() == _connectimer) {
    killTimer(_connectimer);
    _connectimer = 0;

    if (_sock->state() != QSocket::Connected) {
      // Detects error
      cleanup();
      Config::writeEntry("_grpremote", FALSE);

      QMessageBox::warning(0, "Kanji Server Connection Error",
			   tr("Canna とのセッションを確立できません。\n"
			      "かな漢字変換を停止します。"),
			   QMessageBox::Ok | QMessageBox::Default, 0);
    }
  }
}


EXPORT_KANJIENGINE(CannaEngine)
