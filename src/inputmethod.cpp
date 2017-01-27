#include "inputmethod.h"
#include "kimeraglobal.h"
#include "ximethod.h"
#include "kanjiconvert.h"
#include "config.h"
#include "debug.h"
#include <qapplication.h>
#include <qdatastream.h>
#include <qevent.h>
#include <qmessagebox.h>
#include <qkeysequence.h>
#include <qtimer.h>
#include <ctype.h>
#include <string.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/keysym.h>
#include <X11/Xutil.h>

enum {
  XKeyPress   = KeyPress,
  XKeyRelease = KeyRelease
};

#undef KeyPress
#undef KeyRelease

void xEvent2XEvent(Display* dis, const void* src, unsigned int serial, XEvent& e);

const char *const SERVER_ATOM_STRING = "@server=kimera";


InputMethod::InputMethod() : _cltwinstack(QValueStack<Window>()),
			     _crnt_im(0),
			     _crnt_ic(0),
			     _server_atom(0),
			     _xim_xconnect(0),
			     _xim_protocol(0),
			     _xim_moredata(0),
			     _locales(0),
			     _transport(0),
			     _compound_text(0),
			     _kimera_atom(0),
			     _inputon(FALSE)
{
  QTimer::singleShot(0, this, SLOT(init()));

  _xim = new XIMethod();
  _buffer = new QBuffer();
  _buffer->open( IO_WriteOnly | IO_Append );

  // Create instance KanjiConvert class
  _kanjiconvt = new KanjiConvert();

  connect(_kanjiconvt, SIGNAL(decideSegments(const QString&)), this,  SLOT(slotDecideSegments(const QString&)));
}


InputMethod::~InputMethod()
{
  if ( _xim ) delete _xim;
  delete _kanjiconvt;
  _buffer->close();
  delete _buffer;
}


void 
InputMethod::init()
{
  DEBUG_TRACEFUNC();
  Q_CHECK_PTR(qt_xdisplay());
  qDebug("IMS Window ID: %lu", winId());

  // Create Atom
  _server_atom = XInternAtom(qt_xdisplay(), SERVER_ATOM_STRING, False);
  qDebug("@server=kimera: %lu", _server_atom);
  _xim_xconnect = XInternAtom(qt_xdisplay(), "_XIM_XCONNECT", False);
  qDebug("_XIM_XCONNECT: %lu", _xim_xconnect);
  _xim_protocol = XInternAtom(qt_xdisplay(), "_XIM_PROTOCOL", False);
  qDebug("_XIM_PROTOCOL: %lu", _xim_protocol);
  _xim_moredata = XInternAtom(qt_xdisplay(), "_XIM_MOREDATA", False);
  qDebug("_XIM_MOREDATA: %lu", _xim_moredata);
  _locales = XInternAtom(qt_xdisplay(), "LOCALES", False);
  qDebug("LOCALES: %lu", _locales);
  _transport = XInternAtom(qt_xdisplay(), "TRANSPORT", False);
  qDebug("TRANSPORT: %lu", _transport);
  _compound_text = XInternAtom(qt_xdisplay(), "COMPOUND_TEXT", False);
  qDebug("COMPOUND_TEXT: %lu", _compound_text);
  _kimera_atom = XInternAtom(qt_xdisplay(), "KIMERA_ATOM", False);
  qDebug("KIMERA_ATOM: %lu", _kimera_atom);
 
  // Set Selection XIM_SERVER Atom (@im=kimera)  
  XSetSelectionOwner(qt_xdisplay(), _server_atom, winId(), 0);
  Q_ASSERT(XGetSelectionOwner(qt_xdisplay(), _server_atom) == winId());
}


XIMStyle
InputMethod::currentXIMStyle() const
{
  return  _xim->getInputStyle(_crnt_im, _crnt_ic);
}


void
InputMethod::sendPreeditString(int caret, const QStringList& strlst, int attention_idx)
{ 
  DEBUG_TRACEFUNC();
  if (!_crnt_im || !_crnt_ic) {
    // Do nothing
    return;
  }
  
  QByteArray  feedback;
  QDataStream ds(feedback, IO_WriteOnly);
  ds.setByteOrder(sysByteOrder());
  for (int i = 0; i < (int)strlst.count(); ++i) {
    int feed = (i == attention_idx) ? FT_REVERSE : FT_UNDERLINE;
    for (int j = 0; j < (int)strlst[i].length(); ++j)
      ds << feed;
  }

  static int prevstrlen = 0;
  sendXIMPreeditDraw(_crnt_im, _crnt_ic, caret, 0, prevstrlen, strlst.join(""), feedback);
  prevstrlen = strlst.join("").length();
}


void
InputMethod::garbageCollectIM()
{
  DEBUG_TRACEFUNC();
  QValueList<u_int16_t> lst = _xim->getIMList();
  for (QValueListIterator<u_int16_t> it = lst.begin(); it != lst.end(); ++it) {
    if ( !checkCommWindow(*it) ) {
      _xim->removeIM(*it);
    }
  }
  _xim->disconnectIM();
}


void
InputMethod::recvXIMError()
{
  DEBUG_TRACEFUNC();
  QByteArray buf = _buffer->buffer();   // received data
  QDataStream dsbuf(buf, IO_ReadOnly);
  dsbuf.setByteOrder(sysByteOrder());
  
  char unused[4];
  u_int16_t im, ic, flag, errcode, errlen, errtype;
  dsbuf.readRawBytes((char*)unused, 4);   // header
  dsbuf >> im >> ic >> flag >> errcode >> errlen >> errtype;
  qWarning("XIM Error received. IM:%d IC:%d flag:%d ErrorCode:%d ErrorType:%d  %s:%d",
	   im, ic, flag, errcode, errtype, __FILE__, __LINE__);
  if (errlen > 0) {
    char errdetail[256] = { 0 };
    dsbuf.readRawBytes((char*)errdetail, QMIN(errlen, sizeof(errdetail) - 1));   // header
    qDebug("Error detail: %s", errdetail);
  }
}


void
InputMethod::recvXIMGetIMValues() const
{
  DEBUG_TRACEFUNC();
  QByteArray buf = _buffer->buffer();   // received data
  QDataStream dsbuf(buf, IO_ReadOnly);
  dsbuf.setByteOrder(sysByteOrder());
  
  char unused[4];
  u_int16_t im, n;
  dsbuf.readRawBytes((char*)unused, 4);   // header
  dsbuf >> im >> n;
  
  QByteArray listximattr;   // reply LISTofXICATTRIBUTE
  QDataStream dsxim(listximattr, IO_WriteOnly);
  dsxim.setByteOrder(sysByteOrder());
  
  u_int16_t id;
  dsbuf >> id;
  Q_ASSERT(n == 2 && id == 0);
  qDebug("Get IM values request IM:%d id:%d", im, id);  
  QByteArray ba(_xim->queryInputStyle());
  dsxim << id << (u_int16_t)ba.size();
  dsxim.writeRawBytes(ba.data(), ba.size());
  for (int j = 0; j < pad(ba.size()); j++) 
    dsxim << (u_int8_t)0;  

  QByteArray data;         // reply data
  QDataStream ds(data, IO_WriteOnly);
  ds.setByteOrder(sysByteOrder()); 

  ds << (u_int8_t)XIM_GET_IM_VALUES_REPLY << (u_int8_t)0 << numElements(4 + listximattr.size())
     << im << (u_int16_t)listximattr.size();
  ds.writeRawBytes(listximattr.data(), listximattr.size());

  sendClientMessage(im, data);
  qDebug("XSendEvent XIM_GET_IM_VALUES_REPLY");
}


