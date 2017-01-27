#include "xicontext.h"
#include "xicattribute.h"

QValueList<SupportAttr> XIContext::_attr_list = QValueList<SupportAttr>();

void
XIContext::initSuppICAttr()
{
  if ( !_attr_list.isEmpty() ) return;
  
  u_int16_t i = 0;
  _attr_list << SupportAttr(i++, XNInputStyle, TYPE_LONG);
  _attr_list << SupportAttr(i++, XNClientWindow,TYPE_WINDOW);
  _attr_list << SupportAttr(i++, XNPreeditAttributes, TYPE_NESTEDLIST);
  _attr_list << SupportAttr(i++, XNStatusAttributes, TYPE_NESTEDLIST);
  _attr_list << SupportAttr(i++, XNFilterEvents, TYPE_WORD);
  _attr_list << SupportAttr(i++, XNFocusWindow, TYPE_WINDOW);
  _attr_list << SupportAttr(i++, XNFontSet, TYPE_XFONTSET);
  _attr_list << SupportAttr(i++, XNSpotLocation, TYPE_XPOINT);
  _attr_list << SupportAttr(i++, XNAreaNeeded, TYPE_XRECTANGLE);
  _attr_list << SupportAttr(i++, XNArea, TYPE_XRECTANGLE);
  _attr_list << SupportAttr(i++, XNForeground, TYPE_LONG);
  _attr_list << SupportAttr(i++, XNBackground, TYPE_LONG);
  _attr_list << SupportAttr(i++, XNBackgroundPixmap, TYPE_LONG);
  _attr_list << SupportAttr(i++, XNColormap, TYPE_WORD);
  _attr_list << SupportAttr(i++, XNStdColormap, TYPE_WORD);
  //_attr_list << SupportAttr(i++, XNLineSpace, TYPE_WORD);
  _attr_list << SupportAttr(i++, XNLineSpace, TYPE_LONG);
  _attr_list << SupportAttr(i++, XNCursor, TYPE_WORD);

  Q_ASSERT(_attr_list.count() == 17);
}


QValueList<SupportAttr>
XIContext::getSuppICAttr() 
{
  if ( _attr_list.isEmpty() ) initSuppICAttr();
  return _attr_list; 
}


XIContext::XIContext()
{
  _iclist.setAutoDelete(TRUE);
}


XIContext::~XIContext()
{

}


u_int16_t     
XIContext::createIC()
{
  u_int16_t newic;   // 1 origin
  for (newic = 1; _iclist[newic]; ++newic);   // search new IC
  
  _iclist.replace(newic, new XICAttribute());
  return newic;
}


void  
XIContext::removeIC(u_int16_t ic) 
{ 
  _iclist.remove(ic); 
}


void
XIContext::setValue(u_int16_t ic, u_int16_t id, const QByteArray& data)
{
  XICAttribute* pxic = _iclist[ic];
  if ( pxic ) pxic->setValue(id, data);
}


QByteArray
XIContext::getValue(u_int16_t ic, u_int16_t id) const
{
  XICAttribute* pxic = _iclist[ic];
  return pxic ? pxic->getValue(id) : QByteArray();
}


XIMStyle
XIContext::inputStyle(u_int16_t ic) const
{
  XICAttribute* pxic = _iclist[ic];
  return pxic ? pxic->inputStyle() : 0;
}


Window
XIContext::focusWindow(u_int16_t ic) const
{
  XICAttribute* pxic = _iclist[ic];
  return pxic ? pxic->focusWindow() : 0;
}


QString
XIContext::fontPreedit(u_int16_t ic) const
{
  XICAttribute* pxic = _iclist[ic];
  return pxic ? pxic->fontPreedit() : QString();
}


void
XIContext::setSpotPreedit(u_int16_t ic, const QPoint& p)
{
  XICAttribute* pxic = _iclist[ic];
  if ( pxic ) pxic->setSpotPreedit( p );
}


QPoint
XIContext::spotPreedit(u_int16_t ic) const
{
  XICAttribute* pxic = _iclist[ic];
  return pxic ? pxic->spotPreedit() : QPoint();
}


bool
XIContext::ximPreeditStarted(u_int16_t ic) const
{
  XICAttribute* pxic = _iclist[ic];
  return pxic ? pxic->ximPreeditStarted() : FALSE;
}


void
XIContext::setXIMPreeditStarted(u_int16_t ic, bool b)
{
  XICAttribute* pxic = _iclist[ic];
  if ( pxic ) pxic->setXIMPreeditStarted( b );
}


QValueList<u_int16_t>
XIContext::icList() const
{
  QValueList<u_int16_t> lst;
  for (QIntDictIterator<XICAttribute> it(_iclist); it.current(); ++it) {
    if (it.currentKey() > 0) {
      lst << it.currentKey();
    }
  }
  
  return lst;
}
