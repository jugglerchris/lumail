/**
 * lua_tmpl.h - Implementation of templated Lua/C++ access.
 *
 * This file is part of lumail: http://lumail.org/
 *
 * Copyright (c) 2013-2014 by Steve Kemp.  All rights reserved.
 *
 **
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 dated June, 1991, or (at your
 * option) any later version.
 *
 * On Debian GNU/Linux systems, the complete text of version 2 of the GNU
 * General Public License can be found in `/usr/share/common-licenses/GPL-2'
 */

#include "version.h"

template <typename Ret>
Ret null();

template <> 
inline std::string null<std::string>() {
    return "";
}

template <>
inline bool null<bool>() {
    return false;
}

template <typename T>
T from_lua(lua_State *L, int index);

template <>
inline std::string from_lua<std::string>(lua_State *L, int index)
{
    std::string result = lua_tostring(L, index);
    return result;
}

template <>
inline bool from_lua<bool>(lua_State *L, int index)
{
    return lua_toboolean(L, index);
}
template <typename T>
void push_one(lua_State *L, T param);

template <typename... Ts>
void push_vals(lua_State *L, Ts... params);

inline void push_vals(lua_State *L) {}

template <typename T, typename... Ts>
inline void push_vals(lua_State *L, T first, Ts... params)
{
    push_one(L, first);
    push_vals(L, params...);
}

template <typename Ret, typename... Params>
inline Ret call(lua_State *L, const char *name, Params... p)
{
    lua_getglobal(L, name);
    
    if (!lua_isfunction(L, -1))
    {
        return null<Ret>();
    }
    
    push_vals(L, p...);
    
    int error = lua_pcall(L, sizeof...(Params), 1, 0);
    
    if (error)
    {
        lua_getglobal(L, "on_error");
        /* We could check for an error, but what can we do?  Instead we'll
         * just get an error from lua_pcall. */
         
        /* Push the error string from the previous pcall onto the top of
         * the stack. */
        lua_pushvalue(L, -2);
        
        /* And call the error handler. */
        lua_pcall(L, 1, 0, 0);
         
        return null<Ret>();
    }
    
    return from_lua<Ret>(L, -1);
}