void
InputMethod::recvXIMSetICFocus()
{
  DEBUG_TRACEFUNC();
  QByteArray buf = _buffer->buffer();   // received data
  QDataStream dsbuf(buf, IO_ReadOnly);
  dsbuf.setByteOrder(sysByteOrder());
  
  char unused[4];
  dsbuf.readRawBytes((char*)unused, 4);   // header
  dsbuf >> _crnt_im >> _crnt_ic;
  qDebug("SetICFocus IM: %d IC: %d", _crnt_im, _crnt_ic);
  
  // Set font of preedit
  QString ftnamelist = _xim->fontPreedit(_crnt_im, _crnt_ic);
  QString ftname( ftnamelist.section(',', 0, 0) );
  
  // set font size
  QFont   ft;       // Default font
  QString strsize( ftname.section('-', 7, 7) );
  if (!strsize.isEmpty() && strsize != '*') {
    int size = strsize.toInt();
    if (size > 0) {
      ft.setPixelSize( size );
      //ft.setPointSize( size );
    }
  }

  qDebug("set font: %s", ft.rawName().data());
  qDebug("set font pixel size:%d  point size:%d", ft.pixelSize(), ft.pointSize());  
  _kanjiconvt->setFont(ft);
  
  if ( _inputon ) {
    setXIMInputtingEnabled( _inputon );  // Re-send XIM_SET_EVENT_MASK
    slotDecideSegments("");     // Sends dummy string for data flush
  }
}


void
InputMethod::recvXIMUnsetICFocus()
{
  DEBUG_TRACEFUNC();
  QByteArray buf = _buffer->buffer();   // received data
  QDataStream dsbuf(buf, IO_ReadOnly);
  dsbuf.setByteOrder(sysByteOrder());
  
  char unused[4];
  u_int16_t  im, ic;
  dsbuf.readRawBytes((char*)unused, 4);   // header
  dsbuf >> im >> ic;
  qDebug("UnsetICFocus IM: %d IC: %d", im, ic);
  if (_crnt_im > 0 || _crnt_ic > 0) {
    if (im == _crnt_im && ic == _crnt_ic) {
      // Clear characters of preedit
      _crnt_im = _crnt_ic = 0;  // Bug fix, not to send XIM_PREEDIT_DRAW
      _kanjiconvt->clear();
      _crnt_im = im;
      _crnt_ic = ic;

      // Garbage collection IM
      garbageCollectIM();
    }
  }
}


void
InputMethod::recvXIMSetICValues() const
{
  DEBUG_TRACEFUNC();
  QByteArray buf = _buffer->buffer();   // received data
  QDataStream dsbuf(buf, IO_ReadOnly);
  dsbuf.setByteOrder(sysByteOrder());
  
  char unused[4];
  u_int16_t  im, ic, n;
  dsbuf.readRawBytes((char*)unused, 4);   // header
  dsbuf >> im >> ic >> n;
  dsbuf.readRawBytes((char*)unused, 2);  // unused

  qDebug("XIMSetICValues Request IM:%d IC:%d data size:%d", im, ic, buf.size());
  
  uint pos = dsbuf.device()->at();
  QByteArray xicattr;
  while ( !dsbuf.atEnd() && dsbuf.device()->at() < pos + n) {
    u_int16_t id, len;
    dsbuf >> id >> len;
    xicattr.resize(len);
    dsbuf.readRawBytes(xicattr.data(), len);
    dsbuf.readRawBytes((char*)unused, pad(len));         // unused
    _xim->setICValue(im, ic, id, xicattr);   // set IC value
  }

  QByteArray data;         // reply data
  QDataStream ds(data, IO_WriteOnly);
  ds.setByteOrder(sysByteOrder());  
  ds << (u_int8_t)XIM_SET_IC_VALUES_REPLY << (u_int8_t)0 << numElements( 4 )
     << im << ic;
  
  sendClientMessage(im, data);
  qDebug("XSendEvent XIM_SET_IC_VALUES_REPLY: IM:%d IC:%d", im, ic);

  if (im == _crnt_im && ic == _crnt_ic) {
    bool ok;
    QPoint pos = calcPreeditPoint(im, ic, ok);
    if ( ok ) {
      _kanjiconvt->setPreeditPoint(pos);
    } else {
      // Disconnect IM
      _xim->removeIM(im);
    }
  }
}


void 
InputMethod::recvXIMGetICValues() const
{
  DEBUG_TRACEFUNC();
  QByteArray buf = _buffer->buffer();   // received data
  QDataStream dsbuf(buf, IO_ReadOnly);
  dsbuf.setByteOrder(sysByteOrder());
 
  char unused[4];
  u_int16_t im, ic, n;
  dsbuf.readRawBytes((char*)unused, 4);   // header
  dsbuf >> im >> ic >> n;
  qDebug("XIMGetICValues Request IM:%d IC:%d", im, ic);

  QByteArray listxicattr;  // reply LISTofXICATTRIBUTE
  QDataStream dsxic(listxicattr, IO_WriteOnly);
  dsxic.setByteOrder(sysByteOrder()); 

  for (int i = 0; i < n/2; i++) {
    u_int16_t id;
    dsbuf >> id;
    QByteArray value = _xim->getICValue(im, ic, id);
    dsxic << id << (u_int16_t)value.size();
    dsxic.writeRawBytes(value.data(), value.size());
    for (int j = 0; j < pad(value.size()); ++j)
      dsxic << (u_int8_t)0; // unused
  }
  
  QByteArray data;         // reply data
  QDataStream ds(data, IO_WriteOnly);
  ds.setByteOrder(sysByteOrder());

  ds << (u_int8_t)XIM_GET_IC_VALUES_REPLY << (u_int8_t)0 << numElements(8 + listxicattr.size())
     << im << ic << (u_int16_t)listxicattr.size()
     << (u_int16_t)0;       // unused
  ds.writeRawBytes(listxicattr.data(), listxicattr.size());
  
  sendClientMessage(im, data);
  qDebug("XSendEvent XIM_GET_IC_VALUES_REPLY");
}


void 
InputMethod::recvXIMCreateIC() const
{
  DEBUG_TRACEFUNC();
  char unused[4];
  u_int16_t  im, n;
  QByteArray buf = _buffer->buffer();
  QDataStream dsbuf(buf, IO_ReadOnly);
  dsbuf.setByteOrder(sysByteOrder());
  dsbuf.readRawBytes((char*)unused, 4);   // header
  dsbuf >> im >> n;

  if ( !_xim->exists(im) ) {
    qWarning("no such IM: %d", im);
    sendXIMError(im, 0, XIM_FLAG_VALID_IM, XIM_BAD_SOMETHING);
    return;
  }

  u_int16_t ic = _xim->createXIC(im);   // create IC
  qDebug("IM:%d  Create IC id:%d data size:%d", im, ic, buf.size());

  QByteArray listofxic(n);
  QDataStream dslistxic(listofxic, IO_ReadOnly);
  dslistxic.setByteOrder(sysByteOrder());
  dsbuf.readRawBytes(listofxic.data(), n);

  while ( !dslistxic.atEnd() ) {
    u_int16_t id, len;
    dslistxic >> id >> len;
    QByteArray ba(len);
    dslistxic.readRawBytes(ba.data(), len);
    dslistxic.readRawBytes((char*)unused, pad(len));  // unused
    _xim->setICValue(im, ic, id, ba);   // set IC value
  }

  QByteArray data;         // reply data
  QDataStream ds(data, IO_WriteOnly);
  ds.setByteOrder(sysByteOrder());

  ds << (u_int8_t)XIM_CREATE_IC_REPLY << (u_int8_t)0 << numElements(4)
     << im << ic;

  sendClientMessage(im, data);
  qDebug("XSendEvent XIM_CREATE_IC_REPLY");
}


