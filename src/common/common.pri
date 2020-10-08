
INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD

QT += network

# directories ($$PWD) are set before files because of an error in Qt5

SOURCES += $$PWD/datamodel.cpp \
           $$PWD/comm.cpp \
           $$PWD/messageclientserver.cpp \
           $$PWD/utilclasses.cpp \
           $$PWD/uuid.cpp \
           $$PWD/controlsignaling.cpp

HEADERS += $$PWD/datamodel.h \
           $$PWD/comm.h \
           $$PWD/messageclientserver.h \
           $$PWD/utilclasses.h \
           $$PWD/uuid.h \
           $$PWD/controlsignaling.h \
           $$PWD/ohtbaseconfig.h \
           $$PWD/debug.h

# define platform variable
linux {
    DEFINES += LINUX_OHT
}
win32 {
    DEFINES += WINDOWS_OHT
}



# add here your boost includes if needed
linux {
    message(" Linux Boost libs selected")
    #LIBS += -L/opt/boost/boost_1_60_0/lib/
    #INCLUDEPATH += /opt/boost/boost_1_60_0/include
    LIBS += -L/usr/lib64/boost169
    INCLUDEPATH += /usr/include/boost169/
    LIBS += -lboost_serialization
}
win32 {
    message(" Win32 Boost libs selected")
    LIBS += -LC:/boost/lib
    INCLUDEPATH += C:/boost/include/
    LIBS += -lboost_serialization-mgw49-mt-d-1_60
}

