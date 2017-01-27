#ifndef MAINFORM_H
#define MAINFORM_H

#include "kimeraapp.h"
#include "kanjiconvert.h"
#include <qframe.h>
#include <qvariant.h>
#include <qtoolbutton.h>
#include <qpopupmenu.h>
#include <qcursor.h>
#include <qprocess.h>
#include <qmainwindow.h>

class QMainWindow;
class QToolBar;
class QVBoxLayout; 
class QHBoxLayout; 
class QGridLayout; 
class QToolButton;
class QPopupMenu;
class InputMethod;
class InputMode;
class PropertyDialog;
class Mover;
class Popup;
class KanjiEngine;

/**
 *
 */
class MainForm : public QFrame { 
  Q_OBJECT
  
public:
  MainForm();
  ~MainForm();

  QPoint loadPos() const;
  bool isHideState();

public slots:
  void  polish();
  void  slotTriggerNotify(bool);
  void  initIM();
  void  savePos();
  void  update(const InputMode&);

signals:
  void  selected(const InputMode&);
  void  triggerNotify(bool);
  void  decided(const QString&);

protected slots:
  void  setButton1Text(int id);
  void  setButton2Text(int id);
  void  execDictTool();
  void  execHandWritingTool();
  void  slotDecided(const QString& string);
  void  showPropertyDialog();

private:
  QMainWindow*      _mainwin;
  QToolBar*         _tbar;
  Mover*            _leftmover;
  QToolButton*      _toolbtn1;
  QToolButton*      _toolbtn2;
  QToolButton*      _toolbtn3;
  QToolButton*      _toolbtn4;
  Popup*            _pupmenu1;
  Popup*            _pupmenu2;
  Popup*            _pupmenu3;
  PropertyDialog*   _propdlg;
  QDict<int>        _modemap;
  QIntDict<QString> _labelmap;
  KanjiEngine*      _handwriting;
};


/**
 *
 */
class Mover : public QToolButton {
  Q_OBJECT
    
public:
  Mover(QWidget* parent, const char* name=0);

signals:
  void mouseMoved(const QPoint& pos);
  void mouseMoveStopped(const QPoint& pos);

protected:
  void mousePressEvent(QMouseEvent* e);
  void mouseReleaseEvent(QMouseEvent* e);
  void mouseMoveEvent(QMouseEvent* e);
  void enterEvent(QEvent* e);
  void leaveEvent(QEvent* e);

private:
  QPoint _p;
};


/**
 *
 */
class Popup : public QPopupMenu {
 Q_OBJECT

public:
  Popup(QWidget* parent=0, const char* name=0) : QPopupMenu(parent, name) { }

public slots:
  void hide() { clearFocus(); QPopupMenu::hide(); }
};

#endif // MAINFORM_H
