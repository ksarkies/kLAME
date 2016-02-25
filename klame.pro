PROJECT =       kLAME
TEMPLATE        = app
TARGET         += 
DEPENDPATH     += .
INCLUDEPATH    += .
LANGUAGE	    = C++
OBJECTS_DIR     = obj
MOC_DIR         = moc
UI_DIR          = ui
RCC_DIR         = ui
CONFIG	       += qt thread warn_on release
QT             += widgets

# Look in the qt installation parent directory under Linux. Use -L to change.
unix:LIBS	   += -L/usr/lib/x86_64-linux-gnu -lmp3lame
# Change this to search in the appropriate MinGW library directory
win32:LIBS	   += -LD:/Development/MinGW/lib -lmp3lame

# Input
FORMS          += klamemainformbase.ui \
                  klameoptionsdialoguebase.ui \
                  helpbase.ui
HEADERS        += klamemainform.h \
                  klameoptionsdialog.h \
                  help.h \
                  lame.h
SOURCES        += main.cpp \
                  klamemainform.cpp \
                  klameoptionsdialog.cpp \
                  help.cpp
RESOURCES      += icons.qrc
