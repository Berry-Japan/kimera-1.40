#include "mainform.h"
#include "kimeraglobal.h"
#include "kimeraapp.h"
#include "inputmethod.h"
#include "inputmode.h"
#include "propertydialog.h"
#include "keyassigner.h"
#include "config.h"
#include "kanjiengine.h"
#include "debug.h"
#include <qtoolbar.h>
#include <qvariant.h>
#include <qtoolbutton.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <qwhatsthis.h>
#include <qstring.h>
#include <qtimer.h>
#include <qmessagebox.h>
using namespace Kimera;

static const char* property_xpm[] = {
  "16 16 79 2",
  "   c None",
  ".a c #383858",
  ".e c #5a5a79",
  ".K c #676786",
  ".o c #6a6a88",
  ".8 c #6d6d8c",
  ".0 c #71718f",
  "#j c #737391",
  ".P c #757592",
  "#e c #767693",
  "#m c #777795",
  ".t c #797b97",
  "#i c #7b7b98",
  ".3 c #7c7c98",
  ".2 c #7e7e9a",
  "#d c #7f7e9a",
  "#h c #7f7e9b",
  ".1 c #80809b",
  ".f c #82829e",
  "#c c #83829e",
  ".# c #83839f",
  "## c #8886a2",
  ".Q c #8989a3",
  "#. c #8b8ba6",
  ".7 c #8f8fa9",
  ".6 c #9595ae",
  "#l c #9797af",
  ".Z c #9a9ab2",
  ".Y c #9f9fb6",
  "#g c #a1a1b8",
  ".X c #a5a5bb",
  ".O c #a5a6bc",
  "#b c #a8a8bd",
  ".N c #aaabbf",
  ".J c #abaabf",
  ".9 c #afafc3",
  ".E c #b0afc4",
  ".I c #b0b0c4",
  "#k c #b1b1c5",
  "#f c #b2b2c5",
  "#a c #b5b5c8",
  ".z c #b6b6c9",
  ".M c #b7b7c9",
  ".5 c #b8b8ca",
  ".4 c #b9b9cb",
  ".x c #bcbccc",
  ".W c #c1c0d1",
  ".r c #c1c1d1",
  ".q c #c7c5d5",
  ".w c #c7c7d5",
  ".p c #cbcbd8",
  ".D c #cbcbda",
  ".n c #cbccda",
  ".H c #ccccd8",
  ".v c #ccccda",
  ".m c #cfd0dc",
  ".k c #d0cfdc",
  ".u c #d0d0dc",
  ".j c #d5d5e0",
  ".C c #d8d8e2",
  ".G c #d8d9e2",
  ".l c #d9d8e2",
  ".i c #d9d9e2",
  ".h c #dbdbe6",
  ".d c #dddbe6",
  ".B c #dddde6",
  ".g c #dfdfe8",
  ".c c #e0dfe8",
  ".y c #e0e0e8",
  ".V c #e1e1e9",
  ".b c #e2e2e9",
  ".U c #e3e3eb",
  ".T c #e5e5ec",
  ".S c #e8e8ed",
  ".R c #eaeaf0",
  ".L c #ededf2",
  ".F c #f1f1f5",
  ".A c #f2f2f5",
  ".s c #f3f3f6",
  "        .#.#.#.#.#              ",
  "        .a.b.b.c.d.e.e          ",
  "        .f.a.g.h.i.j.k.e        ",
  "          .f.a.l.j.m.n.e        ",
  ".o.o        .f.#.k.p.q.r.e      ",
  ".o.s.#      .t.u.v.w.r.x.e      ",
  ".o.s.y.#  .t.u.p.w.r.x.z.e      ",
  ".o.A.B.C.t.u.D.w.r.x.z.E.e      ",
  ".o.F.G.j.k.H.w.r.x.z.I.J.e      ",
  ".o.K.L.u.v.w.r.x.M.I.N.O.P.e    ",
  "  .a.Q.R.S.T.U.V.u.W.X.Y.Z.0.e  ",
  "  .f.a.a.f.1.2.3.f.4.5.Z.6.7.8.e",
  "      .f.a.a.a.a.a.2.M.9.7#.##.o",
  "                .f.a.t#a#b###c#d",
  "                  .f.a#e#f#g#h#i",
  "                    .f.a#j#k#l#m"
};

