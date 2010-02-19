/**\file			planets.cpp
 * \author			Chris Thielen (chris@luethy.net)
 * \date			Created: Unknown (2006?)
 * \date			Modified: Monday, November 16 2009
 * \brief
 * \details
 */

#include "includes.h"
#include "Sprites/planets.h"
#include "Utilities/log.h"
#include "Utilities/parser.h"
#include "Engine/models.h"
#include "Engine/engines.h"
#include "Engine/weapons.h"
#include "Sprites/spritemanager.h"


list<Model*> cPlanet::GetModels() {
	list<Model*> models;
	list<Technology*>::iterator techiter;
	list<Model*>::iterator listiter;
	for(techiter=technologies.begin(); techiter!=technologies.end(); ++techiter) {
		list<Model*> model_list = (*techiter)->GetModels();
		for(listiter=model_list.begin(); listiter!=model_list.end(); ++listiter) {
			models.push_back( *listiter );
		}
	}
	models.unique();
	return models;
}

list<Engine*> cPlanet::GetEngines() {
	list<Engine*> engines;
	list<Technology*>::iterator techiter;
	list<Engine*>::iterator listiter;
	for(techiter=technologies.begin(); techiter!=technologies.end(); ++techiter) {
		list<Engine*> engine_list = (*techiter)->GetEngines();
		for(listiter=engine_list.begin(); listiter!=engine_list.end(); ++listiter) {
			engines.push_back( *listiter );
		}
	}
	engines.unique();
	return engines;
}

list<Weapon*> cPlanet::GetWeapons() {
	list<Weapon*> weapons;
	list<Technology*>::iterator techiter;
	list<Weapon*>::iterator listiter;
	for(techiter=technologies.begin(); techiter!=technologies.end(); ++techiter) {
		list<Weapon*> weapon_list = (*techiter)->GetWeapons();
		for(listiter=weapon_list.begin(); listiter!=weapon_list.end(); ++listiter) {
			weapons.push_back( *listiter );
		}
	}
	weapons.unique();
	return weapons;
}

/**\class Planets
 * \brief Planets. */

Planets *Planets::pInstance = 0; // initialize pointer

Planets *Planets::Instance( void ) {
	if( pInstance == 0 ) { // is this the first call?
		pInstance = new Planets; // create the sold instance
	}
	return( pInstance );
}

bool Planets::Load( string filename ) {
	Parser<cPlanet> parser;
	
	SpriteManager* sprites = SpriteManager::Instance();
	planets = parser.Parse( filename, "planets", "planet" );

	for( list<cPlanet *>::iterator i = planets.begin(); i != planets.end(); ++i ) {
		(*i)->_dbg_PrintInfo();
		sprites->Add( (*i) );
	}

	return true;
}

void Planets_Lua::RegisterPlanets(lua_State *L){
	static const luaL_Reg PlanetFunctions[] = {
		// Normally we would put a "new" function here.
		// Lua may not ever need to create planets though.
		{NULL, NULL}
	};

	static const luaL_Reg PlanetMethods[] = {
		{"Name", &Planets_Lua::GetName},
		{"GetID", &Planets_Lua::GetID},
		{"GetType", &Planets_Lua::GetType},
		{"Position", &Planets_Lua::GetPosition},
		{"Alliance", &Planets_Lua::GetAlliance},
		{"Traffic", &Planets_Lua::GetTraffic},
		{"MilitiaSize", &Planets_Lua::GetMilitiaSize},
		{"Landable", &Planets_Lua::GetLandable},
		{"GetModels", &Planets_Lua::GetModels},
		{"GetEngines", &Planets_Lua::GetEngines},
		{"GetWeapons", &Planets_Lua::GetWeapons},
		{NULL, NULL}
	};
	luaL_newmetatable(L, EPIAR_PLANET);

	lua_pushstring(L, "__index");
	lua_pushvalue(L, -2);  /* pushes the metatable */
	lua_settable(L, -3);  /* metatable.__index = metatable */

	luaL_openlib(L, NULL, PlanetMethods,0);
	luaL_openlib(L, EPIAR_PLANET, PlanetFunctions,0);  
}


cPlanet **Planets_Lua::pushPlanet(lua_State *L){
	cPlanet **s = (cPlanet **)lua_newuserdata(L, sizeof(cPlanet*));
	*s = new cPlanet();
	luaL_getmetatable(L, EPIAR_PLANET);
	lua_setmetatable(L, -2);
	return s;
}

cPlanet **Planets_Lua::checkPlanet(lua_State *L, int index){
	cPlanet **p;
	luaL_checktype(L, index, LUA_TUSERDATA);
	p = (cPlanet**)luaL_checkudata(L, index, EPIAR_PLANET);
	if (p == NULL) luaL_typerror(L, index, EPIAR_PLANET);
	return p;
}

int Planets_Lua::GetName(lua_State* L){
	int n = lua_gettop(L);  // Number of arguments
	if (n == 1) {
		cPlanet** planet= checkPlanet(L,1);
		lua_pushstring(L, (*planet)->GetName().c_str());
	} else {
		luaL_error(L, "Got %d arguments expected 1 (self)", n); 
	}
	return 1;
}

