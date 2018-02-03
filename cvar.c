/*
Copyright (C) 1996-1997 Id Software, Inc.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.


*/
// cvar.c -- dynamic variable tracking

#ifdef SERVERONLY
#include "qwsvdef.h"

#define VAR_HASHPOOL_SIZE 32
#else
#include "common.h"
#ifdef WITH_TCL
#include "embed_tcl.h"
#endif
//#include "teamplay.h"
//#include "rulesets.h"
#include "keys.h"
//#include "utils.h"
#include "parser.h"

extern void CL_UserinfoChanged (char *key, char *value);
extern void SV_ServerinfoChanged (char *key, char *value);
extern void Help_DescribeCvar (cvar_t *v);

extern cvar_t r_fullbrightSkins;
extern cvar_t cl_fakeshaft;
extern cvar_t allow_scripts;

cvar_t	cvar_viewdefault = {"cvar_viewdefault", "1"};
cvar_t	cvar_viewhelp    = {"cvar_viewhelp",    "1"};
cvar_t  cvar_viewlatched = {"cvar_viewlatched", "1"};

#define VAR_HASHPOOL_SIZE 1024

static void Cvar_AddCvarToGroup(cvar_t *var);
#endif

static cvar_t *cvar_hash[VAR_HASHPOOL_SIZE];
cvar_t *cvar_vars;
static char	*cvar_null_string = "";

// Use this to walk through all vars
cvar_t *Cvar_Next (cvar_t *var)
{
	if (var)
		return var->next;
	else
		return cvar_vars;
}

