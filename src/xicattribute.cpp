#include "xicattribute.h"
#include "kimeraglobal.h"
#include "debug.h"

XICAttribute::XICAttribute()
  : _xim_predtstarted(FALSE),
    _inputstyle(0),
    _clientwin(0),
    _filterevents(KeyPressMask),
    _focuswin(0),
    _fontset_predt(),
    _spot_predt(),
    _areand_predt(),
    _area_predt(),
    _fg_predt(0),
    _bg_predt(0),
    _bgpix_predt(),
    _color_predt(),
    _stdclr_predt(0),  
    _line_predt(0),
    _cur_predt(),
    _fontset_stat(),
    _spot_stat(),
    _areand_stat(), 
    _area_stat(),
    _fg_stat(0),
    _bg_stat(0),
    _bgpix_stat(),   
    _color_stat(),
    _stdclr_stat(0), 
    _line_stat(0),
    _cur_stat()
{
}


XICAttribute::~XICAttribute()
{
}


QByteArray
XICAttribute::getValue(u_int16_t id) const
{
  QByteArray ret(0);
  QDataStream ds(ret, IO_WriteOnly);
  ds.setByteOrder(sysByteOrder());
  
  switch (id) {
  case 0:
    ds << _inputstyle;
    break;

  case 1:
    ds << _clientwin;
    break;

  case 4:
    ds << _filterevents;
    break;

  default:
    qWarning("%s: unsupported id:%d", __func__, id);
    Q_ASSERT(0);
    break;
  }

  return ret;
}


void
XICAttribute::setValue(u_int16_t id, const QByteArray& data)
{
  QDataStream ds(data, IO_ReadOnly);
  ds.setByteOrder(sysByteOrder());
  char unused[4];

  switch (id) {
  case 0:
    Q_ASSERT(data.size() == sizeof(XIMStyle));
    ds >> _inputstyle;
    qDebug("client request input styles:%lx", _inputstyle);
    Q_ASSERT(_inputstyle == OVER_THE_SPOT_STYLE || _inputstyle == ON_THE_SPOT_STYLE
	     || _inputstyle == ROOT_WINDOW_STYLE);
    break;

  case 1:
    Q_ASSERT(data.size() == sizeof(Window));
    ds >> _clientwin;
    break;

  case 2:
    while ( !ds.atEnd() ) {
      u_int16_t id, len;
      ds >> id >> len;
      QByteArray attr(len);
      ds.readRawBytes(attr.data(), len);
      ds.readRawBytes((char*)unused, pad(len));    // unused
      setValuePreeditAttr(id, attr);
    }
    break;
    
  case 3:
    while ( !ds.atEnd() ) {
      u_int16_t id, len;
      ds >> id >> len;
      QByteArray attr(len);  
      ds.readRawBytes(attr.data(), len);
      ds.readRawBytes((char*)unused, pad(len));    // unused
      setValueStatusAttr(id, attr);
    }
    break;
      
  case 4:
    Q_ASSERT(data.size() == sizeof(ulong));
    ds >> _filterevents;
    break;

  case 5:
    Q_ASSERT(data.size() == sizeof(Window));
    ds >> _focuswin;
    break;;

  default:
    qDebug("%s:unsupported id:%d", __func__, id);
    Q_ASSERT(0);
    break;
  }
}


