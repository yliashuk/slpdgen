%{
#include <stdio.h>
#include <iostream>
#include <string>
#include <cstring>
#include <cstdlib>
#include "Analyzer/Analyzer.h"

Formater formater;
string currBlockName;
DataStructType strT;
EnumType enmType;

bool errorState = false;
bool isSuccess();
void errorPrint(ComplexStatus s);
long int GetNum(const char * string);
void yyerror(string s);

extern "C"{
        int curr_line = 1;
        int yylex(void);
        int yywrap(){return 1;}
}

%}

%union
{
        char *str;
};

//key words
%token KW_SECTION
%token KW_SECTION_END
%token KW_HEADER
%token KW_TYPE
%token KW_CODE
%token KW_ENUM
%token KW_STRUCT

%token KW_S_BASE
%token KW_S_DEFINITIONS
%token KW_S_MESSAGES
%token KW_S_RULES

//other tokens
%token ST_BLOCK
%token END_BLOCK
%token LSQB   // symbol [
%token RSQB   // symbol ]
%token SEPARATOR
%token EQUAL
%token COLON
%token DOUBLE_COLON
%token THEREFORE
%token EMPTY
%token REVERSE
%token NONE
%token WORD
//u8 volume: from 0x00 to 0x0A
%token LOCAL
%token REMOTE
%token FROM
%token TO
%expect 2
%type <str> WORD
//%right WORD
%start strt
%%
strt:
     | strt BLOCKS

BLOCKS:
        KW_SECTION KW_S_BASE
        s_base_contents_input
        KW_SECTION_END

        |KW_SECTION KW_S_DEFINITIONS
         s_definitions_contents_input
         KW_SECTION_END

        |KW_SECTION KW_S_MESSAGES
         s_messages_contents_input
         KW_SECTION_END

        |KW_SECTION KW_S_RULES {currBlockName = "SECTION RULES";}
         s_rules_contents_input
         KW_SECTION_END


s_base_contents_input:
             | s_base_contents_input s_base_contents
s_base_contents:
         KW_HEADER WORD ST_BLOCK
         {
            strT = Hdr; currBlockName = $2;
            auto status = formater.AddStructDeclaration(strT,currBlockName);
            errorPrint(status);
         }
         data_struct_field_parse_input
         END_BLOCK
       | KW_TYPE WORD ST_BLOCK
         {
             enmType = Tp; currBlockName = $2;
             auto status = formater.AddEnumDeclaration(enmType,currBlockName);
             errorPrint(status);
         }
         enum_field_parse_input
         END_BLOCK
       | KW_CODE WORD ST_BLOCK
         {
             enmType = Cd; currBlockName = $2/*.str*/;
             auto status = formater.AddEnumDeclaration(enmType, currBlockName);
             errorPrint(status);
         }
         enum_field_parse_input
         END_BLOCK

s_definitions_contents_input:
             | s_definitions_contents_input s_definitions_contents
s_definitions_contents:
               KW_STRUCT WORD ST_BLOCK
               {
                   strT = SmplStr;currBlockName = $2;
                   auto status = formater.AddStructDeclaration(strT, $2);
                   errorPrint(status);
               }
               data_struct_field_parse_input
               END_BLOCK
             | KW_ENUM WORD ST_BLOCK
               {
                   enmType = SmplEnm;currBlockName = $2;
                   auto status = formater.AddEnumDeclaration(enmType, $2);
                   errorPrint(status);
               }
               enum_field_parse_input
               END_BLOCK

s_messages_contents_input:
             | s_messages_contents_input s_messages_contents
s_messages_contents:
               WORD ST_BLOCK
               {
                    strT = Pckt;
                    currBlockName = $1;
                    auto status = formater.AddStructDeclaration(strT, $1);
                    errorPrint(status);
                }
               data_struct_field_parse_input
               END_BLOCK

s_rules_contents_input:
             | s_rules_contents_input s_rules_contents
s_rules_contents:
             rules_parse_input




data_struct_field_parse_input:
          | data_struct_field_parse_input data_struct_field_parse
data_struct_field_parse:
            WORD WORD
            {
                auto dataStruct = FieldInfoBuilder().SetCommon($1, $2).Build();
                auto status = formater.AddStructField(strT, currBlockName, *dataStruct);
                errorPrint(status);
            }
          | WORD WORD LSQB WORD RSQB
            {
                uint64_t var = GetNum($4);
                unique_ptr<FieldInfo> dataStruct;
                auto builder = FieldInfoBuilder().SetCommon($1, $2);
                if(var)
                    dataStruct = builder.SetLenDefiningVar(var).Build();
                else
                    dataStruct = builder.SetLenDefiningVar($4).Build();
                auto status = formater.AddStructField(strT, currBlockName, *dataStruct);
                errorPrint(status);
            }
          | WORD WORD EQUAL WORD
            {
                auto dataStruct = FieldInfoBuilder().SetCommon($1, $2)
                                                    .SetDefaultVal(GetNum($4))
                                                    .Build();
                auto status = formater.AddStructField(strT, currBlockName, *dataStruct);
                errorPrint(status);
            }
          | WORD WORD EQUAL LOCAL
            {
                    auto dataStruct = FieldInfoBuilder().SetCommon($1, $2)
                                                        .SetSpecialType("local")
                                                        .Build();
                    auto status = formater.AddStructField(strT, currBlockName,
                                                           *dataStruct);
                    errorPrint(status);
            }
          | WORD WORD EQUAL REMOTE
            {
                    auto dataStruct = FieldInfoBuilder().SetCommon($1, $2)
                                                        .SetSpecialType("remote")
                                                        .Build();
                    auto status = formater.AddStructField(strT, currBlockName,
                                                           *dataStruct);
                    errorPrint(status);
            }
          | WORD WORD COLON FROM WORD TO WORD
            {
                auto dataStruct = FieldInfoBuilder().SetCommon($1, $2)
                                                    .SetValRange(GetNum($5), (uint64_t)GetNum($7))
                                                    .Build();
                auto status = formater.AddStructField(strT, currBlockName, *dataStruct);
            }