void 
InputMethod::recvXIMDestroyIC()
{
  DEBUG_TRACEFUNC();
  char unused[4];
  QByteArray buf = _buffer->buffer();
  QDataStream dsbuf(buf, IO_ReadOnly);
  dsbuf.setByteOrder(sysByteOrder());
  u_int16_t im, ic;
  dsbuf.readRawBytes((char*)unused, 4);   // header
  dsbuf >> im >> ic;

  qDebug("IM:%d  Destory IC id:%d", im, ic);
  _xim->removeXIC(im, ic);

  QByteArray data;         // reply data
  QDataStream ds(data, IO_WriteOnly);
  ds.setByteOrder(sysByteOrder());
  
  ds << (u_int8_t)XIM_DESTROY_IC_REPLY << (u_int8_t)0 << numElements(4)
     << im << ic;
  
  sendClientMessage(im, data);
  qDebug("XSendEvent XIM_DESTROY_IC_REPLY");
}


void
InputMethod::recvXIMSync() const
{
  DEBUG_TRACEFUNC();
  char unused[4];
  QByteArray buf = _buffer->buffer();
  QDataStream dsbuf(buf, IO_ReadOnly);
  dsbuf.setByteOrder(sysByteOrder());

  u_int16_t im, ic;
  dsbuf.readRawBytes((char*)unused, 4);   // header
  dsbuf >> im >> ic;
  qDebug("Synchronuze IM:%d  IC:%d", im, ic);

  QByteArray data;         // reply data
  QDataStream ds(data, IO_WriteOnly);
  ds.setByteOrder(sysByteOrder());

  ds << (u_int8_t)XIM_SYNC_REPLY << (u_int8_t)0 << numElements(4)
     << im << ic;

  sendClientMessage(im, data);
  qDebug("XSendEvent XIM_SYNC_REPLY");
}


void 
InputMethod::recvXIMSyncReply() const
{
  DEBUG_TRACEFUNC();
  char unused[4];
  QByteArray buf = _buffer->buffer();
  QDataStream dsbuf(buf, IO_ReadOnly);
  dsbuf.setByteOrder(sysByteOrder());
  u_int16_t im, ic;
  dsbuf.readRawBytes((char*)unused, 4);   // header
  dsbuf >> im >> ic;
  qDebug("Receive XIM_SYNC_REPLY");
}


void 
InputMethod::recvXIMTriggerNotify()
{
  DEBUG_TRACEFUNC();
  char unused[4];
  ulong  flag, index, mask;
  QByteArray buf = _buffer->buffer();
  QDataStream dsbuf(buf, IO_ReadOnly);
  dsbuf.setByteOrder(sysByteOrder());
  u_int16_t im, ic;
  dsbuf.readRawBytes((char*)unused, 4);   // header
  dsbuf >> im >> ic >> flag >> index >> mask;
  qDebug("XIMTriggerNotify Request IM: %d IC:%d flag:%lu mask:%lu", im, ic, flag, mask);

  if (_crnt_im > 0 && im != _crnt_im || _crnt_ic > 0 && ic != _crnt_ic) {
    qWarning("Bad im or ic  im:%d crnt_im:%d ic:%d crnt_ic:%d", im, _crnt_im, ic, _crnt_ic);
    //sendXIMError(im, ic, XIM_FLAG_VALID_IM_IC, XIM_BAD_SOMETHING);
    //return;
  }

  // Sets current IM and IC
  _crnt_im = im;
  _crnt_ic = ic;

  // Sends XIM_TRIGGER_NOTIFY_REPLY
  QByteArray data;         // reply data
  QDataStream ds(data, IO_WriteOnly);
  ds.setByteOrder(sysByteOrder());
  
  ds << (u_int8_t)XIM_TRIGGER_NOTIFY_REPLY << (u_int8_t)0 << numElements(4)
     << _crnt_im << _crnt_ic;
  
  sendClientMessage(_crnt_im, data);
  qDebug("XSendEvent XIM_TRIGGER_NOTIFY_REPLY");

  emit triggerNotify( !_inputon );  // trigger signal  
  _kanjiconvt->clear();   // preedit clear, after sendClientMessage
}


void 
InputMethod::recvXIMEncodingNegotiation() const
{
  DEBUG_TRACEFUNC();
  char unused[4];
  u_int16_t  im, n;
  QByteArray buf = _buffer->buffer();
  QDataStream dsbuf(buf, IO_ReadOnly);
  dsbuf.setByteOrder(sysByteOrder());
  
  dsbuf.readRawBytes((char*)unused, 4);   // header
  dsbuf >> im >> n;
  qDebug("XIMEncodingNegotiation Request IM: %d", im);
  qDebug("byte length of encoding list: %d", n);

  int index = 0;
  QCString encode;
  while ( !dsbuf.atEnd() ) {
    u_int8_t len;
    dsbuf >> len;
    QCString str(len + 1);
    dsbuf.readRawBytes(str.data(), len);
    qDebug("client supported encoding:%s", str.data());
   
    if ((encode = str) == QCString("COMPOUND_TEXT")) break;
    index++;
  }
  Q_ASSERT(encode == QCString("COMPOUND_TEXT"));

  QByteArray data;         // reply data
  QDataStream ds(data, IO_WriteOnly);
  ds.setByteOrder(sysByteOrder());

  ds << (u_int8_t)XIM_ENCODING_NEGOTIATION_REPLY << (u_int8_t)0 << numElements(8)
     << im << (u_int16_t)0 << (int16_t)index << (u_int16_t)0;
  
  sendClientMessage(im, data);
  qDebug("XSendEvent XIM_ENCODING_NEGOTIATION_REPLY");
}


void
InputMethod::recvXIMQureyExtension()
{
  DEBUG_TRACEFUNC();
  char unused[4];
  u_int16_t  im, n;
  QByteArray buf = _buffer->buffer();
  QDataStream dsbuf(buf, IO_ReadOnly);
  dsbuf.setByteOrder(sysByteOrder());

  dsbuf.readRawBytes((char*)unused, 4);   // header
  dsbuf >> im >> n;
  qDebug("XIMQureyExtension Request IM:%d len:%d", im, n);
 
  if ( n ) {
    uint pos = dsbuf.device()->at();
    while ( !dsbuf.atEnd() &&  dsbuf.device()->at() < pos + n) {
      u_int8_t len;
      dsbuf >> len;
      QCString str(len + 1);
      dsbuf.readRawBytes(str.data(), len);
      qDebug("extensions supported(IM lib):%s", str.data());
    }
  }
  
  QByteArray data;
  QDataStream ds(data, IO_WriteOnly);
  ds.setByteOrder(sysByteOrder());

  ds << (u_int8_t)XIM_QUERY_EXTENSION_REPLY << (u_int8_t)0
     << (u_int16_t)1
     << im << (u_int16_t)0;
  sendClientMessage(im, data);
  qDebug("XSendEvent XIM_QUERY_EXTENSION_REPLY");
}


