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


void QI_Draw() {

    int spacer = 4;
    int left = 80;
    int top = 40;
    int drawSize = 16;
    int width = 16;
    int height = 16;

    Player* player = GET_PLAYER(gPlayState);
    InterfaceContext* interfaceCtx = &gPlayState->interfaceCtx;

    OPEN_DISPS(gPlayState->state.gfxCtx);

    Gfx_SetupDL_39Overlay(gPlayState->state.gfxCtx);

//set render flags to draw headers outlines

    gDPSetCombineLERP(OVERLAY_DISP++, 0, 0, 0, PRIMITIVE, 0, 0, 0, TEXEL0, 0, 0, 0, PRIMITIVE, 0, 0, 0, TEXEL0);
    gDPSetPrimColor(OVERLAY_DISP++, 0, 0, 255, 255, 255, 128);
    gDPSetEnvColor(OVERLAY_DISP++, 255, 255, 255, 0);
    gDPSetBlendColor(OVERLAY_DISP++, 255, 255, 255, 0);

    int idx = 0;    //draw header outlines
    for(auto &category : qi_inv.inventories[gSaveContext.linkAge]) {

        Sprite* sprite = OTRGlobals::Instance->gRandoContext->GetSeedTexture(category.header);
        int m_width_factor = (1 << 10) * sprite->width / (width + 2);
        int m_height_factor = (1 << 10) * sprite->height / (height + 2);


        gDPLoadTextureBlock(OVERLAY_DISP++, sprite->tex, sprite->im_fmt, G_IM_SIZ_32b, sprite->width, sprite->height, 0, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOLOD);
        gSPWideTextureRectangle(OVERLAY_DISP++, left -2 << 2, top - 2 + (drawSize * idx) << 2, left -2 + drawSize << 2, top -2 + drawSize + (drawSize * idx) << 2, G_TX_RENDERTILE, 0, 0, m_width_factor, m_height_factor);
        idx++;
    }


//set render flags to draw headers
    Gfx_SetupDL_39Overlay(gPlayState->state.gfxCtx);

    gDPSetPrimColor(OVERLAY_DISP++, 0, 0, 255, 255, 255, 255);
    gDPSetEnvColor(OVERLAY_DISP++, 255, 255, 255, 0);
    gDPSetBlendColor(OVERLAY_DISP++, 255, 255, 255, 0);


    idx = 0;    //draw headers
    for(auto &category : qi_inv.inventories[gSaveContext.linkAge]) {

        Sprite* sprite = OTRGlobals::Instance->gRandoContext->GetSeedTexture(category.header);
        int width_factor = (1 << 10) * sprite->width / width;
        int height_factor = (1 << 10) * sprite->height / height;

        gDPLoadTextureBlock(OVERLAY_DISP++, sprite->tex, sprite->im_fmt, G_IM_SIZ_32b, sprite->width, sprite->height, 0, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOLOD);
        gSPWideTextureRectangle(OVERLAY_DISP++, left -1 << 2, top -1 + (drawSize * idx) << 2, left -1 + drawSize << 2, top -1 + drawSize + (drawSize * idx) << 2, G_TX_RENDERTILE, 0, 0, width_factor, height_factor);
        idx++;
    }


//set render flags to draw icons
    gDPPipeSync(OVERLAY_DISP++);
    gDPSetPrimColor(OVERLAY_DISP++, 0, 0, 255, 255, 255, interfaceCtx->bAlpha);
    gDPSetCombineMode(OVERLAY_DISP++, G_CC_MODULATERGBA_PRIM, G_CC_MODULATERGBA_PRIM);

    //filter out items we dont own, then map slots to item numbers.
    auto ownedItems = qi_inv.inventories[gSaveContext.linkAge][qi_inv.cursor].slots | std::views::filter([](u16 f_slot) { return gSaveContext.inventory.items[f_slot] != ITEM_NONE; }) | std::views::transform([](u16 f_slot) { return gSaveContext.inventory.items[f_slot]; });

    idx = 0;        //draw items
    for(u16 item : ownedItems) {

        Sprite *sprite = OTRGlobals::Instance->gRandoContext->GetSeedTexture(item);

        int width_factor = (1 << 10) * sprite->width / width;
        int height_factor = (1 << 10) * sprite->height / height;

        int y_offs = 0;
        if (qi_inv.inventories[gSaveContext.linkAge][qi_inv.cursor].cursor == idx) {
            y_offs = -5;
        }

        gDPLoadTextureBlock(OVERLAY_DISP++, sprite->tex, sprite->im_fmt, G_IM_SIZ_32b, sprite->width, sprite->height, 0, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOLOD);
        gSPWideTextureRectangle(OVERLAY_DISP++, left + spacer + drawSize + (drawSize * idx) << 2, y_offs + top + (drawSize * qi_inv.cursor) << 2, left + spacer + drawSize + drawSize + (drawSize * idx) << 2, y_offs + top + drawSize + (drawSize * qi_inv.cursor) << 2, G_TX_RENDERTILE, 0, 0, width_factor, height_factor);
        idx++;
    }


    CLOSE_DISPS(gPlayState->state.gfxCtx);
}

void OnQuickInventory(void* arg_input) {

    if (!GameInteractor::IsSaveLoaded(true)) {
        return;
    }

    Player* player = GET_PLAYER(gPlayState);
    Input *input = (Input*)arg_input;



    if (CHECK_BTN_ALL(input->cur.button, BTN_Z)) {
        QI_Draw();
    }

}

void RegisterQuickInventory() {
    COND_HOOK(OnInput, CVAR_QUICK_INVENTORY_VALUE, OnQuickInventory);
}

static RegisterShipInitFunc initFunc(RegisterQuickInventory, { CVAR_QUICK_INVENTORY_NAME });


