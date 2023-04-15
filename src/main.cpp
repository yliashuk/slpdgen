#include <string>
#include <iostream>
#include "Analyzer/Analyzer.h"
#include "Generator/CodeGenerator.h"

using namespace std;

string GetBaseFileName(string fileName)
{
    string fName(fileName);
    size_t pos = fName.rfind(".");
    if(pos == string::npos)  // No extension.
        return fName;

    if(pos == 0)    // . is at the front. Not an extension.
        return fName;

    return fName.substr(0, pos);
}

struct Options
{
    bool qtOption;
    bool qtCppOption;
    bool addrOption;
    string filePath;
};

Options ParametersCheck(int argc, char **argv)
{
    Options options {};

    if(argc < 1)
        exit(0);

    for(int i = 1; i < argc; i++)
    {
        if(string(argv[i]) == "-v")
        {
            cout << "SLPD Version 1.1 " << endl<<"Build " << __TIME__ << ' ' << __DATE__
                 << endl;
            exit(0);
        }
        else if(string(argv[i]) == "-qt")
            options.qtOption = true;
        else if(string(argv[i]) == "-qt+" || string(argv[i]) == "-qt++")
            options.qtCppOption = true;
        else if(string(argv[i]) == "-ip")
            options.addrOption = true;
        else if(string(argv[i]) == "-c")
            options.qtOption = false;
        else if(*string(argv[i]).begin() == '-') {
            cout<<string(argv[i]) << " Invalid flag" << endl;
            exit(0);
        }
            options.filePath = argv[i];
    }
    if(options.filePath == "")
        exit(0);

    return options;
}

int main(int argc, char **argv)
{
    auto options = ParametersCheck(argc, argv);

    auto filePath = options.filePath;
    auto fileName = filePath.substr(filePath.find_last_of("/\\") + 1);
    auto baseFileName = GetBaseFileName(fileName);

    yyin = fopen(filePath.c_str(), "r");
    if(yyin == nullptr) {
        cout << "Cannot open file" << endl;
        return 0;
    }
    yyparse();
    if(isSuccess())
    {
        CodeGenerator gen(baseFileName, formater, options.addrOption,
                          options.qtOption, options.qtCppOption);
        gen.Generate();
    }

    return 0;
}