void 
InputMethod::recvSelectionRequest(const XEvent& e) const
{
  DEBUG_TRACEFUNC();
  if (e.xselectionrequest.property != _locales && 
      e.xselectionrequest.property != _transport) return;
  
  XSelectionEvent se;
  se.type = SelectionNotify;
  se.requestor = e.xselectionrequest.requestor;
  se.selection = e.xselectionrequest.selection;
  se.target = e.xselectionrequest.target;
  se.time = e.xselectionrequest.time;
  se.property = e.xselectionrequest.property;
  qDebug("selection: %lu", se.selection);
  qDebug("target: %lu", e.xselectionrequest.target);
  qDebug("property: %lu", se.property);
  
  if (se.property == _locales) {
    qDebug("Convert locales");
    char strlocales[] = "@locale=ja_JP.eucJP,ja_JP,japanese,japan,ja";  // ja_JP.SJIS ...
    XChangeProperty(qt_xdisplay(), se.requestor, se.property, se.target, 8,
		    PropModeAppend, (u_int8_t*)strlocales, 
		    strlen(strlocales));
    XSendEvent(qt_xdisplay(), se.requestor, False, 0, (XEvent*)&se);

  } else if (se.property == _transport ){
    qDebug("Convert transport");
    char strtransport[] = "@transport=X/";
    XChangeProperty(qt_xdisplay(), se.requestor, se.property, se.target, 8,
		    PropModeAppend, (u_int8_t*)strtransport, 
		    strlen(strtransport));
    XSendEvent(qt_xdisplay(), se.requestor, False, 0, (XEvent*)&se);
  }

  XFlush(qt_xdisplay());
}


void
InputMethod::recvSelectionClear(const XEvent& e)
{
  DEBUG_TRACEFUNC();
  if (e.xselectionclear.window == winId()
      && e.xselectionclear.selection == _server_atom) {
    Window newowner = XGetSelectionOwner(qt_xdisplay(), _server_atom);
    QMessageBox::critical(0, QString("Critical Message"),
			  QString("Received Selection Clear event.\n"
				  "Kimera stops the service.\n\n"
				  "Window: %1  Atom: %2\n"
				  "New Selection Owner: %3").arg(e.xselectionclear.window).arg(e.xselectionclear.selection).arg(newowner),
			  QMessageBox::Ok | QMessageBox::Default, 0);
  }
}


void 
InputMethod::recvXIMOpen(const XEvent& e)
{
  DEBUG_TRACEFUNC();
  qDebug("length(%d): %s", e.xclient.data.b[4], e.xclient.data.b + 5);
  if (!e.xclient.data.b[4])
    qWarning("Bad local name!");

  QByteArray listofximattr;   // LISTofXIMATTR
  QDataStream dsxim(listofximattr, IO_WriteOnly);
  dsxim.setByteOrder(sysByteOrder());
  
  QValueList<SupportAttr> imattr = XIMethod::getSuppIMAttr();
  Q_ASSERT(imattr.count() != 0);
 
  for (QValueList<SupportAttr>::const_iterator it = imattr.begin(); it != imattr.end(); ++it) {
    dsxim << (u_int16_t)(*it).id()
	  << (u_int16_t)(*it).type()
	  << (u_int16_t)(*it).attribute().length(); 
    dsxim.writeRawBytes((*it).attribute().data(), (*it).attribute().length());
    for (int i = 0; i < pad(2 + (*it).attribute().length()); i++) {
      dsxim << (u_int8_t)0;   // unused
    }
  } 
  
  QByteArray listofxicattr;   // LISTofXICATTR
  QDataStream dsxic(listofxicattr, IO_WriteOnly);
  dsxic.setByteOrder(sysByteOrder());
  
  QValueList<SupportAttr> icattr = XIMethod::getSuppICAttr();
  Q_ASSERT(icattr.count() != 0);

  for (QValueList<SupportAttr>::const_iterator it = icattr.begin(); it != icattr.end(); ++it) {
    dsxic << (u_int16_t)(*it).id()
	  << (u_int16_t)(*it).type()
	  << (u_int16_t)(*it).attribute().length();
    dsxic.writeRawBytes((*it).attribute().data(), (*it).attribute().length());
    for (int i = 0; i < pad(2 + (*it).attribute().length()); i++) {
      dsxic << (u_int8_t)0;   // unused
    }
  }

  QByteArray data;
  QDataStream ds(data, IO_WriteOnly);
  ds.setByteOrder(sysByteOrder());
  ds << (u_int8_t)XIM_OPEN_REPLY << (u_int8_t)0  // set opcode
     << (u_int16_t)numElements(8 + listofximattr.size() + 
			       listofxicattr.size()); // length

  if ( !_cltwinstack.count() ) {
    qFatal("no client window  %s:%d", __FILE__, __LINE__);    
    return;
  }

  u_int16_t im = _xim->createIM(_cltwinstack.top());
  ds << im << (u_int16_t)listofximattr.size();
  ds.writeRawBytes(listofximattr.data(), listofximattr.size());
  ds << (u_int16_t)listofxicattr.size()
     << (u_int16_t)0;   // unused
  
  ds.writeRawBytes(listofxicattr.data(), listofxicattr.size());
  sendXIMRegisterTriggerkeys(im); // XIM_REGISTER_TRIGGERKEY
  sendClientMessage(im, data);
  qDebug("XSendEvent XIM_OPEN_REPLY");
}


void
InputMethod::recvXIMClose()
{
  DEBUG_TRACEFUNC();
  char unused[4];
  QByteArray buf = _buffer->buffer();
  QDataStream dsbuf(buf, IO_ReadOnly);
  dsbuf.setByteOrder(sysByteOrder());
  u_int16_t im;
  dsbuf.readRawBytes((char*)unused, 4);   // header
  dsbuf >> im;
  qDebug("Close request input-method-ID:%d", im);
  
  QByteArray data;
  QDataStream ds(data, IO_WriteOnly);
  ds.setByteOrder(sysByteOrder());
  ds << (u_int8_t)XIM_CLOSE_REPLY << (u_int8_t)0 << (u_int16_t)numElements( 4 )
     << (u_int16_t)im << (u_int16_t)0;   // unused
  
  sendClientMessage(im, data);
  qDebug("XSendEvent XIM_CLOSE_REPLY");
  _xim->removeIM(im);
}


void 
InputMethod::recvXIMConnect(const XEvent& e)
{
  DEBUG_TRACEFUNC();
 
  // Check byte order and connect to XIM
  Window client = _cltwinstack.top();
  if (e.xclient.data.b[4] == 'B' && sysByteOrder() == QDataStream::BigEndian) {
    qDebug("byte order: BigEndian");
    _xim->connectIM(client);
    
  } else if (e.xclient.data.b[4] == 'l' && sysByteOrder() == QDataStream::LittleEndian) {
    qDebug("byte order: LittleEndian");
    _xim->connectIM(client);

  } else {
    qFatal("byte order: format incorrect");
    sendXIMError(0, 0, XIM_FLAG_INVALID_IM_IC, XIM_BAD_PROTOCOL);
    return;
  }

  qDebug("client-major-protocol-version: %d", e.xclient.data.s[3]);
  qDebug("client-minor-protocol-version: %d", e.xclient.data.s[4]);
  qDebug("number of client-auth-protocol-names: %d", e.xclient.data.s[5]);
 
  Q_ASSERT(e.xclient.data.s[3] == 1);  // check protocol version
  Q_ASSERT(e.xclient.data.s[4] == 0);  // check protocol version
  Q_ASSERT( !e.xclient.data.s[5] );    // unsupported authorization

  XClientMessageEvent cm;
  memset(cm.data.b, 0, 20);
  cm.data.b[0] = (u_int8_t)XIM_CONNECT_REPLY;
  cm.data.s[1] = 1;  // length
  cm.data.s[2] = 1;  // set protocol version

  sendClientMessageProtocol(client, cm);
  qDebug("XSendEvent XIM_CONNECT_REPLY");
}


