#include "Game_master.hpp"
#include "Ability.hpp"

Game_master::Ability::Binding::Binding(Game_master::Cause& cause, Game_master::Effect effect) : trig(cause), key()
{
    trig.insert({key, effect});
}

Game_master::Ability::Binding::~Binding()
{
    trig.erase(key);
}