/**\file			simulation_lua.cpp
 * \author			Matt Zweig
 * \date			Created: Friday, September 3, 2010
 * \date			Modified: Friday, September 3, 2010
 * \brief			Simulation Managment from Lua
 * \details
 */

#include "includes.h"
#include "common.h"

#include "Audio/audio.h"
#include "Audio/audio_lua.h"
#include "Engine/console.h"
#include "Engine/simulation.h"
#include "Engine/simulation_lua.h"
#include "Engine/models.h"
#include "Engine/alliances.h"
#include "Utilities/log.h"
#include "Utilities/lua.h"
#include "AI/ai_lua.h"
#include "UI/ui_lua.h"
#include "UI/ui.h"
#include "UI/ui_window.h"
#include "UI/ui_label.h"
#include "UI/ui_button.h"
#include "Graphics/video.h"
#include "Sprites/player.h"
#include "Sprites/sprite.h"
#include "Sprites/planets.h"
#include "Sprites/gate.h"
#include "Utilities/camera.h"
#include "Input/input.h"
#include "Utilities/file.h"
#include "Utilities/filesystem.h"

#include "Engine/hud.h"

void Simulation_Lua::RegisterSimulation(lua_State *L) {
	Lua::RegisterGlobal("WIDTH", Video::GetWidth() );
	Lua::RegisterGlobal("HEIGHT", Video::GetHeight() );

	static const luaL_Reg EngineFunctions[] = {
		//{"echo", &Simulation_Lua::console_echo},
		{"pause", &Simulation_Lua::pause},
		{"unpause", &Simulation_Lua::unpause},
		{"ispaused", &Simulation_Lua::ispaused},
		{"getoption", &Simulation_Lua::getoption},
		{"setoption", &Simulation_Lua::setoption},
		{"loadPlayer", &Simulation_Lua::loadPlayer},
		{"savePlayer", &Simulation_Lua::savePlayer},
		{"newPlayer", &Simulation_Lua::newPlayer},
		{"players", &Simulation_Lua::getPlayerNames},
		{"player", &Simulation_Lua::getPlayer},
		{"setLastPlanet", &Simulation_Lua::setLastPlanet},
		{"NewGatePair", &Simulation_Lua::NewGatePair},
		{"getCamera", &Simulation_Lua::getCamera},
		{"moveCamera", &Simulation_Lua::moveCamera},
		{"shakeCamera", &Simulation_Lua::shakeCamera},
		{"focusCamera", &Simulation_Lua::focusCamera},
		{"commodities", &Simulation_Lua::getCommodityNames},
		{"alliances", &Simulation_Lua::getAllianceNames},
		{"models", &Simulation_Lua::getModelNames},
		{"weapons", &Simulation_Lua::getWeaponNames},
		{"outfits", &Simulation_Lua::getOutfitNames},
		{"engines", &Simulation_Lua::getEngineNames},
		{"technologies", &Simulation_Lua::getTechnologyNames},
		{"planetNames", &Simulation_Lua::getPlanetNames},
		{"gateNames", &Simulation_Lua::getGateNames},
		{"getSprite", &Simulation_Lua::getSpriteByID},
		{"getMSRP", &Simulation_Lua::getMSRP},
		{"ships", &Simulation_Lua::getShips},
		{"planets", &Simulation_Lua::getPlanets},
		{"gates", &Simulation_Lua::getGates},
		{"nearestShip", &Simulation_Lua::getNearestShip},
		{"nearestPlanet", &Simulation_Lua::getNearestPlanet},
		{"RegisterKey", &Simulation_Lua::RegisterKey},
		{"UnRegisterKey", &Simulation_Lua::UnRegisterKey},
		{"getCommodityInfo", &Simulation_Lua::getCommodityInfo},
		{"getAllianceInfo", &Simulation_Lua::getAllianceInfo},
		{"getModelInfo", &Simulation_Lua::getModelInfo},
		{"getPlanetInfo", &Simulation_Lua::getPlanetInfo},
		{"getGateInfo", &Simulation_Lua::getGateInfo},
		{"getWeaponInfo", &Simulation_Lua::getWeaponInfo},
		{"getEngineInfo", &Simulation_Lua::getEngineInfo},
		{"getOutfitInfo", &Simulation_Lua::getOutfitInfo},
		{"getTechnologyInfo", &Simulation_Lua::getTechnologyInfo},
		{"setInfo", &Simulation_Lua::setInfo},
		{"saveComponents", &Simulation_Lua::saveComponents},
		{"listImages", &Simulation_Lua::listImages},
		{NULL, NULL}
	};
	luaL_register(L,"Epiar",EngineFunctions);

}

void Simulation_Lua::StoreSimulation(lua_State *L, Simulation *sim) {
	// Store A pointer to the simulation is stored in the LUA_REGISTRYINDEX table.
	lua_pushstring(L,"EPIAR_SIMULATION"); // Key
	lua_pushlightuserdata(L, sim); // Value
	lua_settable(L,LUA_REGISTRYINDEX);
	lua_pop(L,1);
}

Simulation *Simulation_Lua::GetSimulation(lua_State *L) {
	Simulation* sim;
	// A pointer to the simulation is stored in the LUA_REGISTRYINDEX table.
	lua_pushstring(L,"EPIAR_SIMULATION"); // Key
	lua_gettable(L,LUA_REGISTRYINDEX);
	sim = (Simulation*)lua_topointer(L,-1);
	lua_pop(L,1);
	return sim;
}



/*
int Simulation_Lua::console_echo(lua_State *L) {
	const char *str = lua_tostring(L, 1); // get argument

	if(str == NULL)
		Console::InsertResult("nil");
	else
		Console::InsertResult(str);

	return 0;
}
*/

