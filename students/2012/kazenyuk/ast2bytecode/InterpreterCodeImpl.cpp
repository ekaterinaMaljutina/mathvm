#include <iostream>
#include <vector>
#include <string>
#include <map>

#include "InterpreterCodeImpl.h"

namespace mathvm {

inline const char* bcName(Instruction insn, size_t& length) {
    static const struct {
        const char* name;
        Instruction insn;
        size_t length;
    } names[] = {
#define BC_NAME(b, d, l) {#b, BC_##b, l},
        FOR_BYTECODES(BC_NAME)
    };

    if (insn >= BC_INVALID && insn < BC_LAST) {
        length = names[insn].length;
        return names[insn].name;
    }

    assert(false);
    return 0;
}

class ByteStorage {
private:
    vector<uint8_t> _data;
public:

    uint8_t get(uint32_t index) const {
        if (index >= _data.size()) {
           return 0;
        }
        return _data[index];
    }

    void set(uint32_t index, uint8_t v) {
        if (index >= _data.size()) {
            _data.resize(index+1);
        }
        _data[index] = v;
    }

    template<class T> T getTyped() const {
        union {
            T val;
            uint8_t bits[sizeof(T)];
        } u;
        for (uint32_t i=0; i<sizeof(u.bits); i++) {
            u.bits[i] = get(i);
        }
        return u.val;
    }

    template<class T> void setTyped(T d) {
        union {
            T val;
            uint8_t bits[sizeof(T)];
        } u;

        u.val = d;
        for (uint32_t i=0; i<sizeof(u.bits); i++) {
            set(i, u.bits[i]);
        }
    }

    uint8_t getByte() const {
        return get(0);
    }

    void setByte(uint8_t v) {
        set(0, v);
    }

    double getDouble() const {
        return getTyped<double>();
    }

    void setDouble(double d) {
        setTyped<double>(d);
    }

    int16_t getInt16() const {
        return getTyped<int16_t>();
    }

    void setInt16(int16_t value) {
        setTyped<int16_t>(value);
    }

    uint16_t getUInt16() const {
        return getTyped<uint16_t>();
    }

    void setUInt16(uint16_t value) {
        setTyped<uint16_t>(value);
    }

    int32_t getInt32() const {
        return getTyped<int32_t>();
    }

    void setInt32(int32_t value) {
        setTyped<int32_t>(value);
    }

    int64_t getInt64() const {
        return getTyped<int64_t>();
    }

    void setInt64(int64_t value) {
        setTyped<int64_t>(value);
    }

};

class ByteStack {
 protected:
    vector<uint8_t> _data;

 public:
    void push(uint8_t b) {
        _data.push_back(b);
    }

    uint8_t pop() {
        uint8_t val = _data.back();
        _data.pop_back();
        return val;
    }

    template<class T> T popTyped() {
        union {
            T val;
            uint8_t bits[sizeof(T)];
        } u;
//        for (uint32_t i=0; i<sizeof(u.bits); i++) {
//            u.bits[i] = pop();
//        }
        for (uint32_t i=sizeof(u.bits); i>0; i--) {
            u.bits[i-1] = pop();
        }
        return u.val;
    }

    template<class T> void pushTyped(T d) {
        union {
            T val;
            uint8_t bits[sizeof(T)];
        } u;

        u.val = d;
        for (uint32_t i=0; i<sizeof(u.bits); i++) {
            push(u.bits[i]);
        }
    }

    void pushByte(uint8_t b) {
        push(b);
    }

    uint8_t popByte() {
        return pop();
    }

    double popDouble() {
        return popTyped<double>();
    }

    void pushDouble(double d) {
        pushTyped<double>(d);
    }

    int16_t popInt16() {
        return popTyped<int16_t>();
    }

    void pushInt16(int16_t value) {
        pushTyped<int16_t>(value);
    }

    uint16_t popUInt16() {
        return popTyped<uint16_t>();
    }

    void pushUInt16(uint16_t value) {
        pushTyped<uint16_t>(value);
    }

    int32_t popInt32() {
        return popTyped<int32_t>();
    }

    void pushInt32(int32_t value) {
        pushTyped<int32_t>(value);
    }

    int64_t popInt64() {
        return popTyped<int64_t>();
    }

