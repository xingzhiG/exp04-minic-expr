///
/// @file GlobalVariable.h
/// @brief 全局变量描述类
///
/// @author zenglj (zenglj@live.com)
/// @version 1.0
/// @date 2024-09-29
///
/// @copyright Copyright (c) 2024
///
/// @par 修改日志:
/// <table>
/// <tr><th>Date       <th>Version <th>Author  <th>Description
/// <tr><td>2024-09-29 <td>1.0     <td>zenglj  <td>新建
/// </table>
///
#pragma once

#include "GlobalValue.h"
#include "IRConstant.h"
#include "ConstInt.h"

#include "Types/ArrayType.h"

///
/// @brief 全局变量，寻址时通过符号名或变量名来寻址
///
class GlobalVariable : public GlobalValue {

public:
    ///
    /// @brief 构建全局变量，默认对齐为4字节
    /// @param _type 类型
    /// @param _name 名字
    /// @param _initValue 初始值（可选）
    ///
    explicit GlobalVariable(Type * _type, std::string _name, Constant * _initValue = nullptr) 
        : GlobalValue(_type, _name), initializer(_initValue)
    {
        // 设置对齐大小
        setAlignment(4);
        
        // 如果有非零初始值，则不在BSS段
        if (_initValue && !isZeroInitializer(_initValue)) {
            inBSSSection = false;
        }
    }

    ///
    /// @brief  检查是否是函数
    /// @return true 是函数
    /// @return false 不是函数
    ///
    [[nodiscard]] bool isGlobalVarible() const override
    {
        return true;
    }

    ///
    /// @brief 是否属于BSS段的变量，即未初始化过的变量，或者初值都为0的变量
    /// @return true
    /// @return false
    ///
    [[nodiscard]] bool isInBSSSection() const
    {
        return this->inBSSSection;
    }

    ///
    /// @brief 取得变量所在的作用域层级
    /// @return int32_t 层级
    ///
    int32_t getScopeLevel() override
    {
        return 0;
    }

    ///
    /// @brief 对该Value进行Load用的寄存器编号
    /// @return int32_t 寄存器编号
    ///
    int32_t getLoadRegId() override
    {
        return this->loadRegNo;
    }

    ///
    /// @brief 对该Value进行Load用的寄存器编号
    /// @return int32_t 寄存器编号
    ///
    void setLoadRegId(int32_t regId) override
    {
        this->loadRegNo = regId;
    }

    ///
    /// @brief 设置初始值
    /// @param _initValue 初始值
    ///
    void setInitializer(Constant * _initValue)
    {
        initializer = _initValue;
        if (_initValue && !isZeroInitializer(_initValue)) {
            inBSSSection = false;
        }
    }

    ///
    /// @brief 获取初始值
    /// @return 初始值，可能为nullptr
    ///
    Constant * getInitializer() const
    {
        return initializer;
    }

    ///
    /// @brief 检查是否有初始值
    /// @return true: 有初始值，false: 无初始值
    ///
    bool hasInitializer() const
    {
        return initializer != nullptr;
    }

    ///
    /// @brief Declare指令IR显示，支持初始值
    /// @param str
    ///
    void toDeclareString(std::string & str)
    {
        // 递归剥离数组类型，收集所有维度
        Type * ty = getType();
        std::vector<int> dims;
        while (ty->isArrayType()) {
            auto * arrTy = dynamic_cast<ArrayType *>(ty);
            if (arrTy) {
                dims.push_back(arrTy->getNumElements());
                ty = arrTy->getElementType();
            } else {
                break;
            }
        }

        // 输出基础类型
        str = "declare " + ty->toString() + " " + getIRName();

        // 追加所有维度
        for (int dim : dims) {
            str += "[" + std::to_string(dim) + "]";
        }

        // 添加初始值
        if (hasInitializer()) {
            str += " = " + initializer->getIRName();
        }
    }

private:
    ///
    /// @brief 变量加载到寄存器中时对应的寄存器编号
    ///
    int32_t loadRegNo = -1;

    ///
    /// @brief 默认全局变量在BSS段，没有初始化，或者即使初始化过，但都值都为0
    ///
    bool inBSSSection = true;

    ///
    /// @brief 全局变量的初始值
    ///
    Constant * initializer = nullptr;

    ///
    /// @brief 检查常量是否为零初始化
    /// @param constant 常量
    /// @return true: 是零值，false: 不是零值
    ///
    bool isZeroInitializer(Constant * constant) const
    {
        if (auto constInt = dynamic_cast<ConstInt*>(constant)) {
            return constInt->getVal() == 0;
        }
        // 可以添加其他类型的零值检查
        return false;
    }
};