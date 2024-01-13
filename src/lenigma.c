#include "lua.h"
#include "lauxlib.h"

#include "enigma.h"

typedef struct
{
    int reflect_ref;
    int replace_ref;
    enigma_machine_t *m;
    lua_State *L;
} lenigma_t;

static int
lenigma_gc(lua_State *L)
{
    if (lua_gettop(L) != 1)
    {
        return luaL_error(L, "must have a enigma instance");
    }
    lenigma_t *self = (lenigma_t *) luaL_checkudata(L, 1, "enigma.Enigma");
    luaL_unref(L, LUA_REGISTRYINDEX, self->reflect_ref);
    luaL_unref(L, LUA_REGISTRYINDEX, self->replace_ref);
    enigma_machine_del(self->m);
    self->m = NULL;
    return 0;
}

static int
lenigma_roll(lua_State *L)
{
    if (lua_gettop(L) != 3)
    {
        return luaL_error(L, "must have a enigma instance, roller index and count");
    }
    lenigma_t *self = (lenigma_t *) luaL_checkudata(L, 1, "enigma.Enigma");
    size_t idx = (size_t) luaL_checkinteger(L, 2);
    int count = (int) luaL_checkinteger(L, 3);
    enigma_machine_roll(self->m, idx, count);
    return 0;
}

static int
lenigma_encode(lua_State *L)
{
    if (lua_gettop(L) != 2)
    {
        return luaL_error(L, "must have a enigma instance, data");
    }
    lenigma_t *self = (lenigma_t *) luaL_checkudata(L, 1, "enigma.Enigma");
    size_t size = 0;
    const uint8_t *data = (const uint8_t *) luaL_checklstring(L, 2, &size);
    uint8_t *out = (uint8_t *) lua_newuserdata(L, size);
    enigma_machine_encode_into(self->m, data, size, out);
    lua_pushlstring(L, (const char *) out, size);
    return 1;
}

static int
lenigma_dup(lua_State *L)
{
    if (lua_gettop(L) != 1)
    {
        return luaL_error(L, "must have a enigma instance");
    }
    lenigma_t *self = (lenigma_t *) luaL_checkudata(L, 1, "enigma.Enigma");
    enigma_machine_t *new = enigma_machine_dup(self->m);
    if (new == NULL)
    {
        return luaL_error(L, "failed to dup enigma_machine_t");
    }
    lenigma_t *newself = (lenigma_t *) lua_newuserdata(L, sizeof(lenigma_t));
    newself->L = L;
    newself->m = new;
    lua_rawgeti(L, LUA_REGISTRYINDEX, self->reflect_ref);
    newself->reflect_ref = luaL_ref(L, LUA_REGISTRYINDEX);
    lua_rawgeti(L, LUA_REGISTRYINDEX, self->replace_ref);
    newself->replace_ref = luaL_ref(L, LUA_REGISTRYINDEX);
    luaL_setmetatable(L, "enigma.Enigma");
    return 1;
}

static int
lenigma_test_replace(lua_State *L)
{
    if (lua_gettop(L) != 1)
    {
        return luaL_error(L, "must have a enigma instance");
    }
    lenigma_t *self = (lenigma_t *) luaL_checkudata(L, 1, "enigma.Enigma");
    bool ret = enigma_machine_test_replace(self->m);
    if(!ret)
    {
        return luaL_error(L, "test replace callback fail");
    }
    lua_pushboolean(L, (int)ret);
    return 1;
}

static int
lenigma_test_reflect(lua_State *L)
{
    if (lua_gettop(L) != 1)
    {
        return luaL_error(L, "must have a enigma instance");
    }
    lenigma_t *self = (lenigma_t *) luaL_checkudata(L, 1, "enigma.Enigma");
    bool ret = enigma_machine_test_reflect(self->m);
    if(!ret)
    {
        return luaL_error(L, "test reflect callback fail");
    }
    lua_pushboolean(L, (int)ret);
    return 1;
}

static int
lenigma_dump_replace_table(lua_State *L)
{
    if (lua_gettop(L) != 1)
    {
        return luaL_error(L, "must have a enigma instance");
    }
    lenigma_t *self = (lenigma_t *) luaL_checkudata(L, 1, "enigma.Enigma");
    uint8_t *out = (uint8_t *)lua_newuserdata(L, 256);
    enigma_machine_dump_replace_table(self->m, out);
    lua_pushlstring(L, out, 256);
    return 1;
}

static int
lenigma_dump_reflect_table(lua_State *L)
{
    if (lua_gettop(L) != 1)
    {
        return luaL_error(L, "must have a enigma instance");
    }
    lenigma_t *self = (lenigma_t *) luaL_checkudata(L, 1, "enigma.Enigma");
    uint8_t *out = (uint8_t *)lua_newuserdata(L, 256);
    enigma_machine_dump_reflect_table(self->m, out);
    lua_pushlstring(L, out, 256);
    return 1;
}

static int
lenigma_encode_count(lua_State *L)
{
    if (lua_gettop(L) != 1)
    {
        return luaL_error(L, "must have a enigma instance");
    }
    lenigma_t *self = (lenigma_t *) luaL_checkudata(L, 1, "enigma.Enigma");
    lua_pushinteger(L, (lua_Integer)self->m->encode_count);
    return 1;
}

