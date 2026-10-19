// SPDX-License-Identifier: GPL-2.0-or-later
// V_CNDMASK_B32 must preserve subnormal-looking register bits used by CTR's fur shader.
#include <cassert>
#include "shader_recompiler/frontend/translate/translate.h"
#include "shader_recompiler/profile.h"

int main() {
    using namespace Shader;
    Info info{};
    info.hw_stage = HwStage::Fragment;
    info.sw_stage = SwStage::Fragment;
    RuntimeInfo runtime{};
    runtime.Initialize(info.hw_stage, info.sw_stage);
    Profile profile{};
    Common::ObjectPool<IR::Inst> pool;
    IR::Block block(pool);

    Gcn::GcnInst inst{};
    inst.opcode = Gcn::Opcode::V_CNDMASK_B32;
    inst.category = Gcn::InstCategory::VectorALU;
    inst.length = 4;
    inst.src[0].field = Gcn::OperandField::LiteralConst;
    inst.src[0].code = 3;
    inst.src[1].field = Gcn::OperandField::LiteralConst;
    inst.src[1].code = 2;
    inst.dst[0].field = Gcn::OperandField::VectorGPR;
    inst.dst[0].code = 0;

    Gcn::Translator translator(info, runtime, profile);
    translator.Translate(&block, 0, IR::Condition::True, std::span{&inst, 1});

    int selects = 0;
    for (const auto& ir_inst : block.Instructions()) {
        if (ir_inst.GetOpcode() == IR::Opcode::SelectU32) {
            ++selects;
            assert(ir_inst.Arg(1).Type() == IR::Type::U32);
            assert(ir_inst.Arg(2).Type() == IR::Type::U32);
            assert(ir_inst.Arg(1).U32() == 2);
            assert(ir_inst.Arg(2).U32() == 3);
        }
        assert(ir_inst.GetOpcode() != IR::Opcode::SelectF32);
    }
    assert(selects == 1);
}