int Simulation_Lua::pause(lua_State *L){
	Simulation *sim = GetSimulation(L);
	sim->pause();
	return 0;
}

int Simulation_Lua::unpause(lua_State *L){
	Simulation *sim = GetSimulation(L);
	sim->unpause();
	return 0;
}

int Simulation_Lua::ispaused(lua_State *L){
	Simulation *sim = GetSimulation(L);
	lua_pushnumber(L, (int) sim->isPaused() );
	return 1;
}

int Simulation_Lua::savePlayer(lua_State *L){
	Simulation *sim = GetSimulation(L);
	sim->save();
	return 0;
}

int Simulation_Lua::getoption(lua_State *L) {
	int n = lua_gettop(L);  // Number of arguments
	if (n != 1)
		return luaL_error(L, "Got %d arguments expected 1 (option)", n);
	string path = (string)lua_tostring(L, 1);
	string value = OPTION(string,path);
	lua_pushstring(L, value.c_str());
	return 1;
}

int Simulation_Lua::setoption(lua_State *L) {
	int n = lua_gettop(L);  // Number of arguments
	if (n != 2)
		return luaL_error(L, "Got %d arguments expected 1 (option,value)", n);
	string path = (string)lua_tostring(L, 1);
	string value = (string)lua_tostring(L, 2);
	SETOPTION(path,value);
	return 0;
}

/**\brief Lua callable function to register a key.
 */
int Simulation_Lua::RegisterKey(lua_State *L) {
	int n = lua_gettop(L);  // Number of arguments
	if(n == 3) {
		Simulation *sim = GetSimulation(L);
		int triggerKey;
		if( lua_isnumber(L,1) ) {
			triggerKey = (int)(luaL_checkint(L,1));
		} else {
			triggerKey = (int)(luaL_checkstring(L,1)[0]);
		}
		keyState triggerState = (keyState)(luaL_checkint(L,2));
		string command = (string)luaL_checkstring(L,3);
		sim->GetInput()->RegisterCallBack(InputEvent(KEY, triggerState, triggerKey), command);
	} else {
		luaL_error(L, "Got %d arguments expected 3 (Key, State, Command)", n);
	}
	return 0;
}

/**\brief Lua callable function to unregister a key.
 */
int Simulation_Lua::UnRegisterKey(lua_State *L) {
	int n = lua_gettop(L);  // Number of arguments
	if(n == 2) {
		Simulation *sim = GetSimulation(L);
		int triggerKey;
		if( lua_isnumber(L,1) ) {
			triggerKey = (int)(luaL_checkint(L,1));
		} else {
			triggerKey = (int)(luaL_checkstring(L,1)[0]);
		}
		keyState triggerState = (keyState)(luaL_checkint(L,2));
		sim->GetInput()->UnRegisterCallBack(InputEvent(KEY, triggerState, triggerKey));
	} else {
		luaL_error(L, "Got %d arguments expected 2 (Key, State)", n);
	}
	return 0;
}

int Simulation_Lua::getPlayerNames(lua_State *L) {
	list<string> *names = GetSimulation(L)->GetPlayers()->GetNames();
	Lua::pushStringList(L,names);
	return 1;
}

int Simulation_Lua::loadPlayer(lua_State *L) {
	int n = lua_gettop(L);
	if (n != 1) {
		return luaL_error(L, "Loading a Player expects a name");
	}
	string playerName = (string) luaL_checkstring(L,1);
	cout<<"Loading Player: "<<playerName<<endl;
	Player* newPlayer = GetSimulation(L)->GetPlayers()->GetPlayer( playerName );
	if( newPlayer==NULL ) {
		return luaL_error(L, "There is no Player by the name '%s'",playerName.c_str());
	}
	GetSimulation(L)->GetPlayers()->LoadPlayer(playerName);
	return 0;
}


int Simulation_Lua::newPlayer(lua_State *L) {
	int n = lua_gettop(L);
	if (n != 1) {
		return luaL_error(L, "Loading a Player expects a name");
	}

	string playerName = (string) luaL_checkstring(L,1);
	cout<<"Creating Player: "<<playerName<<endl;

	GetSimulation(L)->GetPlayers()->CreateNew(playerName);

	return 0;
}

int Simulation_Lua::getPlayer(lua_State *L){
	Simulation_Lua::pushSprite(L,Player::Instance() );
	return 1;
}

int Simulation_Lua::setLastPlanet(lua_State *L){
	int n=lua_gettop(L);
	if(n!=1){
		return luaL_error(L, "%d arguments expected 1 argument the planet's name");
	}
	string planetName=(string) luaL_checkstring(L,1);
	Player::Instance()->setLastPlanet(planetName);
	return 0;
}
	
int Simulation_Lua::NewGatePair(lua_State *L){
	int n = lua_gettop(L);
	if (n != 4) {
		return luaL_error(L, "Only %d arguments. Gates require two x,y pairs (x1,y1,x2,y2)", n);
	}

	Gate* gate_1 = new Gate( Coordinate( luaL_checkinteger(L,1), luaL_checkinteger(L,2)));
	Gate* gate_2 = new Gate( Coordinate( luaL_checkinteger(L,3), luaL_checkinteger(L,4)));
	GetSimulation(L)->GetSpriteManager()->Add((Sprite*)gate_1);
	GetSimulation(L)->GetSpriteManager()->Add((Sprite*)gate_2);
	// Note that we need to set the exit _after_ adding to the SpriteManager since SetExit checks that the Sprite exists.
	Gate::SetPair( gate_1, gate_2 );

	return 0;
}

