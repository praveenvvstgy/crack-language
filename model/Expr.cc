// Copyright 2009 Google Inc.

#include "Expr.h"

#include "builder/Builder.h"
#include "Context.h"
#include "TypeDef.h"

using namespace model;

Expr::Expr(TypeDef *type) : type(type) {}

Expr::~Expr() {}

void Expr::emitCond(Context &context) {
    context.builder.emitTest(context, this);
}

ExprPtr Expr::convert(Context &context, TypeDef *newType) {
    
    // see if we're already of the right type
    if (newType->matches(*type))
        return this;

    // see if there's a converter
    FuncDefPtr converter = type->getConverter(context, *newType);
    if (converter) {
        FuncCallPtr convCall =
           context.builder.createFuncCall(converter.get());
        convCall->receiver = this;
        return convCall;
    }
    
    // can't convert
    return 0;
}

bool Expr::isProductive() const {
    // by default, all expresssions returning pointer types are productive
    return type->pointer;
}

bool Expr::isAdaptive() const {
    return false;
}

ExprPtr Expr::foldConstants() { return this; }

void Expr::dump() const { writeTo(std::cerr); }