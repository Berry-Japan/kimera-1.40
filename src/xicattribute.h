#ifndef XICATTRIBUTE_H
#define XICATTRIBUTE_H

#include <qcstring.h>
#include <qvaluelist.h>
#include <qpoint.h>
#include <X11/Xlib.h>

const XIMStyle  OVER_THE_SPOT_STYLE = XIMPreeditPosition  | XIMStatusNothing;
const XIMStyle  ON_THE_SPOT_STYLE   = XIMPreeditCallbacks | XIMStatusNothing;
const XIMStyle  ROOT_WINDOW_STYLE   = XIMPreeditNothing   | XIMStatusNothing;

class XICAttribute {
public:
  XICAttribute();
  ~XICAttribute();

  void        setValue(u_int16_t, const QByteArray&); 
  QByteArray  getValue(u_int16_t) const;
  void        setXIMPreeditStarted(bool b) { _xim_predtstarted = b; }
  bool        ximPreeditStarted() const { return _xim_predtstarted; }
  XIMStyle    inputStyle() const { return _inputstyle; }
  Window      focusWindow() const { return _focuswin ? _focuswin : _clientwin; }
  QCString    fontPreedit() const { return _fontset_predt; }
  void        setSpotPreedit(const QPoint& p) { _spot_predt.x = p.x(); _spot_predt.y = p.y(); }
  QPoint      spotPreedit() const { return QPoint(_spot_predt.x, _spot_predt.y); }

protected:
  void  setValuePreeditAttr(u_int16_t, const QByteArray&);
  void  setValueStatusAttr(u_int16_t, const QByteArray&);
  int   pad(int n) const { return (4 - (n % 4)) % 4; }

private:
  bool        _xim_predtstarted;   // on-the-spot preedit start flag

  XIMStyle    _inputstyle;    // attribute ID: 0
  Window      _clientwin;     // attribute ID: 1
  ulong       _filterevents;  // attribute ID: 4
  Window      _focuswin;      // attribute ID: 5

  // Preedit Attributes       // attribute ID: 2
  QCString    _fontset_predt; // attribute ID: 6
  XPoint      _spot_predt;    // attribute ID: 7
  XRectangle  _areand_predt;  // attribute ID: 8
  XRectangle  _area_predt;    // attribute ID: 9
  ulong       _fg_predt;      // attribute ID: 10
  ulong       _bg_predt;      // attribute ID: 11
  Pixmap      _bgpix_predt;   // attribute ID: 12
  Colormap    _color_predt;   // attribute ID: 13
  Atom        _stdclr_predt;  // attribute ID: 14
  int         _line_predt;    // attribute ID: 15
  Cursor      _cur_predt;     // attribute ID: 16

  // Status Attributes        // attribute ID: 3
  QCString    _fontset_stat;  // attribute ID: 6
  XPoint      _spot_stat;     // attribute ID: 7
  XRectangle  _areand_stat;   // attribute ID: 8
  XRectangle  _area_stat;     // attribute ID: 9
  ulong       _fg_stat;       // attribute ID: 10
  ulong       _bg_stat;       // attribute ID: 11
  Pixmap      _bgpix_stat;    // attribute ID: 12
  Colormap    _color_stat;    // attribute ID: 13
  Atom        _stdclr_stat;   // attribute ID: 14
  int         _line_stat;     // attribute ID: 15
  Cursor      _cur_stat;      // attribute ID: 16
};

#endif // XICATTRIBUTE_H