int Simulation_Lua::getCamera(lua_State *L){
	int n = lua_gettop(L);
	if (n != 0) {
		return luaL_error(L, "Getting the Camera Coordinates didn't expect %d arguments. But thanks anyway", n);
	}
	Coordinate c = GetSimulation(L)->GetCamera()->GetFocusCoordinate();
	lua_pushinteger(L,static_cast<lua_Integer>(c.GetX()));
	lua_pushinteger(L,static_cast<lua_Integer>(c.GetY()));
	return 2;
}

int Simulation_Lua::moveCamera(lua_State *L){
	int n = lua_gettop(L);
	if (n != 2) {
		return luaL_error(L, "Moving the Camera needs 2 arguments (X,Y) not %d arguments", n);
	}
	int x = luaL_checkinteger(L,1);
	int y = luaL_checkinteger(L,2);
	GetSimulation(L)->GetCamera()->Focus((Sprite*)NULL); // This unattaches the Camera from the focusSprite
	GetSimulation(L)->GetCamera()->Move(-x,y);
	return 0;
}
//Allow camera shaking from Lua
int Simulation_Lua::shakeCamera(lua_State *L){
	if (lua_gettop(L) == 4) {
		Camera *pInstance = GetSimulation(L)->GetCamera();
		pInstance->Shake(int(luaL_checknumber(L, 1)), int(luaL_checknumber(L,
						2)),  new Coordinate(luaL_checknumber(L, 3),luaL_checknumber(L, 2)));
	}
	return 0;
}

int Simulation_Lua::focusCamera(lua_State *L){
	int n = lua_gettop(L);
	if (n == 1) {
		int id = (int)(luaL_checkint(L,1));
		SpriteManager *sprites= GetSimulation(L)->GetSpriteManager();
		Sprite* target = sprites->GetSpriteByID(id);
		if(target!=NULL)
			GetSimulation(L)->GetCamera()->Focus( target );
	} else if (n == 2) {
		double x,y;
		x = (luaL_checknumber(L,1));
		y = (luaL_checknumber(L,2));
		GetSimulation(L)->GetCamera()->Focus((Sprite*)NULL);
		GetSimulation(L)->GetCamera()->Focus(x,y);
	} else {
		return luaL_error(L, "Got %d arguments expected 1 (SpriteID) or 2 (X,Y)", n);
	}
	return 0;
}

int Simulation_Lua::getCommodityNames(lua_State *L){
	list<string> *names = GetSimulation(L)->GetCommodities()->GetNames();
	Lua::pushStringList(L,names);
	return 1;
}

int Simulation_Lua::getAllianceNames(lua_State *L){
	list<string> *names = GetSimulation(L)->GetAlliances()->GetNames();
	Lua::pushStringList(L,names);
	return 1;
}

int Simulation_Lua::getWeaponNames(lua_State *L){
	list<string> *names = GetSimulation(L)->GetWeapons()->GetNames();
	Lua::pushStringList(L,names);
	return 1;
}

int Simulation_Lua::getOutfitNames(lua_State *L){
	list<string> *names = GetSimulation(L)->GetOutfits()->GetNames();
	Lua::pushStringList(L,names);
	return 1;
}

int Simulation_Lua::getModelNames(lua_State *L){
	list<string> *names = GetSimulation(L)->GetModels()->GetNames();
	Lua::pushStringList(L,names);
	return 1;
}

int Simulation_Lua::getEngineNames(lua_State *L){
	list<string> *names = GetSimulation(L)->GetEngines()->GetNames();
	Lua::pushStringList(L,names);
	return 1;
}

int Simulation_Lua::getTechnologyNames(lua_State *L){
	list<string> *names = GetSimulation(L)->GetTechnologies()->GetNames();
	Lua::pushStringList(L,names);
	return 1;
}

int Simulation_Lua::getPlanetNames(lua_State *L){
	list<string> *names = GetSimulation(L)->GetPlanets()->GetNames();
	Lua::pushStringList(L,names);
	return 1;
}

int Simulation_Lua::getGateNames(lua_State *L){
	list<string> *names = GetSimulation(L)->GetGates()->GetNames();
	Lua::pushStringList(L,names);
	return 1;
}

void Simulation_Lua::pushSprite(lua_State *L,Sprite* s){
	int* id = (int*)lua_newuserdata(L, sizeof(int*));
	*id = s->GetID();
	switch(s->GetDrawOrder()){
	case DRAW_ORDER_SHIP:
	case DRAW_ORDER_PLAYER:
		luaL_getmetatable(L, EPIAR_SHIP);
		lua_setmetatable(L, -2);
		break;
	case DRAW_ORDER_PLANET:
		luaL_getmetatable(L, EPIAR_PLANET);
		lua_setmetatable(L, -2);
		break;
	default:
		LogMsg(ERR,"Accidentally pushing sprite #%d with invalid kind: %d",s->GetID(),s->GetDrawOrder());
		//assert(s->GetDrawOrder() & (DRAW_ORDER_SHIP | DRAW_ORDER_PLAYER | DRAW_ORDER_PLANET) );
		luaL_getmetatable(L, EPIAR_SHIP);
		lua_setmetatable(L, -2);
	}
}

/*
Sprite* Simulation_Lua::checkSprite(lua_State *L,int id ){
	int* idptr = (int*)luaL_checkudata(L, index, EPIAR_SHIP);
	cout<<"Checking ID "<<(*idptr)<<endl;
	luaL_argcheck(L, idptr != NULL, index, "`EPIAR_SHIP' expected");
	Sprite* s;
	s = GetSimulation(L)->GetSpriteManager()->GetSpriteByID(*idptr);
	return s;
}
*/

void Simulation_Lua::pushComponents(lua_State *L, list<Component*> *components){
	lua_createtable(L, components->size(), 0);
	int newTable = lua_gettop(L);
	int index = 1;
	list<Component*>::const_iterator iter = components->begin();
	while(iter != components->end()) {
		lua_pushstring(L, (*iter)->GetName().c_str());
		lua_rawseti(L, newTable, index);
		++iter;
		++index;
	}
}