void
InputMethod::recvXIMDisconnect(const XEvent&)
{
  DEBUG_TRACEFUNC();
  QValueList<Window> cltlst = _xim->disconnectIM();
  if (cltlst.count() > 0) {
    for (QValueListIterator<Window> it = cltlst.begin(); it != cltlst.end(); ++it) {
      XClientMessageEvent cm;
      memset(cm.data.b, 0, 20);
      cm.data.b[0] = (u_int8_t)XIM_DISCONNECT_REPLY;
      sendClientMessageProtocol(*it, cm);
      qDebug("XSendEvent XIM_DISCONNECT_REPLY");
    }
  } else {
    qFatal("Bad parameter  %s:%d", __FILE__, __LINE__);
  }
}


void 
InputMethod::recvProperty(const XEvent& e)
{
  DEBUG_TRACEFUNC();
  Q_ASSERT(_buffer->size() == 0);

  Atom  type;
  int   format;
  ulong nitems;
  ulong bytes_after;
  u_int8_t* value = 0;

  // Get the Atom contained in XIM_SERVERS Property
  int res = XGetWindowProperty(qt_xdisplay(), winId(), e.xclient.data.l[1],
                               0, e.xclient.data.l[0], True, XA_STRING, 
			       &type, &format, &nitems, &bytes_after, &value); 

  if (res == Success && (int)nitems == e.xclient.data.l[0] && bytes_after == 0 && value) {
    // Write the receved data to buffer
    Q_ASSERT(_buffer->isOpen());
    _buffer->writeBlock((char*)value, e.xclient.data.l[0]); 
  } else {
    qWarning("Bad response  XGetWindowProperty");
    Q_ASSERT(res == Success);
    Q_ASSERT((int)nitems == e.xclient.data.l[0]);
    Q_ASSERT(bytes_after == 0);
  }

  if ( value )
    XFree((char*)value);
}


void 
InputMethod::recvXIMForwardEvent() const
{
  DEBUG_TRACEFUNC();
  QByteArray buf = _buffer->buffer();   // received data
  QDataStream dsbuf(buf, IO_ReadOnly);
  dsbuf.setByteOrder(sysByteOrder());

  char  unused[4];
  u_int16_t im, ic, flag, serial;
  dsbuf.readRawBytes((char*)unused, 4);   // header
  dsbuf >> im >> ic >> flag >> serial;
  qDebug("Forward event im:%d ic:%d, flag:%d serial:%d", im, ic, flag, serial);

  XEvent event;
  xEvent2XEvent(qt_xdisplay(), (const char*)buf.data() + 12, serial, event);
  event.xkey.window = winId();     // target for this widget
  qApp->x11ProcessEvent(&event);   // To translate button event
                                   //  see keyPressEvent function
}


void 
InputMethod::keyPressEvent(QKeyEvent* e)
{
  DEBUG_TRACEFUNC();

  bool ok = TRUE;
  QPoint pos = calcPreeditPoint(_crnt_im, _crnt_ic, ok); 
  if ( ok ) {
    // Set preedit point
    _kanjiconvt->setPreeditPoint(pos);

    if ( !_inputon || _kanjiconvt->processKeyEvent(*e) == FALSE ) {
      // XIM FORWARD EVENT
      qDebug("Send XIM_FORWARD_EVENT because of no function for the key");
      Q_ASSERT(_buffer->buffer().size() > 0);
      sendClientMessage(_crnt_im, _buffer->buffer());
    }
  } else {
    // Remove IM
    _xim->removeIM(_crnt_im);
    _crnt_im = _crnt_ic = 0;
  }
}


void 
InputMethod::recvXIMResetIC()
{
  DEBUG_TRACEFUNC();
  char unused[4];
  QByteArray buf = _buffer->buffer();
  QDataStream dsbuf(buf, IO_ReadOnly);
  dsbuf.setByteOrder(sysByteOrder());
  u_int16_t im, ic;
  dsbuf.readRawBytes((char*)unused, 4);   // header
  dsbuf >> im >> ic;
  qDebug("XIMResetIC request IM:%d IC:%d", im, ic);
  
  QByteArray data;
  QDataStream ds(data, IO_WriteOnly);
  ds.setByteOrder(sysByteOrder());
  ds << (u_int8_t)XIM_RESET_IC_REPLY << (u_int8_t)0 << (u_int16_t)numElements(4)
     << im << ic << (u_int16_t)0;
  for (int i = 0; i < pad(2); i++) {
    ds << (u_int8_t)0;   // unused
  }
  
  sendClientMessage(im, data);
  qDebug("XSendEvent XIM_RESET_IC_REPLAY");
  
  // preedit clear
  _crnt_im = im;
  _crnt_ic = ic;
  _kanjiconvt->clear();
}


void
InputMethod::slotDecideSegments(const QString& str)
{
  DEBUG_TRACEFUNC();

  if (!_crnt_im || !_crnt_im) {
    qWarning("Bad param  im:%d ic:%d", _crnt_im, _crnt_ic);
    return;
  }
  if (!isXIMInputtingEnabled()) {
    qDebug("XIM Inputting Disabled");
    return;
  }

  uint len = str.local8Bit().length();
  qDebug("Decide segments:%s (%d)", str.local8Bit().data(), str.length());

  char* pstr = (char*)alloca(len + 1);
  memcpy(pstr, str.local8Bit().data(), len + 1);
  
  XTextProperty tp;
  memset(&tp, 0, sizeof(tp));
  int res = XmbTextListToTextProperty(qt_xdisplay(), &pstr, 1, XCompoundTextStyle, &tp);
  Q_ASSERT(res == Success);
  
  QByteArray data;         // send data
  QDataStream ds(data, IO_WriteOnly);
  ds.setByteOrder(sysByteOrder());
  ds << (u_int8_t)XIM_COMMIT << (u_int8_t)0 << numElements(8 + tp.nitems)
     << _crnt_im << _crnt_ic << (u_int16_t)0x0003
     << (u_int16_t)tp.nitems;
  
  if ( tp.value ) {
    ds.writeRawBytes((char*)tp.value, tp.nitems);
    XFree(tp.value);
  }

  for (int i = 0; i < pad(tp.nitems); i++) {
    ds << (u_int8_t)0;  // unused
  }
  
  sendClientMessage(_crnt_im, data);
  qDebug("IM:%d IC:%d XSendEvent XIM_COMMIT", _crnt_im, _crnt_ic);
}


void
InputMethod::sendXIMSetEventMask(u_int16_t im, u_int16_t ic, ulong fwrd, ulong sync) const
{
  DEBUG_TRACEFUNC();
  QByteArray data;         // send data
  QDataStream ds(data, IO_WriteOnly);
  ds.setByteOrder(sysByteOrder());
  
  ds << (u_int8_t)XIM_SET_EVENT_MASK << (u_int8_t)0 << numElements(12)
     << im << ic << fwrd << sync;

  sendClientMessage(im, data);
  qDebug("XSendEvent XIM_SET_EVENT_MASK");
}