static char* dict_xpm[] = {
  "16 16 69 1",
  "< c None",
  "  c #643634",
  ". c #A48A6C",
  "X c #7C2624",
  "o c #AC6A74",
  "O c #B4A18C",
  "+ c #94765C",
  "@ c #CCB0B4",
  "# c #9C5E64",
  "$ c #98161B",
  "% c #C0AEA8",
  "& c #A05254",
  "* c #943A34",
  "= c #5C1514",
  "- c #891B1C",
  "> c #9C8684",
  "1 c #946E54",
  "2 c #A86054",
  "3 c #C4B6A4",
  "4 c #BDB6B4",
  "5 c #CC8684",
  "6 c #704244",
  "8 c #9C7E7C",
  "9 c #981A1C",
  "0 c #B4726C",
  "q c #8C3331",
  "w c #7C1214",
  "e c #740E0C",
  "r c #541A14",
  "t c #846264",
  "u c #AF9E84",
  "p c #670D0C",
  "a c #AC9A84",
  "s c #B4868C",
  "d c #6C3A38",
  "f c #882A2C",
  "g c #E0C2C4",
  "j c #9C7274",
  "k c #AC6C5C",
  "l c #C4BABC",
  "z c #542420",
  "x c #BCB2B4",
  "c c #D4A6A4",
  "v c #945644",
  "b c #7C5654",
  "n c #9C464C",
  "M c #6C4644",
  "N c #9C9294",
  "B c #A97D6C",
  "V c #AC987C",
  "C c #A48E74",
  "Z c #BC6E64",
  "A c #B49194",
  "S c #B4625C",
  "D c #D4C6C7",
  "F c #DCA2A4",
  "G c #946250",
  "H c #BCAAAC",
  "J c #8C5A5C",
  "K c #846654",
  "L c #B4A2A4",
  "P c #7C4E4C",
  "I c #8B1619",
  "U c #94483C",
  "Y c #C4928C",
  "T c #94423C",
  "R c #C48A8C",
  "E c #6C463C",
  "W c #9C8E8C",
  "<<<<<<<<<<@AA<<<",
  "<D#&nqX9I-wIX<<<",
  "<dpIII$II$wI-g<<",
  "g pI9$$$IIII-s<<",
  "LMpI$$99$I9I-&<<",
  "H6rw$$$$$$9I9f<<",
  "4P=w$9$99I99--@<",
  "xPze9$9999$9I-o<",
  "%P pI9$9$I99IIq<",
  "lJ p$9$$99IIII-D",
  "xj =I9$$99I$--XH",
  "x86zq*TTTUvG1+.<",
  "x>6ECVVVVaauaV3<",
  "4WbKauOOOOOaaV%<",
  "4WtGBBBkkS2S0Z5<",
  "4NARYcFg<<<<<<<<",
};


