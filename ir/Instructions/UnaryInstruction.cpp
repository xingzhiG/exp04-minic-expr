///
/// @file UnaryInstruction.cpp
/// @brief 一元操作指令
///
/// @author Gxz (xingzhiguo1101@gmail.com)
/// @version 1.0
/// @date 2025-05-09
///
/// @copyright Copyright (c) 2025
///
/// @par 修改日志:
/// <table>
/// <tr><th>Date       <th>Version <th>Author  <th>Description
/// <tr><td>2025-05-09 <td>1.0     <td>Gxz     <td>新建
/// </table>
///
#include "UnaryInstruction.h"

/// @brief 构造函数
/// @param _op 操作符
/// @param _result 结果操作数
/// @param _srcVal 源操作数
UnaryInstruction::UnaryInstruction(Function * _func,
                                   IRInstOperator _op,
                                   Value * _srcVal,
                                   Type * _type)
    : Instruction(_func, _op, _type)
{
    addOperand(_srcVal);
}

/// @brief 转换成字符串
/// @param str 转换后的字符串
void UnaryInstruction::toString(std::string & str)
{
    Value *src = getOperand(0);

    switch (op) {
        case IRInstOperator::IRINST_OP_NEG_I:
            // 取负指令，单目运算
            str = getIRName() + " = neg " + src->getIRName();
            break;

        default:
            // 未知指令
            Instruction::toString(str);
            break;
    }
}