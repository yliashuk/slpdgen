#include <string>
#include <iostream>
#include "AppOptions.h"
#include "Analyzer/Analyzer.h"
#include "Generator/CodeGenerator.h"
#include "VersionInfo.h"

using namespace std;

string GetBaseFileName(string filePath)
{
    string fName(filePath.substr(filePath.find_last_of("/\\") + 1));
    size_t pos = fName.rfind(".");
    if(pos == string::npos)  // No extension.
        return fName;

    if(pos == 0)    // . is at the front. Not an extension.
        return fName;

    return fName.substr(0, pos);
}

AppOptions ParametersCheck(int argc, char **argv)
{
    AppOptions options {};

    if(argc < 1)
        exit(0);

    for(int i = 1; i < argc; i++)
    {
        if(string(argv[i]) == "-v")
        {
            cout << fmt("SLPD Version %s.%s.%s \nBuild %s %s\n",
            {to_string(MAJOR_VERSION), to_string(MINOR_VERSION),
             to_string(PATCH_VERSION), APP_TIME, APP_DATE});

            exit(0);
        }

        if(string(argv[i]) == "-qt" || string(argv[i]) == "-qt+")
        {
            options.isQt = true;
            options.isCpp = true;
        }
        else if(string(argv[i]) == "-ip") {
            options.hasAddr = true;
        }
        else if(string(argv[i]) == "-c") {
            options.isQt = false;
            options.isCpp = false;
        }
        else if(*string(argv[i]).begin() == '-') {
            cout<<string(argv[i]) << " Invalid flag" << endl;
            exit(0);
        }
            options.filePath = argv[i];
            options.fileName = GetBaseFileName(options.filePath);
    }
    if(options.filePath == "")
        exit(0);

    return options;
}

int main(int argc, char **argv)
{
    auto options = ParametersCheck(argc, argv);

    yyin = fopen(options.filePath.c_str(), "r");
    if(yyin == nullptr) {
        cout << "Cannot open file" << endl;
        return 0;
    }
    yyparse();
    if(isSuccess())
    {
        formater.ToJson();
        CodeGenerator gen(options, formater);
        gen.Generate();
    }

    return 0;
}
