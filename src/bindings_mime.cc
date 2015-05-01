/**
 * bindings_mime.cc - Bindings for all MIME-related Lua primitives.
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
 *
 */


#include <algorithm>
#include <cstdlib>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>


#include "bindings.h"
#include "debug.h"
#include "file.h"
#include "global.h"
#include "lang.h"
#include "lua.h"
#include "message.h"




/**
 * Get the contents of the attachment.
 */
int attachment(lua_State *L)
{
    int offset = lua_tointeger(L,-1);

    std::shared_ptr<CMessage> msg = get_message_for_operation( NULL );
    if ( msg == NULL )
    {
        CLua *lua = CLua::Instance();
        lua->execute( "msg(\"" MISSING_MESSAGE "\");" );
        return( 0 );
    }

    std::shared_ptr<CAttachment> c = msg->get_attachment(offset);
    if ( !c )
    {
        return(0);
    }
    else
    {
        /**
         * NOTE: Using the long-form here, so we can push the
         * string even though it might contain NULLs.
         */
        lua_pushlstring(L, (char *)c->body(), c->size() );
        return( 1 );
    }
}


/**
 * Get a table of attachment names for this mail.
 */
int attachments(lua_State *L)
{
    /**
     * Get the path (optional).
     */
    const char *str = NULL;

    if (lua_isstring(L, -1))
        str = lua_tostring(L, 1);

    std::shared_ptr<CMessage> msg = get_message_for_operation( str );
    if ( msg == NULL )
    {
        CLua *lua = CLua::Instance();
        lua->execute( "msg(\"" MISSING_MESSAGE "\");" );
        return( 0 );
    }

    /**
     * Count the attachments.
     */
    std::vector<std::string> attachments = msg->attachments();

    /**
     * create a new table.
     */
    lua_newtable(L);

    /**
     * Lua indexes start at one.
     */
    int i = 1;

    /**
     * For each attachment, add it to the table.
     */
    for (std::string name : attachments)
    {
        lua_pushnumber(L,i);
        lua_pushstring(L,name.c_str());
        lua_settable(L,-3);
        i++;
    }

    return( 1 );

}


/**
 * Count attachments in this mail.
 */
int count_attachments(lua_State *L)
{
    /**
     * Get the path (optional).
     */
    const char *str = NULL;

    if (lua_isstring(L, -1))
        str = lua_tostring(L, 1);

    std::shared_ptr<CMessage> msg = get_message_for_operation( str );
    if ( msg == NULL )
    {
        CLua *lua = CLua::Instance();
        lua->execute( "msg(\"" MISSING_MESSAGE "\");" );
        return( 0 );
    }

    /**
     * Count the attachments.
     */
    std::vector<std::string> attachments = msg->attachments();
    int count = attachments.size();

    /**
     * Setup the return values.
     */
    lua_pushinteger(L, count );

    return( 1 );

}


/**
 * Count the MIME-parts in this message.
 */
int count_body_parts(lua_State *L)
{
    /**
     * Get the path (optional).
     */
    const char *str = NULL;

    if (lua_isstring(L, -1))
        str = lua_tostring(L, 1);

    std::shared_ptr<CMessage> msg = get_message_for_operation( str );
    if ( msg == NULL )
    {
        CLua *lua = CLua::Instance();
        lua->execute( "msg(\"" MISSING_MESSAGE "\");" );
        return( 0 );
    }

    /**
     * Get the parts, and store their count.
     */
    std::vector<std::string> parts = msg->body_mime_parts();
    int count = parts.size();

    /**
     * Setup the return values.
     */
    lua_pushinteger(L, count );

    return( 1 );
}


/**
 * Return the single body-part from the message.
 */
int get_body_part(lua_State *L)
{
    /**
     * Get the path to save to.
     */
    int offset       = lua_tointeger(L,-1);

    std::shared_ptr<CMessage> msg = get_message_for_operation(NULL);
    if ( msg == NULL )
    {
        CLua *lua = CLua::Instance();
        lua->execute( "msg(\"" MISSING_MESSAGE "\");" );
        return( 0 );
    }

    /**
     * Get the MIME-parts
     */
    std::vector<std::string> parts = msg->body_mime_parts();
    int count                      = parts.size();

    /**
     * Out of range: return false.
     */
    if ( ( offset < 1 ) || ( offset > count ) )
    {
        lua_pushboolean(L,0);
        return 1;
    }

    /**
     * Where we'll store the size/data.
     */
    char *result = NULL;
    size_t len = 0;

    if ( msg->get_body_part(offset, &result, &len ) )
    {
        lua_pushlstring(L, result, len);
        free(result);

        return 1;
    }
    else
    {
        lua_pushnil( L );
        return 1;
    }
}


/**
 * Return a table of the body-parts this message possesses.
 */