    void pushInt64(int64_t value) {
        pushTyped<int64_t>(value);
    }
};

Status* InterpreterCodeImpl::execute(std::vector<mathvm::Var*>&) {
    ByteStack stack;
    typedef std::map<uint16_t, ByteStorage> VarMap;
    VarMap var_map;

    BytecodeFunction* function = (BytecodeFunction*) functionById(0);
    Bytecode* bytecode = function->bytecode();

    for (size_t bci = 0; bci < bytecode->length();) {
        size_t length;
        Instruction insn = bytecode->getInsn(bci);
        const char* name = bcName(insn, length);
        std::clog << bci << ": " << name << std::endl;
        switch (insn) {
            case BC_INVALID:
                return new Status(name, bci);
                break;
            case BC_DLOAD:
                stack.pushTyped(bytecode->getDouble(bci + 1));
                break;
            case BC_ILOAD:
                stack.pushTyped(bytecode->getInt64(bci + 1));
                break;
            case BC_SLOAD:
                stack.pushTyped(bytecode->getUInt16(bci + 1));
                break;
            case BC_DLOAD0:
                stack.pushTyped(double(0));
                break;
            case BC_ILOAD0:
                stack.pushTyped(int64_t(0));
                break;
            case BC_SLOAD0:
                stack.pushTyped(uint16_t(0));
                break;
            case BC_DLOAD1:
                stack.pushTyped(double(1));
                break;
            case BC_ILOAD1:
                stack.pushTyped(int64_t(1));
                break;
            case BC_DADD:
                stack.pushDouble(stack.popDouble() + stack.popDouble());
                break;
            case BC_IADD:
                stack.pushInt64(stack.popInt64() + stack.popInt64());
                break;
            case BC_IPRINT:
                std::cout << stack.popInt64();
                break;
            case BC_DPRINT:
                std::cout << stack.popDouble();
                break;
            case BC_SPRINT:
                std::cout << constantById(stack.popUInt16());
                break;
            case BC_POP:
                stack.pop();
                break;

            case BC_LOADIVAR0:
                stack.pushInt64(var_map[0].getInt64());
                break;
            case BC_LOADIVAR1:
                stack.pushInt64(var_map[1].getInt64());
                break;
            case BC_LOADIVAR2:
                stack.pushInt64(var_map[2].getInt64());
                break;
            case BC_LOADIVAR3:
                stack.pushInt64(var_map[3].getInt64());
                break;

            case BC_STOREDVAR0:
                var_map[0].setDouble(stack.popDouble());
                break;
            case BC_STOREIVAR0:
                var_map[0].setInt64(stack.popInt64());
                break;
            case BC_STORESVAR0:
                var_map[0].setUInt16(stack.popUInt16());
                break;

            case BC_LOADDVAR:
                stack.pushDouble(var_map[bytecode->getUInt16(bci + 1)].getDouble());
                break;
            case BC_LOADIVAR:
                stack.pushInt64(var_map[bytecode->getUInt16(bci + 1)].getInt64());
                break;
            case BC_LOADSVAR:
                stack.pushUInt16(var_map[bytecode->getUInt16(bci + 1)].getUInt16());
                break;

            case BC_STOREDVAR:
                var_map[bytecode->getUInt16(bci + 1)].setDouble(stack.popDouble());
                break;
            case BC_STOREIVAR:
                var_map[bytecode->getUInt16(bci + 1)].setInt64(stack.popInt64());
                break;
            case BC_STORESVAR:
            //                  out << name << " @" << getUInt16(bci + 1);
                var_map[bytecode->getUInt16(bci + 1)].setUInt16(stack.popUInt16());
                break;

            //              case BC_LOADCTXDVAR:
            //              case BC_STORECTXDVAR:
            //              case BC_LOADCTXIVAR:
            //              case BC_STORECTXIVAR:
            //              case BC_LOADCTXSVAR:
            //              case BC_STORECTXSVAR:
            ////                  out << name << " @" << getUInt16(bci + 1)
            ////                      << ":" << getUInt16(bci + 3);
            //                  break;
            case BC_IFICMPNE:
                if (stack.popInt64() != stack.popInt64()) {
                    bci += bytecode->getInt16(bci + 1) + 1;
                    continue;
                }
                break;
            case BC_IFICMPE:
                if (stack.popInt64() == stack.popInt64()) {
                    bci += bytecode->getInt16(bci + 1) + 1;
                    continue;
                }
                break;
            case BC_IFICMPG:
                if (stack.popInt64() > stack.popInt64()) {
                    bci += bytecode->getInt16(bci + 1) + 1;
                    continue;
                }
                break;
            case BC_IFICMPGE:
                if (stack.popInt64() >= stack.popInt64()) {
                    bci += bytecode->getInt16(bci + 1) + 1;
                    continue;
                }
                break;
            case BC_IFICMPL:
                if (stack.popInt64() < stack.popInt64()) {
                    bci += bytecode->getInt16(bci + 1) + 1;
                    continue;
                }
                break;
            case BC_IFICMPLE:
                if (stack.popInt64() <= stack.popInt64()) {
                    bci += bytecode->getInt16(bci + 1) + 1;
                    continue;
                }
                break;
            case BC_JA:
            //                  out << name << " " << getInt16(bci + 1) + bci + 1;
                bci += bytecode->getInt16(bci + 1) + 1;
                continue;
                break;
            case BC_CALL:{
                stack.pushTyped(bci + length);
                stack.pushTyped(function->id());

                std::clog << "saving return address: " << function->id() << ":" << bci + length << std::endl;
                uint16_t f = stack.popUInt16();
                size_t b = stack.popTyped<size_t>();
                std::clog << "checking return address: " << f << ":" << b << std::endl;
                stack.pushTyped(bci + length);
                stack.pushTyped(function->id());

                function = (BytecodeFunction*) functionById(bytecode->getUInt16(bci + 1));
                if (!function) {
                  return new Status("Unresolved function ID\n", bci);
                }
                bytecode = function->bytecode();
                bci = 0;
                continue;
                break;}
            case BC_CALLNATIVE:
                //                  out << name << " *" << getUInt16(bci + 1);
                return new Status("Native functions are currently not supported\n", bci);
                break;
            case BC_RETURN: {
                uint16_t new_function_id = stack.popUInt16();
                std::clog << "new func id=" << new_function_id << std::endl;
                function = (BytecodeFunction*) functionById(new_function_id);
                if (!function) {
                  return new Status("Unresolved function ID\n", bci);
                }
                bytecode = function->bytecode();
                size_t new_bci = stack.popTyped<size_t>();
                std::clog << "new bci=" << new_bci << std::endl;
                bci = new_bci;
                continue;
                break;
            }
            case BC_BREAK:
                return new Status("Breakpoints are currently not supported\n", bci);
                break;
            default:
                //                  out << name;
                return new Status("Unknown or unsupported instruction\n", bci);
        }
        //          out << endl;
        bci += length;
    }
    std::cout << "Result = " << var_map[0].getInt64() << std::endl;
    return 0;
}

}   // namespace mathvm