int Simulation_Lua::getSpriteByID(lua_State *L){
	int n = lua_gettop(L);  // Number of arguments
	if( n!=1 )
		return luaL_error(L, "Got %d arguments expected 1 (SpriteID)", n);

	// Get the Sprite using the ID
	int id = (int)(luaL_checkint(L,1));
	Sprite* sprite = GetSimulation(L)->GetSpriteManager()->GetSpriteByID(id);

	// Return nil if the sprite no longer exists
	if(sprite==NULL){
		return 0;
	}

	Simulation_Lua::pushSprite(L,sprite);
	return 1;
}

int Simulation_Lua::getSprites(lua_State *L, int kind){
	int n = lua_gettop(L);  // Number of arguments

	list<Sprite *> *sprites = NULL;
	if( n==3 ){
		double x = luaL_checknumber (L, 1);
		double y = luaL_checknumber (L, 2);
		double r = luaL_checknumber (L, 3);
		sprites = GetSimulation(L)->GetSpriteManager()->GetSpritesNear(Coordinate(x,y),static_cast<float>(r),kind);
	} else {
		sprites = GetSimulation(L)->GetSpriteManager()->GetSprites(kind);
	}

	// Populate a Lua table with Sprites
	lua_createtable(L, sprites->size(), 0);
	int newTable = lua_gettop(L);
	int index = 1;
	list<Sprite *>::const_iterator iter = sprites->begin();
	while(iter != sprites->end()) {
		// push userdata
		pushSprite(L,(*iter));
		lua_rawseti(L, newTable, index);
		++iter;
		++index;
	}
	delete sprites;
	return 1;
}

int Simulation_Lua::getMSRP(lua_State *L) {
	int n = lua_gettop(L);  // Number of arguments
	if( n!=1 )
		return luaL_error(L, "Got %d arguments expected 1 (  )", n);
	string name = (string)luaL_checkstring(L,1);

	// Is there a priced Component named 'name'?
	Component* comp = NULL;
	if( (comp = GetSimulation(L)->GetModels()->Get(name)) != NULL )
		lua_pushinteger(L,((Model*)comp)->GetMSRP() );
	else if( (comp = GetSimulation(L)->GetEngines()->Get(name)) != NULL )
		lua_pushinteger(L,((Engine*)comp)->GetMSRP() );
	else if( (comp = GetSimulation(L)->GetWeapons()->Get(name)) != NULL )
		lua_pushinteger(L,((Weapon*)comp)->GetMSRP() );
	else if( (comp = GetSimulation(L)->GetCommodities()->Get(name)) != NULL )
		lua_pushinteger(L,((Commodity*)comp)->GetMSRP() );
	else if( (comp = GetSimulation(L)->GetOutfits()->Get(name)) != NULL )
		lua_pushinteger(L,((Outfit*)comp)->GetMSRP() );
	else {
		return luaL_error(L, "Couldn't find anything by the name: '%s'", name.c_str());
	}
	// One of those should have worked or we would have hit the above else
	assert(comp!=NULL);
	return 1;
}

int Simulation_Lua::getShips(lua_State *L){
	return Simulation_Lua::getSprites(L,DRAW_ORDER_SHIP);
}

int Simulation_Lua::getPlanets(lua_State *L){
	Planets *planets = GetSimulation(L)->GetPlanets();
	list<string>* planetNames = planets->GetNames();

	lua_createtable(L, planetNames->size(), 0);
	int newTable = lua_gettop(L);
	int index = 1;
	for( list<string>::iterator pname = planetNames->begin(); pname != planetNames->end(); ++pname){
		pushSprite(L,planets->GetPlanet(*pname));
		lua_rawseti(L, newTable, index);
		++index;
	}
	return 1;
}

int Simulation_Lua::getGates(lua_State *L){
	Gates *gates = GetSimulation(L)->GetGates();
	list<string>* gateNames = gates->GetNames();

	lua_createtable(L, gateNames->size(), 0);
	int newTable = lua_gettop(L);
	int index = 1;
	for( list<string>::iterator gname = gateNames->begin(); gname != gateNames->end(); ++gname){
		pushSprite(L,gates->GetGate(*gname));
		lua_rawseti(L, newTable, index);
		++index;
	}
	return 1;
}

int Simulation_Lua::getNearestSprite(lua_State *L,int kind) {
	int n = lua_gettop(L);  // Number of arguments
	if( n!=2 ){
		return luaL_error(L, "Got %d arguments expected 1 (ship, range)", n);
	}
	AI* ai = AI_Lua::checkShip(L,1);
	if( ai==NULL ) {
		return 0;
	}
	float r = static_cast<float>(luaL_checknumber (L, 2));
	Sprite *closest = GetSimulation(L)->GetSpriteManager()->GetNearestSprite((ai),r,kind);
	if(closest!=NULL){
		assert(closest->GetDrawOrder() & (kind));
		pushSprite(L,(closest));
		return 1;
	} else {
		return 0;
	}
}

int Simulation_Lua::getNearestShip(lua_State *L) {
	return Simulation_Lua::getNearestSprite(L,DRAW_ORDER_SHIP|DRAW_ORDER_PLAYER);
}

int Simulation_Lua::getNearestPlanet(lua_State *L) {
	return Simulation_Lua::getNearestSprite(L,DRAW_ORDER_PLANET);
}

int Simulation_Lua::getCommodityInfo(lua_State *L) {
	int n = lua_gettop(L);  // Number of arguments
	if( n!=1 )
		return luaL_error(L, "Got %d arguments expected 1 (AllianceName)", n);
	string name = (string)luaL_checkstring(L,1);
	Commodity *commodity = GetSimulation(L)->GetCommodities()->GetCommodity(name);
	if(commodity==NULL){ commodity = new Commodity(); }

	lua_newtable(L);
	Lua::setField("Name", commodity->GetName().c_str());
	Lua::setField("MSRP", commodity->GetMSRP());

	return 1;
}

