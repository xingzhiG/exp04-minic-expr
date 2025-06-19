///
/// @file IRGenerator.cpp
/// @brief AST遍历产生线性IR的源文件
/// @author zenglj (zenglj@live.com)
/// @version 1.1
/// @date 2024-11-23
///
/// @copyright Copyright (c) 2024
///
/// @par 修改日志:
/// <table>
/// <tr><th>Date       <th>Version <th>Author  <th>Description
/// <tr><td>2024-09-29 <td>1.0     <td>zenglj  <td>新建
/// <tr><td>2024-11-23 <td>1.1     <td>zenglj  <td>表达式版增强
/// </table>
///
#include <cstdint>
#include <cstdio>
#include <unordered_map>
#include <vector>
#include <iostream>

#include "AST.h"
#include "Common.h"
#include "Function.h"
#include "IRCode.h"
#include "IRGenerator.h"
#include "Module.h"
#include "EntryInstruction.h"
#include "LabelInstruction.h"
#include "ExitInstruction.h"
#include "FuncCallInstruction.h"
#include "BinaryInstruction.h"
#include "MoveInstruction.h"
#include "GotoInstruction.h"
#include "UnaryInstruction.h"
#include "BranchCondInstruction.h"

#include "ir/Types/ArrayType.h"
#include "ir/Types/PointerType.h"

/// @brief 构造函数
/// @param _root AST的根
/// @param _module 符号表
IRGenerator::IRGenerator(ast_node * _root, Module * _module) : root(_root), module(_module)
{
    /* 叶子节点 */
    ast2ir_handlers[ast_operator_type::AST_OP_LEAF_LITERAL_UINT] = &IRGenerator::ir_leaf_node_uint;
    ast2ir_handlers[ast_operator_type::AST_OP_LEAF_VAR_ID] = &IRGenerator::ir_leaf_node_var_id;
    ast2ir_handlers[ast_operator_type::AST_OP_LEAF_TYPE] = &IRGenerator::ir_leaf_node_type;

    /* 表达式运算 */
    ast2ir_handlers[ast_operator_type::AST_OP_SUB] = &IRGenerator::ir_sub;
    ast2ir_handlers[ast_operator_type::AST_OP_ADD] = &IRGenerator::ir_add;
	ast2ir_handlers[ast_operator_type::AST_OP_MUL] = &IRGenerator::ir_mul;
	ast2ir_handlers[ast_operator_type::AST_OP_DIV] = &IRGenerator::ir_div;
	ast2ir_handlers[ast_operator_type::AST_OP_MOD] = &IRGenerator::ir_mod;
	ast2ir_handlers[ast_operator_type::AST_OP_NEG] = &IRGenerator::ir_neg;

	/* 关系、逻辑运算 */
	ast2ir_handlers[ast_operator_type::AST_OP_EQ] = &IRGenerator::ir_eq;
	ast2ir_handlers[ast_operator_type::AST_OP_NE] = &IRGenerator::ir_ne;
	ast2ir_handlers[ast_operator_type::AST_OP_LT] = &IRGenerator::ir_lt;
	ast2ir_handlers[ast_operator_type::AST_OP_LE] = &IRGenerator::ir_le;
	ast2ir_handlers[ast_operator_type::AST_OP_GT] = &IRGenerator::ir_gt;
	ast2ir_handlers[ast_operator_type::AST_OP_GE] = &IRGenerator::ir_ge;
	ast2ir_handlers[ast_operator_type::AST_OP_AND] = &IRGenerator::ir_and;
	ast2ir_handlers[ast_operator_type::AST_OP_OR] = &IRGenerator::ir_or;
	ast2ir_handlers[ast_operator_type::AST_OP_NOT] = &IRGenerator::ir_not;

    /* 语句 */
    ast2ir_handlers[ast_operator_type::AST_OP_ASSIGN] = &IRGenerator::ir_assign;
    ast2ir_handlers[ast_operator_type::AST_OP_RETURN] = &IRGenerator::ir_return;

	/* 条件语句 */
	ast2ir_handlers[ast_operator_type::AST_OP_IF] = &IRGenerator::ir_if;
	ast2ir_handlers[ast_operator_type::AST_OP_WHILE] = &IRGenerator::ir_while;
    ast2ir_handlers[ast_operator_type::AST_OP_BREAK] = &IRGenerator::ir_break;
    ast2ir_handlers[ast_operator_type::AST_OP_CONTINUE] = &IRGenerator::ir_continue;

    /* 函数调用 */
    ast2ir_handlers[ast_operator_type::AST_OP_FUNC_CALL] = &IRGenerator::ir_function_call;

    /* 函数定义 */
    ast2ir_handlers[ast_operator_type::AST_OP_FUNC_DEF] = &IRGenerator::ir_function_define;
    ast2ir_handlers[ast_operator_type::AST_OP_FUNC_FORMAL_PARAMS] = &IRGenerator::ir_function_formal_params;

    /* 变量定义语句 */
    ast2ir_handlers[ast_operator_type::AST_OP_DECL_STMT] = &IRGenerator::ir_declare_statment;
    ast2ir_handlers[ast_operator_type::AST_OP_VAR_DECL] = &IRGenerator::ir_variable_declare;

	/* 数组访问 */
	ast2ir_handlers[ast_operator_type::AST_OP_ARRAY_ACCESS] = &IRGenerator::ir_array_access;
	ast2ir_handlers[ast_operator_type::AST_OP_ARRAY_DECL] = &IRGenerator::ir_array_decl;

    /* 语句块 */
    ast2ir_handlers[ast_operator_type::AST_OP_BLOCK] = &IRGenerator::ir_block;

    /* 编译单元 */
    ast2ir_handlers[ast_operator_type::AST_OP_COMPILE_UNIT] = &IRGenerator::ir_compile_unit;
}

/// @brief 遍历抽象语法树产生线性IR，保存到IRCode中
/// @param root 抽象语法树
/// @param IRCode 线性IR
/// @return true: 成功 false: 失败
bool IRGenerator::run()
{
    ast_node * node;

    // 从根节点进行遍历
    node = ir_visit_ast_node(root);

    return node != nullptr;
}

/// @brief 根据AST的节点运算符查找对应的翻译函数并执行翻译动作
/// @param node AST节点
/// @return 成功返回node节点，否则返回nullptr
ast_node * IRGenerator::ir_visit_ast_node(ast_node * node)
{
    // 空节点
    if (nullptr == node) {
        return nullptr;
    }

    bool result;

    std::unordered_map<ast_operator_type, ast2ir_handler_t>::const_iterator pIter;
    pIter = ast2ir_handlers.find(node->node_type);
    if (pIter == ast2ir_handlers.end()) {
        // 没有找到，则说明当前不支持
        result = (this->ir_default)(node);
    } else {
        result = (this->*(pIter->second))(node);
    }

    if (!result) {
        // 语义解析错误，则出错返回
        node = nullptr;
    }

    return node;
}

