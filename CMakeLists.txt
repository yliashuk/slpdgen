cmake_minimum_required(VERSION 3.0)
project(slpdgen)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wshadow -Wpointer-arith -pedantic")

find_package(FLEX REQUIRED)
find_package(BISON REQUIRED)

BISON_TARGET(parser ${CMAKE_SOURCE_DIR}/src/Analyzer/parser.y ${CMAKE_CURRENT_BINARY_DIR}/parser.tab.cpp
    COMPILE_FLAGS "--defines=${CMAKE_CURRENT_BINARY_DIR}/parser.tab.h")
FLEX_TARGET(lexer ${CMAKE_SOURCE_DIR}/src/Analyzer/lexer.lex ${CMAKE_CURRENT_BINARY_DIR}/lex.yy.c)
ADD_FLEX_BISON_DEPENDENCY(lexer parser)

include_directories(src ${CMAKE_CURRENT_BINARY_DIR})

set(SOURCES
    src/main.cpp
    src/Analyzer/Rule.cpp
    src/Generator/CalcSizeHelper.cpp
    src/Generator/CodeGenerator.cpp
    src/Generator/MsgHandlerGen.cpp
    src/Generator/StdTypeHandler.cpp
    src/Utils/StringUtils.cpp
    src/Analyzer/Formater.cpp
    src/Analyzer/StructFieldData.cpp
    src/CppConstructs/ForLoopCpp.cpp
    src/CppConstructs/FunctionCpp.cpp
    src/CppConstructs/IfElseStatementCpp.cpp
    src/CppConstructs/StructCpp.cpp
    src/CppConstructs/SwitchCpp.cpp
    src/Generator/ComplexTypeDescription.cpp
    src/Generator/EnumDescription.cpp
    src/Generator/SizeExpr.cpp
    ${FLEX_lexer_OUTPUTS}
    ${BISON_parser_OUTPUTS}
)

set(HEADERS
    src/Analyzer/Rule.h
    src/CppConstructs/FunctionsSrc.h
    src/Generator/CalcSizeHelper.h
    src/Generator/CodeGenerator.h
    src/Generator/MsgHandlerGen.h
    src/Generator/RulesDefinedMessage.h
    src/Generator/StdTypeHandler.h
    src/Utils/AutoEndlStream.h
    src/Utils/ContainerUtils.h
    src/Utils/StringUtils.h
    src/Analyzer/Analyzer.h
    src/Analyzer/Formater.h
    src/Analyzer/Generics.h
    src/Analyzer/StructFieldData.h
    src/CppConstructs/ForLoopCpp.h
    src/CppConstructs/FunctionCpp.h
    src/CppConstructs/IfElseStatementCpp.h
    src/CppConstructs/StructCpp.h
    src/CppConstructs/SwitchCpp.h
    src/Generator/ComplexTypeDescription.h
    src/Generator/EnumDescription.h
    src/Generator/SizeExpr.h
    src/AppOptions.h
    src/VersionInfo.h
)

add_executable(slpdgen ${SOURCES} ${HEADERS})
