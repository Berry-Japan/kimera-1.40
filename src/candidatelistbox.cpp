#include "candidatelistbox.h"
#include "inputmethod.h"
#include "ximattribute.h"
#include "inputmethod.h"
#include <qapplication.h>
#include <qlistbox.h>
#include <qlabel.h>

const int MARGIN = 6;
const int CURRENT_ITEM_LABEL_HEIGHT = 20;

CandidateListBox::CandidateListBox() 
  :  QFrame(0, "candidatelistbox", WStyle_StaysOnTop | WStyle_Customize | WStyle_NoBorder | WStyle_Tool | WX11BypassWM),
     _pos()
{
  setLineWidth( 1 );
  setFrameStyle( QFrame::Plain | QFrame::Box );

  _lbl = new QListBox(this, "no");
  _lbl->move(1, 1);
  _lbl->setSelectionMode( QListBox::NoSelection );
  _lbl->setFrameStyle( QFrame::NoFrame );
  _lbl->setColumnMode( 1 );   // Number of column is 1
  _lbl->setHScrollBarMode( QListBox::AlwaysOff );
  _lbl->setVScrollBarMode( QListBox::AlwaysOff );
  _lbl->setPaletteBackgroundColor(QColor(228, 228, 228));

  _lstbox = new QListBox(this, "candidatelist");
  _lstbox->move(17, 1);
  _lstbox->setFrameStyle( QFrame::NoFrame );
  _lstbox->setColumnMode( 1 );   // Number of column is 1
  _lstbox->setHScrollBarMode( QListBox::AlwaysOff );
  _lstbox->setVScrollBarMode( QListBox::AlwaysOn );

  _crntitem = new QLabel(this, "currentitemlabel");
  _crntitem->setFrameStyle( QFrame::Plain | QFrame::Box );
  _crntitem->setLineWidth( 1 );
  _crntitem->setAlignment( Qt::AlignRight );

  _predictxt = new QLabel(this, "predictiontext");
  _predictxt->setFrameStyle( QFrame::NoFrame );
  _predictxt->setMargin( 2 );
  _predictxt->setLineWidth( 0 );
  _predictxt->setPaletteBackgroundColor(QColor(255, 255, 242));
  _predictxt->move(1, 1);

  connect(_lstbox, SIGNAL(clicked(QListBoxItem*)), this, SIGNAL(clicked(QListBoxItem*)));
  connect(_lstbox, SIGNAL(highlighted(int)), this, SLOT(setLabelValue(int)));
  connect(_lstbox->verticalScrollBar(), SIGNAL(sliderReleased()), this, SLOT(adjustDisplay()));
}


void
CandidateListBox::showList(const QStringList& strs, int crntidx)
{
  Q_ASSERT( !strs.isEmpty() );
  _lstbox->clear();
  _lstbox->resize(0, 0);
  _lstbox->insertStringList(strs);
  _lstbox->setCurrentItem((crntidx < (int)_lstbox->count()) ? crntidx : _lstbox->count() - 1);
  _lstbox->ensureCurrentVisible();

  int w = _lstbox->contentsWidth() + 30;
  int h = _lstbox->itemHeight() * MAX_VISIBLE_ITEMS;
  _lstbox->verticalScrollBar()->setEnabled( (_lstbox->count() > (uint)MAX_VISIBLE_ITEMS) );
  _lstbox->resize(w, h);

  _lbl->clear();
  _lbl->resize(_lbl->width(), h);
  for (int i = 1; i <= QMIN((int)_lstbox->count(), 9); ++i)
    _lbl->insertItem( QString::number(i) );

  _crntitem->move(0, _lstbox->itemHeight() * MAX_VISIBLE_ITEMS + 1);
  _crntitem->resize(w + _lbl->width() + 2, CURRENT_ITEM_LABEL_HEIGHT);

  resize(w + _lbl->width() + 2, h + CURRENT_ITEM_LABEL_HEIGHT + 1);
  qDebug("CandidateListBox w:%d h:%d", w, h);

  _predictxt->hide();
  _lbl->show();
  _lstbox->show();
  _crntitem->show();
  show();
  adjustDisplay();
}


void
CandidateListBox::selectCandidate(int idx)
{
  if (isListVisible()) {
    _lstbox->setCurrentItem((idx < (int)_lstbox->count()) ? idx : _lstbox->count() - 1);
    _lstbox->ensureCurrentVisible();
    adjustDisplay();
  }
}


