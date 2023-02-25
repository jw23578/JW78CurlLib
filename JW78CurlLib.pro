TEMPLATE = app
CONFIG += console c++17
CONFIG -= app_bundle
CONFIG -= qt

LIBS += -lcurl -lpthread

SOURCES += \
        jw78curlwrapper.cpp \
        jw78emailwrapper.cpp \
        jw78imapwrapper.cpp \
        jw78pop3wrapper.cpp \
        jw78smtpwrapper.cpp \
        main.cpp

DISTFILES += \
  JW78CurlLib.pri \
  LICENSE \
  README.md

HEADERS += \
  jw78curlwrapper.h \
  jw78emailwrapper.h \
  jw78imapwrapper.h \
  jw78pop3wrapper.h \
  jw78smtpwrapper.h
