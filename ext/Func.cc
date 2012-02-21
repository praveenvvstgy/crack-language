// Copyright 2010 Google Inc.

#include "Func.h"

#include "model/ArgDef.h"
#include "model/CleanupFrame.h"
#include "model/Context.h"
#include "model/FuncDef.h"
#include "model/Initializers.h"
#include "model/ResultExpr.h"
#include "model/VarRef.h"
#include "builder/Builder.h"
#include "parser/Toker.h"
#include "parser/Parser.h"
#include "Module.h"
#include "Type.h"

#include <sstream>

using namespace crack::ext;
using namespace std;
using namespace model;
using namespace builder;
using namespace parser;

namespace crack { namespace ext {
    struct Arg {
        Type *type;
        string name;
    
        Arg(Type *type, const string &name) :
            type(type),
            name(name) {
        }
    };
}}

void Func::addArg(Type *type, const string &name) {
    assert(!finished && 
            "Attempted to add an argument to a finished function."
           );
    args.push_back(new Arg(type, name));
}

void Func::setIsVariadic(bool isVariadic)
{
    if (isVariadic) {
        flags = static_cast<Func::Flags>(flags | Func::variadic);
    } else {
        flags = static_cast<Func::Flags>(flags & ~Func::variadic);
    }
}

bool Func::isVariadic() const
{
    return flags & Func::variadic;
}

void Func::setBody(const std::string& body)
{
    funcBody = body;
}

std::string Func::body() const
{
    return funcBody;
}

void Func::finish() {
    if (finished || !context)
        return;

    Builder &builder = context->builder;
    std::vector<ArgDefPtr> realArgs(args.size());
    for (int i = 0; i < args.size(); ++i) {
        args[i]->type->checkFinished();
        realArgs[i] = builder.createArgDef(args[i]->type->typeDef, 
                                           args[i]->name
                                           );
    }

    FuncDefPtr funcDef;
    // if this is a method, get the receiver type
    TypeDefPtr receiverType;
    if ((flags & method) || (flags & constructor))
        receiverType = TypeDefPtr::arcast(context->ns);

    // if we have a function pointer, create a extern function for it
    if (funcPtr) {
        funcDef =
            builder.createExternFunc(*context,
                                    static_cast<FuncDef::Flags>(flags & 
                                                                funcDefFlags
                                                                ),
                                    name,
                                    returnType->typeDef,
                                    receiverType.get(),
                                    realArgs,
                                    funcPtr,
                                    (symbolName.empty())?0:symbolName.c_str()
                                    );

    // inline the function body in the constructor; reduces overhead
    } else if (!funcBody.empty() && !(flags & constructor)) {
        ContextPtr funcContext = context->createSubContext(Context::local);
        funcContext->returnType = returnType->typeDef;
        funcContext->toplevel = true;

        if (flags & method) {
            // create the "this" variable
            ArgDefPtr thisDef =
                context->builder.createArgDef(receiverType.get(), "this");
            funcContext->addDef(thisDef.get());
        }

        funcDef =
            context->builder.emitBeginFunc(*funcContext,
                                            static_cast<FuncDef::Flags>(flags & funcDefFlags),
                                            name,
                                            returnType->typeDef,
                                            realArgs,
                                            0
                                            );

        for (int i = 0; i < realArgs.size(); ++i) {
            funcContext->addDef(realArgs[i].get());
        }

        std::istringstream bodyStream(funcBody);
        Toker toker(bodyStream, name.c_str());
        Parser parser(toker, funcContext.get());
        parser.parse();

        if (returnType->typeDef->matches(*context->construct->voidType)) {
            funcContext->builder.emitReturn(*funcContext, 0);
        }
        funcContext->builder.emitEndFunc(*funcContext, funcDef.get());
    }

    if (funcDef) {
        VarDefPtr storedDef = context->addDef(funcDef.get(), receiverType.get());

        // check for a static method, add it to the meta-class
        if (context->scope == Context::instance) {
            TypeDef *type = TypeDefPtr::arcast(context->ns);
            if (type != type->type.get())
                type->type->addAlias(storedDef.get());
        }
    }

    if (flags & constructor) {
        ContextPtr funcContext = context->createSubContext(Context::local);
        funcContext->toplevel = true;
    
        // create the "this" variable
        ArgDefPtr thisDef =
            context->builder.createArgDef(receiverType.get(), "this");
        funcContext->addDef(thisDef.get());
        VarRefPtr thisRef = new VarRef(thisDef.get());
        
        // emit the function
        TypeDef *voidType = context->construct->voidType.get();
        FuncDefPtr newFunc = context->builder.emitBeginFunc(*funcContext,
                                                            FuncDef::method,
                                                            "oper init",
                                                            voidType,
                                                            realArgs,
                                                            0
                                                            );
        
        // emit the initializers
        Initializers inits;
        receiverType->emitInitializers(*funcContext, &inits);

        
        // if we got a function, emit a call to it.
        if (funcDef) {
            FuncCallPtr call = context->builder.createFuncCall(funcDef.get());
            call->receiver = thisRef;
            
            // populate the arg list with references to the existing args
            for (int i = 0; i < realArgs.size(); ++i) {
                VarRefPtr ref =
                    context->builder.createVarRef(realArgs[i].get());
                call->args.push_back(ref.get());
            }
            
            funcContext->createCleanupFrame();
            call->emit(*funcContext)->handleTransient(*funcContext);
            funcContext->closeCleanupFrame();
        } else if (!funcBody.empty()) {
            for (int i = 0; i < realArgs.size(); ++i) {
                funcContext->addDef(realArgs[i].get());
            }

            std::istringstream bodyStream(funcBody);
            Toker toker(bodyStream, name.c_str());
            Parser parser(toker, funcContext.get());
            parser.parse();
        }

        // close it off
        funcContext->builder.emitReturn(*funcContext, 0);
        funcContext->builder.emitEndFunc(*funcContext, newFunc.get());
        context->addDef(newFunc.get(), receiverType.get());

        receiverType->createNewFunc(*context, newFunc.get());
    }

    finished = true;
}
