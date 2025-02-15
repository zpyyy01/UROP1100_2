#include <iostream>
#include <filesystem>
#include <clang/Tooling/Tooling.h>
#include <clang/Tooling/CommonOptionsParser.h>
#include <clang/AST/AST.h>
#include <clang/AST/RecursiveASTVisitor.h>

using namespace clang;
using namespace clang::tooling;

class EqualSignVisitor : public RecursiveASTVisitor<EqualSignVisitor> {
public:
    void VisitBinaryOperator(BinaryOperator *BO) {
        if (BO->getOpcode() == BO_EQ) {
            count++;
        }
    }

    int getCount() const { return count; }

private:
    int count = 0;
};

class EqualSignASTConsumer : public ASTConsumer {
public:
    void HandleTranslationUnit(ASTContext &Context) override {
        EqualSignVisitor visitor;
        visitor.TraverseDecl(Context.getTranslationUnitDecl());
        totalCount += visitor.getCount();
    }

    static int totalCount;
};

int EqualSignASTConsumer::totalCount = 0;

class EqualSignAction : public ASTFrontendAction {
public:
    std::unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance &CI, StringRef InFile) override {
        return std::make_unique<EqualSignASTConsumer>();
    }
};

int main(int argc, const char **argv) {
    llvm::cl::OptionCategory MyToolCategory("my-tool options");
    CommonOptionsParser OptionsParser(argc, argv, MyToolCategory);
    ClangTool Tool(OptionsParser.getCompilations(), OptionsParser.getSourcePathList());

    Tool.run(newFrontendActionFactory<EqualSignAction>().get());

    std::cout << "Total '==' found: " << EqualSignASTConsumer::totalCount << std::endl;
    return 0;
}