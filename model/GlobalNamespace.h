// Copyright 2010 Google Inc.

#ifndef _model_GlobalNamespace_h_
#define _model_GlobalNamespace_h_

#include "LocalNamespace.h"

namespace model {

SPUG_RCPTR(GlobalNamespace);

class GlobalNamespace : public LocalNamespace {
    public:
        GlobalNamespace(Namespace *parent, const std::string &cName,
                        Construct *construct
                        ) :
                LocalNamespace(parent, cName, construct) {
        }
};

} // namespace model

#endif
