#include <string>
#include <iostream>
#include <fstream>
#include "AppOptions.h"
#include "Analyzer/Analyzer.h"
#include "Generator/CodeGenerator.h"
#include "VersionInfo.h"

using namespace std;

string getBaseFileName(string filePath)
{
    string fName(filePath.substr(filePath.find_last_of("/\\") + 1));
    size_t pos = fName.rfind(".");
    if(pos == string::npos)  // No extension.
        return fName;

    if(pos == 0)    // . is at the front. Not an extension.
        return fName;

    return fName.substr(0, pos);
}

const string HelpMessage =
R"(Usage: slpdgen [options] <slpd_file_name>

Options:
  -h                 show this help message
  -v                 show SLPD version
  -qt, -qt+          generate protocol for Qt
  -cpp               generate protocol for pure C++
  -c                 generate protocol for pure C
  -ip                generate protocol using IPv4 address
  -json              generate JSON file converted from SLPD file

Examples:
  slpdgen -json proto.slpd
  slpdgen -qt -ip proto.slpd
)";

AppOptions checkParameters(int argc, char **argv)
{
    AppOptions options {};

    if(argc == 1)
    {
        cout << HelpMessage;
        exit(0);
    }

    for(int i = 1; i < argc; i++)
    {
        if(string(argv[i]) == "-v")
        {
            cout << fmt("SLPD Version %s.%s.%s \nBuild %s %s\n",
            {to_string(MAJOR_VERSION), to_string(MINOR_VERSION),
             to_string(PATCH_VERSION), APP_TIME, APP_DATE});

            exit(0);
        }
        else if(string(argv[i]) == "-h")
        {
            cout << HelpMessage;
            exit(0);
        }else if(string(argv[i]) == "-qt" || string(argv[i]) == "-qt+")
        {
            options.isQt = true;
            options.isCpp = true;
        }else if(string(argv[i]) == "-c")
        {
            options.isC = true;
        }else if(string(argv[i]) == "-cpp")
        {
            options.isCpp = true;
        }else if(string(argv[i]) == "-ip")
        {
            options.hasAddr = true;
        }else if(string(argv[i]) == "-json")
        {
            options.isJson = true;
        }else if(*string(argv[i]).begin() == '-')
        {
            cout<<string(argv[i]) << " Invalid flag" << endl;
            exit(0);
        }
            options.filePath = argv[i];
            options.fileName = getBaseFileName(options.filePath);
    }
    if(options.filePath == "")
    {
        exit(0);
    }

    if(!options.isQt && !options.isCpp)
    {
        options.isC = true;
    }

    return options;
}

int main(int argc, char **argv)
{
    auto options = checkParameters(argc, argv);

    yyin = fopen(options.filePath.c_str(), "r");
    if(yyin == nullptr) {
        cout << "Cannot open file" << endl;
        return 0;
    }
    yyparse();
    if(isSuccess())
    {
        if(options.isJson)
        {
            std::ofstream(options.fileName + ".json") << formater.toJson();
        } else
        {
            CodeGenerator gen(options, formater);
            gen.generate();
        }
    }

    return 0;
}
