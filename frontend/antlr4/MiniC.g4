grammar MiniC;

// 词法规则名总是以大写字母开头

// 语法规则名总是以小写字母开头

// 每个非终结符尽量多包含闭包、正闭包或可选符等的EBNF范式描述

// 若非终结符由多个产生式组成，则建议在每个产生式的尾部追加# 名称来区分，详细可查看非终结符statement的描述

// 语法规则描述：EBNF范式

// 源文件编译单元定义
compileUnit: (funcDef | varDecl)* EOF;

// 函数返回值类型
funcType: T_INT | T_VOID;

// 形参列表
funcParamList: funcParam (T_COMMA funcParam)*;

// 单个形参
funcParam: basicType T_ID paramDims?;

// 用于形参的维度描述，支持省略最高维
paramDims: '[' ']' ('[' expr ']')* | ('[' expr ']')+;

// 函数定义，实现对形参和void返回类型的支持
funcDef: funcType T_ID T_L_PAREN funcParamList? T_R_PAREN block;

// 语句块看用作函数体，这里允许多个语句，并且不含任何语句
block: T_L_BRACE blockItemList? T_R_BRACE;

// 每个ItemList可包含至少一个Item
blockItemList: blockItem+;

// 每个Item可以是一个语句，或者变量声明语句
blockItem: statement | varDecl;

// 变量声明，目前不支持变量含有初值
varDecl: basicType varDef (T_COMMA varDef)* T_SEMICOLON;

// 基本类型
basicType: T_INT;

// 变量定义，修改以支持初始化
varDef: T_ID dims? (T_ASSIGN expr)?;

// 数组维度
dims: ('[' expr ']')+; // 支持多维数组

// 语句文法：扩展以支持条件语句和循环语句
statement:
	T_RETURN expr T_SEMICOLON										# returnStatement
	| lVal T_ASSIGN expr T_SEMICOLON								# assignStatement
	| block															# blockStatement
	| expr? T_SEMICOLON												# expressionStatement
	| T_IF T_L_PAREN expr T_R_PAREN statement (T_ELSE statement)?	# ifStatement
	| T_WHILE T_L_PAREN expr T_R_PAREN statement					# whileStatement
	| T_BREAK T_SEMICOLON											# breakStatement
	| T_CONTINUE T_SEMICOLON										# continueStatement;

// 表达式文法：扩展以支持关系表达式和逻辑表达式
expr: orExp;

// 逻辑或表达式
orExp: andExp (T_OR andExp)*;

// 逻辑与表达式
andExp: relExp (T_AND relExp)*;

// 关系表达式
relExp: addExp (relOp addExp)*;

// 关系运算符
relOp: T_LT | T_GT | T_LE | T_GE | T_EQ | T_NE;

// 逻辑运算符
logicOp: T_AND | T_OR;

// 乘除求余表达式
multExp: unaryExp (multOp unaryExp)*;

// 乘除求余运算符
multOp: T_MUL | T_DIV | T_MOD;

// 加减表达式
addExp: multExp (addOp multExp)*;

// 加减运算符
addOp: T_ADD | T_SUB;

// 一元表达式
unaryExp:
	primaryExp
	| T_ID T_L_PAREN realParamList? T_R_PAREN
	| T_SUB unaryExp
	| T_NOT unaryExp; //支持逻辑非

// 基本表达式：括号表达式、整数、左值表达式
primaryExp:
	T_L_PAREN expr T_R_PAREN
	| T_HEX
	| T_OCTAL
	| T_DECIMAL
	| lVal;

// 实参列表
realParamList: expr (T_COMMA expr)*;

// 左值表达式
lVal: T_ID dimsAccess*;

// 数组访问
dimsAccess: '[' expr ']';

// 用正规式来进行词法规则的描述

T_L_PAREN: '(';
T_R_PAREN: ')';
T_SEMICOLON: ';';
T_L_BRACE: '{';
T_R_BRACE: '}';

T_ASSIGN: '=';
T_COMMA: ',';

T_ADD: '+';
T_SUB: '-';
T_MUL: '*';
T_DIV: '/';
T_MOD: '%';

// 增加关系表达式的词法规则

T_LT: '<';
T_GT: '>';
T_LE: '<=';
T_GE: '>=';
T_EQ: '==';
T_NE: '!=';
T_AND: '&&';
T_OR: '||';
T_NOT: '!';

// 增加 条件 if 与 循环 while 的词法规则
T_IF: 'if';
T_ELSE: 'else';
T_WHILE: 'while';
T_BREAK: 'break';
T_CONTINUE: 'continue';

// 要注意关键字同样也属于T_ID，因此必须放在T_ID的前面，否则会识别成T_ID
T_RETURN: 'return';
T_INT: 'int';
T_VOID: 'void';

T_ID: [a-zA-Z_][a-zA-Z0-9_]*;
// T_DIGIT: '0' | [1-9][0-9]*;
T_HEX: '0' [xX] [0-9a-fA-F]+; // 十六进制，以0x或0X开头
T_OCTAL: '0' [0-7]+; // 八进制，以0开头
T_DECIMAL: '0' | [1-9][0-9]*; // 十进制

/* 空白符丢弃 */
WS: [ \r\n\t]+ -> skip;
COMMENT: '/*' .*? '*/' -> skip;
LINE_COMMENT: '//' ~[\r\n]* -> skip;