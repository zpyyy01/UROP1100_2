#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendAction.h"
#include "clang/Tooling/Tooling.h"
#include "clang/Rewrite/Core/Rewriter.h"
#include "llvm/Support/raw_ostream.h"
#include <iostream>
#include <fstream>

using namespace clang;

class ReplaceEqualOperatorVisitor : public RecursiveASTVisitor<ReplaceEqualOperatorVisitor> {
public:
    explicit ReplaceEqualOperatorVisitor(Rewriter &R) : Rewrite(R) {}

    bool VisitBinaryOperator(BinaryOperator *BO) {
        if (BO->getOpcode() == BO_EQ) {
            SourceRange Range = BO->getOperatorLoc();
            Rewrite.ReplaceText(Range, "!=");
        }
        return true;
    }

private:
    Rewriter &Rewrite;
};

class ReplaceEqualOperatorConsumer : public ASTConsumer {
public:
    explicit ReplaceEqualOperatorConsumer(Rewriter &R) : Visitor(R) {}

    void HandleTranslationUnit(ASTContext &Context) override {
        Visitor.TraverseDecl(Context.getTranslationUnitDecl());
    }

private:
    ReplaceEqualOperatorVisitor Visitor;
};

class ReplaceEqualOperatorAction : public ASTFrontendAction {
public:
    static std::string OutputFilePath;

    std::unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance &CI, StringRef file) override {
        Rewrite.setSourceMgr(CI.getSourceManager(), CI.getLangOpts());
        return std::make_unique<ReplaceEqualOperatorConsumer>(Rewrite);
    }

    void EndSourceFileAction() override {
        std::error_code EC;
        llvm::raw_fd_ostream OutFile(OutputFilePath, EC);
        if (EC) {
            llvm::errs() << "Error opening output file: " << EC.message() << "\n";
            return;
        }
        Rewrite.getEditBuffer(Rewrite.getSourceMgr().getMainFileID()).write(OutFile);
        OutFile.close();
    }

private:
    Rewriter Rewrite;
};

std::string ReplaceEqualOperatorAction::OutputFilePath;

int process_code(int id_of_code) {
    std::string FilePath = "../compilable_codes/code_"+std::to_string(id_of_code)+".c";
    llvm::outs() << "Processing: " << FilePath << "\n";

    std::ifstream FileStream(FilePath);
    if (!FileStream) {
        llvm::errs() << "Failed to open file: " << FilePath << "\n";
        return 1;
    }
    std::string Code((std::istreambuf_iterator<char>(FileStream)),
                     std::istreambuf_iterator<char>());
    FileStream.close();

    
    ReplaceEqualOperatorAction::OutputFilePath = FilePath;

    if (clang::tooling::runToolOnCode(std::make_unique<ReplaceEqualOperatorAction>(), Code, FilePath)) {
        llvm::outs() << "Successfully replaced all == with != in " << FilePath << "\n";
    } else {
        llvm::errs() << "Failed to process " << FilePath << "\n";
    }
    return 0;
}


int main() {
    for(int i=1;i<=100;i++){
        process_code(i);
    }
    return 0;
}