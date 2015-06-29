/*
* lbci.c
* Lua bytecode inspector
* Luiz Henrique de Figueiredo <lhf@tecgraf.puc-rio.br>
* 25 Apr 2013 13:18:03
* This code is hereby placed in the public domain.
*/

#define LUA_CORE

#include "lua.h"
#include "lauxlib.h"

#include "ldebug.h"
#include "lobject.h"
#include "lstate.h"

/* this allows lbci to be a shared library */
#define luaP_opnames lbci_opnames
#define luaP_opmodes lbci_opmodes
#include "lopcodes.c"

static const Proto* Pget(lua_State *L, int i)
{
 if (lua_isuserdata(L,i)) return lua_touserdata(L,i);
 if (!lua_isfunction(L,i) || lua_iscfunction(L,i))
 {
  const char *msg = lua_pushfstring(L, "Lua function expected, got %s",
                                    luaL_typename(L,i));
  luaL_argerror(L,i,msg);
  return NULL;
 }
 return ((LClosure*)lua_topointer(L,i))->p;
}

#define UPVALNAME(x) ((f->upvalues[x].name) ? getstr(f->upvalues[x].name) : "-")

static int do_getupvalue(lua_State *L)		/** getupvalue(f,i) */
{
 const Proto* f=Pget(L,1);
 int i=luaL_checkinteger(L,2);
 if (i<=0 || i>f->sizeupvalues || f->upvalues==NULL) return 0;
 i--;
 lua_pushstring(L,UPVALNAME(i));
 lua_pushinteger(L,f->upvalues[i].instack);
 lua_pushinteger(L,f->upvalues[i].idx);
 return 3;
}

static int do_getlocal(lua_State *L)		/** getlocal(f,i) */
{
 const Proto* f=Pget(L,1);
 int i=luaL_checkinteger(L,2);
 if (i<=0 || i>f->sizelocvars || f->locvars==NULL) return 0;
 i--;
 lua_pushstring(L,getstr(f->locvars[i].varname));
 lua_pushinteger(L,f->locvars[i].startpc+1);
 lua_pushinteger(L,f->locvars[i].endpc+1);
 return 3;
}

static int do_getconstant(lua_State *L)		/** getconstant(f,i) */
{
 const Proto* f=Pget(L,1);
 int i=luaL_checkinteger(L,2);
 if (i<=0 || i>f->sizek || f->k==NULL) return 0;
 i--;
 lua_pushnil(L);
 L->top[-1]=f->k[i];
 return 1;
}

static int do_getfunction(lua_State *L)		/** getfunction(f,i) */
{
 const Proto* f=Pget(L,1);
 int i=luaL_checkinteger(L,2);
 if (i<=0 || i>f->sizep) return 0;
 i--;
 lua_pushlightuserdata(L,f->p[i]);
 return 1;
}

#define MYK(x)		(-1-(x))

static int do_getinstruction(lua_State *L)	/** getinstruction(f,i) */
{
 const Proto* f=Pget(L,1);
 int pc=luaL_checkinteger(L,2);
 if (pc<=0 || pc>f->sizecode || f->code==NULL) return 0;
 pc--;
 {
 const Instruction* code=f->code;
 Instruction i=code[pc];
 OpCode o=GET_OPCODE(i);
 int a=GETARG_A(i);
 int b=GETARG_B(i);
 int c=GETARG_C(i);
 int ax=GETARG_Ax(i);
 int bx=GETARG_Bx(i);
 int sbx=GETARG_sBx(i);
 int line=getfuncline(f,pc);
 if (line>0) lua_pushinteger(L,line); else lua_pushnil(L);
 lua_pushstring(L,luaP_opnames[o]);
 switch (getOpMode(o))
 {
  case iABC:
   lua_pushinteger(L,a);
   if (getBMode(o)!=OpArgN) lua_pushinteger(L,ISK(b) ? (MYK(INDEXK(b))) : b);
   else lua_pushnil(L);
   if (getCMode(o)!=OpArgN) lua_pushinteger(L,ISK(c) ? (MYK(INDEXK(c))) : c);
   else lua_pushnil(L);
   break;
  case iABx:
   lua_pushinteger(L,a);
   if (getBMode(o)==OpArgK) lua_pushinteger(L,MYK(bx)); else lua_pushinteger(L,bx);
   lua_pushnil(L);
   break;
  case iAsBx:
   lua_pushinteger(L,a);
   lua_pushinteger(L,sbx);
   lua_pushnil(L);
   break;
  case iAx:
   lua_pushinteger(L,MYK(ax));
   lua_pushnil(L);
   lua_pushnil(L);
   break;
 }
 switch (o)
 {
   case OP_JMP:
   case OP_FORLOOP:
   case OP_FORPREP:
   case OP_TFORLOOP:
    lua_pop(L,1);
    lua_pushinteger(L,sbx+pc+2);
    break;
   default:
    break;
 }
 }
 return 5;
}

#define setsfield(L,n,v) lua_pushstring(L,v); lua_setfield(L,-2,n)
#define setifield(L,n,v) lua_pushinteger(L,v); lua_setfield(L,-2,n)
#define setbfield(L,n,v) lua_pushboolean(L,v); lua_setfield(L,-2,n)

static int do_getheader(lua_State *L)		/** getheader(f,i) */
{
 const Proto* f=Pget(L,1);
 const char* s=f->source ? getstr(f->source) : "=?";
 if (*s=='@' || *s=='=')
  s++;
 else if (*s==LUA_SIGNATURE[0])
  s="(bstring)";
 else
  s="(string)";
 lua_newtable(L);
 setsfield(L,"source",s);
 setifield(L,"line",f->linedefined);
 setifield(L,"lastline",f->lastlinedefined);
 setifield(L,"instructions",f->sizecode);
 setifield(L,"params",f->numparams);
 setbfield(L,"isvararg",f->is_vararg);
 setifield(L,"slots",f->maxstacksize);
 setifield(L,"upvalues",f->sizeupvalues);
 setifield(L,"locals",f->sizelocvars);
 setifield(L,"constants",f->sizek);
 setifield(L,"functions",f->sizep);
 return 1;
}

static int do_setconstant(lua_State *L)		/** setconstant(f,i,v) */
{
 const Proto* f=Pget(L,1);
 int i=luaL_checkinteger(L,2);
 if (i<=0 || i>f->sizek || f->k==NULL) return 0;
 i--;
 lua_settop(L,3);
 f->k[i]=L->top[-1];
 return 0;
}

static const luaL_Reg R[] =
{
	{ "getconstant",	do_getconstant },
	{ "getfunction",	do_getfunction },
	{ "getheader",		do_getheader },
	{ "getinstruction",	do_getinstruction },
	{ "getlocal",		do_getlocal },
	{ "getupvalue",		do_getupvalue },
	{ "setconstant",	do_setconstant },
	{ NULL,			NULL	}
};

LUALIB_API int luaopen_bci(lua_State *L)
{
 luaL_newlib(L,R);
 return 1;
}
