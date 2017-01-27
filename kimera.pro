TEMPLATE        = app
CONFIG         += qt x11 warn_on release
TARGET          = kimera-bin
VERSION         = 1.40
isEmpty( target.path ) {
  target.path   = /usr/lib/kimera-$$VERSION
}
system( sed -e 's:^\(KIMERA_BIN=\).*\(kimera-bin\)$:\1$$target.path/\2:' tools/kimera > kimera && chmod +x kimera )
message(This project will be installed in $$target.path)
isEmpty( script.path ) {
  script.path     = /usr/bin
}
message(Kimera startup script will be installed in $$script.path)
script.files    = kimera
dic.files       = dic/hiragana.dic \
                  dic/katakana.dic \
                  dic/hankakukana.dic \
                  dic/numeralsymbols.dic \
                  dic/dakuten.dic \
                  dic/kanainput.dic \
                  dic/zenkakualphabet.dic
dic.path        = $$target.path/dic
INSTALLS        = target \
                  script \
                  dic
DEFINES        += KIMERA_VERSION=\"$$VERSION\" INSTALL_PATH=\"$$target.path\"
!contains(CONFIG, debug) {
  DEFINES      += KIMERA_NO_DEBUG
}
INCLUDEPATH    += src ui /usr/local/include
DEPENDPATH     += src ui
HEADERS         = \
                  kimeraglobal.h \
                  mainform.h \
                  dicthiragana.h \
                  kimeraapp.h \
                  inputmethod.h \
                  ximattribute.h \
                  xicattribute.h \
                  kanjiengine.h \
                  cannaengine.h \
                  primeengine.h \
                  tomoeengine.h \
                  preeditarea.h \
                  supportattr.h \
                  candidatelistbox.h \
                  kanjiconvert.h \
                  ximethod.h \
                  xicontext.h \
                  inputmode.h \
                  config.h \
                  debug.h
SOURCES         = \
                  main.cpp \
                  kimeraglobal.cpp \
                  mainform.cpp \
                  dicthiragana.cpp \
                  kimeraapp.cpp \
                  inputmethod.cpp \
                  inputmethod_x11.cpp \
                  xicattribute.cpp \
                  kanjiengine.cpp \
                  cannaengine.cpp \
                  primeengine.cpp \
                  tomoeengine.cpp \
                  preeditarea.cpp \
                  candidatelistbox.cpp \
                  kanjiconvert.cpp \
                  ximethod.cpp \
                  xicontext.cpp \
                  config.cpp \
                  debug.cpp

isEmpty( no_anthy ) {
  HEADERS      += anthyengine.h
  SOURCES      += anthyengine.cpp
#  LIBS         += -ldl
  isEmpty( default_kanjiengine ) {
    default_kanjiengine = Anthy
  }
}
isEmpty( default_kanjiengine ) {
  default_kanjiengine = Canna
}
DEFINES        += DEFAULT_KANJIENGINE=\"$$default_kanjiengine\"

FORMS           = \
                  keyassigner.ui \
                  propertydialog.ui
DISTFILES      += \
                  AUTHORS \
                  INSTALL-ja.eucJP \
                  COPYING \
                  README-en \
                  README-ja.eucJP \
                  tools/kimera \
                  dic/hiragana.dic \
                  dic/katakana.dic \
                  dic/hankakukana.dic \
                  dic/numeralsymbols.dic \
                  dic/dakuten.dic \
                  dic/kanainput.dic \
                  dic/zenkakualphabet.dic
DESTDIR         = .
OBJECTS_DIR     = .obj
MOC_DIR         = .obj
UI_DIR          = ui
DEFAULTCODEC    = eucJP