/// @brief 未知节点类型的节点处理
/// @param node AST节点
/// @return 翻译是否成功，true：成功，false：失败
bool IRGenerator::ir_default(ast_node * node)
{
    // 未知的节点
    printf("Unkown node(%d)\n", (int) node->node_type);
    return true;
}

/// @brief 编译单元AST节点翻译成线性中间IR
/// @param node AST节点
/// @return 翻译是否成功，true：成功，false：失败
bool IRGenerator::ir_compile_unit(ast_node * node)
{
    module->setCurrentFunction(nullptr);

    for (auto son: node->sons) {

        // 遍历编译单元，要么是函数定义，要么是语句
        ast_node * son_node = ir_visit_ast_node(son);
        if (!son_node) {
            // TODO 自行追加语义错误处理
            return false;
        }
    }

    return true;
}

/// @brief 函数定义AST节点翻译成线性中间IR
/// @param node AST节点
/// @return 翻译是否成功，true：成功，false：失败
bool IRGenerator::ir_function_define(ast_node * node)
{
    bool result;

    // 创建一个函数，用于当前函数处理
    if (module->getCurrentFunction()) {
        // 函数中嵌套定义函数，这是不允许的，错误退出
        // TODO 自行追加语义错误处理
        return false;
    }

    // 函数定义的AST包含四个孩子
    // 第一个孩子：函数返回类型
    // 第二个孩子：函数名字
    // 第三个孩子：形参列表
    // 第四个孩子：函数体即block
    ast_node * type_node = node->sons[0];
    ast_node * name_node = node->sons[1];
    ast_node * param_node = node->sons[2];
    ast_node * block_node = node->sons[3];

    // 创建一个新的函数定义
    Function * newFunc = module->newFunction(name_node->name, type_node->type);
    if (!newFunc) {
        // 新定义的函数已经存在，则失败返回。
        // TODO 自行追加语义错误处理
        return false;
    }

    // 当前函数设置有效，变更为当前的函数
    module->setCurrentFunction(newFunc);

    // 进入函数的作用域
    module->enterScope();

    // 获取函数的IR代码列表，用于后面追加指令用，注意这里用的是引用传值
    InterCode & irCode = newFunc->getInterCode();

    // 这里也可增加一个函数入口Label指令，便于后续基本块划分

    // 创建并加入Entry入口指令
    irCode.addInst(new EntryInstruction(newFunc));

    // 创建出口指令并不加入出口指令，等函数内的指令处理完毕后加入出口指令
    LabelInstruction * exitLabelInst = new LabelInstruction(newFunc);

    // 函数出口指令保存到函数信息中，因为在语义分析函数体时return语句需要跳转到函数尾部，需要这个label指令
    newFunc->setExitLabel(exitLabelInst);

    // 遍历形参，没有IR指令，不需要追加
    result = ir_function_formal_params(param_node);
    if (!result) {
        // 形参解析失败
        // TODO 自行追加语义错误处理
        return false;
    }
    node->blockInsts.addInst(param_node->blockInsts);

    // 新建一个Value，用于保存函数的返回值，如果没有返回值可不用申请
    LocalVariable * retValue = nullptr;
    if (!type_node->type->isVoidType()) {

        // 保存函数返回值变量到函数信息中，在return语句翻译时需要设置值到这个变量中
        retValue = static_cast<LocalVariable *>(module->newVarValue(type_node->type));
        newFunc->setReturnValue(retValue);
    
		// 这里最好设置返回值变量的初值为0，以便在没有返回值时能够返回0
		// if (name_node->name == "main") {
        //     MoveInstruction *initRet = new MoveInstruction(newFunc, retValue, module->newConstInt(0));
        //     irCode.addInst(initRet);
        // }
	} else {
        newFunc->setReturnValue(nullptr);
    }

    // 函数内已经进入作用域，内部不再需要做变量的作用域管理
    block_node->needScope = false;

    // 遍历block
    result = ir_block(block_node);
    if (!result) {
        // block解析失败
        // TODO 自行追加语义错误处理
        return false;
    }

    // IR指令追加到当前的节点中
    node->blockInsts.addInst(block_node->blockInsts);

    // 此时，所有指令都加入到当前函数中，也就是node->blockInsts

    // node节点的指令移动到函数的IR指令列表中
    irCode.addInst(node->blockInsts);

    // 添加函数出口Label指令，主要用于return语句跳转到这里进行函数的退出
    irCode.addInst(exitLabelInst);

    // 函数出口指令
    irCode.addInst(new ExitInstruction(newFunc, retValue));

    // 恢复成外部函数
    module->setCurrentFunction(nullptr);

    // 退出函数的作用域
    module->leaveScope();

    return true;
}

/// @brief 形式参数AST节点翻译成线性中间IR
/// @param node AST节点
/// @return 翻译是否成功，true：成功，false：失败
bool IRGenerator::ir_function_formal_params(ast_node * node)
{
    Function * func = module->getCurrentFunction();
    if (!func || !node) return true;

    // 遍历所有形参
    for (size_t i = 0; i < node->sons.size(); ++i) {
        auto *paramAst = node->sons[i];
        if (!paramAst || paramAst->sons.empty() || !paramAst->sons[0]) continue;
        std::string paramName = paramAst->sons[0]->name;
        Type * paramType = paramAst->sons[0]->type;

        // 1. 创建FormalParam并加入函数参数列表
        FormalParam* formalParam = new FormalParam{paramType, paramName};
        func->getParams().push_back(formalParam);

        // 2. 创建局部变量Value（函数体内用），自动插入作用域
        LocalVariable * localVar = static_cast<LocalVariable *>(module->newVarValue(paramType, paramName));

        // 3. 入口参数Value（参数顺序与形参一致）
		Value * paramValue = func->getParams()[i]; // 入口参数Value

		// 生成赋值指令，将入口参数赋值给局部变量
		MoveInstruction * assignInst = new MoveInstruction(func, localVar, paramValue);
		node->blockInsts.addInst(assignInst);
    }
    return true;
}

