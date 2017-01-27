#include "ximethod.h"
#include "xicattribute.h"
#include "xicontext.h"
#include "kimeraglobal.h"
#include "debug.h"

QValueList<SupportAttr> XIMethod::_attr_list = QValueList<SupportAttr>();

XIMethod::XIMethod()
{
  _imlist.setAutoDelete(TRUE);
}


void
XIMethod::initSuppIMAttr()
{
  if ( !_attr_list.isEmpty() ) return;
  
  _attr_list << SupportAttr(0, XNQueryInputStyle, TYPE_XIMSTYLES);
  Q_ASSERT(_attr_list.count() == 1);
}


QValueList<SupportAttr>
XIMethod::getSuppIMAttr()
{
  if ( _attr_list.isEmpty() ) initSuppIMAttr();
  return _attr_list;
}


QValueList<SupportAttr>
XIMethod::getSuppICAttr() 
{ 
  return XIContext::getSuppICAttr(); 
}


void
XIMethod::connectIM(Window w)
{
  _disconnect_wins.remove(w);
}


QValueList<Window>
XIMethod::disconnectIM()
{
  QValueList<Window> res(_disconnect_wins);
  _disconnect_wins.clear();
  return res;
}


bool
XIMethod::exists(u_int16_t im) const
{
  return (bool)_imlist[im];
}


u_int16_t 
XIMethod::createIM(Window w)
{
  u_int16_t newim;    // 1 origin
  for (newim = 1; _imlist[newim]; ++newim);   // search new IM
  
  _imlist.replace(newim, new XIMAttribute(w));
  qDebug("create IM:%d", newim);
  return newim;
}


void
XIMethod::removeIM(u_int16_t im)
{
  XIMAttribute* pxim = _imlist[im];
  if ( pxim ) {
    _disconnect_wins << pxim->commWin();
    _imlist.remove( im );
    qDebug("Remove IM:%d  number of IMs:%d", im, _imlist.count());
  }
}


u_int16_t
XIMethod::createXIC(u_int16_t im)
{
  XIMAttribute* pxim = _imlist[im];
  return pxim ? pxim->_xic.createIC() : 0;
}


void
XIMethod::removeXIC(u_int16_t im, u_int16_t ic)
{
  XIMAttribute* pxim = _imlist[im];
  if ( pxim ) {
    pxim->_xic.removeIC(ic);
  }
}


void
XIMethod::setICValue(u_int16_t im, u_int16_t ic, u_int16_t id, const QByteArray& data)
{
  qDebug("%s: IM:%d IC:%d attributes-id:%d data-size:%d",
	 __func__, im, ic, id, data.size());

  XIMAttribute* pxim = _imlist[im];
  if ( pxim )
    pxim->_xic.setValue(ic, id, data);
}


QByteArray 
XIMethod::getICValue(u_int16_t im, u_int16_t ic, u_int16_t id)
{
  qDebug("%s: IM:%d IC:%d attributes-id:%d", __func__, im, ic, id);
  XIMAttribute* pxim = _imlist[im];
  return pxim ? pxim->_xic.getValue(ic, id) : QByteArray();
}


QString
XIMethod::fontPreedit(u_int16_t im, u_int16_t ic)
{
  XIMAttribute* pxim = _imlist[im];
  return pxim ? pxim->_xic.fontPreedit(ic) : QString();
}


void
XIMethod::setSpotPreedit(u_int16_t im, u_int16_t ic, const QPoint& p)
{
  XIMAttribute*  pxim = _imlist[im];
  if ( pxim ) {
    pxim->_xic.setSpotPreedit(ic, p);
  }
}


QPoint
XIMethod::spotPreedit(u_int16_t im, u_int16_t ic)
{
  XIMAttribute* pxim = _imlist[im];
  return pxim ? pxim->_xic.spotPreedit(ic) : QPoint();
}


Window
XIMethod::commWin(u_int16_t im) const
{
  XIMAttribute* pxim = _imlist[im];
  return pxim ? pxim->commWin() : 0;
}


Window
XIMethod::focusWindow(u_int16_t im, u_int16_t ic) const
{
  XIMAttribute* pxim = _imlist[im];
  return pxim ? pxim->_xic.focusWindow(ic) : 0;
}



XIMStyle
XIMethod::getInputStyle(u_int16_t im, u_int16_t ic) const
{
  XIMAttribute*  pxim = _imlist[im];
  return pxim ? pxim->_xic.inputStyle(ic) : 0;
}


QByteArray
XIMethod::queryInputStyle() const
{
  QByteArray  ret(0);
  QDataStream ds(ret, IO_WriteOnly);
  ds.setByteOrder(sysByteOrder());
  ds << (u_int16_t)(sizeof(uint) * 3) << (u_int16_t)0
     << (uint)OVER_THE_SPOT_STYLE << (uint)ON_THE_SPOT_STYLE << (uint)ROOT_WINDOW_STYLE;

  return ret;
}


void
XIMethod::setXIMPreeditStarted(u_int16_t im, u_int16_t ic, bool b)
{
  XIMAttribute*  pxim = _imlist[im];
  if ( pxim ) {
    pxim->_xic.setXIMPreeditStarted(ic, b);
  }
}


bool
XIMethod::ximPreeditStarted(u_int16_t im, u_int16_t ic) const
{
  XIMAttribute*  pxim = _imlist[im];
  return pxim ? pxim->_xic.ximPreeditStarted(ic) : FALSE;
}


QValueList<u_int16_t>
XIMethod::getIMList() const
{
  QValueList<u_int16_t> lst;
  for (QIntDictIterator<XIMAttribute> it(_imlist); it.current(); ++it) {
    if (it.currentKey() > 0) {
      lst << it.currentKey();
    }
  }
  return lst;
}


QValueList<u_int16_t>
XIMethod::getICList(u_int16_t im) const
{
  XIMAttribute*  pxim = _imlist[im];
  return pxim ? pxim->xic().icList() : QValueList<u_int16_t>();
}

