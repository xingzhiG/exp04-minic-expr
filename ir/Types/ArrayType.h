///
/// @file ArrayType.h
/// @brief 数组类型描述类
/// @author Gxz (xingzhiguo1101@gmail.com)
/// @version 1.0
/// @date 2025-06-18
///
/// @copyright Copyright (c) 2025
///
/// @par 修改日志:
/// <table>
/// <tr><th>Date       <th>Version <th>Author  <th>Description
/// <tr><td>2025-06-18 <td>1.0     <td>Gxz     <td>新建
/// </table>
///

#pragma once

#include <string>

#include "Type.h"

class ArrayType : public Type {
public:
    ArrayType(Type* elemType, int numElements)
        : Type(Type::ArrayTyID), elemType(elemType), numElements(numElements) {}

    /// 获取当前数组的元素类型
    Type* getElementType() const { return elemType; }

    /// 获取当前数组的元素个数
    int getNumElements() const { return numElements; }

    /// 获取类型字符串表示
    std::string toString() const override {
        return "[" + std::to_string(numElements) + " x " + elemType->toString() + "]";
    }

    /// 获取数组总字节大小
    int getSize() const override {
        return numElements * elemType->getSize();
    }

    /// 判断是否为数组类型
    bool isArrayType() const { return true; }

private:
    Type* elemType;
    int numElements;
};