void 
InputMethod::sendXIMRegisterTriggerkeys(u_int16_t im) const
{
  DEBUG_TRACEFUNC();
  QByteArray listofkey;
  QDataStream dskey(listofkey, IO_WriteOnly);
  dskey.setByteOrder(sysByteOrder()); 
  QString s;
  
  // Set default starting key.
  if ((s = Config::readEntry("_cmbstartkey", "Kanji")) == "Kanji") {
    dskey << (uint)XK_Kanji << 0L << 0L;
    
  } else if (s == "Zenkaku_Hankaku") {
    dskey << (uint)XK_Zenkaku_Hankaku << 0L << 0L;

  } else {
    QKeySequence ks = QKeySequence( s );
    int c = ks[0] & 0xff;

    switch (ks[0] & Qt::MODIFIER_MASK) {
    case Qt::SHIFT:
      dskey << c << (uint)ShiftMask << (uint)ShiftMask;
      break;
      
    case Qt::CTRL:
      dskey << c << (uint)ControlMask << (uint)ControlMask;
      if (tolower(c) && tolower(c) != c)
        dskey << (uint)tolower(c) << (uint)ControlMask << (uint)ControlMask;
      break;
      
    case Qt::ALT:
      dskey << c << (uint)Mod1Mask << (uint)Mod1Mask;
      if (tolower(c) && tolower(c) != c) 
        dskey << (uint)tolower(c) << (uint)Mod1Mask << (uint)Mod1Mask;
      break;
      
    default:
      dskey << c << 0L << 0L;
      if (tolower(c) && tolower(c) != c) 
        dskey << (uint)tolower(c) << 0L << 0L;
      break;
    }
  }

  QByteArray data;
  QDataStream ds(data, IO_WriteOnly);
  ds.setByteOrder(sysByteOrder());
  
  ds << (u_int8_t)XIM_REGISTER_TRIGGERKEYS << (u_int8_t)0 << numElements(12 + 2 * listofkey.size())
     << im << (u_int16_t)0 << (ulong)listofkey.size();
  ds.writeRawBytes(listofkey.data(), listofkey.size()); // on-keys
  ds << (ulong)listofkey.size();
  ds.writeRawBytes(listofkey.data(), listofkey.size()); // off-keys

  sendClientMessage(im, data);
  qDebug("XSendEvent XIM_REGISTER_TRIGGERKEYS");
}


void
InputMethod::sendXIMSync(u_int16_t im, u_int16_t ic) const
{
  DEBUG_TRACEFUNC("im:%d ic:%d", im, ic);
  QByteArray data;
  QDataStream ds(data, IO_WriteOnly);
  ds.setByteOrder(sysByteOrder());

  ds << (u_int8_t)XIM_SYNC << (u_int8_t)0 << numElements(4)
     << im << ic; 

  sendClientMessage(im, data);
  qDebug("XSendEvent XIM_SYNC");
}


void
InputMethod::sendXIMError(u_int16_t im, u_int16_t ic, XIMErrorFlag flag, XIMErrorCode error) const
{
  DEBUG_TRACEFUNC("im:%d ic:%d flag:%d error:%d", im, ic, flag, error);
  QByteArray data;
  QDataStream ds(data, IO_WriteOnly);
  ds.setByteOrder(sysByteOrder());

  ds << (u_int8_t)XIM_ERROR << (u_int8_t)0 << numElements(12) 
     << im << ic << (u_int16_t)flag << (u_int16_t)error 
     << (u_int16_t)0 << (u_int16_t)0;

  sendClientMessage(im, data);
  qDebug("XSendEvent XIM_ERROR");
}


void
InputMethod::sendXIMError(Window client, XIMErrorCode error) const
{
  DEBUG_TRACEFUNC("client:%ld error:%d", client, error);
  QByteArray data;
  QDataStream ds(data, IO_WriteOnly);
  ds.setByteOrder(sysByteOrder());

  ds << (u_int8_t)XIM_ERROR << (u_int8_t)0 << numElements(12) 
     << (u_int16_t)0 << (u_int16_t)0 << (u_int16_t)XIM_FLAG_INVALID_IM_IC << (u_int16_t)error
     << (u_int16_t)0 << (u_int16_t)0;

  XClientMessageEvent cm;
  memcpy(cm.data.b, data.data(), 20);
  sendClientMessageProtocol(client, cm);
  qDebug("XSendEvent XIM_ERROR");
}


void
InputMethod::sendXIMPreeditStart(u_int16_t im, u_int16_t ic) const
{
  DEBUG_TRACEFUNC("im:%d ic:%d", im, ic);

  if (_xim->getInputStyle(im, ic) != ON_THE_SPOT_STYLE) {
    qDebug("Not on-the-spot style");
    return;
  }
  if ( _xim->ximPreeditStarted(im, ic) ) {
    // Do nothing
    return;
  }

  QByteArray data;
  QDataStream ds(data, IO_WriteOnly);
  ds.setByteOrder(sysByteOrder());

  ds << (u_int8_t)XIM_PREEDIT_START << (u_int8_t)0 << numElements(4)
     << im << ic; 

  sendClientMessage(im, data);
  qDebug("XSendEvent XIM_PREEDIT_START");
  _xim->setXIMPreeditStarted(im, ic, TRUE);
}


void
InputMethod::recvXIMPreeditStartReply() const
{
  DEBUG_TRACEFUNC();
  char unused[4];
  QByteArray buf = _buffer->buffer();
  QDataStream dsbuf(buf, IO_ReadOnly);
  dsbuf.setByteOrder(sysByteOrder());
 
  int res;
  u_int16_t im ,ic;  
  dsbuf.readRawBytes((char*)unused, 4);   // header
  dsbuf >> im >> ic >> res;
  qDebug("XIMPreeditStartReply  im:%d ic:%d  res:%d", im, ic, res);
}


void
InputMethod::sendXIMPreeditDraw(u_int16_t im, u_int16_t ic, int caret, int index, int length, const QString& str, const QByteArray& feedback) const
{
  DEBUG_TRACEFUNC("str: %s", str.local8Bit().data());
  
  if (_xim->getInputStyle(im, ic) != ON_THE_SPOT_STYLE) {
    qWarning("Not on-the-spot style");
    return;
  }
  if ( !_xim->ximPreeditStarted(im, ic) ) {
    // Do nothing
    return;
  }
  if (str.length() != feedback.size() / sizeof(int)) {
    qFatal("Bad parameter  %s:%d", __FILE__, __LINE__);
    return;
  }

  XTextProperty tp;
  memset(&tp, 0, sizeof(tp));
  if ( !str.isEmpty() ) {
    uint buflen = str.local8Bit().length() + 1;
    char* pstr = (char*)alloca(buflen);
    strncpy(pstr, str.local8Bit().data(), buflen);
    int res = XmbTextListToTextProperty(qt_xdisplay(), &pstr, 1, XCompoundTextStyle, &tp);
    Q_ASSERT(res == Success);
  }

  QByteArray  data;
  QDataStream ds(data, IO_WriteOnly);
  ds.setByteOrder(sysByteOrder());
  ds << im << ic << caret << index << length;
  
  if (!str.isEmpty() && tp.value && tp.nitems > 0) {
    ds << (int)0 << (u_int16_t)tp.nitems;
    ds.writeRawBytes((char*)tp.value, tp.nitems);
    XFree(tp.value);

    for (int i = 0; i < pad(2 + tp.nitems); ++i)
      ds << (u_int8_t)0;       // unused
    
    ds << (u_int16_t)feedback.size() << (u_int16_t)0;
    ds.writeRawBytes(feedback.data(), feedback.size());
  } else {
    ds << (int)0x3;
  }

  QByteArray  msg;
  QDataStream dsmsg(msg, IO_WriteOnly);
  dsmsg.setByteOrder(sysByteOrder());
  dsmsg << (u_int8_t)XIM_PREEDIT_DRAW << (u_int8_t)0 << numElements(data.size());
  dsmsg.writeRawBytes(data.data(), data.size());
  
  sendClientMessage(im, msg);
  qDebug("XSendEvent XIM_PREEDIT_DRAW");
}


void
InputMethod::sendXIMPreeditDone(u_int16_t im, u_int16_t ic) const
{
  DEBUG_TRACEFUNC();

  if (_xim->getInputStyle(im, ic) != ON_THE_SPOT_STYLE) {
    qDebug("Not on-the-spot style");
    return;
  }
  if ( !_xim->ximPreeditStarted(im, ic) ) {
    // Do nothing
    return;
  }

  QByteArray data;
  QDataStream ds(data, IO_WriteOnly);
  ds.setByteOrder(sysByteOrder());

  ds << (u_int8_t)XIM_PREEDIT_DONE << (u_int8_t)0 << numElements(4)
     << im << ic; 

  sendClientMessage(im, data);
  qDebug("XSendEvent XIM_PREEDIT_DONE");
  _xim->setXIMPreeditStarted(im, ic, FALSE);
}


