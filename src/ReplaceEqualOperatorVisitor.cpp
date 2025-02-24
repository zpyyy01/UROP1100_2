#include "ReplaceEqualOperatorVisitor.h"

ReplaceEqualOperatorVisitor::ReplaceEqualOperatorVisitor(clang::Rewriter &R) : Rewrite(R) {}

bool ReplaceEqualOperatorVisitor::VisitBinaryOperator(clang::BinaryOperator *BO) {
    if (BO->getOpcode() == clang::BO_EQ) {
        clang::SourceRange Range = BO->getOperatorLoc();
        Rewrite.ReplaceText(Range, "!=");
    }
    return true;
}

ReplaceEqualOperatorConsumer::ReplaceEqualOperatorConsumer(clang::Rewriter &R) : Visitor(R) {}

void ReplaceEqualOperatorConsumer::HandleTranslationUnit(clang::ASTContext &Context) {
    Visitor.TraverseDecl(Context.getTranslationUnitDecl());
}

std::string ReplaceEqualOperatorAction::OutputFilePath = "";

std::unique_ptr<clang::ASTConsumer> ReplaceEqualOperatorAction::CreateASTConsumer(clang::CompilerInstance &CI, clang::StringRef file) {
    Rewrite.setSourceMgr(CI.getSourceManager(), CI.getLangOpts());
    return std::make_unique<ReplaceEqualOperatorConsumer>(Rewrite);
}

void ReplaceEqualOperatorAction::EndSourceFileAction() {
    std::error_code EC;
    llvm::raw_fd_ostream OutFile(OutputFilePath, EC);
    if (EC) {
        llvm::errs() << "Error opening output file: " << EC.message() << "\n";
        return;
    }
    Rewrite.getEditBuffer(Rewrite.getSourceMgr().getMainFileID()).write(OutFile);
    OutFile.close();
}