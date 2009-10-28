
#ifndef _builder_LLVMBUilder_h_
#define _builder_LLVMBUilder_h_

#include "Builder.h"

#include <llvm/Support/IRBuilder.h>

namespace llvm {
    class Module;
    class Function;
    class BasicBlock;
    class Value;
    class Function;
    class ExecutionEngine;
};

namespace builder {

class LLVMBuilder : public Builder {
    private:

        llvm::Module *module;
        llvm::Function *func;
        llvm::BasicBlock *block;
        llvm::IRBuilder<> builder;
        
        llvm::Value *lastValue;
        llvm::ExecutionEngine *execEng;

    public:
        LLVMBuilder();

        virtual void emitFuncCall(model::Context &context, 
                                  const model::FuncDefPtr &func,
                                  const model::FuncCall::ExprVector &args
                                  );
        
        virtual void emitStrConst(model::Context &context,
                                  const model::StrConstPtr &strConst
                                  );
        
        virtual void emitIntConst(model::Context &context,
                                  const model::IntConst &val);

        virtual model::BranchpointPtr emitIf(model::Context &context,
                                             const model::ExprPtr &cond);
        
        virtual model::BranchpointPtr
            emitElse(model::Context &context,
                     const model::BranchpointPtr &pos);
        
        virtual void emitEndIf(model::Context &context,
                               const model::BranchpointPtr &pos);

        virtual model::BranchpointPtr
            emitWhile(model::Context &context, const model::ExprPtr &cond);

        virtual void emitEndWhile(model::Context &context,
                                  const model::BranchpointPtr &pos);

        virtual model::FuncDefPtr
            emitBeginFunc(model::Context &context,
                          const std::string &name,
                          const model::TypeDefPtr &returnType,
                          const std::vector<model::ArgDefPtr> &args);
        
        virtual void emitEndFunc(model::Context &context,
                                 const model::FuncDefPtr &funcDef);

        virtual void emitReturn(model::Context &context,
                                const model::ExprPtr &expr);

        virtual model::VarDefPtr emitVarDef(model::Context &container,
                                            const model::TypeDefPtr &type,
                                            const std::string &name,
                                            const model::ExprPtr &initializer,
                                            bool staticScope
                                            );

        // for definitions, we're going to just let the builder be a factory 
        // so that it can tie whatever special information it wants to the 
        // definition.
        
        virtual model::FuncCallPtr createFuncCall(const std::string &funcName);
        virtual model::ArgDefPtr createArgDef(const model::TypeDefPtr &type,
                                              const std::string &name
                                              );
        virtual model::FuncDefPtr createFuncDef(const char *name);
        virtual model::VarRefPtr createVarRef(const model::VarDefPtr &varDef);

        virtual void createModule(const char *name);
        virtual void closeModule();
        virtual model::StrConstPtr createStrConst(model::Context &context,
                                                  const std::string &val);
        virtual model::IntConstPtr createIntConst(model::Context &context,
                                                  long val);
        virtual void registerPrimFuncs(model::Context &context);
        
        virtual void run();
        
        // internal functions used by our VarDefImpl to generate the 
        // appropriate variable references.
        void emitMemVarRef(model::Context &context, llvm::Value *val);
        void emitArgVarRef(model::Context &context, llvm::Value *val);
};

} // namespace builder
#endif

