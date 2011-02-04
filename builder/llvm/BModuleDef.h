// Copyright 2010 Google Inc, Shannon Weyrick <weyrick@mozek.us>

#ifndef _builder_llvm_BModuleDef_h_
#define _builder_llvm_BModuleDef_h_

#include "model/ModuleDef.h"
#include <spug/RCPtr.h>
#include <string>

namespace model {
    class Context;
}

namespace builder {
namespace mvll {

SPUG_RCPTR(BModuleDef);

class BModuleDef : public model::ModuleDef {

public:
    // primitive cleanup function
    void (*cleanup)();

    BModuleDef(const std::string &canonicalName, model::Namespace *parent,
               model::Construct *construct
               ) :
        ModuleDef(canonicalName, parent, construct),
        cleanup(0) {
    }

    void callDestructor() {
        if (cleanup)
            cleanup();
    }
};

} // end namespace builder::vmll
} // end namespace builder

#endif
