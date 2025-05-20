///
/// @file BranchCondInstruction.h
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

#pragma once

#include "Instruction.h"
#include "LabelInstruction.h"
#include "Function.h"

///
/// @brief 条件跳转指令
///
class BranchCondInstruction : public Instruction {
public:
    BranchCondInstruction(Function* func, Value* cond, LabelInstruction* trueLabel, LabelInstruction* falseLabel);

    void toString(std::string& str) override;

    Value* getCond() const;
    LabelInstruction* getTrueLabel() const;
    LabelInstruction* getFalseLabel() const;

private:
    Value* cond;
    LabelInstruction* trueLabel;
    LabelInstruction* falseLabel;
};