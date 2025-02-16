
#include "clang/Frontend/FrontendActions.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"
#include "llvm/Support/CommandLine.h"

using namespace clang::tooling;
using namespace llvm;
using namespace std;

static cl::OptionCategory MyToolCategory("my-tool options");
static cl::extrahelp CommonHelp(CommonOptionsParser::HelpMessage);
static cl::extrahelp MoreHelp("\nMore help text...\n");

int main(){
    const string filePath = "/compilable_codes/code_1.cpp"; // Change the number as needed
    // Set up command-line options (dummy options for the fixed path)
    const char* argv[] = { "MyTool", filePath.c_str() };
    
    int argc = sizeof(argv) / sizeof(argv[0]);
    auto ExpectedParser = CommonOptionsParser::create(argc, argv, MyToolCategory, llvm::cl::OneOrMore);
    if (!ExpectedParser){
        llvm::errs() << ExpectedParser.takeError();
        return 1;
    }
    CommonOptionsParser &OptionsParser = ExpectedParser.get();
    ClangTool Tool(OptionsParser.getCompilations(),
                   OptionsParser.getSourcePathList());
    return Tool.run(newFrontendActionFactory<clang::SyntaxOnlyAction>().get());
}