#include "kimeraapp.h"
#include "inputmethod.h"
#include "config.h"
#include <qregexp.h>

static InputMethod* inpmethod = 0;

KimeraApp::KimeraApp(int &argc, char **argv) : QApplication(argc, argv)
{
  qAddPostRoutine( cleanup_ptr );   // delete object

  InputMethod::registerXIMServerKimera();  // Register Kimera to X
}

//
// Returns a pointer to the application global inputmethod.
//
InputMethod* 
KimeraApp::inputmethod()
{
  if ( !inpmethod ) {
    inpmethod = new InputMethod();
    Q_CHECK_PTR( inpmethod );
  }
  
  return inpmethod;
}


void
KimeraApp::cleanup_ptr()
{
  if (inpmethod)
    delete inpmethod;
  inpmethod = 0;
}


bool 
KimeraApp::x11EventFilter(XEvent* event)
{
  if ( !inpmethod )
    return FALSE;

  if (event->type == ClientMessage) {
    ClientMessageEvent e(event);
    sendEvent(inpmethod, &e);
    return FALSE;
  } else if (event->type >= SelectionClear && event->type <= SelectionNotify) {
    SelectionEvent e(event);
    sendEvent(inpmethod, &e);
    return TRUE;
  } 
  
  return FALSE;
}


bool
KimeraApp::isXIMInputtingEnabled()
{
  return  inputmethod()->isXIMInputtingEnabled();
}