void 
InputMethod::sendClientMessageProtocol(Window w, XClientMessageEvent& cm) const
{
  DEBUG_TRACEFUNC();
  cm.type = ClientMessage;
  cm.display = qt_xdisplay();
  cm.window = w;
  cm.message_type = _xim_protocol;
  cm.format = 8;  
  XSendEvent(qt_xdisplay(), w, False, 0, (XEvent*)&cm);
  XFlush(qt_xdisplay());
}


void
InputMethod::sendClientMessageMoredata(Window w, XClientMessageEvent& cm) const
{ 
  DEBUG_TRACEFUNC();
  cm.type = ClientMessage;
  cm.display = qt_xdisplay();
  cm.window = w;
  cm.message_type = _xim_moredata;
  cm.format = 8; 
  XSendEvent(qt_xdisplay(), w, False, 0, (XEvent*)&cm);
}


void 
InputMethod::sendClientMessage(u_int16_t im, const QByteArray& data) const
{
  DEBUG_TRACEFUNC();
  uint len = data.size();
  if (len > DIVIDINGSIZE) {
    sendPropertywithCM(im, data);   // send Property with CM
    return;
  }

  qDebug("Send message. major-protocol-number:%d length:%d", 
	 (u_int8_t)data[0], len);
  XClientMessageEvent cm;

  Window w = _xim->commWin(im);
  int j = 0;
  for (;;) {
    if (len > 20) {
      for (int i = 0; i < 20; i++) {
	cm.data.b[i] = data[j++];
      }

      sendClientMessageMoredata(w, cm);
      len -= 20;

    } else {
      memset(cm.data.b, 0, 20);
      for (int i = 0; i < (int)len; i++) {
	cm.data.b[i] = data[j++];
      }
      sendClientMessageProtocol(w, cm);
      break;
    }
  }
}


void 
InputMethod::sendPropertywithCM(u_int16_t im, const QByteArray& data) const
{
  DEBUG_TRACEFUNC();
  Window commwin = _xim->commWin(im);
  XChangeProperty(qt_xdisplay(), commwin, _kimera_atom, 
		  XA_STRING, 8, PropModeAppend, (u_int8_t*)data.data(),
		  data.size());
  
  XClientMessageEvent cm;
  cm.type = ClientMessage;
  cm.display = qt_xdisplay();
  cm.window = commwin;
  cm.message_type = _xim_protocol;
  cm.format = 32;
  cm.data.l[0] = data.size();
  cm.data.l[1] = _kimera_atom;
  XSendEvent(qt_xdisplay(), commwin, False, 0, (XEvent*)&cm);
  XFlush(qt_xdisplay());
}


// X Transport Connection
// Receive X Transport Connection ClientMessage
void 
InputMethod::recvXTransportConnection(const XEvent& e)
{
  DEBUG_TRACEFUNC();
  if (e.xclient.message_type != _xim_xconnect || 
      e.xclient.format != 32) {
    qFatal("incorrect format  %s:%d", __FILE__, __LINE__);
    return;
  }
  
  // Check list count
  if (_cltwinstack.count() > 10) {
    // Exceeds max
    _cltwinstack.pop_front();
    qDebug("Removes the bottom item from the client-window-stack"); 
  }
  
  Window cltwin = e.xclient.data.l[0];  // set client window
  _cltwinstack.push(cltwin);
  qDebug("Client communication window ID: %ld", cltwin);
  qDebug("Request client-major-transport-version: %ld", e.xclient.data.l[1]);
  qDebug("Request client-minor-transport-version: %ld", e.xclient.data.l[2]);
  
  XClientMessageEvent cm;
  memset(cm.data.b, 0, 20);
  cm.type = e.xclient.type;
  cm.display = e.xclient.display;
  cm.window = cltwin;    
  cm.message_type = e.xclient.message_type;
  cm.format = 32;
  cm.data.l[0] = winId();
  cm.data.l[1] = 0;  // set major transport version
  cm.data.l[2] = 2;  // set minor transport version
  cm.data.l[3] = DIVIDINGSIZE;
  
  qDebug("Reply client-major-transport-version: %ld", cm.data.l[1]);
  qDebug("Reply client-minor-transport-version: %ld", cm.data.l[2]);
  
  XSendEvent(cm.display, cm.window, False, 0, (XEvent*)&cm);
  XFlush(qt_xdisplay());
  qDebug("XSendEvent XTransportConnection reply");
}


void 
InputMethod::customEvent(QCustomEvent* e)
{
  DEBUG_TRACEFUNC();
  if (e->type() == (QEvent::Type)KimeraApp::Selection) {
    recvSelectionEvent(e);
    return;
  }
  if (e->type() != (QEvent::Type)KimeraApp::XIMClientMessage || !_xim) {
    return;
  }

  XEvent* event = (XEvent*)e->data();
  
  // Dispatch Client Message
  if (event->xclient.message_type == _xim_xconnect) {
    recvXTransportConnection(*event);
    return;
  } else if (event->xclient.message_type == _xim_protocol 
	     && event->xclient.format == 32) {
    recvProperty(*event);
  } else if (event->xclient.message_type != _xim_protocol 
	     && event->xclient.message_type != _xim_moredata) {
    qDebug("Unexpected ClientMessage message_type: %lu", event->xclient.message_type);
    qDebug("Unexpected ClientMessage format: %d", event->xclient.format);
    return;
  } else {
    Q_ASSERT(event->xclient.format == 8);
    Q_ASSERT(_buffer->isOpen());
    _buffer->writeBlock(event->xclient.data.b, 20);
  }
  
  if (event->xclient.message_type == _xim_protocol && _buffer->size() > 0) {
    Q_ASSERT( !_buffer->buffer().at(1) );  // minor-opcode must be zero
    qDebug("=== Received Message Protocol  number:%d  size:%d",
	   _buffer->buffer().at(0), _buffer->buffer().size());
  
    switch (_buffer->buffer().at(0)) {  // major-opcode of received packet
    case XIM_CONNECT:
      recvXIMConnect( *event );
      break;

    case XIM_DISCONNECT:
      recvXIMDisconnect( *event );
      break;
      
    case XIM_OPEN:
      recvXIMOpen( *event );
      break;
      
    case XIM_CLOSE:
      recvXIMClose();
      break;

    case XIM_QUERY_EXTENSION:
      recvXIMQureyExtension();
      break;
      
    case XIM_ENCODING_NEGOTIATION:
      recvXIMEncodingNegotiation();
      break;

    case XIM_GET_IM_VALUES:
      recvXIMGetIMValues();
      break;

    case XIM_CREATE_IC:
      recvXIMCreateIC();
      break;
      
    case XIM_DESTROY_IC:
      recvXIMDestroyIC();
      break;

    case XIM_GET_IC_VALUES:
      recvXIMGetICValues();
      break;
 
    case XIM_SET_IC_VALUES:
      recvXIMSetICValues();
      break;
      
    case XIM_SET_IC_FOCUS:
      recvXIMSetICFocus();
      break;

    case XIM_UNSET_IC_FOCUS:
      recvXIMUnsetICFocus();
      break;

    case XIM_TRIGGER_NOTIFY:
      recvXIMTriggerNotify();
      break;

    case XIM_FORWARD_EVENT:
      recvXIMForwardEvent();
      break;
      
    case XIM_ERROR:
      recvXIMError();
      break;
      
    case XIM_RESET_IC:
      recvXIMResetIC();
      break;

    case XIM_SYNC:
      recvXIMSync();
      break;

    case XIM_SYNC_REPLY:
      recvXIMSyncReply();
      break;

    case XIM_PREEDIT_START_REPLY:
      recvXIMPreeditStartReply();
      break;

    default:
      qDebug("major-opcode: %d", (u_int8_t)_buffer->buffer().at(0));
      qDebug("length(byte): %d", *(u_int16_t*)(_buffer->buffer().data() + 2) * 4);
      break;
    }
    
    _buffer->close();
    _buffer->setBuffer(QByteArray(0));   // clean buffer
    Q_ASSERT(_buffer->size() == 0); 
    _buffer->open( IO_WriteOnly | IO_Append );
  }
}