enum_field_parse_input:
          | enum_field_parse_input enum_field_parse
enum_field_parse:
            WORD
            {
                auto status = formater.AddEnumField(enmType,currBlockName, $1);
                errorPrint(status);
            }
          | WORD EQUAL WORD
            {
                formater.AddEnumField(enmType,currBlockName, $1, GetNum($3));
            }


rules_parse_input:
          | rules_parse_input rules_parse
rules_parse:
            WORD DOUBLE_COLON WORD COLON WORD EMPTY THEREFORE NONE
            {
                Rule rule = RuleBuilder().command($3).sendType($5).build();

                auto status = formater.AddRule(rule);
                errorPrint(status);    
            }
          | WORD DOUBLE_COLON WORD COLON WORD WORD THEREFORE NONE
            {
                Rule rule = RuleBuilder().command($3).sendType($5).
                        sendPacket($6).build();

                auto status = formater.AddRule(rule);
                errorPrint(status);
            }
          | WORD DOUBLE_COLON WORD COLON WORD EMPTY THEREFORE WORD EMPTY
            {
                Rule rule = RuleBuilder().command($3).sendType($5).
                        responseType($8).build();

                auto status = formater.AddRule(rule);
                errorPrint(status);
            }
          | WORD DOUBLE_COLON WORD COLON WORD WORD THEREFORE WORD EMPTY
            {
                Rule rule = RuleBuilder().command($3).sendType($5).sendPacket($6).
                        responseType($8).build();

                auto status = formater.AddRule(rule);
                errorPrint(status);
            }
          | WORD DOUBLE_COLON WORD COLON WORD EMPTY THEREFORE WORD WORD
            {
                Rule rule = RuleBuilder().command($3).sendType($5).responseType($8).
                        responsePacket($9).build();

                auto status = formater.AddRule(rule);
                errorPrint(status);
            }
          | WORD DOUBLE_COLON WORD COLON WORD WORD THEREFORE WORD WORD
            {
                Rule rule = RuleBuilder().command($3).sendType($5).sendPacket($6).responseType($8).
                        responsePacket($9).build();

                auto status = formater.AddRule(rule);
                errorPrint(status);
            }
          | WORD DOUBLE_COLON WORD COLON WORD EMPTY THEREFORE WORD EMPTY REVERSE
            {
                cout << "warning:" << '"' << "reverse" << '"' << " unused"
                     << ", line:" << curr_line << endl;

                Rule rule = RuleBuilder().command($3).sendType($5).
                        responseType($8).build();

                auto status = formater.AddRule(rule, true);
                errorPrint(status);
            }
          | WORD DOUBLE_COLON WORD COLON WORD WORD THEREFORE WORD EMPTY REVERSE
            {
                Rule rule = RuleBuilder().command($3).sendType($5).sendPacket($6).
                        responseType($8).build();

                auto status = formater.AddRule(rule, true);
                errorPrint(status);
            }
//          | WORD DOUBLE_COLON WORD COLON WORD WORD THEREFORE WORD WORD REVERSE
//            {
//                auto status = formater.addRule($3/*.str*/,$5/*.str*/,$6/*.str*/,string($8/*.str*/),string($9/*.str*/),true);
//                errorPrint(status);
//            }

%%

void yyerror(string s)
{
    cout<<s<<", token " << '"' << yylval.str << '"' << ", line:" << curr_line<<endl;
    errorState = true;
}

void errorPrint(ComplexStatus s)
{
    switch(s.status)
    {
        case DuplStruct:
            cout << "Duplicate struct:" << '"' << s.wrongName << '"' << " in the "
                 << '"' << currBlockName << '"' << ", line:" << curr_line<<endl;
        break;
        case DuplField:
            cout << "Duplicate field:" << '"' << s.wrongName << '"' << " in the "
                 << '"'<<currBlockName << '"' << ", line:" << curr_line<<endl;
        break;
        case EnumFieldNotContained:
            cout << "Field " << '"' << s.wrongName << '"'
                 << " is not contained in the " << '"' << currBlockName
                 << '"'<<", line:"<<curr_line<<endl;
        break;
        case StructNameNotContained:
            cout << "Struct " << '"' << s.wrongName << '"'
                 << " is not contained in the " << '"' << currBlockName << '"'
                 << ", line:"<<curr_line<<endl;
        break;
        case FieldNameNotContained:
            cout << "Struct " << '"' << s.wrongName << '"'
                 << " is not contained in the " << '"' << currBlockName << '"'
                 << ", line:"<<curr_line<<endl;
        break;
        case DefiningVarNotContained:
            cout << "Field " << '"' << s.wrongName << '"'
                 << " does not contain the definition of the length-determining "
                    "variable in the " << '"' << currBlockName << '"' << ", line:"
                 << curr_line<<endl;
        break;
        case Ok:
        break;
    }
    if(s.status != Ok) errorState = true;
}

long int GetNum(const char * string)
{
    auto var = strtol(string, NULL, 10);
    if(var == 0)
    {
        size_t found = std::string(string).find("0x");
        if(found == std::string::npos)
            found = std::string(string).find("0X");
        if(found != std::string::npos)
            var = strtol(string, NULL, 16);
    }
    return var;
}

bool isSuccess()
{
    return !errorState;
}
