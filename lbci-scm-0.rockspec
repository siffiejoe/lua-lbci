package="lbci"
version="scm-0"

source = {
   url = "https://github.com/siffiejoe/lua-lbci/archive/master.zip",
   dir = "lua-lbci-master",
}

description = {
   summary = "A bytecode inspector library",
   detailed = [[
      A bytecode inspector library.
   ]],
   homepage = "http://www.tecgraf.puc-rio.br/~lhf/ftp/lua/#lbci",
   license = "Public domain, MIT"
}

dependencies = {
  -- Uses Lua internals. Includes support for Lua 5.1, 5.2, and 5.3.
   "lua >= 5.1, < 5.4"
}

build = {
   type = "builtin",
   modules = {
      bci = {
         sources = {
            "lbci_multi.c"
         },
         incdirs = {
            "dummy"
         }
      }
   }
}

