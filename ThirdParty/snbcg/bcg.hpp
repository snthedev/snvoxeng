#pragma once

#include <type_traits>
#include <string_view>
#include <iterator>
#include <span>
#include <vector>

// policy : return (hpp/cpp)
#define SNBCG_POLICY_RETURN_CREF            val
#define SNBCG_POLICY_RETURN_COPY            val
#define SNBCG_POLICY_RETURN_UNPTR          *val
#define SNBCG_POLICY_RETURN_PTR            &val
#define SNBCG_POLICY_RETURN_VIEW          ::std::string_view{ val }
#define SNBCG_POLICY_RETURN_SPAN            val

#define DETAIL_SNBCG_POLICY_RETURN_CREF_t(T)       std::add_lvalue_reference_t<std::add_const_t<T>>
#define DETAIL_SNBCG_POLICY_RETURN_COPY_t(T)        T
#define DETAIL_SNBCG_POLICY_RETURN_UNPTR_t(T)      std::add_lvalue_reference_t<std::add_const_t<std::remove_pointer_t<T>>>
#define DETAIL_SNBCG_POLICY_RETURN_PTR_t(T)        std::add_const_t<std::add_pointer_t<T>>
#define DETAIL_SNBCG_POLICY_RETURN_VIEW_t(T)       std::string_view
#define DETAIL_SNBCG_POLICY_RETURN_SPAN_t(T)       std::span<std::add_const_t<T::value_type>>

// policy : store (cpp)
#define SNBCG_POLICY_STORE_COPY(T)             arg
#define SNBCG_POLICY_STORE_MOVE(T)             ::std::move(arg)
#define SNBCG_POLICY_STORE_ADDR(T)             &arg
#define SNBCG_POLICY_STORE_STATIC_CAST(T)      static_cast<T>(arg)
#define SNBCG_POLICY_STORE_REINTERPRET_CAST(T) reinterpret_cast<T>(arg)
#define SNBCG_POLICY_STORE_CONST_CAST(T)       const_cast<T>(arg)
#define SNBCG_POLICY_STORE_DYNAMIC_CAST(T)     dynamic_cast<T>(arg)
#define SNBCG_POLICY_STORE_SPAN_COPY(T)        T{ arg.begin(), arg.end() }
#define SNBCG_POLICY_STORE_CONSTRUCT_PAREN(T)  T( arg )
#define SNBCG_POLICY_STORE_CONSTRUCT_BRACE(T)  T{ arg }

// action : append (cpp)
#define DETAIL_SNBCG_ACTION_APPEND_EMPLACE_SINGLE       val.emplace_back(arg)
#define DETAIL_SNBCG_ACTION_APPEND_PUSH_SINGLE          val.push_back(arg)
#define DETAIL_SNBCG_ACTION_APPEND_EMPLACE_MOVE_SINGLE  val.emplace_back(::std::move(arg))
#define DETAIL_SNBCG_ACTION_APPEND_PUSH_MOVE_SINGLE     val.push_back(::std::move(arg))

#define DETAIL_SNBCG_ACTION_APPEND_EMPLACE_MULTI        val.insert(val.end(), args.begin(), args.end())
#define DETAIL_SNBCG_ACTION_APPEND_PUSH_MULTI           val.insert(val.end(), args.begin(), args.end())
#define DETAIL_SNBCG_ACTION_APPEND_EMPLACE_MOVE_MULTI ::std::move(args.begin(), args.end(), ::std::back_inserter(val))
#define DETAIL_SNBCG_ACTION_APPEND_PUSH_MOVE_MULTI    ::std::move(args.begin(), args.end(), ::std::back_inserter(val))

#define SNBCG_ACTION_APPEND_EMPLACE SNBCG_ACTION_APPEND_EMPLACE
#define SNBCG_ACTION_APPEND_PUSH SNBCG_ACTION_APPEND_PUSH
#define SNBCG_ACTION_APPEND_EMPLACE_MOVE SNBCG_ACTION_APPEND_EMPLACE_MOVE
#define SNBCG_ACTION_APPEND_PUSH_MOVE SNBCG_ACTION_APPEND_PUSH_MOVE

// debug
#ifndef NDEBUG
#define DETAIL_SNBCG_DEBUG
#endif // ^ ~NDEBUG ^

// utils
#define DETAIL_SNBCG_CONCAT_IMPL(a, b) a##b
#define DETAIL_SNBCG_CONCAT(a, b) DETAIL_SNBCG_CONCAT_IMPL(a, b)

#define DETAIL_SNBCG_MACRO(_1, _2) _1
#define DETAIL_SNBCG_MACRO_NOT(_1, _2) _2
#define DETAIL_SNBCG_MACRO_ISEMPTY_IMPL(on_empty, on_else, ...) DETAIL_SNBCG_CONCAT(DETAIL_SNBCG_MACRO, __VA_OPT__(_NOT))(on_empty, on_else)
#define DETAIL_SNBCG_MACRO_ISEMPTY(what, on_empty, on_else) DETAIL_SNBCG_MACRO_ISEMPTY_IMPL(on_empty, on_else, what)