/// @brief 函数调用AST节点翻译成线性中间IR
/// @param node AST节点
/// @return 翻译是否成功，true：成功，false：失败
bool IRGenerator::ir_function_call(ast_node * node)
{
    std::vector<Value *> realParams;

    // 获取当前正在处理的函数
    Function * currentFunc = module->getCurrentFunction();

    // 函数调用的节点包含两个节点：
    // 第一个节点：函数名节点
    // 第二个节点：实参列表节点

    std::string funcName = node->sons[0]->name;
    int64_t lineno = node->sons[0]->line_no;

    ast_node * paramsNode = node->sons[1];

    // 根据函数名查找函数，看是否存在。若不存在则出错
    auto calledFunction = module->findFunction(funcName);
    if (nullptr == calledFunction) {
        minic_log(LOG_ERROR, "函数(%s)未定义或声明", funcName.c_str());
        return false;
    }

    // 当前函数存在函数调用
    currentFunc->setExistFuncCall(true);

    // 如果没有孩子，也认为是没有参数
    if (!paramsNode->sons.empty()) {

        int32_t argsCount = (int32_t) paramsNode->sons.size();

        // 当前函数中调用函数实参个数最大值统计
        if (argsCount > currentFunc->getMaxFuncCallArgCnt()) {
            currentFunc->setMaxFuncCallArgCnt(argsCount);
        }

        // 遍历参数列表，孩子是表达式
        for (auto son: paramsNode->sons) {

            // 递归处理参数表达式
            ast_node * temp = ir_visit_ast_node(son);
            if (!temp) {
                return false;
            }

            // 直接传递参数值，不做特殊处理
            // 你的IR格式支持直接传递数组变量（如 @array[2][2][2][2]）
            realParams.push_back(temp->val);
            node->blockInsts.addInst(temp->blockInsts);
        }
    }

    // 参数数量检查
    if (realParams.size() != calledFunction->getParams().size()) {
        minic_log(LOG_ERROR, "第%lld行的被调用函数(%s)参数数量不匹配", (long long) lineno, funcName.c_str());
        return false;
    }

    // 返回调用有返回值，则需要分配临时变量，用于保存函数调用的返回值
    Type * type = calledFunction->getReturnType();

    FuncCallInstruction * funcCallInst = new FuncCallInstruction(currentFunc, calledFunction, realParams, type);

    // 创建函数调用指令
    node->blockInsts.addInst(funcCallInst);

    // 函数调用结果Value保存到node中，可能为空，上层节点可利用这个值
    node->val = funcCallInst;

    return true;
}

/// @brief 语句块（含函数体）AST节点翻译成线性中间IR
/// @param node AST节点
/// @return 翻译是否成功，true：成功，false：失败
bool IRGenerator::ir_block(ast_node * node)
{
    // 进入作用域
    if (node->needScope) {
        module->enterScope();
    }

    std::vector<ast_node *>::iterator pIter;
    for (pIter = node->sons.begin(); pIter != node->sons.end(); ++pIter) {

        // 遍历Block的每个语句，进行显示或者运算
        ast_node * temp = ir_visit_ast_node(*pIter);
        if (!temp) {
            return false;
        }

        node->blockInsts.addInst(temp->blockInsts);
    }

    // 离开作用域
    if (node->needScope) {
        module->leaveScope();
    }

    return true;
}

/// @brief 整数加法AST节点翻译成线性中间IR
/// @param node AST节点
/// @return 翻译是否成功，true：成功，false：失败
bool IRGenerator::ir_add(ast_node * node)
{
    ast_node * src1_node = node->sons[0];
    ast_node * src2_node = node->sons[1];

    // 加法节点，左结合，先计算左节点，后计算右节点

    // 加法的左边操作数
    ast_node * left = ir_visit_ast_node(src1_node);
    if (!left) {
        // 某个变量没有定值
        return false;
    }

    // 加法的右边操作数
    ast_node * right = ir_visit_ast_node(src2_node);
    if (!right) {
        // 某个变量没有定值
        return false;
    }

    // 这里只处理整型的数据，如需支持实数，则需要针对类型进行处理

    BinaryInstruction * addInst = new BinaryInstruction(module->getCurrentFunction(),
                                                        IRInstOperator::IRINST_OP_ADD_I,
                                                        left->val,
                                                        right->val,
                                                        IntegerType::getTypeInt());

    // 创建临时变量保存IR的值，以及线性IR指令
    node->blockInsts.addInst(left->blockInsts);
    node->blockInsts.addInst(right->blockInsts);
    node->blockInsts.addInst(addInst);

    node->val = addInst;

    return true;
}

/// @brief 整数减法AST节点翻译成线性中间IR
/// @param node AST节点
/// @return 翻译是否成功，true：成功，false：失败
bool IRGenerator::ir_sub(ast_node * node)
{
    ast_node * src1_node = node->sons[0];
    ast_node * src2_node = node->sons[1];

    // 加法节点，左结合，先计算左节点，后计算右节点

    // 加法的左边操作数
    ast_node * left = ir_visit_ast_node(src1_node);
    if (!left) {
        // 某个变量没有定值
        return false;
    }

    // 加法的右边操作数
    ast_node * right = ir_visit_ast_node(src2_node);
    if (!right) {
        // 某个变量没有定值
        return false;
    }

    // 这里只处理整型的数据，如需支持实数，则需要针对类型进行处理

    BinaryInstruction * subInst = new BinaryInstruction(module->getCurrentFunction(),
                                                        IRInstOperator::IRINST_OP_SUB_I,
                                                        left->val,
                                                        right->val,
                                                        IntegerType::getTypeInt());

    // 创建临时变量保存IR的值，以及线性IR指令
    node->blockInsts.addInst(left->blockInsts);
    node->blockInsts.addInst(right->blockInsts);
    node->blockInsts.addInst(subInst);

    node->val = subInst;

    return true;
}

/// @brief 赋值AST节点翻译成线性中间IR
/// @param node AST节点
/// @return 翻译是否成功，true：成功，false：失败
bool IRGenerator::ir_assign(ast_node * node)
{
    ast_node * son1_node = node->sons[0];
    ast_node * son2_node = node->sons[1];

    // 赋值节点，自右往左运算

    // 先处理右侧表达式
    ast_node * right = ir_visit_ast_node(son2_node);
    if (!right) return false;

    // 再处理左侧表达式
    ast_node * left = ir_visit_ast_node(son1_node);
    if (!left) return false;

    node->blockInsts.addInst(right->blockInsts);
    node->blockInsts.addInst(left->blockInsts);

    // 判断左值类型
    if (son1_node->node_type == ast_operator_type::AST_OP_ARRAY_ACCESS) {
        // 数组元素赋值：left->val 已经是地址，直接生成store指令
        BinaryInstruction * storeInst = new BinaryInstruction(
            module->getCurrentFunction(),
            IRInstOperator::IRINST_OP_STORE,
            left->val,   // 地址（来自ir_array_access）
            right->val,  // 要写入的值
            right->val->getType() // 使用右值的类型
        );
        node->blockInsts.addInst(storeInst);
        node->val = storeInst;
    } else {
        // 普通变量赋值
        MoveInstruction * movInst = new MoveInstruction(
            module->getCurrentFunction(),
            left->val,
            right->val
        );
        node->blockInsts.addInst(movInst);
        node->val = movInst;
    }

    return true;
}

/// @brief 整数取负AST节点翻译成线性中间IR
/// @param node AST节点
/// @return 翻译是否成功，true：成功，false：失败
bool IRGenerator::ir_neg(ast_node * node)
{
    ast_node * src_node = node->sons[0];

    // 取负节点，先计算子节点
    ast_node * operand = ir_visit_ast_node(src_node);
    if (!operand) {
        // 某个变量没有定值
        return false;
    }

    // 这里只处理整型的数据，如需支持实数，则需要针对类型进行处理

    UnaryInstruction * negInst = new UnaryInstruction(module->getCurrentFunction(),
                                                        IRInstOperator::IRINST_OP_NEG_I,
                                                        operand->val,
                                                        IntegerType::getTypeInt());

    // 创建临时变量保存IR的值，以及线性IR指令
    node->blockInsts.addInst(operand->blockInsts);
    node->blockInsts.addInst(negInst);

    node->val = negInst;

    return true;
}

