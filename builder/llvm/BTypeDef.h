// Copyright 2010 Shannon Weyrick <weyrick@mozek.us>
// Copyright 2010-2011 Google Inc.
//
//   This Source Code Form is subject to the terms of the Mozilla Public
//   License, v. 2.0. If a copy of the MPL was not distributed with this
//   file, You can obtain one at http://mozilla.org/MPL/2.0/.
//

#ifndef _builder_llvm_BTypeDef_h_
#define _builder_llvm_BTypeDef_h_

#include "model/TypeDef.h"
#include <llvm/Type.h>
#include <llvm/DebugInfo.h>
#include "model/Context.h"

#include <map>
#include <string>
#include <vector>

namespace llvm {
    class Type;
    class Constant;
    class ExecutionEngine;
    class GlobalVariable;
}

namespace builder {
namespace mvll {

class PlaceholderInstruction;
class VTableBuilder;

SPUG_RCPTR(BTypeDef);

// (RCPTR defined above)
class BTypeDef : public model::TypeDef {
public:
    unsigned fieldCount;
    llvm::GlobalVariable *classInst;
    llvm::Type *rep;
    llvm::DIType debugInfo; // debug info (wrapper for MDNode*)
    unsigned nextVTableSlot;
    std::vector<PlaceholderInstruction *> placeholders;
    typedef std::vector< std::pair<BTypeDefPtr, model::ContextPtr> >
        IncompleteChildVec;
    IncompleteChildVec incompleteChildren;

    // mapping from base types to their vtables.
    std::map<BTypeDef *, llvm::Constant *> vtables;
    llvm::Type *firstVTableType;

    BTypeDef(TypeDef *metaType, const std::string &name,
             llvm::Type *rep,
             bool pointer = false,
             unsigned nextVTableSlot = 0
             ) :
        model::TypeDef(metaType, name, pointer),
        fieldCount(0),
        classInst(0),
        rep(rep),
        nextVTableSlot(nextVTableSlot),
        firstVTableType(0) {
    }

    virtual void getDependents(std::vector<model::TypeDefPtr> &deps);

    // add all of my virtual functions to 'vtb'
    void extendVTables(VTableBuilder &vtb);

    /**
     * Create all of the vtables for 'type'.
     *
     * @param vtb the vtable builder
     * @param name the name stem for the VTable global variables.
     * @param firstVTable if true, we have not yet discovered the first
     *  vtable in the class schema.
     */
    void createAllVTables(VTableBuilder &vtb, const std::string &name,
                          bool firstVTable = true
                          );

    /**
     * Add a base class to the type.
     */
    void addBaseClass(BTypeDef *base);

    /**
     * register a placeholder instruction to be replaced when the class is
     * completed.
     */
    void addPlaceholder(PlaceholderInstruction *inst);

    /**
     * Find the ancestor with our first vtable.
     */
    BTypeDef *findFirstVTable(BTypeDef *vtableBaseType);

    /**
     * Get the global variable, creating an extern instance in the module if
     * it lives in another module.
     * @param execEng the execution engine (may be null if this is unknown or
     *  there is no execution engine)
     */
    llvm::GlobalVariable *getClassInstRep(llvm::Module *module,
                                          llvm::ExecutionEngine *execEng
                                          );

    /**
     * Add a dependent type - this is a nested type that is derived from an
     * incomplete enclosing type.  fixIncompletes() will be called on all of
     * the dependents during the completion of the receiver.
     */
    void addDependent(BTypeDef *type, model::Context *context);

    /**
     * Generate the base class offsets array global variable used for
     * offsetting during type casts.
     */
    void createBaseOffsets(model::Context &context) const;

    /**
     * Generate an empty offsets initializer array (useful for primitive
     * classes where createBaseOffsets() won't work).
     */
    void createEmptyOffsetsInitializer(model::Context &context);

    /**
     * Fix all incomplete instructions and incomplete children and set the
     * "complete" flag.
     */
    void fixIncompletes(model::Context &context);

    // See VarDef.
    virtual void onDeserialized(model::Context &context);

protected:

    /**
     * Utility method for use by array and function types.  If any of the
     * params in 'types' are stubs, returns a generic stub for the type.
     * Otherwise returns null.
     */
    model::TypeDefPtr getSpecializationStubSafe(
        model::Context &context,
        model::TypeDef::TypeVecObj *types
    );

};

} // end namespace builder::vmll
} // end namespace builder

#endif
