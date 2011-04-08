// Copyright 2010 Google Inc., Shannon Weyrick <weyrick@mozek.us>
//
// This is a simple stub that defines the public extension interface used
// during compile-time extension initialization. This stub is only used when
// linking AOT crack binaries to resolve the (unused at runtime) init functions.
// This is somewhat of a hack, and may be better replaced by a scheme that
// keeps the compile-time portions of extension init (i.e. type definitions, etc)
// separate from the runtime portions.

#ifndef _crack_ext_Stub_h_
#define _crack_ext_Stub_h_

#include <string>
#include <stdint.h>
#include <vector>

namespace model {
    class Context;
}

namespace crack { namespace ext {

struct Arg;
class Module;
class Func;
class Type;

class Func {

    public:

        void setIsVariadic(bool isVariadic);
        bool isVariadic();
        void setSymbolName(const std::string &name);
        void addArg(Type *type, const std::string &name);
        void finish();

};

class Module {
    public:
        Module(model::Context *context);

        Type *getClassType();
        Type *getVoidType();
        Type *getVoidptrType();
        Type *getBoolType();
        Type *getByteptrType();
        Type *getByteType();
        Type *getInt32Type();
        Type *getInt64Type();
        Type *getUint32Type();
        Type *getUint64Type();
        Type *getIntType();
        Type *getUintType();
        Type *getFloat32Type();
        Type *getFloat64Type();
        Type *getFloatType();
        Type *getVTableBaseType();
        Type *getObjectType();
        Type *getStringType();
        Type *getStaticStringType();
        Type *getOverloadType();
        
        Type *getType(const char *name);
        Type *addType(const char *name);
        Func *addFunc(Type *returnType, const char *name, void *funcPtr,
                      const char *symbolName=0);
        void addConstant(Type *type, const std::string &name, double val);
        void addConstant(Type *type, const std::string &name, int64_t val);
        void addConstant(Type *type, const std::string &name, int val);
        void finish();
};

class Type {
    private:

        void checkFinished();

    public:
        
        void addBase(Type *base);
        void addInstVar(Type *type, const std::string &name);

        Func *addMethod(Type *returnType, const std::string &name,
                        void *funcPtr
                        );

        Func *addConstructor(const char *name = 0, void *funcPtr = 0);

        Func *addStaticMethod(Type *returnType, const std::string &name,
                              void *funcPtr
                              );

        Type *getSpecialization(const std::vector<Type *> &params);
        void finish();
};
    

}} // namespace crack::ext

#endif