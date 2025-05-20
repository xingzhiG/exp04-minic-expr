///
/// @file BranchCondInstruction.cpp
/// @brief 条件跳转指令
///
/// @author Gxz (xingzhiguo1101@gmail.com)
/// @version 1.0
/// @date 2025-05-20
///
/// @copyright Copyright (c) 2025
///
/// @par 修改日志:
/// <table>
/// <tr><th>Date       <th>Version <th>Author  <th>Description
/// <tr><td>2025-05-20 <td>1.0     <td>Gxz     <td>内容
/// </table>
///


#include "BranchCondInstruction.h"
#include "VoidType.h"

BranchCondInstruction::BranchCondInstruction(Function* func, Value* cond, LabelInstruction* trueLabel, LabelInstruction* falseLabel)
    : Instruction(func, IRInstOperator::IRINST_OP_BRANCH_COND, VoidType::getType()),
      cond(cond), trueLabel(trueLabel), falseLabel(falseLabel) {}

void BranchCondInstruction::toString(std::string& str) {
    str = "bc " + cond->getIRName() + ", label " + trueLabel->getIRName() + ", label " + falseLabel->getIRName();
}

Value* BranchCondInstruction::getCond() const { return cond; }
LabelInstruction* BranchCondInstruction::getTrueLabel() const { return trueLabel; }
LabelInstruction* BranchCondInstruction::getFalseLabel() const { return falseLabel; }