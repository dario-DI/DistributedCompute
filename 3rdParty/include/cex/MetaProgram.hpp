/// \file Meta.hpp Copyright (C) Sharewin Inc.
/// \brief meta program( macro recursively, generic recursively )
///
///
/// \note:
/// \author: DI
/// \time: 2012/7/9 15:57
#ifndef _TDE_METAPROGRAMA_HPP_ 
#define _TDE_METAPROGRAMA_HPP_ 

#pragma once
#include <boost\ref.hpp>

/// 字符n的实现
#define MT_CHAR_0(fn) 
#define MT_CHAR_1(fn) fn(1)
#define MT_CHAR_2(fn) fn(1), fn(2)
#define MT_CHAR_3(fn) fn(1), fn(2), fn(3)
#define MT_CHAR_4(fn) fn(1), fn(2), fn(3), fn(4)
#define MT_CHAR_5(fn) fn(1), fn(2), fn(3), fn(4), fn(5)
#define MT_CHAR_6(fn) fn(1), fn(2), fn(3), fn(4), fn(5), fn(6)
#define MT_CHAR_7(fn) fn(1), fn(2), fn(3), fn(4), fn(5), fn(6), fn(7)

/// -1
#define MT_DECR(n) MT_DECR_##n
#define MT_DECR_1 0
#define MT_DECR_2 1
#define MT_DECR_3 2
#define MT_DECR_4 3
#define MT_DECR_5 4
#define MT_DECR_6 5
#define MT_DECR_7 6
#define MT_DECR_8 7
#define MT_DECR_9 8
#define MT_DECR_0 9

/// +1
#define MT_INCR(n) MT_INCR_##n
#define MT_INCR_0 1
#define MT_INCR_1 2
#define MT_INCR_2 3
#define MT_INCR_3 4
#define MT_INCR_4 5
#define MT_INCR_5 6
#define MT_INCR_6 7
#define MT_INCR_7 8
#define MT_INCR_8 9
#define MT_INCR_9 0

/// 连接两字符作为宏
#define MT_CHR(x,y) MT_CHR_IMPL(x,y)
#define MT_CHR_IMPL(x,y) x##y

/// 单文档无重名变量
#define MT_SEQUENCE_NAME MT_CHR(mt_nameof_, __COUNTER__)

/// ,实现. 0, 无。 >0时，有
#define MT_COMMA(n) MT_CHR(MT_COMMA_, n)
#define MT_COMMA_0 
#define MT_COMMA_1 ,
#define MT_COMMA_2 MT_COMMA_1
#define MT_COMMA_3 MT_COMMA_1
#define MT_COMMA_4 MT_COMMA_1
#define MT_COMMA_5 MT_COMMA_1
#define MT_COMMA_6 MT_COMMA_1
#define MT_COMMA_7 MT_COMMA_1

/// 形参 P1 p1, P2 p2...
#define MT_PARAM_FN(N) P##N p##N
#define MT_PARAM(N) MT_CHR(MT_CHAR_, N)(MT_PARAM_FN)

/// 实参 p1, p2...
#define MT_ARG_FN(N) p##N
#define MT_ARG(N) MT_CHR(MT_CHAR_, N)(MT_ARG_FN)

/// 占位符 _1, _2, ...
#define MT_PLACEHOLDER_FN(N) _##N
#define MT_PLACEHOLDER(N) MT_CHR(MT_CHAR_, N)(MT_PLACEHOLDER_FN)

/// 泛型形参 typename P1, typename P2...
#define MT_TPARAM_FN(N) typename P##N
#define MT_TPARAM(N) MT_CHR(MT_CHAR_, N)(MT_TPARAM_FN)

/// 泛型实参 P1, P2...
#define MT_TARG_FN(N) P##N
#define MT_TARG(N) MT_CHR(MT_CHAR_, N)(MT_TARG_FN)


/// 表达式K0~K9
#define MT_EXPRESSION_IMPL_K0_K9( expression, K ) \
	expression(K##0) \
	expression(K##1) \
	expression(K##2) \
	expression(K##3) \
	expression(K##4) \
	expression(K##5) \
	expression(K##6) \
	expression(K##7) \
	expression(K##8) \
	expression(K##9)

/// 表达式K00~K99
#define MT_EXPRESSION_IMPL_K00_K99( expression, K ) \
	MT_EXPRESSION_IMPL_K0_K9(expression,K##0)\
	MT_EXPRESSION_IMPL_K0_K9(expression,K##1)\
	MT_EXPRESSION_IMPL_K0_K9(expression,K##2)\
	MT_EXPRESSION_IMPL_K0_K9(expression,K##3)\
	MT_EXPRESSION_IMPL_K0_K9(expression,K##4)\
	MT_EXPRESSION_IMPL_K0_K9(expression,K##5)\
	MT_EXPRESSION_IMPL_K0_K9(expression,K##6)\
	MT_EXPRESSION_IMPL_K0_K9(expression,K##7)\
	MT_EXPRESSION_IMPL_K0_K9(expression,K##8)\
	MT_EXPRESSION_IMPL_K0_K9(expression,K##9)

/// 0~7个表达式实现
#define MT_EXPRESSION_IMPL_0_7( expression ) \
	expression(0) \
	expression(1) \
	expression(2) \
	expression(3) \
	expression(4) \
	expression(5) \
	expression(6) \
	expression(7)

/// 0~9个表达式实现
#define MT_EXPRESSION_IMPL_0_9( expression ) \
	expression(0) \
	expression(1) \
	expression(2) \
	expression(3) \
	expression(4) \
	expression(5) \
	expression(6) \
	expression(7) \
	expression(8) \
	expression(9)

/// 0~99种表达式实现
#define MT_EXPRESSION_IMPL_0_99( expression ) \
	MT_EXPRESSION_IMPL_0_9(expression)\
	MT_EXPRESSION_IMPL_K0_K9(expression,1) \
	MT_EXPRESSION_IMPL_K0_K9(expression,2) \
	MT_EXPRESSION_IMPL_K0_K9(expression,3) \
	MT_EXPRESSION_IMPL_K0_K9(expression,4) \
	MT_EXPRESSION_IMPL_K0_K9(expression,5) \
	MT_EXPRESSION_IMPL_K0_K9(expression,6) \
	MT_EXPRESSION_IMPL_K0_K9(expression,7) \
	MT_EXPRESSION_IMPL_K0_K9(expression,8) \
	MT_EXPRESSION_IMPL_K0_K9(expression,9)

/// 0~999种表达式实现
#define MT_EXPRESSION_IMPL_0_999( expression ) \
	MT_EXPRESSION_IMPL_0_99(expression)\
	MT_EXPRESSION_IMPL_K00_K99(expression,1) \
	MT_EXPRESSION_IMPL_K00_K99(expression,2) \
	MT_EXPRESSION_IMPL_K00_K99(expression,3) \
	MT_EXPRESSION_IMPL_K00_K99(expression,4) \
	MT_EXPRESSION_IMPL_K00_K99(expression,5) \
	MT_EXPRESSION_IMPL_K00_K99(expression,6) \
	MT_EXPRESSION_IMPL_K00_K99(expression,7) \
	MT_EXPRESSION_IMPL_K00_K99(expression,8) \
	MT_EXPRESSION_IMPL_K00_K99(expression,9)

#endif