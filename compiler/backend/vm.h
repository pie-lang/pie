#ifndef __PIE_BACKEND_VM__
#define __PIE_BACKEND_VM__

#include "compiler/ast.h"
#include "compiler/ast/op.h"
#include "compiler/ast/primitive.h"
#include "compiler/ast/assign.h"

#include "runtime/vm/bytecode.h"

namespace pie { namespace compiler {


class VmCompilerVistor : Visitor 
{

};

}}
#endif

