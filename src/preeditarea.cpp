#include "preeditarea.h"
#include "kimeraapp.h"
#include "inputmethod.h"
#include "xicattribute.h"
#include "config.h"
#include <qlabel.h>
#include <qrect.h>
#include <qtimer.h>
#include <qmessagebox.h>

const QString  KeyColor = "_colorsettingcolor";
const QString  KeyUnderline = "_colorsettingunderline";

PreeditArea::PreeditArea() 
  : QWidget((QWidget*)0, "preedit", WStyle_StaysOnTop | WStyle_Customize | WStyle_NoBorder | WStyle_Tool | WX11BypassWM), _state(Input)
{
  _attentseg  = new QLabel(this, "attention_segment");
  _backseg    = new QLabel(this, "back_segment");
  _forwardseg = new QLabel(this, "forward_segment");
  _caret = new QLabel(this, "caret");
  resize(0, 0);
}


void 
PreeditArea::polish()
{
  init();
  _attentseg->setMargin( 0 );
  _attentseg->setIndent( 0 );
  _attentseg->setAutoMask( FALSE );
  _attentseg->setFrameStyle( QFrame::Plain | QFrame::Box );
  _attentseg->setLineWidth( 0 );
  _attentseg->setMidLineWidth( 0 );
  _attentseg->setAlignment( AlignAuto | AlignVCenter );

  _backseg->setMargin( 0 );
  _backseg->setIndent( 0 );
  _backseg->setAutoMask( FALSE );
  _backseg->setFrameStyle( QFrame::Plain | QFrame::Box );
  _backseg->setLineWidth( 0 );
  _backseg->setMidLineWidth( 0 );
  _backseg->setAlignment( AlignAuto | AlignVCenter );
  _backseg->move(0, 0);

  _forwardseg->setMargin( 0 );
  _forwardseg->setIndent( 0 );
  _forwardseg->setAutoMask( FALSE );
  _forwardseg->setFrameStyle( QFrame::Plain | QFrame::Box );
  _forwardseg->setLineWidth( 0 );
  _forwardseg->setMidLineWidth( 0 );
  _forwardseg->setAlignment( AlignAuto | AlignVCenter );  

  _caret->setBackgroundColor( QColor(0, 0, 0) );
}


void 
PreeditArea::init()
{
  debug("PreeditArea init");
  // Loads colors
  for (int i = 0; i < NumColors; i++) {
    _colorlist[i] = QColor( Config::readEntry(KeyColor + QString::number(i), "black") );
  }
}


void 
PreeditArea::setFont(const QFont & f)
{
  for (int i = 0; i < NumStates; i++) {
      _font[i] = f;
      _font[i].setUnderline(Config::readEntry(KeyUnderline + QString::number(i), "0") == QString("1"));
  }

  _attentseg->setFont( f );
  _backseg->setFont( f );
  _forwardseg->setFont( f );
  _caret->resize(2, QFontMetrics( f ).height());
}


void
PreeditArea::showText(const QStringList& strlist, uint attention, uint caret_pos)
{
  if (attention >= strlist.count()) {
    qFatal("Bad parameter");
    return;
  }
  if (strlist.isEmpty()) {
    hide();
    return;
  }
  
  // Sets segments
  QString str;
  for (int i = 0; i < (int)attention; ++i)
    str += strlist[i];
  _backseg->setText(str);
  _attentseg->setText(strlist[attention]);
  str = QString::null;
  for (int i = attention + 1; i < (int)strlist.count(); ++i)
    str += strlist[i];
  _forwardseg->setText(str);

  // Show
  switch ( KimeraApp::inputmethod()->currentXIMStyle() ) {
  case OVER_THE_SPOT_STYLE:
  case ROOT_WINDOW_STYLE:
    _backseg->text().isEmpty() ? _backseg->hide() : _backseg->show();
    _attentseg->show();
    _forwardseg->text().isEmpty() ? _forwardseg->hide() : _forwardseg->show();
    if (_state != Input) {
      _caret->hide();
    } else {
      QFontMetrics fm( _attentseg->font() );
      int pos = fm.width(_attentseg->text(), caret_pos);
      if (caret_pos == _attentseg->text().length())
	pos += _attentseg->margin() + 1;
      _caret->move(QPoint(pos, 1));
      _caret->raise();
      _caret->show();
    }
    show();
    adjustSize();
    break;

  case ON_THE_SPOT_STYLE:
    if (_state == Input) {
      KimeraApp::inputmethod()->sendPreeditString(caret_pos, _attentseg->text());
    } else {
      KimeraApp::inputmethod()->sendPreeditString(strlist.join("").length(), strlist, attention);
    }
    break;
    
  default:
    qWarning("Bad XIMStyle: %lu", KimeraApp::inputmethod()->currentXIMStyle());
    Q_ASSERT(0);
    return;
    break;
  }
  
  qDebug("showText  back: %s  attention: %s  forward: %s", _backseg->text().local8Bit().data(),
	 _attentseg->text().local8Bit().data(), _forwardseg->text().local8Bit().data());
}