MainForm::MainForm()
  : QFrame(0, "MainForm", WStyle_Customize | WStyle_StaysOnTop | WStyle_NoBorder | WX11BypassWM | WGroupLeader | WDestructiveClose),
    _mainwin(0),
    _tbar(0),
    _leftmover(0),
    _toolbtn1(0),
    _toolbtn2(0),
    _toolbtn3(0),
    _toolbtn4(0),
    _pupmenu1(0),
    _pupmenu2(0),
    _pupmenu3(0),
    _propdlg(0),
    _modemap(),
    _labelmap(),
    _handwriting(0)
{
  setFrameStyle(QFrame::StyledPanel | QFrame::Raised);
  setLineWidth( 1 );

  QFont f("gothic", 9);
  _mainwin = new QMainWindow(this, "main window", WStyle_Customize | WStyle_NoBorder);
  _mainwin->setGeometry(QRect(3, 3, 200, 30));
  _mainwin->setDockMenuEnabled(FALSE);

  const QSize btnsize(30, 26);
  _tbar = new QToolBar(_mainwin, "main bar");
  _tbar->setMargin( 0 );
  _tbar->setMovingEnabled(FALSE);
  _tbar->setFont( f );  
  _leftmover = new Mover( _tbar, "left mover" );
  _leftmover->setMaximumHeight( btnsize.height() );
  _toolbtn1 = new QToolButton( _tbar, "ToolButton1" );
  _toolbtn1->setMaximumSize( btnsize );
  _toolbtn2 = new QToolButton( _tbar, "ToolButton2" );
  _toolbtn2->setMaximumSize( btnsize + QSize(6, 0) );
  _toolbtn3 = new QToolButton(_tbar, "ToolButton3");
  _toolbtn3->setIconSet( QIconSet(QPixmap((const char**)dict_xpm)) );
  _toolbtn3->setMaximumSize( btnsize );
  _toolbtn4 = new QToolButton(QIconSet(QPixmap((const char**)property_xpm)), tr("プロパティ"), QString::null,
			      this, SLOT(showPropertyDialog()), _tbar, "ToolButton4" );
  _toolbtn4->setMaximumSize( btnsize );
  _pupmenu1 = new Popup( _tbar );
  _pupmenu2 = new Popup( _tbar );
  _pupmenu3 = new Popup( _tbar );
  _propdlg  = new PropertyDialog(this, "property dialog");
  _handwriting = KanjiEngine::kanjiEngine("Tomoe");
  Q_CHECK_PTR( _handwriting );

  _toolbtn1->setTextLabel("_A", FALSE);
  _toolbtn1->setUsesTextLabel(TRUE);
  _toolbtn2->setTextLabel(tr("ロ-マ"), FALSE);
  _toolbtn2->setUsesTextLabel(TRUE);

  _pupmenu1->clear();
  _pupmenu1->insertItem(tr("ひらがな"), Mode_Hiragana);
  _pupmenu1->insertItem(tr("カタカナ"), Mode_Katakana);
  _pupmenu1->insertItem(tr("半角カナ"), Mode_HankakuKana);
  _pupmenu1->insertItem(tr("全角英数"), Mode_ZenkakuEisu);
  _pupmenu1->insertItem(tr("直接入力"), Mode_DirectInput);
  _pupmenu1->setCheckable(TRUE);

  _pupmenu2->clear();
  _pupmenu2->insertItem(tr("ローマ字入力"), Mode_RomaInput);
  _pupmenu2->insertItem(tr("かな入力"), Mode_KanaInput);
  _pupmenu2->setCheckable(TRUE);

  _pupmenu3->clear();
  _pupmenu3->insertItem(tr("辞書ツール"), this, SLOT(execDictTool()));
  _pupmenu3->insertItem(tr("手書き認識"), this, SLOT(execHandWritingTool()));
  _pupmenu3->setCheckable(TRUE);

  _toolbtn1->setPopup(_pupmenu1);
  _toolbtn1->setPopupDelay(1);
  _toolbtn2->setPopup(_pupmenu2);
  _toolbtn2->setPopupDelay(1);
  _toolbtn3->setPopup(_pupmenu3);
  _toolbtn3->setPopupDelay(1);
  
  _modemap.setAutoDelete( TRUE );
  _modemap.insert(tr("ひらがな"),     new int(Mode_Hiragana));
  _modemap.insert(tr("カタカナ"),     new int(Mode_Katakana));
  _modemap.insert(tr("半角カナ"),     new int(Mode_HankakuKana));
  _modemap.insert(tr("全角英数"),     new int(Mode_ZenkakuEisu));
  _modemap.insert(tr("ローマ字入力"), new int(Mode_RomaInput));
  _modemap.insert(tr("かな入力"),     new int(Mode_KanaInput));
  _modemap.insert(tr("直接入力"),     new int(Mode_DirectInput));

  _labelmap.setAutoDelete( TRUE );
  _labelmap.insert(Mode_Hiragana,    new QString(tr("あ")));
  _labelmap.insert(Mode_Katakana,    new QString(tr("ア")));
  _labelmap.insert(Mode_HankakuKana, new QString(tr("_ｱ")));
  _labelmap.insert(Mode_ZenkakuEisu, new QString(tr("Ａ")));
  _labelmap.insert(Mode_RomaInput,   new QString(tr("ロ-マ")));
  _labelmap.insert(Mode_KanaInput,   new QString(tr("かな ")));
  _labelmap.insert(Mode_DirectInput, new QString(tr("_A")));

  // Sets tooltip
  QToolTip::add(_toolbtn1, tr("入力モード"));
  QToolTip::add(_toolbtn2, tr("入力方式"));
  QToolTip::add(_toolbtn3, tr("ツール"));

  _mainwin->resize(_tbar->sizeHint());
  resize(_tbar->sizeHint() + QSize(8, 8));
  
  // signals and slots connections
  InputMethod* im = KimeraApp::inputmethod();
  connect(im, SIGNAL(triggerNotify(bool)), this, SLOT(slotTriggerNotify(bool)));
  connect(this, SIGNAL(triggerNotify(bool)), im, SLOT(setXIMInputtingEnabled(bool)));
  connect(this, SIGNAL(selected(const InputMode&)), im->kanjiConvert(), SLOT(setInputMode(const InputMode&)));
  connect(im->kanjiConvert(), SIGNAL(inputModeChanged(const InputMode&)), this, SLOT(update(const InputMode&)));
  connect(_pupmenu1,   SIGNAL(activated(int)), this, SLOT(setButton1Text(int)));
  connect(_pupmenu2,   SIGNAL(activated(int)), this, SLOT(setButton2Text(int))); 
  connect(_leftmover,  SIGNAL(mouseMoved(const QPoint&)), this, SLOT(move(const QPoint&)));
  connect(_leftmover,  SIGNAL(mouseMoveStopped(const QPoint&)), this, SLOT(savePos()));
  connect(im->kanjiConvert(), SIGNAL(dictToolActivated()), this, SLOT(execDictTool()));
  connect(im->kanjiConvert(), SIGNAL(propertyDialogActivated()), this, SLOT(showPropertyDialog()));
  connect(_propdlg, SIGNAL(settingChanged()), this, SLOT(initIM()));
  connect(_handwriting, SIGNAL(decided(const QString&)), this, SLOT(slotDecided(const QString&)));
  connect(this, SIGNAL(decided(const QString&)), KimeraApp::inputmethod()->kanjiConvert(), SIGNAL(decideSegments(const QString&)));

  for (int i = 1; i <= 20; ++i)
    QTimer::singleShot(5000 * i, this, SLOT(raise()));  // To stay on top at starting up on Gnome

  // Save Default setting
  PropertyDialog::saveDefaultSetting();
  KeyAssigner::saveDefaultSetting();
}