static int
lenigma_rollers(lua_State *L)
{
    if (lua_gettop(L) != 1)
    {
        return luaL_error(L, "must have a enigma instance");
    }
    lenigma_t *self = (lenigma_t *) luaL_checkudata(L, 1, "enigma.Enigma");
    lua_pushinteger(L, (lua_Integer)self->m->rollers);
    return 1;
}

static int
lenigma_current_position(lua_State *L)
{
    if (lua_gettop(L) != 1)
    {
        return luaL_error(L, "must have a enigma instance");
    }
    lenigma_t *self = (lenigma_t *) luaL_checkudata(L, 1, "enigma.Enigma");
    for(int i=0;i<self->m->rollers;i++)
    {
        lua_pushinteger(L, (lua_Integer) self->m->offset[i]);
    }
    return (int)self->m->rollers;
}


static int
lenigma__ref(lua_State *L)
{
    if (lua_gettop(L) != 1)
    {
        return luaL_error(L, "must have a enigma instance");
    }
    lenigma_t *self = (lenigma_t *) luaL_checkudata(L, 1, "enigma.Enigma");
    lua_pushinteger(L, (lua_Integer)self->replace_ref);
    lua_pushinteger(L, (lua_Integer)self->reflect_ref);
    return 2;
}


static luaL_Reg lua_enigma_methods[] = {
        {"__gc", &lenigma_gc},
        {"roll", &lenigma_roll},
        {"encode", &lenigma_encode},
        {"dup", &lenigma_dup},
        {"test_replace", &lenigma_test_replace},
        {"test_reflect", &lenigma_test_reflect},
        {"dump_replace_table", &lenigma_dump_replace_table},
        {"dump_reflect_table", &lenigma_dump_reflect_table},
        {"encode_count", &lenigma_encode_count},
        {"rollers",&lenigma_rollers},
        {"current_position", &lenigma_current_position},
        {"_ref", &lenigma__ref},
        {NULL, NULL}
};

static uint8_t reflect_func_c(void *ud, uint8_t c)
{
    lenigma_t *self = (lenigma_t *) ud;
    lua_State *L = self->L;
    int oldtop = lua_gettop(L);
    lua_rawgeti(L, LUA_REGISTRYINDEX, self->reflect_ref); // func
    lua_pushinteger(L, (lua_Integer) c); // func arg
    lua_call(L, 1, 1); // ret
    uint8_t ret = (uint8_t) luaL_checkinteger(L, -1);
    lua_settop(L, oldtop);
    return ret;
}


static uint8_t replace_func_c(void *ud, uint8_t c)
{
    lenigma_t *self = (lenigma_t *) ud;
    lua_State *L = self->L;
    int oldtop = lua_gettop(L);
    lua_rawgeti(L, LUA_REGISTRYINDEX, self->replace_ref); // func
    lua_pushinteger(L, (lua_Integer) c); // func arg
    lua_call(L, 1, 1); // ret
    uint8_t ret = (uint8_t) luaL_checkinteger(L, -1);
    lua_settop(L, oldtop);
    return ret;
}

static int
newenigma(lua_State *L)
{
    if (lua_gettop(L) != 3)
    {
        return luaL_error(L, "wrong arguments, expect map, reflect func and replace func");
    }

    if (lua_type(L, 2) != LUA_TFUNCTION)
    {
        return luaL_argerror(L, 2, "reflect func must be a lua function");
    }
    if (lua_type(L, 3) != LUA_TFUNCTION)
    {
        return luaL_argerror(L, 3, "replace func must be a lua function");
    }
    int replace_ref = luaL_ref(L, LUA_REGISTRYINDEX);
    int reflect_ref = luaL_ref(L, LUA_REGISTRYINDEX);

    lenigma_t *self = (lenigma_t *) lua_newuserdata(L, sizeof(lenigma_t));
    self->L = L;
    size_t size = 0;
    const uint8_t *maps = (const uint8_t *) luaL_checklstring(L, 1, &size);
    self->m = enigma_machine_new(maps, size, &reflect_func_c, self, &replace_func_c, self);
    if (self->m == NULL)
    {
        luaL_unref(L, LUA_REGISTRYINDEX, replace_ref);
        luaL_unref(L, LUA_REGISTRYINDEX, reflect_ref);
        return luaL_error(L, "failed to malloc enigma_machine_t");
    }
    self->replace_ref = replace_ref;
    self->reflect_ref = reflect_ref;
    luaL_setmetatable(L, "enigma.Enigma");
    return 1;
}

static luaL_Reg lua_funcs[] = {
        {"new", &newenigma},
        {NULL, NULL}
};

DLLEXPORT int luaopen_enigma(lua_State *L)
{
    if (!luaL_newmetatable(L, "enigma.Enigma"))
    {
        return luaL_error(L, "enigma.Enigma already in register");
    }
    lua_pushvalue(L, -1); // mt mt
    lua_setfield(L, -2, "__index"); // mt
    luaL_setfuncs(L, lua_enigma_methods, 0); // mt
    luaL_newlib(L, lua_funcs);
    return 1;
}