void 
PreeditArea::showInputingString(const QString& str, uint caret_pos)
{
  Q_ASSERT( !str.isEmpty() );
  _state = Input;
  _attentseg->setFont( _font[Input] );
  _attentseg->setPaletteForegroundColor( _colorlist[FgInput] );
  _attentseg->setPaletteBackgroundColor( _colorlist[BgInput] );
  showText(QStringList(str), 0, caret_pos);
}


void 
PreeditArea::showConvertingSegments(const QStringList& strlist, uint attention)
{
  Q_ASSERT( !strlist.empty() );
  _state = Attention;
  _attentseg->setFont( _font[Attention] ); 
  _attentseg->setPaletteForegroundColor( _colorlist[FgAttention] );
  _attentseg->setPaletteBackgroundColor( _colorlist[BgAttention] );

  _backseg->setFont( _font[Converted] ); 
  _backseg->setPaletteForegroundColor( _colorlist[FgConverted] );
  _backseg->setPaletteBackgroundColor( _colorlist[BgConverted] );

  _forwardseg->setFont( _font[Converted] ); 
  _forwardseg->setPaletteForegroundColor( _colorlist[FgConverted] );
  _forwardseg->setPaletteBackgroundColor( _colorlist[BgConverted] );

  showText(strlist, attention);
}


void 
PreeditArea::showChangingSegmentLength(const QStringList& strlist, uint attention)
{
  Q_ASSERT( !strlist.empty() );
  _state = Changing;
  _attentseg->setFont( _font[Changing] ); 
  _attentseg->setPaletteForegroundColor( _colorlist[FgChanging] );
  _attentseg->setPaletteBackgroundColor( _colorlist[BgChanging] );

  _backseg->setFont( _font[Converted] ); 
  _backseg->setPaletteForegroundColor( _colorlist[FgConverted] );
  _backseg->setPaletteBackgroundColor( _colorlist[BgConverted] );

  _forwardseg->setFont( _font[Converted] ); 
  _forwardseg->setPaletteForegroundColor( _colorlist[FgConverted] );
  _forwardseg->setPaletteBackgroundColor( _colorlist[BgConverted] );

  showText(strlist, attention);
}


void
PreeditArea::adjustSize()
{
  QWidget::adjustSize();

  const QSize hmargin(0, 2);
  if ( !_attentseg->text().isEmpty() ) {
    _backseg->resize( _backseg->sizeHint() + hmargin );

    QPoint p(0, 0);
    if ( !_backseg->text().isEmpty() )
      p += QPoint(_backseg->size().width(), 0);
  
    _attentseg->move( p );
    _attentseg->resize( _attentseg->sizeHint() + hmargin );
    emit listPointChanged( mapToGlobal(p + QPoint(0, _attentseg->height()) + QPoint(-4, 1)) );   // Current segment pos
    
    p += QPoint(_attentseg->size().width(), 0);
    _forwardseg->move( p );
    _forwardseg->resize( _forwardseg->sizeHint() + hmargin );

    if ( !_forwardseg->text().isEmpty() )
      p += QPoint(_forwardseg->size().width(), 0);
  
    resize( QSize(p.x(), _attentseg->size().height()) );
    qDebug("back: %s  attention: %s  forward: %s", _backseg->text().local8Bit().data(),
	   _attentseg->text().local8Bit().data(), _forwardseg->text().local8Bit().data());
  }
}


void
PreeditArea::movePreeditPos(const QPoint& pos)
{
  int offset = QFontMetrics(_attentseg->font()).height() - 1;
  move(pos - QPoint(0, offset));

  if (KimeraApp::inputmethod()->currentXIMStyle() == ON_THE_SPOT_STYLE)
    emit listPointChanged(pos + QPoint(12, 1));   // Current segment pos
}


QString
PreeditArea::text() const
{
  return _backseg->text() + _attentseg->text() + _forwardseg->text();
}


QString
PreeditArea::backwardText() const
{
  return _backseg->text();
}


QString
PreeditArea::attentionText() const
{
  return _attentseg->text();
}


QString
PreeditArea::forwardText() const
{
  return _forwardseg->text();
}


void
PreeditArea::hide()
{
  if (KimeraApp::inputmethod()->currentXIMStyle() == ON_THE_SPOT_STYLE) {
    KimeraApp::inputmethod()->sendPreeditString();  // Clear string
  }
  _attentseg->setText(QString::null);
  _backseg->setText(QString::null);
  _forwardseg->setText(QString::null);
  QWidget::hide();
}
