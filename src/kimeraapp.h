#ifndef KIMERAAPP_H
#define KIMERAAPP_H

#include <qapplication.h>
#include <qevent.h>

class InputMethod;
class KanjiEngine;


class KimeraApp : public QApplication {
public:  
  KimeraApp(int &argc, char **argv);
  ~KimeraApp() { }
  
  static InputMethod* inputmethod();
  static bool isXIMInputtingEnabled();

  enum Type {
    XIMClientMessage = 2000,
    Selection = 2001,
    InputMode = 2002
  };

protected:
  bool x11EventFilter(XEvent *);
  static void cleanup_ptr();
};


//
// Selection event class
//
class SelectionEvent : public QCustomEvent {
public:
  SelectionEvent(void* data) : QCustomEvent((Type)KimeraApp::Selection, data) {}
};


//
// ClientMessage event class
//
class ClientMessageEvent : public QCustomEvent {
public:
  ClientMessageEvent(void* data) : QCustomEvent((Type)KimeraApp::XIMClientMessage, data) {}
};



#endif // KIMERAAPP_H