int Simulation_Lua::getAllianceInfo(lua_State *L) {
	int n = lua_gettop(L);  // Number of arguments
	if( n!=1 )
		return luaL_error(L, "Got %d arguments expected 1 (AllianceName)", n);
	Color color;
	char color_buffer[9];
	string name = (string)luaL_checkstring(L,1);
	Alliance *alliance = GetSimulation(L)->GetAlliances()->GetAlliance(name);
	if(alliance==NULL){ alliance = new Alliance(); }

	color = alliance->GetColor();
	snprintf(color_buffer, sizeof(color_buffer), "0x%02X%02X%02X", int(0xFF*color.r), int(0xFF*color.g), int(0xFF*color.b) );

	lua_newtable(L);
	Lua::setField("Name", alliance->GetName().c_str());
	Lua::setField("AttackSize", alliance->GetAttackSize());
	Lua::setField("Aggressiveness", alliance->GetAggressiveness());
	Lua::setField("Currency", alliance->GetCurrency().c_str() );
	Lua::setField("Color", color_buffer );

	return 1;
}


int Simulation_Lua::getModelInfo(lua_State *L) {
	int n = lua_gettop(L);  // Number of arguments
	if( n!=1 )
		return luaL_error(L, "Got %d arguments expected 1 (modelName)", n);
	string modelName = (string)luaL_checkstring(L,1);
	Model *model = GetSimulation(L)->GetModels()->GetModel(modelName);
	if(model==NULL){ model = new Model(); }

	lua_newtable(L);
	Lua::setField("Name", model->GetName().c_str());
	Lua::setField("Image", (model->GetImage()!=NULL)
	                ? (model->GetImage()->GetPath().c_str())
	                : "" );
	Lua::setField("Mass", model->GetMass());
	Lua::setField("Thrust", model->GetThrustOffset());
	Lua::setField("Rotation", model->GetRotationsPerSecond());
	Lua::setField("MaxSpeed", model->GetMaxSpeed());
	Lua::setField("MaxHull", model->GetHullStrength());
	Lua::setField("MaxShield", model->GetShieldStrength());
	Lua::setField("MSRP", model->GetMSRP());
	Lua::setField("Cargo", model->GetCargoSpace());
	Lua::setField("SurfaceArea", model->GetSurfaceArea());

	return 1;
}

int Simulation_Lua::getPlanetInfo(lua_State *L) {
	int n = lua_gettop(L);  // Number of arguments
	if( n!=1 )
		return luaL_error(L, "Got %d arguments expected 1 (planetID)", n);

	// Figure out which planet we're fetching
	Planet* p = NULL;
	if( lua_isnumber(L,1)){
		int id = luaL_checkinteger(L,1);
		Sprite* sprite = GetSimulation(L)->GetSpriteManager()->GetSpriteByID(id);
		if( sprite->GetDrawOrder() != DRAW_ORDER_PLANET)
			return luaL_error(L, "ID #%d does not point to a Planet", id);
		p = (Planet*)(sprite);
	} else if( lua_isstring(L,1)){
		string name = luaL_checkstring(L,1);
		p = GetSimulation(L)->GetPlanets()->GetPlanet(name);
	}
	if(p==NULL){ p = new Planet(); }

	// Populate the Info Table.
	lua_newtable(L);
	Lua::setField("Name", p->GetName().c_str());
	Lua::setField("X", static_cast<float>(p->GetWorldPosition().GetX()));
	Lua::setField("Y", static_cast<float>(p->GetWorldPosition().GetY()));
	Lua::setField("Image", (p->GetImage()!=NULL)
	                ? (p->GetImage()->GetPath().c_str())
	                : "" );
	Lua::setField("Alliance", p->GetAlliance()->GetName().c_str());
	Lua::setField("Traffic", p->GetTraffic());
	Lua::setField("Militia", p->GetMilitiaSize());
	Lua::setField("Landable", p->GetLandable());
	Lua::setField("Influence", p->GetInfluence());
	lua_pushstring(L, "Technologies");
	list<Technology*> techs =  p->GetTechnologies();
	pushComponents(L,  (list<Component*>*)&techs );
	lua_settable(L, -3);
	return 1;
}

int Simulation_Lua::getGateInfo(lua_State *L) {
	int n = lua_gettop(L);  // Number of arguments
	if( n!=1 )
		return luaL_error(L, "Got %d arguments expected 1 (planetID)", n);

	// Figure out which gate we're fetching
	Gate* g = NULL;
	if( lua_isnumber(L,1)){
		int id = luaL_checkinteger(L,1);
		Sprite* sprite = GetSimulation(L)->GetSpriteManager()->GetSpriteByID(id);
		if( ! (sprite->GetDrawOrder() & (DRAW_ORDER_GATE_TOP|DRAW_ORDER_GATE_BOTTOM)) )
			return luaL_error(L, "ID #%d does not point to a Gate", id);
		g = (Gate*)(sprite);
	} else if( lua_isstring(L,1)){
		string name = luaL_checkstring(L,1);
		g = GetSimulation(L)->GetGates()->GetGate(name);
	}
	if(g==NULL){ g = new Gate(); }

	// Populate the Info Table.
	lua_newtable(L);
	Lua::setField("Name", g->GetName().c_str());
	Lua::setField("X", static_cast<float>(g->GetWorldPosition().GetX()));
	Lua::setField("Y", static_cast<float>(g->GetWorldPosition().GetY()));
	Lua::setField("Exit", (g->GetExit() != NULL) && (g->GetExit()->GetDrawOrder() & (DRAW_ORDER_GATE_TOP|DRAW_ORDER_GATE_BOTTOM))
	                    ? ( (Gate*)g->GetExit() )->GetName().c_str()
						: "" );

	return 1;
}

