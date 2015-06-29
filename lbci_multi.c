#define LUA_CORE
#include "lua.h"
#include "lauxlib.h"

#if LUA_VERSION_NUM == 503

#include "lua5.3/ldebug.h"
#include "lua5.3/lobject.h"
#include "lua5.3/lstate.h"
#define luaP_opnames lbci_opnames
#define luaP_opmodes lbci_opmodes
#include "lua5.3/lopcodes.c"
#undef luaP_opnames
#undef luaP_opmodes
#undef LUA_CORE
/* the lbci code for Lua 5.2 also works for Lua 5.3: */
#include "lbci-5.2.c"

#elif LUA_VERSION_NUM == 502

#include "lua5.2/ldebug.h"
#include "lua5.2/lobject.h"
#include "lua5.2/lstate.h"
#define luaP_opnames lbci_opnames
#define luaP_opmodes lbci_opmodes
#include "lua5.2/lopcodes.c"
#undef luaP_opnames
#undef luaP_opmodes
#undef LUA_CORE
#include "lbci-5.2.c"

#elif LUA_VERSION_NUM == 501

#include "lua5.1/ldebug.h"
#include "lua5.1/lobject.h"
#include "lua5.1/lstate.h"
#define luaP_opnames lbci_opnames
#define luaP_opmodes lbci_opmodes
#include "lua5.1/lopcodes.c"
#undef luaP_opnames
#undef luaP_opmodes
#undef LUA_CORE
#include "lbci-5.1.c"

#else

#error "unsupported Lua version for lbci (i.e. not Lua 5.1, 5.2, or 5.3)"

#endif