/// @brief 整数乘法AST节点翻译成线性中间IR
/// @param node AST节点
/// @return 翻译是否成功，true：成功，false：失败
bool IRGenerator::ir_mul(ast_node * node)
{
    ast_node * src1_node = node->sons[0];
    ast_node * src2_node = node->sons[1];

    // 乘法节点，左结合，先计算左节点，后计算右节点

    // 乘法的左边操作数
    ast_node * left = ir_visit_ast_node(src1_node);
    if (!left) {
        // 某个变量没有定值
        return false;
    }

    // 乘法的右边操作数
    ast_node * right = ir_visit_ast_node(src2_node);
    if (!right) {
        // 某个变量没有定值
        return false;
    }

    // 这里只处理整型的数据，如需支持实数，则需要针对类型进行处理

    BinaryInstruction * mulInst = new BinaryInstruction(module->getCurrentFunction(),
                                                        IRInstOperator::IRINST_OP_MUL_I,
                                                        left->val,
                                                        right->val,
                                                        IntegerType::getTypeInt());

    // 创建临时变量保存IR的值，以及线性IR指令
    node->blockInsts.addInst(left->blockInsts);
    node->blockInsts.addInst(right->blockInsts);
    node->blockInsts.addInst(mulInst);

    node->val = mulInst;

    return true;
}

/// @brief 整数除法AST节点翻译成线性中间IR
/// @param node AST节点
/// @return 翻译是否成功，true：成功，false：失败
bool IRGenerator::ir_div(ast_node * node)
{
    ast_node * src1_node = node->sons[0];
    ast_node * src2_node = node->sons[1];

    // 除法节点，左结合，先计算左节点，后计算右节点

    // 除法的左边操作数
    ast_node * left = ir_visit_ast_node(src1_node);
    if (!left) {
        // 某个变量没有定值
        return false;
    }

    // 除法的右边操作数
    ast_node * right = ir_visit_ast_node(src2_node);
    if (!right) {
        // 某个变量没有定值
        return false;
    }

    // 这里只处理整型的数据，如需支持实数，则需要针对类型进行处理

    BinaryInstruction * divInst = new BinaryInstruction(module->getCurrentFunction(),
                                                         IRInstOperator::IRINST_OP_DIV_I,
                                                         left->val,
                                                         right->val,
                                                         IntegerType::getTypeInt());

    // 创建临时变量保存IR的值，以及线性IR指令
    node->blockInsts.addInst(left->blockInsts);
    node->blockInsts.addInst(right->blockInsts);
    node->blockInsts.addInst(divInst);

    node->val = divInst;

    return true;
}

/// @brief 整数取余AST节点翻译成线性中间IR
/// @param node AST节点
/// @return 翻译是否成功，true：成功，false：失败
bool IRGenerator::ir_mod(ast_node * node)
{
    ast_node * src1_node = node->sons[0];
    ast_node * src2_node = node->sons[1];

    // 取余节点，左结合，先计算左节点，后计算右节点

    // 取余的左边操作数
    ast_node * left = ir_visit_ast_node(src1_node);
    if (!left) {
        // 某个变量没有定值
        return false;
    }

    // 取余的右边操作数
    ast_node * right = ir_visit_ast_node(src2_node);
    if (!right) {
        // 某个变量没有定值
        return false;
    }

    // 这里只处理整型的数据，如需支持实数，则需要针对类型进行处理

    BinaryInstruction * modInst = new BinaryInstruction(module->getCurrentFunction(),
                                                         IRInstOperator::IRINST_OP_MOD_I,
                                                         left->val,
                                                         right->val,
                                                         IntegerType::getTypeInt());

    // 创建临时变量保存IR的值，以及线性IR指令
    node->blockInsts.addInst(left->blockInsts);
    node->blockInsts.addInst(right->blockInsts);
    node->blockInsts.addInst(modInst);

    node->val = modInst;

    return true;
}

// 等于 ==
bool IRGenerator::ir_eq(ast_node * node) {
    ast_node * left = ir_visit_ast_node(node->sons[0]);
    ast_node * right = ir_visit_ast_node(node->sons[1]);
    if (!left || !right) return false;

    BinaryInstruction * eqInst = new BinaryInstruction(module->getCurrentFunction(),
        IRInstOperator::IRINST_OP_EQ, left->val, right->val, IntegerType::getTypeBool());
    node->blockInsts.addInst(left->blockInsts);
    node->blockInsts.addInst(right->blockInsts);
    node->blockInsts.addInst(eqInst);

    node->val = eqInst; // 始终设置 val

    if (node->trueLabel && node->falseLabel) {
        node->blockInsts.addInst(new BranchCondInstruction(
            module->getCurrentFunction(), eqInst, node->trueLabel, node->falseLabel));
    }
    return true;
}

// 不等于 !=
bool IRGenerator::ir_ne(ast_node * node) {
    ast_node * left = ir_visit_ast_node(node->sons[0]);
    ast_node * right = ir_visit_ast_node(node->sons[1]);
    if (!left || !right) return false;

    BinaryInstruction * neInst = new BinaryInstruction(module->getCurrentFunction(),
        IRInstOperator::IRINST_OP_NE, left->val, right->val, IntegerType::getTypeBool());
    node->blockInsts.addInst(left->blockInsts);
    node->blockInsts.addInst(right->blockInsts);
    node->blockInsts.addInst(neInst);

    node->val = neInst;

    if (node->trueLabel && node->falseLabel) {
        node->blockInsts.addInst(new BranchCondInstruction(
            module->getCurrentFunction(), neInst, node->trueLabel, node->falseLabel));
    }
    return true;
}

// 小于 <
bool IRGenerator::ir_lt(ast_node * node) {
    ast_node * left = ir_visit_ast_node(node->sons[0]);
    ast_node * right = ir_visit_ast_node(node->sons[1]);
    if (!left || !right) return false;

    BinaryInstruction * ltInst = new BinaryInstruction(module->getCurrentFunction(),
        IRInstOperator::IRINST_OP_LT, left->val, right->val, IntegerType::getTypeBool());
    node->blockInsts.addInst(left->blockInsts);
    node->blockInsts.addInst(right->blockInsts);
    node->blockInsts.addInst(ltInst);

    node->val = ltInst;

    if (node->trueLabel && node->falseLabel) {
        node->blockInsts.addInst(new BranchCondInstruction(
            module->getCurrentFunction(), ltInst, node->trueLabel, node->falseLabel));
    }
    return true;
}

// 小于等于 <=
bool IRGenerator::ir_le(ast_node * node) {
    ast_node * left = ir_visit_ast_node(node->sons[0]);
    ast_node * right = ir_visit_ast_node(node->sons[1]);
    if (!left || !right) return false;

    BinaryInstruction * leInst = new BinaryInstruction(module->getCurrentFunction(),
        IRInstOperator::IRINST_OP_LE, left->val, right->val, IntegerType::getTypeBool());
    node->blockInsts.addInst(left->blockInsts);
    node->blockInsts.addInst(right->blockInsts);
    node->blockInsts.addInst(leInst);

    node->val = leInst;

    if (node->trueLabel && node->falseLabel) {
        node->blockInsts.addInst(new BranchCondInstruction(
            module->getCurrentFunction(), leInst, node->trueLabel, node->falseLabel));
    }
    return true;
}