int Simulation_Lua::getWeaponInfo(lua_State *L) {
	int n = lua_gettop(L);  // Number of arguments
	if( n!=1 )
		return luaL_error(L, "Got %d arguments expected 1 (weaponName)", n);
	string weaponName = (string)luaL_checkstring(L,1);
	Weapon* weapon = GetSimulation(L)->GetWeapons()->GetWeapon(weaponName);
	if(weapon==NULL){ weapon = new Weapon(); }

	lua_newtable(L);
	Lua::setField("Name", weapon->GetName().c_str());
	Lua::setField("Image", (weapon->GetImage()!=NULL)
	                ? (weapon->GetImage()->GetPath().c_str())
	                : "" );
	Lua::setField("Picture", (weapon->GetPicture()!=NULL)
	                  ? (weapon->GetPicture()->GetPath().c_str())
	                  : "" );
	Lua::setField("Payload", weapon->GetPayload());
	Lua::setField("Velocity", weapon->GetVelocity());
	Lua::setField("Acceleration", weapon->GetAcceleration());
	Lua::setField("FireDelay", weapon->GetFireDelay());
	Lua::setField("Lifetime", weapon->GetLifetime());
	Lua::setField("Tracking", weapon->GetTracking());
	Lua::setField("MSRP", weapon->GetMSRP());
	Lua::setField("Type", weapon->GetType());
	Lua::setField("Ammo Type", Weapon::AmmoTypeToName(weapon->GetAmmoType()).c_str() );
	Lua::setField("Ammo Consumption", weapon->GetAmmoConsumption());
	Lua::setField("Sound", (weapon->sound!=NULL)
	                ? weapon->sound->GetPath().c_str()
	                : "" );
	return 1;
}

int Simulation_Lua::getEngineInfo(lua_State *L) {
	int n = lua_gettop(L);  // Number of arguments
	if( n!=1 )
		return luaL_error(L, "Got %d arguments expected 1 (outfitName)", n);
	string engineName = (string)luaL_checkstring(L,1);
	Engine* engine = GetSimulation(L)->GetEngines()->GetEngine(engineName);
	if(engine==NULL){ engine = new Engine(); }

	lua_newtable(L);
	Lua::setField("Name", engine->GetName().c_str());
	Lua::setField("Picture", (engine->GetPicture()!=NULL)
	                  ? (engine->GetPicture()->GetPath().c_str())
	                  : "" );
	Lua::setField("Force", engine->GetForceOutput());
	Lua::setField("Animation", engine->GetFlareAnimation().c_str());
	Lua::setField("MSRP", engine->GetMSRP());
	Lua::setField("Fold Drive", engine->GetFoldDrive());
	Lua::setField("Sound", (engine->GetSound() != NULL)
	                ? (engine->GetSound()->GetPath().c_str())
	                : "");
	return 1;
}

int Simulation_Lua::getOutfitInfo(lua_State *L) {
	int n = lua_gettop(L);  // Number of arguments
	if( n!=1 )
		return luaL_error(L, "Got %d arguments expected 1 (outfitName)", n);
	string outfitName = (string)luaL_checkstring(L,1);
	Outfit* outfit = GetSimulation(L)->GetOutfits()->GetOutfit(outfitName);
	if(outfit==NULL){ outfit = new Outfit(); }

	lua_newtable(L);
	Lua::setField("Name", outfit->GetName().c_str());
	Lua::setField("Picture", (outfit->GetPicture()!=NULL)
	                  ? (outfit->GetPicture()->GetPath().c_str())
	                  : "" );
	Lua::setField("Force", outfit->GetForceOutput());
	Lua::setField("Mass", outfit->GetMass());
	Lua::setField("Rotation", outfit->GetRotationsPerSecond());
	Lua::setField("MaxSpeed", outfit->GetMaxSpeed());
	Lua::setField("MaxHull", outfit->GetHullStrength());
	Lua::setField("MaxShield", outfit->GetShieldStrength());
	Lua::setField("MSRP", outfit->GetMSRP());
	Lua::setField("Cargo", outfit->GetCargoSpace());
	Lua::setField("SurfaceArea", outfit->GetSurfaceArea());
	return 1;
}

int Simulation_Lua::getTechnologyInfo(lua_State *L) {
	int newTable;
	int n = lua_gettop(L);  // Number of arguments
	if( n!=1 )
		return luaL_error(L, "Got %d arguments expected 1 (techName)", n);
	string techName = (string)luaL_checkstring(L,1);
	Technology* tech = GetSimulation(L)->GetTechnologies()->GetTechnology(techName);
	if( tech == NULL)
	{
		lua_createtable(L, 4, 0);
		newTable = lua_gettop(L);

		lua_createtable(L, 0, 0);
		lua_rawseti(L, newTable, 1);

		lua_createtable(L, 0, 0);
		lua_rawseti(L, newTable, 2);

		lua_createtable(L, 0, 0);
		lua_rawseti(L, newTable, 3);

		lua_createtable(L, 0, 0);
		lua_rawseti(L, newTable, 4);

		return 1;
	}

	lua_createtable(L, 4, 0);
	newTable = lua_gettop(L);

	// Push the Models Table
	list<Model*> models = tech->GetModels();
	pushComponents(L, (list<Component*>*) &models );
	lua_rawseti(L, newTable, 1);

	// Push the Weapons Table
	list<Weapon*> weapons = tech->GetWeapons();
	pushComponents(L, (list<Component*>*) &weapons );
	lua_rawseti(L, newTable, 2);

	// Push the Engines Table
	list<Engine*> engines = tech->GetEngines();
	pushComponents(L, (list<Component*>*) &engines );
	lua_rawseti(L, newTable, 3);

	// Push the Outfits Table
	list<Outfit*> outfits = tech->GetOutfits();
	pushComponents(L, (list<Component*>*) &outfits );
	lua_rawseti(L, newTable, 4);

	return 1;
}