void
CandidateListBox::selectPreviousCandidate(uint decrement)
{
  if ( isHidden() ) {
    qFatal("Assert  %s:%d", __FILE__, __LINE__);
    return;
  } else if ( !_lstbox->count() ) {
    Q_ASSERT( _lstbox->count() );
    return;
  }

  int prev = _lstbox->currentItem() - decrement;
  if (prev < 0) {
    if (_lstbox->topItem() > 0) {
      _lstbox->setCurrentItem(0);
    } else {
      _lstbox->setCurrentItem(_lstbox->count() - 1);
    }

  } else {
    if (prev < _lstbox->topItem()) {
      int top = (decrement == MAX_VISIBLE_ITEMS) ? prev : prev - MAX_VISIBLE_ITEMS + 1;
      _lstbox->setTopItem(top);
    }
    _lstbox->setCurrentItem(prev);
  }

  _lstbox->ensureCurrentVisible();
  adjustDisplay();
}


void
CandidateListBox::selectNextCandidate(uint increment)
{
  if ( isHidden() ) {
    qFatal("Assert  %s:%d", __FILE__, __LINE__);
    return;
  } else if ( !_lstbox->count() ) {
    Q_ASSERT( _lstbox->count() );
    return;
  }

  int next = _lstbox->currentItem() + increment;
  if (next < (int)_lstbox->count()) {
    if (next >= _lstbox->topItem() + MAX_VISIBLE_ITEMS) {
      _lstbox->setTopItem(next);
    }
    _lstbox->setCurrentItem(next);

  } else {
    _lstbox->setCurrentItem(0);
  }

  _lstbox->ensureCurrentVisible();
  adjustDisplay();
}


void
CandidateListBox::showEvent(QShowEvent* e)
{
  // Adjusts pos
  const int offset = 2;
  int x = _pos.x() + 1;
  if ( !_predictxt->isVisible() )
    x -= _lbl->width();
  x = QMIN(x, QApplication::desktop()->width() - width() - offset);
  int y = _pos.y();
  if (y > QApplication::desktop()->height() - height() - offset) {
    if (KimeraApp::inputmethod()->currentXIMStyle() != ON_THE_SPOT_STYLE) {
      QFontMetrics fm( font() ); 
      y -= height() + fm.height() + 1;
    } else {
      y = QMIN(y, QApplication::desktop()->height() - height() - offset);
    }
  }
  move(QMAX(x, 0), QMAX(y, 0));
  QFrame::showEvent(e);
}


void
CandidateListBox::hideEvent(QHideEvent* e)
{
  _lstbox->clear();
  QFrame::hideEvent(e);
}


void
CandidateListBox::setFont(const QFont& f)
{
  QFont font(f);
  if (QFontInfo(font).pixelSize() > 20)
    font.setPixelSize(20);
  
  _lbl->setFont(font);
  QFontMetrics fm(font);
  int w = fm.width("0") * 2;
  _lbl->resize(w, _lbl->height());

  _lstbox->move(w + 1, 1);
  _lstbox->setFont(font);
  
  QFrame::setFont(font);

  font.setPixelSize(12);
  _crntitem->setFont( font );
}


bool
CandidateListBox::isListVisible() const
{
  return isVisible() && !_predictxt->isVisible();
}


bool
CandidateListBox::isPredictionsVisible() const
{
  return isVisible() && _predictxt->isVisible();
}


void
CandidateListBox::setPos(const QPoint& p)
{
  _pos = p;
}


void
CandidateListBox::showCandidatePredicted(const QString& str)
{
  if (!isVisible() || _predictxt->isVisible()) {
    _predictxt->setText(str);
    if (!str.isEmpty()) {
      _predictxt->adjustSize();
      _predictxt->show();
      _lbl->hide();
      _lstbox->hide();
      _crntitem->hide();
      adjustSize();
      show();
    } else {
      _predictxt->hide();
      hide();
    }
  }
}


QString
CandidateListBox::candidatePredicted() const
{
  return _predictxt->isVisible() ? _predictxt->text() : QString::null;
}


QString
CandidateListBox::currentText() const
{
  return _lstbox->currentText();
}

void
CandidateListBox::setLabelValue(int d)
{
  QString text = QString("%1/%2").arg(d + 1).arg(_lstbox->count());
  _crntitem->setText(text);
}


void
CandidateListBox::adjustDisplay()
{
  _lstbox->setTopItem( _lstbox->topItem() );
}


QString
CandidateListBox::text(int index) const
{
  return  _lstbox->text(index);
}


int
CandidateListBox::topItem() const
{
  return _lstbox->topItem();
}

uint
CandidateListBox::count() const
{
  return _lstbox->count();
}
