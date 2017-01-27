#ifndef XICONTEXT_H
#define XICONTEXT_H

#include "kimeraglobal.h"
#include "supportattr.h"
#include <qvaluelist.h>
#include <qstring.h>
#include <qpoint.h>
#include <qintdict.h>
#include <qpair.h>
#include <X11/Xlib.h>
using namespace Kimera;

class XICAttribute;

class XIContext {
public:
  XIContext();
  ~XIContext();

  u_int16_t    createIC();
  void         removeIC(u_int16_t ic);
  void         setValue(u_int16_t ic, u_int16_t id, const QByteArray&); 
  QByteArray   getValue(u_int16_t ic, u_int16_t id) const;
  XIMStyle     inputStyle(u_int16_t ic) const;
  Window       focusWindow(u_int16_t ic) const;
  QString      fontPreedit(u_int16_t ic) const;
  void         setSpotPreedit(u_int16_t ic, const QPoint& p);
  QPoint       spotPreedit(u_int16_t ic) const;
  bool         ximPreeditStarted(u_int16_t ic) const;
  void         setXIMPreeditStarted(u_int16_t ic, bool b);
  QValueList<u_int16_t>  icList() const;
  static QValueList<SupportAttr>  getSuppICAttr();

protected:
  static void  initSuppICAttr();  // initialize IC attributes supported

private:
  QIntDict<XICAttribute>  _iclist;    // list of ICs
  static QValueList<SupportAttr>  _attr_list;  // IC attributes supported
};


#endif // XICONTEXT_H
