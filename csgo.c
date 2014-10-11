 /*
	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "input.h"

#define RVA_BASE_ENTITY 0xA5D7C4
#define RVA_ENT_LIST 0x49FFA64
#define ENT_LIST_STRIDE 0x10

#define OFF_IN_CROSS 0x23DC
#define OFF_TEAM_NUM 0xF0

typedef unsigned long uint32_t;
typedef unsigned char BOOL;

uint32_t cdll_base = 0;

/*
 * Returns a pointer to the local player entity
 */
uint32_t get_local()
{
	uint32_t out;
	ReadMemory((void*)(cdll_base + RVA_BASE_ENTITY), &out, sizeof(out));
	
	return out;
}

/*
 * Fill players with an array of 64 player entity pointers
 */
uint32_t get_players(uint32_t players[])
{
	int i;
	for(i = 0; i < 64; i++) {
		uint32_t addr = cdll_base + RVA_ENT_LIST + i * ENT_LIST_STRIDE;
		ReadMemory((void*)(addr), &players[i], sizeof(players[i]));
	}
}

/*
 * Get the ID of the entity under the crosshair
 */
int get_in_cross_id(uint32_t local)
{
	int result;
	ReadMemory((void*)(local + OFF_IN_CROSS), &result, sizeof(result));
	
	return result;
}

/*
 * Return whether ent1 and ent2 are on different teams
 */
BOOL not_on_team(uint32_t ent1, uint32_t ent2)
{
	int team1, team2;
	ReadMemory((void*)(ent1 + OFF_TEAM_NUM), &team1, sizeof(team1));
	ReadMemory((void*)(ent2 + OFF_TEAM_NUM), &team2, sizeof(team2));
	
	return team1 != team2;
}

/*
 * Synthesize a mouse click
 */
void mouse_click()
{
	mdata.ButtonFlags |= MOUSE_LEFT_BUTTON_DOWN;
	SynthesizeMouse(&mdata);
	
	Sleep(50);
	
	mdata.ButtonFlags &= ~MOUSE_LEFT_BUTTON_DOWN;
	mdata.ButtonFlags |= MOUSE_LEFT_BUTTON_UP;
	SynthesizeMouse(&mdata);
}

#define VK_P 25

/*
 * Called every 5ms
 */
void main_loop()
{
	uint32_t local, players[64];
	int in_cross_id;
	
	if(cdll_base == 0 || GetKeyState(VK_P)) {
		ULONGLONG temp_base;
		
		if(AttachToProcess("csgo.exe") != STATUS_SUCCESS)
			return;
			
		GetModuleBase(L"client.dll", &temp_base);
		cdll_base = (uint32_t)(temp_base);
	}

	local = get_local();
	get_players(players);
	in_cross_id = get_in_cross_id(local);
	
	if(in_cross_id >= 1 && in_cross_id <= 64) {
		if(not_on_team(players[in_cross_id - 1], local))
			mouse_click();
	}
}

/*
 * Main thread called from input.h
 */
NTSTATUS SystemRoutine()
{
	BOOL reacq_key_held = FALSE;
	
	while(TRUE) {
		main_loop();
		Sleep(5);
	}
}