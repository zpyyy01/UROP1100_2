#ifndef REPLACE_EQUAL_OPERATOR_VISITOR_H
#define REPLACE_EQUAL_OPERATOR_VISITOR_H

#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendAction.h"
#include "clang/Tooling/Tooling.h"
#include "clang/Rewrite/Core/Rewriter.h"
#include "llvm/Support/raw_ostream.h"
#include <string>
#include <memory>

class ReplaceEqualOperatorVisitor : public clang::RecursiveASTVisitor<ReplaceEqualOperatorVisitor> {
public:
    explicit ReplaceEqualOperatorVisitor(clang::Rewriter &R);
    bool VisitBinaryOperator(clang::BinaryOperator *BO);

private:
    clang::Rewriter &Rewrite;
};

class ReplaceEqualOperatorConsumer : public clang::ASTConsumer {
public:
    explicit ReplaceEqualOperatorConsumer(clang::Rewriter &R);
    void HandleTranslationUnit(clang::ASTContext &Context) override;

private:
    ReplaceEqualOperatorVisitor Visitor;
};

class ReplaceEqualOperatorAction : public clang::ASTFrontendAction {
public:
    static std::string OutputFilePath;

    std::unique_ptr<clang::ASTConsumer> CreateASTConsumer(clang::CompilerInstance &CI, clang::StringRef file) override;
    void EndSourceFileAction() override;

private:
    clang::Rewriter Rewrite;
};

#endif // REPLACE_EQUAL_OPERATOR_VISITOR_H