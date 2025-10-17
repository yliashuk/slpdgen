TEMPLATE = app
CONFIG += console c++17
CONFIG -= app_bundle
CONFIG -= qt
#include(src/Analyzer/Analyzer.pri)

# Generate Parser

FLEXSOURCES =  $$PWD/src/Analyzer/lexer.lex
BISONSOURCES = $$PWD/src/Analyzer/parser.y

unix {
flex.commands = flex ${QMAKE_FILE_IN}
}
win32 {
flex.commands = ${FLEX_PATH} ${QMAKE_FILE_IN}
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
bison.commands = ${BISON_PATH} -d ${QMAKE_FILE_IN} && ren parser.tab.c parser.tab.cpp
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

# Project source code

HEADERS += \
    src/Analyzer/Rule.h \
    src/CppConstructs/FunctionsSrc.h \
    src/Generator/CalcSizeHelper.h \
    src/Generator/CodeGenerator.h \
    src/Generator/MsgHandlerGen.h \
    src/Generator/RulesDefinedMessage.h \
    src/Generator/StdTypeHandler.h \
    src/Utils/ContainerUtils.h \
    src/Utils/StringUtils.h \
    src/Analyzer/Analyzer.h \
    src/Analyzer/Formater.h \
    src/Analyzer/Generics.h \
    src/Analyzer/StructFieldData.h \
    src/CppConstructs/ForLoopCpp.h \
    src/CppConstructs/FunctionCpp.h \
    src/CppConstructs/IfElseStatementCpp.h \
    src/CppConstructs/StructCpp.h \
    src/CppConstructs/SwitchCpp.h \
    src/Generator/ComplexTypeDescription.h \
    src/Generator/EnumDescription.h \
    src/Generator/Generics.h \
    src/Generator/SizeExpr.h \
    src/AppOptions.h \
    src/VersionInfo.h

SOURCES += \
    src/Analyzer/Rule.cpp \
    src/Generator/CalcSizeHelper.cpp \
    src/Generator/CodeGenerator.cpp \
    src/Generator/MsgHandlerGen.cpp \
    src/Generator/StdTypeHandler.cpp \
    src/Utils/StringUtils.cpp \
    src/Analyzer/Formater.cpp \
    src/Analyzer/StructFieldData.cpp \
    src/CppConstructs/ForLoopCpp.cpp \
    src/CppConstructs/FunctionCpp.cpp \
    src/CppConstructs/IfElseStatementCpp.cpp \
    src/CppConstructs/StructCpp.cpp \
    src/CppConstructs/SwitchCpp.cpp \
    src/Generator/ComplexTypeDescription.cpp \
    src/Generator/EnumDescription.cpp \
    src/Generator/SizeExpr.cpp \
    src/main.cpp

INCLUDEPATH += src/