MainForm::~MainForm()
{
  // Do nothing
}


void
MainForm::initIM()
{
  DEBUG_TRACEFUNC();
  KimeraApp::inputmethod()->kanjiConvert()->init();
  if ( isHideState() )
    hide();
}


void
MainForm::polish()
{
  DEBUG_TRACEFUNC();
  int* p = _modemap[ Config::readEntry("_cmbinputmode", "") ];
  if ( p ) {
    setButton1Text( *p );   // Default value
  }
  setButton1Text( Mode_DirectInput );

  p = _modemap[ Config::readEntry("_cmbinputstyle", "") ];
  if ( p ) {
    setButton2Text( *p );
  }

  if ( isHideState() )
    QTimer::singleShot(0, this, SLOT(hide()));

  QWidget::polish();
}


void
MainForm::setButton1Text(int id)
{
  DEBUG_TRACEFUNC("id: 0x%04x", id);
  id &= Mode_ModeMask | Mode_DirectInput;
  if ( _pupmenu1->text(id).isEmpty() )
    return;

  int oldid = 0;
  for (int i = 0; i < (int)_pupmenu1->count(); ++i) {
    if ( _pupmenu1->isItemChecked(_pupmenu1->idAt(i)) ) {
      oldid = _pupmenu1->idAt(i);
      _pupmenu1->setItemChecked(oldid, FALSE);
      break;
    }
  }
  _pupmenu1->setItemChecked(id, TRUE);

  QString* p = _labelmap[id];
  if ( p )
    _toolbtn1->setTextLabel(*p, FALSE);

  if ( !(id & Mode_DirectInput) )
    emit selected(InputMode(id));

  // Check on/off changed
  if ((id & Mode_DirectInput) != (oldid & Mode_DirectInput)) {
    bool on = !(id & Mode_DirectInput);
    emit triggerNotify( on );
  }
}


void
MainForm::setButton2Text(int id)
{ 
  DEBUG_TRACEFUNC("id: 0x%04d", id);
  id &= Mode_InputMask;
  if ( _pupmenu2->text(id).isEmpty() )
    return;
 
  int oldid = 0;
  for (int i = 0; i < (int)_pupmenu2->count(); ++i) {
    if ( _pupmenu2->isItemChecked(_pupmenu2->idAt(i)) ) {
      oldid = _pupmenu2->idAt(i);
      _pupmenu2->setItemChecked(oldid, FALSE);
      break;
    }
  }
  _pupmenu2->setItemChecked(id, TRUE);

  QString* p = _labelmap[id];
  if ( p )
    _toolbtn2->setTextLabel(*p, FALSE);

  if (id != oldid)
    emit selected(InputMode(id));
}