int Simulation_Lua::setInfo(lua_State *L) {
	int n = lua_gettop(L);  // Number of arguments
	if( !(n==2||n==6)  )
		return luaL_error(L, "Got %d arguments expected 1 (infoType,infoTable)", n);
	string kind = luaL_checkstring(L,1);

	if(kind == "Alliance"){
		string name = Lua::getStringField(2,"Name");
		int attack = Lua::getIntField(2,"AttackSize");
		float aggressiveness = Lua::getNumField(2,"Aggressiveness");
		string currency = Lua::getStringField(2,"Currency");
		Color color = Color::Get( Lua::getStringField(2,"Color") );

		Alliance* thisAlliance = new Alliance(name,attack,aggressiveness,currency,color);
		GetSimulation(L)->GetAlliances()->AddOrReplace( thisAlliance );

	} else if(kind == "Commodity"){
		string name = Lua::getStringField(2,"Name");
		int msrp = Lua::getIntField(2,"MSRP");

		Commodity* thisCommodity= new Commodity(name,msrp);
		GetSimulation(L)->GetCommodities()->AddOrReplace( thisCommodity );

	} else if(kind == "Engine"){
		string name = Lua::getStringField(2,"Name");
		string pictureName = Lua::getStringField(2,"Picture");
		int force = Lua::getIntField(2,"Force");
		string flare = Lua::getStringField(2,"Animation");
		int msrp = Lua::getIntField(2,"MSRP");
		int foldDrive = Lua::getIntField(2,"Fold Drive");
		string soundName = Lua::getStringField(2,"Sound");

		Sound *sound = Sound::Get(soundName);
		Image *picture = Image::Get(pictureName);
		if(sound==NULL)
			LogMsg(NOTICE, "Could not create engine: there is no sound file '%s'.",soundName.c_str());
		if(picture==NULL)
			LogMsg(NOTICE, "Could not create engine: there is no image file '%s'.",pictureName.c_str());
		if((sound==NULL) || (picture==NULL))
			return 0;

		Engine* thisEngine = new Engine(name,sound,static_cast<float>(force),msrp,TO_BOOL(foldDrive),flare,picture);
		GetSimulation(L)->GetEngines()->AddOrReplace( thisEngine );

	} else if(kind == "Model"){
		string name = Lua::getStringField(2,"Name");
		string imageName = Lua::getStringField(2,"Image");
		float mass = Lua::getNumField(2,"Mass");
		int thrust = Lua::getIntField(2,"Thrust");
		float rot = Lua::getNumField(2,"Rotation");
		float speed = Lua::getNumField(2,"MaxSpeed");
		int hull = Lua::getIntField(2,"MaxHull");
		int shield = Lua::getIntField(2,"MaxShield");
		int msrp = Lua::getIntField(2,"MSRP");
		int cargo = Lua::getIntField(2,"Cargo");

		Image *image = Image::Get(imageName);
		if(image==NULL)
		{
			LogMsg(NOTICE, "Could not create engine: there is no image file '%s'.",imageName.c_str());
			return 0;
		}

		Model* thisModel = new Model(name,Image::Get(imageName),mass,thrust,rot,speed,hull,shield,msrp,cargo);
		GetSimulation(L)->GetModels()->AddOrReplace(thisModel);

	} else if(kind == "Planet"){
		string name = Lua::getStringField(2,"Name");
		int x = Lua::getIntField(2,"X");
		int y = Lua::getIntField(2,"Y");
		string imageName = Lua::getStringField(2,"Image");
		string allianceName = Lua::getStringField(2,"Alliance");
		int traffic = Lua::getIntField(2,"Traffic");
		int militia = Lua::getIntField(2,"Militia");
		int landable = Lua::getIntField(2,"Landable");
		int influence = Lua::getIntField(2,"Influence");
		list<string> techNames = Lua::getStringListField(2,"Technologies");

		// Process the Tech List
		list<Technology*> techs;
		list<string>::iterator i;
		for(i = techNames.begin(); i != techNames.end(); ++i) {
			if( NULL != GetSimulation(L)->GetTechnologies()->GetTechnology(*i) ) {
				 techs.push_back( GetSimulation(L)->GetTechnologies()->GetTechnology(*i) );
			} else {
				LogMsg(NOTICE, "Could not create planet: there is no Technology Group '%s'.",(*i).c_str());
				return 0;
			}
		}

		if(Image::Get(imageName)==NULL){
			 LogMsg(NOTICE, "Could not create planet: there is no Image at '%s'.",imageName.c_str());
			 return 0;
		}

		if(GetSimulation(L)->GetAlliances()->GetAlliance(allianceName)==NULL){
			 LogMsg(NOTICE, "Could not create planet: there is no Alliance named '%s'.",allianceName.c_str());
			 return 0;
		}

		Planet thisPlanet(name,TO_FLOAT(x),TO_FLOAT(y),Image::Get(imageName),GetSimulation(L)->GetAlliances()->GetAlliance(allianceName),TO_BOOL(landable),traffic,militia,influence,techs);

		Planet* oldPlanet = GetSimulation(L)->GetPlanets()->GetPlanet(name);
		if(oldPlanet!=NULL) {
			LogMsg(INFO,"Saving changes to '%s'",thisPlanet.GetName().c_str());
			*oldPlanet = thisPlanet;
		} else {
			LogMsg(INFO,"Creating new Planet '%s'",thisPlanet.GetName().c_str());
			Planet* newPlanet = new Planet(thisPlanet);
			GetSimulation(L)->GetPlanets()->Add(newPlanet);
			GetSimulation(L)->GetSpriteManager()->Add(newPlanet);
		}

	} else if(kind == "Gate"){
		string gateName = Lua::getStringField(2,"Name");
		int x = Lua::getIntField(2,"X");
		int y = Lua::getIntField(2,"Y");
		string exitName = Lua::getStringField(2,"Exit");

		Gate *gate = GetSimulation(L)->GetGates()->GetGate( gateName );
		Gate *exit = GetSimulation(L)->GetGates()->GetGate( exitName );
		if( gate != NULL ) {
			gate->SetWorldPosition( Coordinate(x,y) );
			if( exit != NULL ) {
				Gate::SetPair( gate,exit );
			}
		}

	} else if(kind == "Technology"){
		list<string>::iterator iter;
		list<Model*> models;
		list<Weapon*> weapons;
		list<Engine*> engines;
		list<Outfit*> outfits;

		string name = luaL_checkstring(L,2);

		list<string> modelNames = Lua::getStringListField(3);
		for(iter=modelNames.begin();iter!=modelNames.end();++iter){
			if(GetSimulation(L)->GetModels()->GetModel(*iter))
				models.push_back( GetSimulation(L)->GetModels()->GetModel(*iter) );
		}

		list<string> weaponNames = Lua::getStringListField(4);
		for(iter=weaponNames.begin();iter!=weaponNames.end();++iter){
			if(GetSimulation(L)->GetWeapons()->GetWeapon(*iter))
				weapons.push_back( GetSimulation(L)->GetWeapons()->GetWeapon(*iter) );
		}

		list<string> engineNames = Lua::getStringListField(5);
		for(iter=engineNames.begin();iter!=engineNames.end();++iter){
			if(GetSimulation(L)->GetEngines()->GetEngine(*iter))
				engines.push_back( GetSimulation(L)->GetEngines()->GetEngine(*iter) );
		}

		list<string> outfitNames = Lua::getStringListField(6);
		for(iter=outfitNames.begin();iter!=outfitNames.end();++iter){
			if(GetSimulation(L)->GetOutfits()->GetOutfit(*iter))
				outfits.push_back( GetSimulation(L)->GetOutfits()->GetOutfit(*iter) );
		}

		Technology* thisTechnology = new Technology(name,models,engines,weapons,outfits);
		GetSimulation(L)->GetTechnologies()->AddOrReplace( thisTechnology );

	} else if(kind == "Weapon"){
		string name = Lua::getStringField(2,"Name");
		string imageName = Lua::getStringField(2,"Image");
		string pictureName = Lua::getStringField(2,"Picture");
		int payload = Lua::getIntField(2,"Payload");
		int velocity = Lua::getIntField(2,"Velocity");
		int acceleration = Lua::getIntField(2,"Acceleration");
		int fireDelay = Lua::getIntField(2,"FireDelay");
		int lifetime = Lua::getIntField(2,"Lifetime");
		float tracking = Lua::getNumField(2,"Tracking");
		int msrp = Lua::getIntField(2,"MSRP");
		int type = Lua::getIntField(2,"Type");
		string ammoTypeName = Lua::getStringField(2,"Ammo Type");
		int ammoConsumption = Lua::getIntField(2,"Ammo Consumption");
		string soundName = Lua::getStringField(2,"Sound");

		Image *picture = Image::Get(pictureName);
		if(picture==NULL)
			return luaL_error(L, "Could not create weapon: there is no image file '%s'.",pictureName.c_str());
		Image *image = Image::Get(imageName);
		if(image==NULL)
			return luaL_error(L, "Could not create weapon: there is no image file '%s'.",imageName.c_str());
		if(Weapon::AmmoNameToType(ammoTypeName)==max_ammo)
			return luaL_error(L, "Could not create weapon: there is no ammo type '%s'.",ammoTypeName.c_str());
		Sound *sound = Sound::Get(soundName);
		if(sound==NULL)
			return luaL_error(L, "Could not create weapon: there is no sound file '%s'.",soundName.c_str());

		Weapon* thisWeaon = new Weapon(name,image,picture,type,payload,velocity,acceleration,Weapon::AmmoNameToType(ammoTypeName),ammoConsumption,fireDelay,lifetime,sound,tracking,msrp);
		GetSimulation(L)->GetWeapons()->AddOrReplace( thisWeaon );

	} else {
		return luaL_error(L, "Cannot set Info for kind '%s' must be one of {Alliance, Engine, Model, Planet, Technology, Weapon} ",kind.c_str());
	}
	return 0;
}

int Simulation_Lua::saveComponents(lua_State *L) {
	GetSimulation(L)->GetAlliances()->Save("Resources/Definitions/alliances-default.xml");
	GetSimulation(L)->GetCommodities()->Save("Resources/Definitions/commodities-default.xml");
	GetSimulation(L)->GetModels()->Save("Resources/Definitions/models-default.xml");
	GetSimulation(L)->GetWeapons()->Save("Resources/Definitions/weapons-default.xml");
	GetSimulation(L)->GetEngines()->Save("Resources/Definitions/engines-default.xml");
	GetSimulation(L)->GetPlanets()->Save("Resources/Definitions/planets-default.xml");
	GetSimulation(L)->GetTechnologies()->Save("Resources/Definitions/technologies-default.xml");
	return 0;
}

int Simulation_Lua::listImages(lua_State *L) {
	list<string> pics = Filesystem::Enumerate("Resources/Graphics/",".png");
	Lua::pushStringList(L,&pics);
	return 1;
}

