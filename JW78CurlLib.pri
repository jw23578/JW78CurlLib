INCLUDEPATH += $$PWD

CONFIG += c++17

LIBS += -lcurl -lpthread

HEADERS +=  \
    $$PWD/jw78curlwrapper.h \
    $$PWD/jw78emailwrapper.h \
    $$PWD/jw78imapwrapper.h \
    $$PWD/jw78pop3wrapper.h \
    $$PWD/jw78smtpwrapper.h

SOURCES +=  \
    $$PWD/jw78curlwrapper.cpp \
    $$PWD/jw78emailwrapper.cpp \
    $$PWD/jw78imapwrapper.cpp \
    $$PWD/jw78pop3wrapper.cpp \
    $$PWD/jw78smtpwrapper.cpp
