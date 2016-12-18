//
// Created by svloyso on 20.11.16.
//

#ifndef MATHVM_CONTEXT_H
#define MATHVM_CONTEXT_H

#include <vector>
#include <stack>
#include <unordered_map>

#include "ast.h"
#include "mathvm.h"

namespace mathvm {

struct Context {
	Scope* scope = nullptr;
	std::stack<BytecodeFunction*> functions;
	std::unordered_map<AstVar*, std::pair<uint16_t, uint16_t> > variables;
	bool usedRegs[4] = {false, false, false, false};

	std::map<uint16_t, uint16_t> lastVarId;

	int freeReg() {
		for(int i = 0 ; i < 4; ++i) {
			if(!usedRegs[i]) {
				usedRegs[i] = true;
				return i;
			}
		}
		return -1;
	}

	Bytecode* bytecode() {
		return functions.top()->bytecode();
	}

	uint16_t declareVar(const std::string& name) {
		AstVar* var = varByName(name);
		assert(var != nullptr);
		uint16_t fid = functions.top()->id();
		uint16_t id = lastVarId[fid]++;
		variables[var] = std::make_pair(fid, id);
		return id;
	}

	AstVar* varByName(const std::string& name) {
		return scope->lookupVariable(name, true);
	}

	int varIdByName(const std::string& name) {
		AstVar* var = varByName(name);
		auto res = variables.find(var);
		if(res == variables.end()) {
			return -1;
		}
		return res->second.second;
	}

	int varFIdByName(const std::string& name) {
		AstVar* var = varByName(name);
		auto res = variables.find(var);
		if(res == variables.end()) {
			return -1;
		}
		return res->second.first;
	}
};

}


#endif //MATHVM_CONTEXT_H
