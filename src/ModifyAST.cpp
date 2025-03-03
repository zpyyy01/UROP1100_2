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
        if (E->getStmtClassName() == "BinaryOperator") {
            BinaryOperator *B = cast<BinaryOperator>(E);
            SourceLocation ST = B->getOperatorLoc();
            rewriter.InsertText(ST, "/*This is a binary operator*/", true, true);
        }
        
        LowerExpr(E);
        //insert a comment before the expression showing the type of the expression
        //ewriter.InsertText(E->getBeginLoc(), "/*This is a " + string(E->getStmtClassName()) + "*/", true, true);
        return true;
    }

private:
    Rewriter &rewriter;

    string LowerExpr(Expr *E) {
        if (auto *UO = dyn_cast<UnaryOperator>(E)) {
            // If the expression is a unary operator, call LowerUnaryOperator
            return LowerUnaryOperator(UO); 
        } else if (auto *BO = dyn_cast<BinaryOperator>(E)) {
            // If the expression is a binary operator, call LowerBinaryOperator
            return LowerBinaryOperator(BO);
        } else {
            // If the expression is a plain expression, return E as string
            return rewriter.getRewrittenText(E->getSourceRange());
        }
    }

    string LowerBinaryOperator(BinaryOperator *B) {
        Expr *LHS = B->getLHS();
        Expr *RHS = B->getRHS();

        // Recursively lower the left and right operands
        string lhsVar = LowerExpr(LHS);  // Assume LowerExpr returns the lowered variable name
        string rhsVar = LowerExpr(RHS);

        // Generate a unique variable name
        string varName = "_temp_" + to_string(variableCounter++);

        // Construct the assignment statement
        string newAssignment = varName + " = " + lhsVar + " " + B->getOpcodeStr().str() + " " + rhsVar; 
        // replace the entire binary operator expression with the generated variable name
        rewriter.ReplaceText(B->getSourceRange(), varName);
        // Insert the assignment statement before the binary operator
        rewriter.InsertText(B->getBeginLoc(), newAssignment + "; ", true, true);

        return varName;
    }
    string LowerUnaryOperator(UnaryOperator *U) {
        Expr *subExpr = U->getSubExpr();
        string subVar = LowerExpr(subExpr);

        // Generate a unique variable name
        string varName = "_temp_" + to_string(variableCounter++);

        // Construct the assignment statement
        string newAssignment = varName + " = " + U->getOpcodeStr(U->getOpcode()).str() + subVar; 
        // replace the entire unary operator expression with the generated variable name
        rewriter.ReplaceText(U->getSourceRange(), varName);
        // Insert the assignment statement before the unary operator
        rewriter.InsertText(U->getBeginLoc(), newAssignment + "; ", true, true);

        return varName;
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