// 大于 >
bool IRGenerator::ir_gt(ast_node * node) {
    ast_node * left = ir_visit_ast_node(node->sons[0]);
    ast_node * right = ir_visit_ast_node(node->sons[1]);
    if (!left || !right) return false;

    BinaryInstruction * gtInst = new BinaryInstruction(module->getCurrentFunction(),
        IRInstOperator::IRINST_OP_GT, left->val, right->val, IntegerType::getTypeBool());
    node->blockInsts.addInst(left->blockInsts);
    node->blockInsts.addInst(right->blockInsts);
    node->blockInsts.addInst(gtInst);

    node->val = gtInst;

    if (node->trueLabel && node->falseLabel) {
        node->blockInsts.addInst(new BranchCondInstruction(
            module->getCurrentFunction(), gtInst, node->trueLabel, node->falseLabel));
    }
    return true;
}

// 大于等于 >=
bool IRGenerator::ir_ge(ast_node * node) {
    ast_node * left = ir_visit_ast_node(node->sons[0]);
    ast_node * right = ir_visit_ast_node(node->sons[1]);
    if (!left || !right) return false;

    BinaryInstruction * geInst = new BinaryInstruction(module->getCurrentFunction(),
        IRInstOperator::IRINST_OP_GE, left->val, right->val, IntegerType::getTypeBool());
    node->blockInsts.addInst(left->blockInsts);
    node->blockInsts.addInst(right->blockInsts);
    node->blockInsts.addInst(geInst);

    node->val = geInst;

    if (node->trueLabel && node->falseLabel) {
        node->blockInsts.addInst(new BranchCondInstruction(
            module->getCurrentFunction(), geInst, node->trueLabel, node->falseLabel));
    }
    return true;
}

/// @brief 辅助函数，判断是否为逻辑表达式
static bool is_logic_expr(ast_node * node) {
    return node->node_type == ast_operator_type::AST_OP_AND ||
           node->node_type == ast_operator_type::AST_OP_OR ||
           node->node_type == ast_operator_type::AST_OP_NOT;
}

/// @brief 辅助函数，判断是否为关系表达式
static bool is_relational_expr(ast_node * node) {
    return node->node_type == ast_operator_type::AST_OP_EQ ||
           node->node_type == ast_operator_type::AST_OP_NE ||
           node->node_type == ast_operator_type::AST_OP_LT ||
           node->node_type == ast_operator_type::AST_OP_LE ||
           node->node_type == ast_operator_type::AST_OP_GT ||
           node->node_type == ast_operator_type::AST_OP_GE;
}

/// @brief 辅助函数，判断是否为条件表达式（逻辑或关系）
static bool is_conditional_expr(ast_node * node) {
    return is_logic_expr(node) || is_relational_expr(node);
}

// 逻辑与 && 短路求值
bool IRGenerator::ir_and(ast_node * node) {
    Function * func = module->getCurrentFunction();
    LabelInstruction * rightLabel = new LabelInstruction(func);

    // 左操作数
    ast_node * leftChild = node->sons[0];
    if (is_conditional_expr(leftChild)) { // 修改这里
        leftChild->trueLabel = rightLabel;
        leftChild->falseLabel = node->falseLabel;
        ir_visit_ast_node(leftChild);
        node->blockInsts.addInst(leftChild->blockInsts);
    } else {
        ast_node * left = ir_visit_ast_node(leftChild);
        if (!left) return false;
        node->blockInsts.addInst(left->blockInsts);
        // 生成 icmp ne
        BinaryInstruction * cmpInst = new BinaryInstruction(
            func,
            IRInstOperator::IRINST_OP_NE,
            left->val,
            module->newConstInt(0),
            IntegerType::getTypeBool()
        );
        node->blockInsts.addInst(cmpInst);
        node->blockInsts.addInst(new BranchCondInstruction(func, cmpInst, rightLabel, node->falseLabel));
    }

    // 右操作数
    node->blockInsts.addInst(rightLabel);
    ast_node * rightChild = node->sons[1];
    if (is_conditional_expr(rightChild)) { // 修改这里
        rightChild->trueLabel = node->trueLabel;
        rightChild->falseLabel = node->falseLabel;
        ir_visit_ast_node(rightChild);
        node->blockInsts.addInst(rightChild->blockInsts);
    } else {
        ast_node * right = ir_visit_ast_node(rightChild);
        if (!right) return false;
        node->blockInsts.addInst(right->blockInsts);
        BinaryInstruction * cmpInst = new BinaryInstruction(
            func,
            IRInstOperator::IRINST_OP_NE,
            right->val,
            module->newConstInt(0),
            IntegerType::getTypeBool()
        );
        node->blockInsts.addInst(cmpInst);
        node->blockInsts.addInst(new BranchCondInstruction(func, cmpInst, node->trueLabel, node->falseLabel));
    }

    return true;
}

// 逻辑或 || 短路求值
bool IRGenerator::ir_or(ast_node * node) {
    Function * func = module->getCurrentFunction();
    LabelInstruction * rightLabel = new LabelInstruction(func);

    // 左操作数
    ast_node * leftChild = node->sons[0];
    if (is_conditional_expr(leftChild)) { // 修改这里
        leftChild->trueLabel = node->trueLabel;
        leftChild->falseLabel = rightLabel;
        ir_visit_ast_node(leftChild);
        node->blockInsts.addInst(leftChild->blockInsts);
    } else {
        ast_node * left = ir_visit_ast_node(leftChild);
        if (!left) return false;
        node->blockInsts.addInst(left->blockInsts);
        // 生成 icmp ne
        BinaryInstruction * cmpInst = new BinaryInstruction(
            func,
            IRInstOperator::IRINST_OP_NE,
            left->val,
            module->newConstInt(0),
            IntegerType::getTypeBool()
        );
        node->blockInsts.addInst(cmpInst);
        node->blockInsts.addInst(new BranchCondInstruction(func, cmpInst, node->trueLabel, rightLabel));
    }

    // 右操作数
    node->blockInsts.addInst(rightLabel);
    ast_node * rightChild = node->sons[1];
    if (is_conditional_expr(rightChild)) { // 修改这里
        rightChild->trueLabel = node->trueLabel;
        rightChild->falseLabel = node->falseLabel;
        ir_visit_ast_node(rightChild);
        node->blockInsts.addInst(rightChild->blockInsts);
    } else {
        ast_node * right = ir_visit_ast_node(rightChild);
        if (!right) return false;
        node->blockInsts.addInst(right->blockInsts);
        BinaryInstruction * cmpInst = new BinaryInstruction(
            func,
            IRInstOperator::IRINST_OP_NE,
            right->val,
            module->newConstInt(0),
            IntegerType::getTypeBool()
        );
        node->blockInsts.addInst(cmpInst);
        node->blockInsts.addInst(new BranchCondInstruction(func, cmpInst, node->trueLabel, node->falseLabel));
    }

    return true;
}

