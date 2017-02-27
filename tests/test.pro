CONFIG += qt console

INCLUDEPATH += $$PWD/../include
SOURCES += $$PWD/../src/*.*

#SOURCES += ../STP/extlibs/pugixml/*.*

SOURCES += main_tmxloader.cpp

QMAKE_CXXFLAGS += -std=c++11 -Wall -Wextra -Werror
LIBS += -lsfml-graphics -lsfml-window -lsfml-system -lsfml-audio

LIBS += -lboost_unit_test_framework

# zlib
LIBS += -lz

# pugic
LIBS += -lpugixml
#DEFINES += PUGIXML_HEADER_ONLY
INCLUDEPATH += /usr/include
INCLUDEPATH += /usr/lib

LIBS += -lBox2D
