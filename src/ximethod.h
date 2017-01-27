#ifndef XIMETHOD_H
#define XIMETHOD_H

#include "kimeraglobal.h"
#include "ximattribute.h"
#include "xicattribute.h"
#include "supportattr.h"
#include <qvaluelist.h>
#include <qintdict.h>
#include <X11/Xlib.h>
using namespace Kimera;

class XIMethod {
public:
  XIMethod();
  ~XIMethod() { }

  void        connectIM(Window w);
  QValueList<Window> disconnectIM();
  u_int16_t   createIM(Window w);
  bool        exists(u_int16_t im) const;
  void        removeIM(u_int16_t im);
  u_int16_t   createXIC(u_int16_t im);
  void        removeXIC(u_int16_t im, u_int16_t ic);
  void        setICValue(u_int16_t im, u_int16_t ic, u_int16_t id, const QByteArray&);
  QByteArray  getICValue(u_int16_t im, u_int16_t ic, u_int16_t id);
  QString     fontPreedit(u_int16_t im, u_int16_t ic);
  void        setSpotPreedit(u_int16_t im, u_int16_t ic, const QPoint& p);
  QPoint      spotPreedit(u_int16_t im, u_int16_t ic);
  Window      commWin(u_int16_t im) const;
  Window      focusWindow(u_int16_t im, u_int16_t ic) const;
  void        setXIMPreeditStarted(u_int16_t im, u_int16_t ic, bool b);
  bool        ximPreeditStarted(u_int16_t im, u_int16_t ic) const;
  XIMStyle    getInputStyle(u_int16_t im, u_int16_t ic) const;
  QByteArray  queryInputStyle() const;
  QValueList<u_int16_t> getIMList() const;
  QValueList<u_int16_t> getICList(u_int16_t im) const;

  static QValueList<SupportAttr>  getSuppIMAttr();
  static QValueList<SupportAttr>  getSuppICAttr();

protected:
  static void     initSuppIMAttr();   // initialize IM attributes supported

private:
  QIntDict<XIMAttribute>  _imlist;          // list of IM
  QValueList<Window>      _disconnect_wins;
  static QValueList<SupportAttr>  _attr_list; // IM attributes supported
};


#endif // XIMETHOD_H