// 逻辑非 !
bool IRGenerator::ir_not(ast_node * node) {
    // 如果有跳转标签，说明在条件上下文中
    if (node->trueLabel && node->falseLabel) {
        ast_node * child = node->sons[0];
        if (is_conditional_expr(child)) {
            child->trueLabel = node->falseLabel;
            child->falseLabel = node->trueLabel;
            ir_visit_ast_node(child);
            node->blockInsts.addInst(child->blockInsts);
            node->val = nullptr;
            return true;
        } else {
            ast_node * operand = ir_visit_ast_node(child);
            if (!operand) return false;
            BinaryInstruction * eqInst = new BinaryInstruction(
                module->getCurrentFunction(),
                IRInstOperator::IRINST_OP_EQ,
                operand->val,
                module->newConstInt(0),
                IntegerType::getTypeBool()
            );
            node->blockInsts.addInst(operand->blockInsts);
            node->blockInsts.addInst(eqInst);
            node->blockInsts.addInst(new BranchCondInstruction(
                module->getCurrentFunction(), eqInst, node->trueLabel, node->falseLabel));
            node->val = eqInst;
            return true;
        }
    }

    // 普通模式：需要返回整数值
    ast_node * operand = ir_visit_ast_node(node->sons[0]);
    if (!operand) return false;

    Function * func = module->getCurrentFunction();
    
    // 创建临时变量存储结果
    Value * resultVar = module->newVarValue(IntegerType::getTypeInt(), "");
    
    // 生成条件分支
    BinaryInstruction * eqInst = new BinaryInstruction(func,
        IRInstOperator::IRINST_OP_EQ, operand->val, module->newConstInt(0), 
        IntegerType::getTypeBool());
    
    LabelInstruction * trueLabel = new LabelInstruction(func);
    LabelInstruction * falseLabel = new LabelInstruction(func);
    LabelInstruction * endLabel = new LabelInstruction(func);
    
    node->blockInsts.addInst(operand->blockInsts);
    node->blockInsts.addInst(eqInst);
    node->blockInsts.addInst(new BranchCondInstruction(func, eqInst, trueLabel, falseLabel));
    
    // true分支：!操作结果为1
    node->blockInsts.addInst(trueLabel);
    node->blockInsts.addInst(new MoveInstruction(func, resultVar, module->newConstInt(1)));
    node->blockInsts.addInst(new GotoInstruction(func, endLabel));
    
    // false分支：!操作结果为0
    node->blockInsts.addInst(falseLabel);
    node->blockInsts.addInst(new MoveInstruction(func, resultVar, module->newConstInt(0)));
    node->blockInsts.addInst(endLabel);
    
    node->val = resultVar;
    return true;
}



/// @brief if/if-else语句翻译成线性中间IR
/// @param node AST节点
/// @return 翻译是否成功，true：成功，false：失败
bool IRGenerator::ir_if(ast_node * node)
{
    Function * func = module->getCurrentFunction();

    LabelInstruction * label_true = new LabelInstruction(func);
    LabelInstruction * label_false = new LabelInstruction(func);
    LabelInstruction * label_end = new LabelInstruction(func);

    ast_node * cond_node = node->sons[0];
    // 对所有条件表达式传递label
    if (is_conditional_expr(cond_node)) { // 修改这里
        cond_node->trueLabel = label_true;
        cond_node->falseLabel = label_false;
    }
    cond_node = ir_visit_ast_node(cond_node);
    if (!cond_node) return false;

    node->blockInsts.addInst(cond_node->blockInsts);

    // 只有非条件表达式才生成 BranchCondInstruction
    if (!is_conditional_expr(cond_node) && cond_node->val) { // 修改这里
        node->blockInsts.addInst(new BranchCondInstruction(func, cond_node->val, label_true, label_false));
    }

    // then分支
    node->blockInsts.addInst(label_true);
    ast_node * then_node = nullptr;
    if (node->sons.size() > 1) {
        then_node = ir_visit_ast_node(node->sons[1]);
    }
    if (then_node) {
        node->blockInsts.addInst(then_node->blockInsts);
    }
    node->blockInsts.addInst(new GotoInstruction(func, label_end));

    // else分支
    node->blockInsts.addInst(label_false);
    if (node->sons.size() > 2 && node->sons[2]) {
        ast_node * else_node = ir_visit_ast_node(node->sons[2]);
        if (!else_node) return false;
        node->blockInsts.addInst(else_node->blockInsts);
    }
    node->blockInsts.addInst(label_end);

    return true;
}

/// @brief 辅助函数，设置循环体的 enclosingLoop 属性
void IRGenerator::set_enclosing_loop(ast_node * node, ast_node * loop_node) {
    if (!node) return;
    node->enclosingLoop = loop_node;
    for (auto *son : node->sons) {
        set_enclosing_loop(son, loop_node);
    }
}

/// @brief while语句翻译成线性中间IR
/// @param node AST节点
/// @return 翻译是否成功，true：成功，false：失败
bool IRGenerator::ir_while(ast_node * node)
{
    Function * func = module->getCurrentFunction();

    LabelInstruction * label_cond = new LabelInstruction(func);
    LabelInstruction * label_body = new LabelInstruction(func);
    LabelInstruction * label_end = new LabelInstruction(func);

    node->breakLabel = label_end;
    node->continueLabel = label_cond;

    if (node->sons.size() > 1 && node->sons[1]) {
        set_enclosing_loop(node->sons[1], node);
    }

    node->blockInsts.addInst(new GotoInstruction(func, label_cond));
    node->blockInsts.addInst(label_cond);

    ast_node * cond_node = node->sons[0];
    if (is_conditional_expr(cond_node)) { // 修改这里
        cond_node->trueLabel = label_body;
        cond_node->falseLabel = label_end;
    }
    cond_node = ir_visit_ast_node(cond_node);
    if (!cond_node) return false;
    node->blockInsts.addInst(cond_node->blockInsts);

    if (!is_conditional_expr(cond_node) && cond_node->val) { // 修改这里
        node->blockInsts.addInst(new BranchCondInstruction(func, cond_node->val, label_body, label_end));
    }

    node->blockInsts.addInst(label_body);
    ast_node * body_node = ir_visit_ast_node(node->sons[1]);
    if (!body_node) return false;
    node->blockInsts.addInst(body_node->blockInsts);
    node->blockInsts.addInst(new GotoInstruction(func, label_cond));

    node->blockInsts.addInst(label_end);

    return true;
}

/// @brief break语句翻译成线性中间IR
/// @param node AST节点
/// @return 翻译是否成功，true：成功，false：失败
bool IRGenerator::ir_break(ast_node * node)
{
    // 找到最近的循环节点
    ast_node * loop_node = node->enclosingLoop;
    if (!loop_node || !loop_node->breakLabel) return false;

    Function * func = module->getCurrentFunction();
    node->blockInsts.addInst(new GotoInstruction(func, loop_node->breakLabel));
    return true;
}

