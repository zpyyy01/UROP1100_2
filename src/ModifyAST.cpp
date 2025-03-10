#include "clang/AST/ASTConsumer.h"
#include "clang/Tooling/Tooling.h"
#include "clang/Rewrite/Core/Rewriter.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendAction.h"
#include "llvm/Support/raw_ostream.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/ParentMap.h"
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
    MyASTVisitor(ASTContext &Context, Rewriter &R): Context(Context), rewriter(R){}
    //if visit a expression, print it
    bool VisitExpr(Expr *E) {
        // if (string(E->getStmtClassName()) == "BinaryOperator") {
        //     BinaryOperator *B = cast<BinaryOperator>(E);
        //     SourceLocation ST = B->getOperatorLoc();
        //     rewriter.InsertText(ST, "/*This is a binary operator*/", true, true);
        // }

        //print the parent of the expression
        // llvm::outs() << "Expr: " << rewriter.getRewrittenText(E->getSourceRange()) << "\n";
        // Stmt *parent = getParent(E);
        // if (parent) {
        //     llvm::outs() << "Parent: " << rewriter.getRewrittenText(parent->getSourceRange()) << "\n";
        // }
        LowerExpr(E);
        return true;
    }
    Stmt *getParent(Stmt *S) {
        auto It = ParentMap.find(S);
        if (It != ParentMap.end()) {
        return dyn_cast<Stmt>(It->second);
        }
        return nullptr;
    }

    bool TraverseStmt(Stmt *S) {
        if (!ParentStack.empty()) {
        ParentMap[S] = ParentStack.top();
        }
        ParentStack.push(S);
        RecursiveASTVisitor<MyASTVisitor>::TraverseStmt(S);
        ParentStack.pop();
        return true;
    }

    std::string getSourceText(SourceRange Range) {
        return rewriter.getRewrittenText(Range);
    }

private:
    ASTContext &Context;
    Rewriter &rewriter;
    std::stack<Stmt *> ParentStack; 
    std::map<Stmt *, Stmt *> ParentMap; 

    string LowerExpr(Expr *E) {
        if (auto *UO = dyn_cast<UnaryOperator>(E)) {
            //return LowerUnaryOperator(UO); 
        } else if (auto *BO = dyn_cast<BinaryOperator>(E)) {
            return LowerBinaryOperator(BO);
        } else {
            // If the expression is a plain expression, return E as string
            return rewriter.getRewrittenText(E->getSourceRange());
        }
    }

    string LowerBinaryOperator(BinaryOperator *B) {
        Expr *LHS = B->getLHS();
        Expr *RHS = B->getRHS();
        //Stmt *parent = B->getParent();
        //print LHS RHS BOP
        llvm::outs() << "LHS: " << rewriter.getRewrittenText(LHS->getSourceRange()) << "\n";
        llvm::outs() << "RHS: " << rewriter.getRewrittenText(RHS->getSourceRange()) << "\n";

        // Recursively lower the left and right operands
        string lhsVar = LowerExpr(LHS);  // Assume LowerExpr returns the lowered variable name
        string rhsVar = LowerExpr(RHS);

        // Generate a unique variable name
        string varName = " temp_" + to_string(variableCounter++);

        // Construct the assignment statement
        string newAssignment = "auto" + varName + " = " + lhsVar + " " + B->getOpcodeStr().str() + " " + rhsVar + ";\n";
        rewriter.ReplaceText(B->getSourceRange(), varName);
        if(Stmt *parent = getParent(B)){
            SourceLocation ParentStart = parent->getBeginLoc();
            rewriter.InsertTextBefore(ParentStart, newAssignment);
        }
        return varName;
    }
};

class MyASTConsumer : public ASTConsumer {

public:
    explicit MyASTConsumer(ASTContext &C, Rewriter &R) : visitor(C, R) {}

    void HandleTranslationUnit(ASTContext &Context) override {
        visitor.TraverseDecl(Context.getTranslationUnitDecl());
    }
private:
    MyASTVisitor visitor;
};

class ModifyASTAction : public ASTFrontendAction {
public:
    static string OutputFilePath;

    unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance &CI, StringRef file) override {
        rewriter.setSourceMgr(CI.getSourceManager(), CI.getLangOpts());
        return make_unique<MyASTConsumer>(CI.getASTContext(), rewriter);
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