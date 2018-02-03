/*
Copyright (C) 2014-2016 ezQuake team 

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

#include "quakedef.h"
#include <SDL/SDL.h>
#include "q_shared.h"
#include "cvar.h"

cvar_t sys_yieldcpu = {"sys_yieldcpu", "0"};
cvar_t sys_inactivesound = {"sys_inactivesound", "0", CVAR_ARCHIVE};
cvar_t sys_inactivesleep = {"sys_inactivesleep", "1"};
cvar_t sys_disable_alt_enter = {"sys_disable_alt_enter", "0"};

void Sys_CvarInit(void)
{
	Cvar_SetCurrentGroup(CVAR_GROUP_SYSTEM_SETTINGS);
	Cvar_Register(&sys_yieldcpu);
	Cvar_Register(&sys_inactivesound);
	Cvar_Register(&sys_inactivesleep);
	Cvar_Register(&sys_disable_alt_enter);
	Cvar_ResetCurrentGroup();

}

wchar *Sys_GetClipboardTextW(void)
{
	char *tmp;
	wchar *wtmp = NULL;

	if (SDL_HasClipboardText()) {
		tmp = SDL_GetClipboardText();
		wtmp = str2wcs(tmp);
		SDL_free(tmp);
	}

	return wtmp;
}

void Sys_CopyToClipboard(char *text)
{
	SDL_SetClipboardText(text);
}


int Sys_CreateDetachedThread(int (*func)(void *), void *data)
{
	SDL_Thread *thread;
	
	thread = SDL_CreateThread((SDL_ThreadFunction)func, NULL, data);
	if (!thread) {
		return -1;
	}

	SDL_DetachThread(thread);

	return 0;
}