/// @brief continue语句翻译成线性中间IR
/// @param node AST节点
/// @return 翻译是否成功，true：成功，false：失败
bool IRGenerator::ir_continue(ast_node * node)
{
    // 找到最近的循环节点
    ast_node * loop_node = node->enclosingLoop;
    if (!loop_node || !loop_node->continueLabel) return false;

    Function * func = module->getCurrentFunction();
    node->blockInsts.addInst(new GotoInstruction(func, loop_node->continueLabel));
    return true;
}

/// @brief return节点翻译成线性中间IR
/// @param node AST节点
/// @return 翻译是否成功，true：成功，false：失败
bool IRGenerator::ir_return(ast_node * node)
{
    ast_node * right = nullptr;

    // return语句可能没有没有表达式，也可能有，因此这里必须进行区分判断
    if (!node->sons.empty()) {

        ast_node * son_node = node->sons[0];

        // 返回的表达式的指令保存在right节点中
        right = ir_visit_ast_node(son_node);
        if (!right) {

            // 某个变量没有定值
            return false;
        }
    }

    // 这里只处理整型的数据，如需支持实数，则需要针对类型进行处理
    Function * currentFunc = module->getCurrentFunction();

    // 返回值存在时则移动指令到node中
    if (right) {

        // 创建临时变量保存IR的值，以及线性IR指令
        node->blockInsts.addInst(right->blockInsts);

        // 返回值赋值到函数返回值变量上，然后跳转到函数的尾部
        node->blockInsts.addInst(new MoveInstruction(currentFunc, currentFunc->getReturnValue(), right->val));

        node->val = right->val;
    } else {
        // 没有返回值
        node->val = nullptr;
    }

    // 跳转到函数的尾部出口指令上
    node->blockInsts.addInst(new GotoInstruction(currentFunc, currentFunc->getExitLabel()));

    return true;
}

/// @brief 类型叶子节点翻译成线性中间IR
/// @param node AST节点
/// @return 翻译是否成功，true：成功，false：失败
bool IRGenerator::ir_leaf_node_type(ast_node * node)
{
    // 不需要做什么，直接从节点中获取即可。

    return true;
}

/// @brief 标识符叶子节点翻译成线性中间IR，变量声明的不走这个语句
/// @param node AST节点
/// @return 翻译是否成功，true：成功，false：失败
bool IRGenerator::ir_leaf_node_var_id(ast_node * node)
{
    Value * val;

    // 查找ID型Value
    // 变量，则需要在符号表中查找对应的值

    val = module->findVarValue(node->name);

    node->val = val;

    return true;
}

/// @brief 无符号整数字面量叶子节点翻译成线性中间IR
/// @param node AST节点
/// @return 翻译是否成功，true：成功，false：失败
bool IRGenerator::ir_leaf_node_uint(ast_node * node)
{
    ConstInt * val;

    // 新建一个整数常量Value
    val = module->newConstInt((int32_t) node->integer_val);

    node->val = val;

    return true;
}

/// @brief 变量声明语句节点翻译成线性中间IR
/// @param node AST节点
/// @return 翻译是否成功，true：成功，false：失败
bool IRGenerator::ir_declare_statment(ast_node * node)
{
    bool result = false;

    for (auto & child: node->sons) {
		// 遍历每个变量声明的AST节点
        result = ir_variable_declare(child);
        if (!result) break;
        // 合并每个变量声明的IR指令到声明语句节点
        node->blockInsts.addInst(child->blockInsts);
    }
    return result;
}

/// @brief 变量声明AST节点翻译成线性中间IR
/// @param node AST节点
/// @return 翻译是否成功，true：成功，false：失败
bool IRGenerator::ir_variable_declare(ast_node * node)
{
    // sons[0]: 类型节点
    // sons[1]: 变量名节点 或 AST_OP_ARRAY_DECL 节点
    // sons[2]: 初始化表达式节点（可选）

    ast_node * type_node = node->sons[0];
    ast_node * name_or_array_node = node->sons[1];

    Value * varValue = nullptr;
    std::string var_name;
    Type * var_type = nullptr;

    if (name_or_array_node->node_type == ast_operator_type::AST_OP_ARRAY_DECL) {
        // 数组声明
        ast_node * id_node = name_or_array_node->sons[0];
        var_name = id_node->name;
        var_type = node->type;
    } else {
        // 普通变量
        var_name = name_or_array_node->name;
        var_type = type_node->type;
    }

    // 判断是否为全局变量（当前函数为空表示在全局作用域）
    bool isGlobal = (module->getCurrentFunction() == nullptr);

    if (isGlobal) {
        // 全局变量处理
        Constant * initValue = nullptr;
        
        if (node->sons.size() > 2 && node->sons[2]) {
            // 有初始化值的全局变量
            ast_node * init_expr_node = ir_visit_ast_node(node->sons[2]);
            if (!init_expr_node) return false;
            
            // 尝试获取或计算常量值
            initValue = extractConstantValue(init_expr_node->val);
            
            if (!initValue && init_expr_node->val) {
                printf("Warning: Global variable '%s' initialized with non-constant value, using default initialization\n", var_name.c_str());
            }
        }
        
        // 创建全局变量
        varValue = module->newVarValue(var_type, var_name, initValue);
        if (!varValue) {
            printf("Error: Failed to create global variable %s\n", var_name.c_str());
            return false;
        }
    } else {
        // 局部变量处理（保持不变）
        varValue = module->newVarValue(var_type, var_name);
        
        // 处理局部变量的初始化表达式
        if (node->sons.size() > 2 && node->sons[2]) {
            ast_node * init_expr_node = ir_visit_ast_node(node->sons[2]);
            if (!init_expr_node) return false;

            MoveInstruction * movInst = new MoveInstruction(
                module->getCurrentFunction(),
                varValue,
                init_expr_node->val
            );
            node->blockInsts.addInst(init_expr_node->blockInsts);
            node->blockInsts.addInst(movInst);
        }
    }

    node->val = varValue;
    return true;
}

/// @brief 从Value中提取或计算常量值（简化版）
/// @param val Value指针
/// @return 常量值，如果不能提取则返回nullptr
Constant * IRGenerator::extractConstantValue(Value * val)
{
    if (!val) {
        return nullptr;
    }
    
    // 直接是常量的情况
    if (auto constInt = dynamic_cast<ConstInt*>(val)) {
        return constInt;
    }
    
    if (auto constant = dynamic_cast<Constant*>(val)) {
        return constant;
    }
    
    // 处理一元指令的情况（主要是取负）
    if (auto unaryInst = dynamic_cast<UnaryInstruction*>(val)) {
        // 检查是否是取负操作
        if (unaryInst->getOp() == IRInstOperator::IRINST_OP_NEG_I) {
            // 获取操作数
            Value * operand = unaryInst->getOperand(0);
            if (auto constInt = dynamic_cast<ConstInt*>(operand)) {
                // 计算取负结果并创建新的常量
                int32_t negValue = -constInt->getVal();
                return module->newConstInt(negValue);
            }
        }
    }
    
    return nullptr;
}

