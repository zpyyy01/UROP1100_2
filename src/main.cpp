#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendAction.h"
#include "clang/Tooling/Tooling.h"
#include "clang/Rewrite/Core/Rewriter.h"
#include "llvm/Support/raw_ostream.h"
#include <iostream>
#include <fstream>
#include "ReplaceEqualOperatorVisitor.h"
#include "ModifyAST.cpp"

using namespace clang;
using namespace ModifyAST;

int process_code(int id_of_code) {
    std::string FilePath = "./compilable_codes/code_"+std::to_string(id_of_code)+".c";
    std::cout<<"Processing: "<<FilePath<<std::endl;

    std::ifstream FileStream(FilePath);
    if (!FileStream) {
        llvm::errs() << "Failed to open file: " << FilePath << "\n";
        return 1;
    }
    std::string Code((std::istreambuf_iterator<char>(FileStream)),
                     std::istreambuf_iterator<char>());
    FileStream.close();

    ModifyASTAction::OutputFilePath = FilePath;

    if (clang::tooling::runToolOnCode(std::make_unique<ModifyASTAction>(), Code, FilePath)) {
        llvm::outs() << "Successfully modified AST in " << FilePath << "\n";
    } else {
        llvm::errs() << "Failed to process " << FilePath << "\n";
    }

    // ReplaceEqualOperatorAction::OutputFilePath = FilePath;

    // if (clang::tooling::runToolOnCode(std::make_unique<ReplaceEqualOperatorAction>(), Code, FilePath)) {
    //     llvm::outs() << "Successfully replaced all == with != in " << FilePath << "\n";
    // } else {
    //     llvm::errs() << "Failed to process " << FilePath << "\n";
    // }
    return 0;
}


int main(int argc, char *argv[]) {
    //get id from argument
    int id_of_code = std::stoi(argv[1]);
    process_code(id_of_code);
    return 0;
}