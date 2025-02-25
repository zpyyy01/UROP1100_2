#include "clang/AST/ASTConsumer.h"
#include "clang/Tooling/Tooling.h"
#include "clang/Rewrite/Core/Rewriter.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendAction.h"
#include "llvm/Support/raw_ostream.h"
#include <memory>
#include <ctime>
static std::atomic<int> variableCounter(0);

using namespace clang::tooling;
using namespace llvm;
using namespace clang;
using namespace std;

namespace ModifyAST {

class MyASTVisitor : public RecursiveASTVisitor<MyASTVisitor> {

public:
    explicit MyASTVisitor(Rewriter &R) : rewriter(R) {
        srand(static_cast<unsigned int>(time(0)));
    }

    //if visit a expression, print it
    bool VisitExpr(Expr *E) {
        // if (E->getStmtClassName() == "BinaryOperator") {
        //     BinaryOperator *B = cast<BinaryOperator>(E);
        //     SourceLocation ST = B->getOperatorLoc();
        //     rewriter.InsertText(ST, "/*This is a binary operator*/", true, true);
        // }
        
        return LowerExpr(E);
    }

private:
    Rewriter &rewriter;

    bool LowerExpr(Expr *E) {
        //if(rand() % 2 == 0) return true;
        if(E->getStmtClassName() == "BinaryOperator") {
            BinaryOperator *B = cast<BinaryOperator>(E);
            return LowerBinaryOperator(B);
        }
        return true;
    }

    bool LowerBinaryOperator(BinaryOperator *B) {
        Expr *LHS = B->getLHS();
        Expr *RHS = B->getRHS();
        LowerExpr(LHS);
        LowerExpr(RHS);
        //create a variable name that does not exist in the code
        string varName = "auto _myvar" + to_string(variableCounter++);
        //create a new variable declaration
        rewriter.InsertText(B->getOperatorLoc(), varName + " = " + LHS->getSourceRange().getBegin().printToString(rewriter.getSourceMgr()) + " " + B->getOpcodeStr().str() + " " + RHS->getSourceRange().getBegin().printToString(rewriter.getSourceMgr()), true, true);
        return true;
    }
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