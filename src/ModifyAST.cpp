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
        
        LowerExpr(E);
        return true;
    }

private:
    Rewriter &rewriter;

    string LowerExpr(Expr *E) {
        if (auto *UO = dyn_cast<UnaryOperator>(E)) {
            // If the expression is a unary operator, call LowerUnaryOperator
            LowerUnaryOperator(UO);
            return "_temp_" + to_string(variableCounter - 1);  // Return the generated variable name
        } else if (auto *BO = dyn_cast<BinaryOperator>(E)) {
            // If the expression is a binary operator, call LowerBinaryOperator
            LowerBinaryOperator(BO);
            return "_temp_" + to_string(variableCounter - 1);  // Return the generated variable name
        } else {
            // If the expression is a plain expression, return its source text
            SourceManager &SM = rewriter.getSourceMgr();
            return Lexer::getSourceText(CharSourceRange::getTokenRange(E->getSourceRange()), SM, LangOptions());
        }
    }

    bool LowerBinaryOperator(BinaryOperator *B) {
        Expr *LHS = B->getLHS();
        Expr *RHS = B->getRHS();

        // Recursively lower the left and right operands
        string lhsVar = LowerExpr(LHS);  // Assume LowerExpr returns the lowered variable name
        string rhsVar = LowerExpr(RHS);

        // Generate a unique variable name
        string varName = "_temp_" + to_string(variableCounter++);

        // Construct the assignment statement
        string newAssignment = varName + " = " + lhsVar + " " + B->getOpcodeStr().str() + " " + rhsVar;

        // Insert the assignment statement before the binary operator
        rewriter.InsertText(B->getBeginLoc(), newAssignment + "; ", true, true);

        // Replace the entire binary operator expression with the generated variable name
        rewriter.ReplaceText(B->getSourceRange(), varName);

        return true;
    }
    bool LowerUnaryOperator(UnaryOperator *U) {
        Expr *E = U->getSubExpr();
    
        // Recursively lower the sub-expression
        string exprVar = LowerExpr(E);  // Assume LowerExpr returns the lowered variable name
    
        // Generate a unique variable name
        string varName = "_temp_" + to_string(variableCounter++);
    
        // Get the operator string using the operator code
        string opStr = UnaryOperator::getOpcodeStr(U->getOpcode()).str();
    
        // Construct the assignment statement
        string newAssignment = varName + " = " + opStr + exprVar;
    
        // Insert the assignment statement before the unary operator
        rewriter.InsertText(U->getBeginLoc(), newAssignment + "; ", true, true);
    
        // Replace the entire unary operator expression with the generated variable name
        rewriter.ReplaceText(U->getSourceRange(), varName);
    
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