int Planets_Lua::GetID(lua_State* L){
	int n = lua_gettop(L);  // Number of arguments
	if (n == 1) {
		cPlanet** planet= checkPlanet(L,1);
		lua_pushinteger(L, (*planet)->GetID());
	} else {
		luaL_error(L, "Got %d arguments expected 1 (self)", n);
	}
	return 1;
}

int Planets_Lua::GetType(lua_State* L){
	int n = lua_gettop(L);  // Number of arguments
	if (n == 1) {
		cPlanet** planet= checkPlanet(L,1);
		lua_pushinteger(L, (*planet)->GetDrawOrder());
	} else {
		luaL_error(L, "Got %d arguments expected 1 (self)", n);
	}
	return 1;
}

int Planets_Lua::GetPosition(lua_State* L){
	int n = lua_gettop(L);  // Number of arguments
	if (n == 1) {
		cPlanet** planet= checkPlanet(L,1);
		lua_pushnumber(L, (*planet)->GetWorldPosition().GetX() );
		lua_pushnumber(L, (*planet)->GetWorldPosition().GetY() );
	} else {
		luaL_error(L, "Got %d arguments expected 1 (self)", n); 
	}
	return 2;
}

int Planets_Lua::GetAlliance(lua_State* L){
	int n = lua_gettop(L);  // Number of arguments
	if (n == 1) {
		cPlanet** planet= checkPlanet(L,1);
		lua_pushstring(L, (*planet)->GetAlliance().c_str());
	} else {
		luaL_error(L, "Got %d arguments expected 1 (self)", n); 
	}
	return 1;
}

int Planets_Lua::GetTraffic(lua_State* L){
	int n = lua_gettop(L);  // Number of arguments
	if (n == 1) {
		cPlanet** planet= checkPlanet(L,1);
		lua_pushnumber(L, (*planet)->GetTraffic() );
	} else {
		luaL_error(L, "Got %d arguments expected 1 (self)", n); 
	}
	return 1;
}

int Planets_Lua::GetMilitiaSize(lua_State* L){
	int n = lua_gettop(L);  // Number of arguments
	if (n == 1) {
		cPlanet** planet= checkPlanet(L,1);
		lua_pushnumber(L, (*planet)->GetMilitiaSize() );
	} else {
		luaL_error(L, "Got %d arguments expected 1 (self)", n); 
	}
	return 1;
}

int Planets_Lua::GetLandable(lua_State* L){
	int n = lua_gettop(L);  // Number of arguments
	if (n == 1) {
		cPlanet** planet= checkPlanet(L,1);
		lua_pushboolean(L, (*planet)->GetLandable() );
	} else {
		luaL_error(L, "Got %d arguments expected 1 (self)", n); 
	}
	return 1;
}

int Planets_Lua::GetModels(lua_State* L){
	int n = lua_gettop(L);  // Number of arguments
	if (n == 1) {
		cPlanet** planet= checkPlanet(L,1);
		list<Model*> models = (*planet)->GetModels();
		list<Model*>::iterator iter;
		lua_createtable(L, models.size(), 0);
		int newTable = lua_gettop(L);
		int index = 1;
		for(iter=models.begin();iter!=models.end();++iter,++index){
			lua_pushstring(L, (*iter)->GetName().c_str() );
			lua_rawseti(L, newTable, index);
		}
	} else {
		luaL_error(L, "Got %d arguments expected 1 (self)", n);
	}
	return 1;
}

int Planets_Lua::GetEngines(lua_State* L){
	int n = lua_gettop(L);  // Number of arguments
	if (n == 1) {
		cPlanet** planet= checkPlanet(L,1);
		list<Engine*> engines = (*planet)->GetEngines();
		list<Engine*>::iterator iter;
		lua_createtable(L, engines.size(), 0);
		int newTable = lua_gettop(L);
		int index = 1;
		for(iter=engines.begin();iter!=engines.end();++iter,++index){
			lua_pushstring(L, (*iter)->GetName().c_str() );
			lua_rawseti(L, newTable, index);
		}
	} else {
		luaL_error(L, "Got %d arguments expected 1 (self)", n);
	}
	return 1;
}

int Planets_Lua::GetWeapons(lua_State* L){
	int n = lua_gettop(L);  // Number of arguments
	if (n == 1) {
		cPlanet** planet= checkPlanet(L,1);
		list<Weapon*> weapons = (*planet)->GetWeapons();
		list<Weapon*>::iterator iter;
		lua_createtable(L, weapons.size(), 0);
		int newTable = lua_gettop(L);
		int index = 1;
		for(iter=weapons.begin();iter!=weapons.end();++iter,++index){
			lua_pushstring(L, (*iter)->GetName().c_str() );
			lua_rawseti(L, newTable, index);
		}
	} else {
		luaL_error(L, "Got %d arguments expected 1 (self)", n);
	}
	return 1;
}