void 
InputMethod::recvSelectionEvent(const QEvent* e)
{
  DEBUG_TRACEFUNC();
  if (e->type() != (QEvent::Type)KimeraApp::Selection) {
    Q_ASSERT(0);
    return;
  }

  XEvent event = *(XEvent*)((QCustomEvent*)e)->data();

  // Dispatch Selection Event
  switch (event.type) {
  case SelectionRequest:
    qDebug("Receive SelectionRequest");
    recvSelectionRequest(event);
    break;

  case SelectionClear:
    qDebug("Receive SelectionClear");
    recvSelectionClear(event);
    break;

  case SelectionNotify:
    qDebug("Receive SelectionNotify");
    Q_ASSERT(0);
    break;

  default: 
    qDebug("unknown selection event:%d", event.type);
    Q_ASSERT(0);
    break;
  }
}


// Calculate preedit point of current window
// Store the specified IM-ID and IC-ID temporarily
QPoint
InputMethod::calcPreeditPoint(u_int16_t im, u_int16_t ic, bool& ok) const
{
  DEBUG_TRACEFUNC("im:%u  ic:%u", im, ic);

  ok = FALSE;
  if (!im || !ic) {
    return QPoint();
  }
  
  // Translate client window coordinates
  Window src_w = _xim->focusWindow(im, ic);
  int x = 0;
  int y = 0;
  Window child_win = None;
  Bool ret;
  for (;;) {
    ret = XTranslateCoordinates(qt_xdisplay(), src_w, qt_xrootwin(), 0, 0, &x, &y, &child_win);
    if (ret == True) {
      qDebug("XTranslateCoordinates  x:%d y:%d", x, y);
      break;
    } else if (child_win == None) {
      qDebug("No such window : %lu", _xim->focusWindow(im, ic));
      return  QPoint();
    }

    src_w = child_win;
    x = 0;
    y = 0;
    child_win = None;
  }
  
  QPoint pos = QPoint(x, y);
  QPoint spotpret = _xim->spotPreedit(im, ic);
  XIMStyle style = _xim->getInputStyle(im, ic);
  if ((style == ON_THE_SPOT_STYLE || style == ROOT_WINDOW_STYLE) && spotpret == QPoint(0, 0)) {
    // Adjusts point in case of on-the-spot style or root-window style
    XWindowAttributes attr;
    Status s = XGetWindowAttributes(qt_xdisplay(), _xim->focusWindow(im, ic), &attr);
    if ( s ) {
      pos += QPoint(0, attr.height);
    }
  } else {
    pos += spotpret;
  }
  
  qDebug("latest focus windows:%ld  IM:%u IC:%u x:%d y:%d",
	 _xim->focusWindow(im, ic), im, ic, pos.x(), pos.y());
  ok = TRUE;
  return pos;
}


// Check communication window property
bool
InputMethod::checkCommWindow(u_int16_t im) const
{
  DEBUG_TRACEFUNC("im:%u", im);
  bool res = FALSE;
  Window commwin = _xim->commWin(im);
  if ( commwin ) {
    XWindowAttributes attributes;
    memset(&attributes, 0, sizeof(attributes));
    XGetWindowAttributes(qt_xdisplay(), commwin, &attributes);
    if (attributes.root == qt_xrootwin()) {
      res = TRUE;
    } else {
      qDebug("no such communication window : %ld   attributes.root:%ld  qt_xrootwin:%ld ",
	     commwin, attributes.root, qt_xrootwin());
    }
  }
  
  return res;
}


void
InputMethod::registerXIMServerKimera()
{
  DEBUG_TRACEFUNC();

  // Create Atom
  Atom xim_servers = XInternAtom(qt_xdisplay(), "XIM_SERVERS", False);
  qDebug("XIM_SERVERS: %lu", xim_servers);
  Atom server_atom = XInternAtom(qt_xdisplay(), SERVER_ATOM_STRING, False);
  qDebug("@server=kimera: %lu", server_atom);

  Atom type;
  int format;
  ulong nitems;
  ulong bytes_after;
  u_int8_t* value = 0;

  // Grab server
  XGrabServer(qt_xdisplay());

  // Get the Atom contained in XIM_SERVERS Property
  int res = XGetWindowProperty(qt_xdisplay(), qt_xrootwin(), xim_servers,
                               0, 1024, False, XA_ATOM, &type,
                               &format, &nitems, &bytes_after, &value);
  Q_ASSERT(res == Success);
  Q_ASSERT(bytes_after == 0);

  // Compare the gotten Atom and XIM_SERVER Atom
  Atom tmpatom = server_atom;
  if (res != Success || type != XA_ATOM || format != 32 || !value ) {
    // Register XIM_SERVER Atom
    XChangeProperty(qt_xdisplay(), qt_xrootwin(), xim_servers,
                    XA_ATOM, 32, PropModeReplace,
                    (u_int8_t *)&tmpatom, 1);
    qDebug("No XIM_SERVER. Regiter IM server");

  } else {
    ulong* atoms = (ulong*)value;
    int i;
    for (i = 0; i < (int)nitems; i++) {
      if (atoms[i] == server_atom) {
        // Already registered
        qDebug("IM server registered already. No change.");
        break;
      }
    }

    if (i == (int)nitems) {
      // Register XIM_SERVER Atom
      XChangeProperty(qt_xdisplay(), qt_xrootwin(), xim_servers,
                      XA_ATOM, 32, PropModeAppend,
                      (u_int8_t *)&tmpatom, 1);
      qDebug("No Kimera. Register IM server");
    }
  }

  if ( value )
    XFree((char*)value);

  // Ungrab server
  XUngrabServer(qt_xdisplay());

  // Flush
  XFlush(qt_xdisplay());
}


void
InputMethod::setXIMInputtingEnabled(bool enable)
{
  DEBUG_TRACEFUNC("enable: %d", enable);
 
  _inputon = enable;  
  if (_crnt_im > 0 && _crnt_ic > 0) {
    if ( _inputon ) {
      // if on-key
      bool ok;
      QPoint pos = calcPreeditPoint(_crnt_im, _crnt_ic, ok);
      if ( ok ) {
	sendXIMSetEventMask(_crnt_im, _crnt_ic, KeyPressMask); // XIM_SET_EVENT_MASK
	_kanjiconvt->setPreeditPoint(pos);
	sendXIMPreeditStart(_crnt_im, _crnt_ic);
      } else {
	qFatal("Assert  %s:%d", __FILE__, __LINE__);
	// Remove IM
	_xim->removeIM(_crnt_im);
	_crnt_im = _crnt_ic = 0;
      }
    } else {
      // if off-key
      sendXIMSetEventMask(_crnt_im, _crnt_ic);  // XIM_SET_EVENT_MASK
      sendXIMPreeditDone(_crnt_im, _crnt_ic);
    }
  }
}