int get_body_parts(lua_State *L)
{
    /**
     * Get the path (optional).
     */
    const char *str = NULL;
    if (lua_isstring(L, -1))
        str = lua_tostring(L, 1);

    std::shared_ptr<CMessage> msg = get_message_for_operation( str );
    if ( msg == NULL )
    {
        CLua *lua = CLua::Instance();
        lua->execute( "msg(\"" MISSING_MESSAGE "\");" );
        return( 0 );
    }

    /**
     * Get the parts, and prepare to iterate over them.
     */
    std::vector<std::string> parts = msg->body_mime_parts();

    /**
     * create a new table.
     */
    lua_newtable(L);

    /**
     * Lua indexes start at one.
     */
    int i = 1;

    /**
     * For each attachment, add it to the table.
     */
    for (std::string name : parts)
    {
        lua_pushnumber(L,i);
        lua_pushstring(L,name.c_str());
        lua_settable(L,-3);
        i++;
    }

    return( 1 );
}



/**
 * Does the given message have a part of the given type?  (e.g. text/plain.)
 */
int has_body_part(lua_State *L)
{
    /**
     * Get the content-type.
     */
    const char *type = NULL;
    if (lua_isstring(L, -1))
        type = lua_tostring(L, 1);

    std::shared_ptr<CMessage> msg = get_message_for_operation( NULL );
    if ( msg == NULL )
    {
        CLua *lua = CLua::Instance();
        lua->execute( "msg(\"" MISSING_MESSAGE "\");" );
        return( 0 );
    }

    /**
     * Get the parts, and prepare to iterate over them.
     */
    std::vector<std::string> parts = msg->body_mime_parts();

    /**
     * Did we find at least one, part that has the specified type?
     */
    for (std::string ct : parts)
    {
        if ( strcmp( ct.c_str(), type ) == 0 )
        {
            /**
             * Found a matching type.
             */
            lua_pushboolean(L,1);
            return 1;
        }
    }

    lua_pushboolean(L, 0 );
    return( 1 );
}


/**
 * Save the specified attachment.
 */
int save_attachment(lua_State *L)
{
    /**
     * Get the path to save to.
     */
    int offset       = lua_tointeger(L,-2);
    const char *path = lua_tostring(L, -1);

    std::shared_ptr<CMessage> msg = get_message_for_operation(NULL);
    if ( msg == NULL )
    {
        CLua *lua = CLua::Instance();
        lua->execute( "msg(\"" MISSING_MESSAGE "\");" );
        return( 0 );
    }

    /**
     * Get the attachments.
     */
    std::vector<std::string> attachments = msg->attachments();
    int count                            = attachments.size();

    /**
     * Out of range: return false.
     */
    if ( ( offset < 1 ) || ( offset > count ) )
    {
        lua_pushboolean(L,0);
        return 1;
    }

    /**
     * Save the message.
     */
    bool ret = msg->save_attachment( offset, path );


    if ( ret )
    {
        assert(CFile::exists( path));
        lua_pushboolean(L, 1 );
    }
    else
    {
        lua_pushboolean(L, 0 );
    }
    return( 1 );

}

/**
 * Delete the Attachment pointer
 */
static int attachment_mt_gc(lua_State *L)
{
    void *ud = luaL_checkudata(L, 1, "attachment_mt");
    if (ud)
    {
        std::shared_ptr<CAttachment> *ud_attachment = static_cast<std::shared_ptr<CAttachment> *>(ud);
        
        /* Call the destructor */
        ud_attachment->~shared_ptr<CAttachment>();
    }
    return 0;
}
/**
 * The attachment metatable entries.
 */
static const luaL_Reg attachment_mt_fields[] = {
    { "__gc",    attachment_mt_gc },
    { NULL, NULL },  /* Terminator */
};


/**
 * Push the attachment metatable onto the Lua stack, creating it if needed.
 */
static void push_attachment_mt(lua_State *L)
{
    int created = luaL_newmetatable(L, "attachment_mt");
    if (created)
    {
        /* A new table was created, set it up now. */
        luaL_register(L, NULL, attachment_mt_fields);
        
        /* Set itself as its __index */
        lua_pushvalue(L, -1);
        lua_setfield(L, -2, "__index");
    }
}

/**
 * Push a message onto the Lua stack as a userdata.
 *
 * Returns true on success.
 */
bool push_attachment(lua_State *L, std::shared_ptr<CAttachment> attachment)
{
    void *ud = lua_newuserdata(L, sizeof(std::shared_ptr<CAttachment>));
    if (!ud)
        return false;
    
    /* Construct a blank shared_ptr.  To be safe, make sure it's a valid
     * object before setting the metatable. */
    std::shared_ptr<CAttachment> *ud_attachment = new (ud) std::shared_ptr<CAttachment>();
    if (!ud_attachment)
    {
        /* Discard the userdata */
        lua_pop(L, 1);
        return false;
    }
    
    /* FIXME: check errors */
    push_attachment_mt(L);
    
    lua_setmetatable(L, -2);
    
    /* And now store the maildir pointer into the userdata */
    *ud_attachment = attachment;
    
    return true;
}
