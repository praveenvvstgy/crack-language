// Copyright 2010 Google Inc, Shannon Weyrick <weyrick@mozek.us>

#include "FuncBuilder.h"

#include "BFuncDef.h"
#include "VarDefs.h"
#include "model/ResultExpr.h"

#include <llvm/LLVMContext.h>

using namespace llvm;
using namespace model;
using namespace std;

namespace builder {
namespace mvll {

FuncBuilder::FuncBuilder(Context &context, FuncDef::Flags flags,
                             BTypeDef *returnType,
                             const string &name,
                             size_t argCount,
                             Function::LinkageTypes linkage
                             ) :
    context(context),
    returnType(returnType),
    funcDef(new BFuncDef(flags, name, argCount)),
    linkage(linkage),
    argIndex(0) {
        funcDef->returnType = returnType;
}

void FuncBuilder::finish(bool storeDef) {
    size_t argCount = funcDef->args.size();
    assert(argIndex == argCount);
    vector<const Type *> llvmArgs(argCount +
                                  (receiverType ? 1 : 0)
                                  );

    // create the array of LLVM arguments
    int i = 0;
    if (receiverType)
        llvmArgs[i++] = receiverType->rep;
    for (vector<ArgDefPtr>::iterator iter =
         funcDef->args.begin();
    iter != funcDef->args.end();
    ++iter, ++i)
        llvmArgs[i] = BTypeDefPtr::rcast((*iter)->type)->rep;

    // register the function with LLVM
    const Type *rawRetType =
            returnType->rep ? returnType->rep :
            Type::getVoidTy(getGlobalContext());
    FunctionType *llvmFuncType =
            FunctionType::get(rawRetType, llvmArgs, false);
    LLVMBuilder &builder =
            dynamic_cast<LLVMBuilder &>(context.builder);
    Function *func = Function::Create(llvmFuncType,
                                      linkage,
                                      funcDef->name,
                                      builder.module
                                      );
    func->setCallingConv(llvm::CallingConv::C);

    // back-fill builder data and set arg names
    Function::arg_iterator llvmArg = func->arg_begin();
    vector<ArgDefPtr>::const_iterator crackArg =
            funcDef->args.begin();
    if (receiverType) {
        llvmArg->setName("this");

        // add the implementation to the "this" var
        receiver = context.ns->lookUp("this");
        assert(receiver &&
               "missing 'this' variable in the context of a "
               "function with a receiver"
               );
        receiver->impl = new BArgVarDefImpl(llvmArg);
        ++llvmArg;
    }
    for (; llvmArg != func->arg_end(); ++llvmArg, ++crackArg) {
        llvmArg->setName((*crackArg)->name);

        // need the address of the value here because it is going
        // to be used in a "load" context.
        (*crackArg)->impl = new BArgVarDefImpl(llvmArg);
    }

    // store the LLVM function in the table for the module
    builder.setModFunc(funcDef.get(), func);

    // get or create the type registered for the function
    BTypeDef *crkFuncType =
            BTypeDefPtr::acast(builder.getFuncType(context,
                                                   llvmFuncType
                                                   )
                               );
    funcDef->type = crkFuncType;

    // create an implementation object to return the function
    // pointer
    funcDef->impl = new BConstDefImpl(func);

    funcDef->rep = func;
    if (storeDef)
        context.ns->addDef(funcDef.get());
}

void FuncBuilder::addArg(const char *name, TypeDef *type) {
    assert(argIndex <= funcDef->args.size());
    funcDef->args[argIndex++] = new ArgDef(type, name);
}

void FuncBuilder::setArgs(const vector<ArgDefPtr> &args) {
    assert(argIndex == 0 && args.size() == funcDef->args.size());
    argIndex = args.size();
    funcDef->args = args;
}


} // end namespace builder::vmll
} // end namespace builder
