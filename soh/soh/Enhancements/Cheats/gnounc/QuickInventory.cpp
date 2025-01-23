#include "global.h"
#include <libultraship/bridge.h>
#include "soh/Enhancements/enhancementTypes.h"
#include "soh/Enhancements/game-interactor/GameInteractor_Hooks.h"
#include "soh/ShipInit.hpp"
#include "soh/OTRGlobals.h"
#include "soh/frame_interpolation.h"
#include <textures/icon_item_static/icon_item_static.h>
#include "Data.cpp"
#include <spdlog/spdlog.h>


extern "C" {
#include "macros.h"
#include "functions.h"

extern PlayState* gPlayState;
extern SaveContext gSaveContext;
}

#define CVAR_QUICK_INVENTORY_NAME CVAR_CHEAT("QuickInventory")
#define CVAR_QUICK_INVENTORY_DEFAULT 1
#define CVAR_QUICK_INVENTORY_VALUE CVarGetInteger(CVAR_QUICK_INVENTORY_NAME, CVAR_QUICK_INVENTORY_DEFAULT)



void OnQuickInventory(void* arg_input) {

    if (!GameInteractor::IsSaveLoaded(true)) {
        return;
    }

    Player* player = GET_PLAYER(gPlayState);
    Input *input = (Input*)arg_input;



    if (CHECK_BTN_ALL(input->cur.button, BTN_Z)) {
        //Do Stuff Here
    }

}

void RegisterQuickInventory() {
    COND_HOOK(OnInput, CVAR_QUICK_INVENTORY_VALUE, OnQuickInventory);
}

static RegisterShipInitFunc initFunc(RegisterQuickInventory, { CVAR_QUICK_INVENTORY_NAME });