void
XICAttribute::setValuePreeditAttr(u_int16_t id, const QByteArray& data)
{
  QDataStream ds(data, IO_ReadOnly);
  ds.setByteOrder(sysByteOrder()); 
  QByteArray fs;
  u_int16_t len;

  qDebug("%s id:%d data size:%d", __func__, id, data.size());
  switch (id) {
  case 6:
    ds >> len;
    fs.resize(len);
    ds.readRawBytes(fs.data(), len);
    _fontset_predt = fs;
    qDebug("Preedit Attributes FontSet:%d :%s", _fontset_predt.length(), _fontset_predt.data());
    break;
    
  case 7:
    Q_ASSERT(data.size() == sizeof(XPoint));
    ds >> _spot_predt.x >> _spot_predt.y;
    qDebug("Preedit Attributes  preedit spot x:%d y:%d", _spot_predt.x, _spot_predt.y);
    break;
  
  case 8:
    Q_ASSERT(data.size() == sizeof(XRectangle));
    ds >> _areand_predt.x >> _areand_predt.y
       >> _areand_predt.width >> _areand_predt.height;
    break;
  
  case 9:
    Q_ASSERT(data.size() == sizeof(XRectangle));
    ds >> _area_predt.x >> _area_predt.y
       >> _area_predt.width >> _area_predt.height;
    break;
  
  case 10:
    Q_ASSERT(data.size() == sizeof(ulong));
    ds >> _fg_predt;
    break;
  
  case 11:
    Q_ASSERT(data.size() == sizeof(ulong));
    ds >> _bg_predt;
    break;
  
  case 12:
    Q_ASSERT(data.size() == sizeof(Pixmap));
    ds >> _bgpix_predt;
    break;
  
  case 13:
    Q_ASSERT(data.size() == sizeof(Colormap));
    ds >> _color_predt;
    break;
  
  case 14:
    Q_ASSERT(data.size() == sizeof(Atom));
    ds >> _stdclr_predt;
    break;
  
  case 15:
    Q_ASSERT(data.size() == sizeof(int));
    ds >> _line_predt;
    break;
  
  case 16:
    Q_ASSERT(data.size() == sizeof(Cursor));
    ds >> _cur_predt;
    break;
  
  default:
    Q_ASSERT(0);
    break;
  }
}


void
XICAttribute::setValueStatusAttr(u_int16_t id, const QByteArray& data)
{
  QDataStream ds(data, IO_ReadOnly);
  ds.setByteOrder(sysByteOrder());
  QByteArray fs;
  u_int16_t len;

  qDebug("%s id:%d data size:%d", __func__, id, data.size());
  switch (id) {
  case 6:
    ds >> len;
    fs.resize(len);
    ds.readRawBytes(fs.data(), len);
    _fontset_stat = QString(fs);
    qDebug("Status Attributes FontSet:%d:%s", _fontset_stat.length(), _fontset_stat.data());
    break;
    
  case 7:
    Q_ASSERT(data.size() == sizeof(XPoint));
    ds >> _spot_stat.x >> _spot_stat.y;
    break;
    
  case 8:
    Q_ASSERT(data.size() == sizeof(XRectangle));
    ds >> _areand_stat.x >> _areand_stat.y
       >> _areand_stat.width >> _areand_stat.height;
    break;
    
  case 9:
    Q_ASSERT(data.size() == sizeof(XRectangle));
    ds >> _area_stat.x >> _area_stat.y
       >> _area_stat.width >> _area_stat.height;
    break;
    
  case 10:
    Q_ASSERT(data.size() == sizeof(ulong));
    ds >> _fg_stat;
    break;
    
  case 11:
    Q_ASSERT(data.size() == sizeof(ulong));
    ds >> _bg_stat;
    break;
    
  case 12:
    Q_ASSERT(data.size() == sizeof(Pixmap));
    ds >> _bgpix_stat;
    break;
    
  case 13:
    Q_ASSERT(data.size() == sizeof(Colormap));
    ds >> _color_stat; 
    break;
    
  case 14:
    Q_ASSERT(data.size() == sizeof(Atom));
    ds >> _stdclr_stat;
    break;
    
  case 15:
    Q_ASSERT(data.size() == sizeof(int));
    ds >> _line_stat;
    break;
    
  case 16:
    Q_ASSERT(data.size() == sizeof(Cursor));
    ds >> _cur_stat;
    break;
    
  default:
    Q_ASSERT(0);
    break;
  }
}
