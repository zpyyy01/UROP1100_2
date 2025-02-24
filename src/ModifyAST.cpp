#include "clang/AST/ASTConsumer.h"
#include "clang/Tooling/Tooling.h"
#include "clang/Rewrite/Core/Rewriter.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendAction.h"
#include "llvm/Support/raw_ostream.h"
#include <memory>

using namespace clang::tooling;
using namespace llvm;
using namespace clang;
using namespace std;

namespace ModifyAST {



class MyASTVisitor : public RecursiveASTVisitor<MyASTVisitor> {

public:
    explicit MyASTVisitor(Rewriter &R) : rewriter(R) {}

    //if visit a expression, print it
    bool VisitExpr(Expr *E) {
        if (E->getStmtClassName() == "BinaryOperator") {
            BinaryOperator *B = cast<BinaryOperator>(E);
            SourceLocation ST = B->getOperatorLoc();
            rewriter.InsertText(ST, "/*This is a binary operator*/", true, true);
        }
        return true;
    }

private:
    Rewriter &rewriter;
};

class MyASTConsumer : public ASTConsumer {

public:
    explicit MyASTConsumer(Rewriter &R) : visitor(R) {}

    void HandleTranslationUnit(ASTContext &Context) override {
        visitor.TraverseDecl(Context.getTranslationUnitDecl());
    }
private:
    MyASTVisitor visitor;
};

class ModifyASTAction : public ASTFrontendAction {
public:
    static string OutputFilePath;

    unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance &CI, StringRef file) override{
        rewriter.setSourceMgr(CI.getSourceManager(), CI.getLangOpts());
        return make_unique<MyASTConsumer>(rewriter);
    }

void EndSourceFileAction() override{    
    std::error_code EC;
    llvm::raw_fd_ostream OutFile(OutputFilePath, EC);
    if (EC) {
        llvm::errs() << "Error opening output file: " << EC.message() << "\n";
        return;
    }
    rewriter.getEditBuffer(rewriter.getSourceMgr().getMainFileID()).write(OutFile);
    OutFile.close();
}

private:
    Rewriter rewriter;
};

string ModifyAST::ModifyASTAction::OutputFilePath = "";

}