void
MainForm::slotTriggerNotify(bool b)
{
  DEBUG_TRACEFUNC("b: %d", b); 
  
  setButton1Text( b ? KimeraApp::inputmethod()->kanjiConvert()->inputMode().id() : Mode_DirectInput );

  if ( isHideState() ) {
    hide();
  } else {
    clearFocus();
    show();
    raise();
    _mainwin->resize(_tbar->sizeHint());
    resize(_tbar->sizeHint() + QSize(8, 8));
  }
}


void
MainForm::execDictTool()
{
  DEBUG_TRACEFUNC();

  static QProcess* proc = 0;
  if (!proc) {
    proc = new QProcess(this);
    proc->setWorkingDirectory(QDir::homeDirPath());
  }
  if (!proc->isRunning()) {
    QStringList cmd = QStringList::split(' ', Config::readEntry("_cmbcmd", ""));
    if (!cmd.isEmpty()) {
      proc->setArguments(cmd);
      if ( !proc->start() ) {
        QMessageBox::warning(0, "Execution failed",
			     tr("辞書ツール（%1）を起動できませんでした").arg(cmd[0]),
			     QMessageBox::Ok | QMessageBox::Default, 0);
        }
    } else {
      QMessageBox::warning(0, "Empty command",
                           tr("辞書ツールが設定されていません。\n"
                              "プロパティからコマンドを設定してください。"),
                           QMessageBox::Ok | QMessageBox::Default, 0);
    }
  }
}


void
MainForm::execHandWritingTool()
{
  DEBUG_TRACEFUNC();

  slotTriggerNotify( TRUE );
  if ( !_handwriting->init() ) {
    QMessageBox::warning(0, "Execution failed",
                         tr("手書き認識ツール（kimera-tomoe-gtk）を起動できませんでした"),
                         QMessageBox::Ok | QMessageBox::Default, 0);
  }
}


void
MainForm::slotDecided(const QString& string)
{
  DEBUG_TRACEFUNC("string: %s", string.local8Bit().data());

  if ( !KimeraApp::isXIMInputtingEnabled() )
    slotTriggerNotify( TRUE );

  emit decided(string);
}


void
MainForm::showPropertyDialog()
{
  DEBUG_TRACEFUNC();
  _propdlg->show();
  _propdlg->raise();
}


bool
MainForm::isHideState()
{
  DEBUG_TRACEFUNC();
  return !KimeraApp::isXIMInputtingEnabled() && Config::readBoolEntry("_chkdispbar");
}


void
MainForm::savePos()
{
  DEBUG_TRACEFUNC();
  Config::writeEntry("point_x", x());
  Config::writeEntry("point_y", y());
  qDebug("saved (%d, %d)", x(), y());
}


QPoint
MainForm::loadPos() const
{
  DEBUG_TRACEFUNC();
  bool okx, oky;
  int x = Config::readNumEntry("point_x", 0, &okx);
  int y = Config::readNumEntry("point_y", 0, &oky);
  if (!okx || !oky) {
    x = QApplication::desktop()->width() - width() - 50;
    y = QApplication::desktop()->height() - 120;
  }
   
  x = QMIN(QMAX(x, 0), QApplication::desktop()->width() - width());
  y = QMIN(QMAX(y, 0), QApplication::desktop()->height() - height()); 
  return QPoint(x, y);
}


void
MainForm::update(const InputMode& mode)
{
  DEBUG_TRACEFUNC("mode: 0x%04x", mode.id());
  setButton1Text(mode.id());
  setButton2Text(mode.id());
}


/**
 * Mover class
 */
Mover::Mover(QWidget* parent, const char* name) : QToolButton(parent, name)
{
  setMaximumWidth(14);
}


void
Mover::mousePressEvent(QMouseEvent* e)
{
  _p = e->globalPos();
  if ( topLevelWidget() ) {
    _p -= topLevelWidget()->pos();
  }
  
  QToolButton::mousePressEvent(e);
}


void
Mover::mouseReleaseEvent(QMouseEvent* e)
{
  emit mouseMoveStopped(e->globalPos() - _p);
  QToolButton::mouseReleaseEvent(e);
}


void
Mover::mouseMoveEvent(QMouseEvent* e) 
{
  emit mouseMoved(e->globalPos() - _p);
  QToolButton::mouseMoveEvent(e);
}


void
Mover::enterEvent(QEvent* e)
{
  QApplication::setOverrideCursor( QCursor(Qt::SizeAllCursor) );
  QToolButton::enterEvent(e);
}


void
Mover::leaveEvent(QEvent* e)
{
  QApplication::restoreOverrideCursor();
  QToolButton::leaveEvent(e);
}
