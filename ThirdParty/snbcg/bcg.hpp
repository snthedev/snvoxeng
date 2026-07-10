#pragma once

#include <type_traits>
#include <string_view>
#include <iterator>

// policy : return
#define SNBCG_POLICY_RETURN_CREF            val
#define SNBCG_POLICY_RETURN_COPY            val
#define SNBCG_POLICY_RETURN_UNPTR          *val
#define SNBCG_POLICY_RETURN_PTR            &val
#define SNBCG_POLICY_RETURN_VIEW          ::std::string_view{ val }

#define DETAIL_SNBCG_POLICY_RETURN_CREF_t(T)       std::add_lvalue_reference_t<std::add_const_t<T>>
#define DETAIL_SNBCG_POLICY_RETURN_COPY_t(T)        T
#define DETAIL_SNBCG_POLICY_RETURN_UNPTR_t(T)      std::add_lvalue_reference_t<std::add_const_t<std::remove_pointer_t<T>>>
#define DETAIL_SNBCG_POLICY_RETURN_PTR_t(T)        std::add_const_t<std::add_pointer_t<T>>
#define DETAIL_SNBCG_POLICY_RETURN_VIEW_t(T)       std::string_view

// policy : store
#define SNBCG_POLICY_STORE_COPY             arg
#define SNBCG_POLICY_STORE_MOVE             ::std::move(arg)
#define SNBCG_POLICY_STORE_ADDR             &arg
#define SNBCG_POLICY_STORE_STATIC_CAST      static_cast<store_t>(arg)
#define SNBCG_POLICY_STORE_REINTERPRET_CAST reinterpret_cast<store_t>(arg)
#define SNBCG_POLICY_STORE_CONST_CAST       const_cast<store_t>(arg)
#define SNBCG_POLICY_STORE_DYNAMIC_CAST     dynamic_cast<store_t>(arg)

// action : append
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
