FLEXSOURCES =  $$PWD/lexer.lex
BISONSOURCES = $$PWD/parser.y

unix {
flex.commands = flex ${QMAKE_FILE_IN}
}
win32 {
flex.commands = D:\work\devtools\win_flex_bison\win_flex.exe ${QMAKE_FILE_IN}
}

flex.input = FLEXSOURCES
flex.output = lex.yy.c
flex.variable_out = SOURCES
flex.depends = parser.tab.h
flex.name = flex
QMAKE_EXTRA_COMPILERS += flex

unix {
bison.commands = bison -d ${QMAKE_FILE_IN} && mv parser.tab.c parser.tab.cpp
}
win32 {
bison.commands = D:\work\devtools\win_flex_bison\win_bison.exe -d ${QMAKE_FILE_IN} && ren parser.tab.c parser.tab.cpp
}
bison.input = BISONSOURCES
bison.output = parser.tab.cpp
bison.variable_out = SOURCES
bison.name = bison
QMAKE_EXTRA_COMPILERS += bison

bisonheader.commands = @true
bisonheader.input = BISONSOURCES
bisonheader.output = parser.tab.h
bisonheader.variable_out = HEADERS
bisonheader.name = bison header
bisonheader.depends = parser.tab.cpp
QMAKE_EXTRA_COMPILERS += bisonheader


HEADERS += \
    $$PWD/Analizer.h \
    $$PWD/Formater.h \
    $$PWD/StructFieldData.h


SOURCES += \
    $$PWD/Formater.cpp \
    $$PWD/StructFieldData.cpp

INCLUDEPATH += $$PWD