/// @brief 数组访问AST节点翻译成线性中间IR（支持多维数组降维）
/// @param node AST节点（AST_OP_ARRAY_ACCESS）
/// @return 翻译是否成功，true：成功，false：失败
bool IRGenerator::ir_array_access(ast_node * node)
{
    Function * func = module->getCurrentFunction();
    
    // 1. 获取数组变量（不递归处理）
    std::string arrayName = node->sons[0]->name;
    Value * arrayVar = module->findVarValue(arrayName);
    
    if (!arrayVar) {
        minic_log(LOG_ERROR, "未找到数组变量: %s", arrayName.c_str());
        return false;
    }

    // 2. 处理所有下标表达式
    std::vector<Value *> indices;
    for (size_t i = 1; i < node->sons.size(); ++i) {
        ast_node * idx_node = ir_visit_ast_node(node->sons[i]);
        if (!idx_node) return false;
        
        indices.push_back(idx_node->val);
        node->blockInsts.addInst(idx_node->blockInsts);
    }

    // 3. 获取数组维度信息
    Type * arrType = arrayVar->getType();
    std::vector<int> dimensions;
    
    Type * currentType = arrType;
    while (currentType->isArrayType()) {
        ArrayType * arrayType = dynamic_cast<ArrayType *>(currentType);
        if (!arrayType) break;
        dimensions.push_back(arrayType->getNumElements());
        currentType = arrayType->getElementType();
    }
    
    Type * elementType = currentType;

    // 4. 严格按照参考IR的计算顺序
    Value * offset = nullptr;
    
    // 按照参考IR的计算模式：mul -> add -> mul -> add ...
    for (size_t i = 0; i < indices.size(); ++i) {
        if (i == 0) {
            // 第一个下标：mul index[0], dim[1]
            if (i + 1 < dimensions.size()) {
                BinaryInstruction * mulInst = new BinaryInstruction(
                    func, IRInstOperator::IRINST_OP_MUL_I,
                    indices[i], module->newConstInt(dimensions[i + 1]),
                    IntegerType::getTypeInt()
                );
                node->blockInsts.addInst(mulInst);
                offset = mulInst;
            } else {
                offset = indices[i];
            }
        } else {
            // 后续下标：add offset, index[i]
            BinaryInstruction * addInst = new BinaryInstruction(
                func, IRInstOperator::IRINST_OP_ADD_I,
                offset, indices[i], IntegerType::getTypeInt()
            );
            node->blockInsts.addInst(addInst);
            offset = addInst;
            
            // 如果还有下一维，mul offset, dim[i+1]
            if (i + 1 < dimensions.size()) {
                BinaryInstruction * mulInst = new BinaryInstruction(
                    func, IRInstOperator::IRINST_OP_MUL_I,
                    offset, module->newConstInt(dimensions[i + 1]),
                    IntegerType::getTypeInt()
                );
                node->blockInsts.addInst(mulInst);
                offset = mulInst;
            }
        }
    }

    // 5. 乘以元素大小
    int elemSize = elementType->getSize();
    if (elemSize != 1) {
        BinaryInstruction * sizeInst = new BinaryInstruction(
            func, IRInstOperator::IRINST_OP_MUL_I,
            offset, module->newConstInt(elemSize),
            IntegerType::getTypeInt()
        );
        node->blockInsts.addInst(sizeInst);
        offset = sizeInst;
    }

    // 检查是否用作函数参数 - 更精确的判断
    bool isFuncParam = false;
    if (node->parent && node->parent->parent && 
        node->parent->parent->node_type == ast_operator_type::AST_OP_FUNC_CALL) {
        if (node->parent == node->parent->parent->sons[1]) {
            // 还需要检查：这是传递数组本身，还是数组的元素值
            // 只有当数组访问后仍然是数组类型时，才是函数参数传递
            
            Type * originalType = arrayVar->getType();
            Type * resultType = originalType;
            
            int accessedDims = indices.size();
            for (int i = 0; i < accessedDims && resultType->isArrayType(); ++i) {
                ArrayType * arrType = dynamic_cast<ArrayType *>(resultType);
                if (arrType) {
                    resultType = arrType->getElementType();
                } else {
                    break;
                }
            }
            
            // 只有当结果仍然是数组类型时，才是数组参数传递
            isFuncParam = resultType->isArrayType();
        }
    }

    // 6. 计算最终地址
    BinaryInstruction * addressInst = nullptr;
    
    if (isFuncParam) {
        // 函数参数：数组降维传递
        Type * originalType = arrayVar->getType();
        Type * reducedType = originalType;
        
        int accessedDims = indices.size();
        for (int i = 0; i < accessedDims && reducedType->isArrayType(); ++i) {
            ArrayType * arrType = dynamic_cast<ArrayType *>(reducedType);
            if (arrType) {
                reducedType = arrType->getElementType();
            } else {
                break;
            }
        }
        
        addressInst = new BinaryInstruction(
            func, IRInstOperator::IRINST_OP_ADD_I,
            arrayVar, offset, const_cast<Type*>(reducedType)
        );
    } else {
        // 非函数参数：正常的指针类型
        Type * ptrType = const_cast<Type *>(static_cast<const Type *>(PointerType::get(elementType)));
        addressInst = new BinaryInstruction(
            func, IRInstOperator::IRINST_OP_ADD_I,
            arrayVar, offset, ptrType
        );
    }
    
    node->blockInsts.addInst(addressInst);

    // 7. 根据上下文决定返回地址还是值
    bool isLValue = (node->parent && 
                     node->parent->node_type == ast_operator_type::AST_OP_ASSIGN &&
                     node->parent->sons[0] == node);

    if (isLValue || isFuncParam) {
        // 左值或数组参数：直接返回地址
        node->val = addressInst;
    } else {
        // 右值：生成load指令获取实际值
        UnaryInstruction * loadInst = new UnaryInstruction(
            func, IRInstOperator::IRINST_OP_LOAD,
            addressInst, elementType
        );
        node->blockInsts.addInst(loadInst);
        node->val = loadInst;
    }

    return true;
}

/// @brief 数组声明AST节点翻译成线性中间IR
/// @param node AST节点（AST_OP_ARRAY_DECL）
/// @return 翻译是否成功，true：成功，false：失败
bool IRGenerator::ir_array_decl(ast_node * node)
{
    // sons[0]: id节点（变量名）
    // sons[1...]: 每个维度的表达式（可为nullptr，表示不定长）

    // 这里只做符号表登记，不生成IR指令
    // 获取变量名和类型
    if (node->sons.empty()) return false;
    ast_node * id_node = node->sons[0];
    std::string var_name = id_node->name;
    Type * var_type = node->type;

    // 生成变量（全局或局部），并注册到符号表
    node->val = module->newVarValue(var_type, var_name);

    // 维度信息可用于后续语义分析或代码生成
    // 若有初始化表达式，可在父节点处理

    return true;
}