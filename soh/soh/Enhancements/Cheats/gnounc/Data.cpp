#include <sstream>
#include <fstream>
#include <iostream>
#include <vector>
#include <string>

#include "global.h"
#include "soh/OTRGlobals.h"
#include "soh/Enhancements/randomizer/randomizerTypes.h"
#include "libultraship/libultra/types.h"
#include "StringHelper.h"
#include <nlohmann/json.hpp>
#include "Context.h"
#include <spdlog/spdlog.h>


extern "C" {
#include "macros.h"
#include "functions.h"
extern PlayState* gPlayState;
extern SaveContext gSaveContext;
}

//SPDLOG_INFO("blah blah {}", 0);

using json = nlohmann::json;


static std::vector<u16> bottle_ids = {SLOT_BOTTLE_1, SLOT_BOTTLE_2, SLOT_BOTTLE_3, SLOT_BOTTLE_4};


//also theres a bug i havent found yet, adult link is showing a deku stick when his inventory says it should be a deku nut
//actually *many* of the icons for adult link are wrong.
//and stick/nut are transposed for child. so i'm probably not indexing things correctly in the draw loop?

struct QuickCategory {
    int cursor;
    std::string name;
    u16 header;
    std::vector<int> slots;

    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(QuickCategory, cursor, name, header, slots)
};


struct QuickInventory {
    int cursor;
    std::vector<std::vector<QuickCategory>> inventories;

    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(QuickCategory, cursor, name, header, slots)
};


static void saveDefaultInventoryJson();

static std::map<std::string, int> slot_enums = {
        {"SLOT_STICK", SLOT_STICK},
        {"SLOT_NUT", SLOT_NUT},
        {"SLOT_BOMB", SLOT_BOMB},
        {"SLOT_BOW", SLOT_BOW},
        {"SLOT_ARROW_FIRE", SLOT_ARROW_FIRE},
        {"SLOT_DINS_FIRE", SLOT_DINS_FIRE},
        {"SLOT_SLINGSHOT", SLOT_SLINGSHOT},
        {"SLOT_OCARINA", SLOT_OCARINA},
        {"SLOT_BOMBCHU", SLOT_BOMBCHU},
        {"SLOT_HOOKSHOT", SLOT_HOOKSHOT},
        {"SLOT_ARROW_ICE", SLOT_ARROW_ICE},
        {"SLOT_FARORES_WIND", SLOT_FARORES_WIND},
        {"SLOT_BOOMERANG", SLOT_BOOMERANG},
        {"SLOT_LENS", SLOT_LENS},
        {"SLOT_BEAN", SLOT_BEAN},
        {"SLOT_HAMMER", SLOT_HAMMER},
        {"SLOT_ARROW_LIGHT", SLOT_ARROW_LIGHT},
        {"SLOT_NAYRUS_LOVE", SLOT_NAYRUS_LOVE},
        {"SLOT_BOTTLE_1", SLOT_BOTTLE_1},
        {"SLOT_BOTTLE_2", SLOT_BOTTLE_2},
        {"SLOT_BOTTLE_3", SLOT_BOTTLE_3},
        {"SLOT_BOTTLE_4", SLOT_BOTTLE_4},
        {"SLOT_TRADE_ADULT", SLOT_TRADE_ADULT},
        {"SLOT_TRADE_CHILD", SLOT_TRADE_CHILD},
        {"SLOT_TUNIC_KOKIRI", SLOT_TUNIC_KOKIRI},
        {"SLOT_TUNIC_GORON", SLOT_TUNIC_GORON},
        {"SLOT_TUNIC_ZORA", SLOT_TUNIC_ZORA},
        {"SLOT_BOOTS_KOKIRI", SLOT_BOOTS_KOKIRI},
        {"SLOT_BOOTS_IRON", SLOT_BOOTS_IRON},
        {"SLOT_BOOTS_HOVER", SLOT_BOOTS_HOVER},
        {"SLOT_NONE", SLOT_NONE}
};

static std::map<std::string, std::pair<int, std::string>> item_enums = {
        {"ITEM_STICK", {ITEM_STICK, "Deku Stick"}},
        {"ITEM_NUT", {ITEM_NUT, "Deku Nut"}},
        {"ITEM_BOMB", {ITEM_BOMB, "Bomb"}},
        {"ITEM_BOW", {ITEM_BOW, "Bow"}},
        {"ITEM_ARROW_FIRE", {ITEM_ARROW_FIRE, "Fire Arrows"}},
        {"ITEM_DINS_FIRE", {ITEM_DINS_FIRE, "Dins Fire"}},
        {"ITEM_SLINGSHOT", {ITEM_SLINGSHOT, "Slingshot"}},
        {"ITEM_OCARINA_FAIRY", {ITEM_OCARINA_FAIRY, "Fairy Ocarina"}},
        {"ITEM_OCARINA_TIME", {ITEM_OCARINA_TIME, "Ocarina of Time"}},
        {"ITEM_BOMBCHU", {ITEM_BOMBCHU, "Bombchu"}},
        {"ITEM_HOOKSHOT", {ITEM_HOOKSHOT , "Hookshot"}},
        {"ITEM_LONGSHOT", {ITEM_HOOKSHOT, "Longshot"}},
        {"ITEM_ARROW_ICE", {ITEM_ARROW_ICE, "Ice Arrows"}},
        {"ITEM_FARORES_WIND", {ITEM_FARORES_WIND , "Farores Wind"}},
        {"ITEM_BOOMERANG", {ITEM_BOOMERANG, "Boomerang"}},
        {"ITEM_LENS", {ITEM_LENS, "Lens of Truth"}},
        {"ITEM_BEAN", {ITEM_BEAN, "Magic Beans"}},
        {"ITEM_HAMMER", {ITEM_HAMMER, "Megaton Hammer"}},
        {"ITEM_ARROW_LIGHT", {ITEM_ARROW_LIGHT, "Light Arrows"}},
        {"ITEM_NAYRUS_LOVE", {ITEM_NAYRUS_LOVE, "Nayrus Love"}},
        {"ITEM_BOTTLE", {ITEM_BOTTLE, "Magic Bottle"}},
        {"ITEM_POTION_RED", {ITEM_POTION_RED, "Red Potion"}},
        {"ITEM_POTION_GREEN", {ITEM_POTION_GREEN, "Green Potion"}},
        {"ITEM_POTION_BLUE", {ITEM_POTION_BLUE, "Blue Potion"}},
        {"ITEM_FAIRY", {ITEM_FAIRY , "Fairy in a Bottle"}},
        {"ITEM_FISH", {ITEM_FISH, "Fish in a Bottle"}},
        {"ITEM_MILK_BOTTLE", {ITEM_MILK_BOTTLE, "Lon-Lon Milk"}},
        {"ITEM_LETTER_RUTO", {ITEM_LETTER_RUTO, "Letter from Ruto"}},
        {"ITEM_BLUE_FIRE", {ITEM_BLUE_FIRE, "Blue Fire"}},
        {"ITEM_BUG", {ITEM_BUG , "Bug in a Bottle"}},
        {"ITEM_BIG_POE", {ITEM_BIG_POE, "Big Poe in a Bottle"}},
        {"ITEM_MILK_HALF", {ITEM_MILK_HALF, "Lon-Lon Milk 1/2"}},
        {"ITEM_POE", {ITEM_POE, "Poe in a Bottle"}},
        {"ITEM_WEIRD_EGG", {ITEM_WEIRD_EGG, "Weird Egg"}},
        {"ITEM_CHICKEN", {ITEM_CHICKEN, "Weird Chicken"}},
        {"ITEM_LETTER_ZELDA", {ITEM_LETTER_ZELDA , "Letter from Zelda"}},
        {"ITEM_MASK_KEATON", {ITEM_MASK_KEATON, "Keaton Mask"}},
        {"ITEM_MASK_SKULL", {ITEM_MASK_SKULL, "Skull Mask"}},
        {"ITEM_MASK_SPOOKY", {ITEM_MASK_SPOOKY, "Spooky Mask"}},
        {"ITEM_MASK_BUNNY", {ITEM_MASK_BUNNY, "Bunny Mask"}},
        {"ITEM_MASK_GORON", {ITEM_MASK_GORON, "Goron Mask"}},
        {"ITEM_MASK_ZORA", {ITEM_MASK_ZORA, "Zora Mask"}},
        {"ITEM_MASK_GERUDO", {ITEM_MASK_GERUDO, "Gerudo Mask"}},
        {"ITEM_MASK_TRUTH", {ITEM_MASK_TRUTH, "Truth Mask"}},
        {"ITEM_SOLD_OUT", {ITEM_SOLD_OUT, "Sold Out!"}},
        {"ITEM_POCKET_EGG", {ITEM_POCKET_EGG, "Pocket Egg"}},
        {"ITEM_POCKET_CUCCO", {ITEM_POCKET_CUCCO, "Pocket Cucco"}},
        {"ITEM_COJIRO", {ITEM_COJIRO, "Cojiro"}},
        {"ITEM_ODD_MUSHROOM", {ITEM_ODD_MUSHROOM, "Odd Mushroom"}},
        {"ITEM_ODD_POTION", {ITEM_ODD_POTION, "Odd Potion"}},
        {"ITEM_SAW", {ITEM_SAW, "Stonecutters Saw"}},
        {"ITEM_SWORD_BROKEN", {ITEM_SWORD_BROKEN, "Broken Sword"}},
        {"ITEM_PRESCRIPTION", {ITEM_PRESCRIPTION, "Presciption"}},
        {"ITEM_FROG", {ITEM_FROG, "Frog"}},
        {"ITEM_EYEDROPS", {ITEM_EYEDROPS, "Eyedrops"}},
        {"ITEM_CLAIM_CHECK", {ITEM_CLAIM_CHECK, "Claim Check"}},
        {"ITEM_BOW_ARROW_FIRE", {ITEM_CLAIM_CHECK, "Bow and Fire Arrows"}},
        {"ITEM_BOW_ARROW_ICE", {ITEM_BOW_ARROW_ICE, "Bow and Ice Arrows"}},
        {"ITEM_BOW_ARROW_LIGHT", {ITEM_BOW_ARROW_LIGHT, "Bow and Light Arrows"}},
        {"ITEM_SWORD_KOKIRI", {ITEM_SWORD_KOKIRI, "Kokiri Sword"}},
        {"ITEM_SWORD_MASTER", {ITEM_SWORD_MASTER, "The Master Sword"}},
        {"ITEM_SWORD_BGS", {ITEM_SWORD_BGS, "The Big Goron Sword"}},
        {"ITEM_SHIELD_DEKU", {ITEM_SHIELD_DEKU, "Deku Shield"}},
        {"ITEM_SHIELD_HYLIAN", {ITEM_SHIELD_HYLIAN, "Hylian Shield"}},
        {"ITEM_SHIELD_MIRROR", {ITEM_SHIELD_MIRROR, "Mirror Shield"}},
        {"ITEM_TUNIC_KOKIRI", {ITEM_TUNIC_KOKIRI, "Kokiri Tunic"}},
        {"ITEM_TUNIC_GORON", {ITEM_TUNIC_GORON, "Goron Tunic"}},
        {"ITEM_TUNIC_ZORA", {ITEM_TUNIC_ZORA, "Zora Tunic"}},
        {"ITEM_BOOTS_KOKIRI", {ITEM_BOOTS_KOKIRI, "Kokiri Boots"}},
        {"ITEM_BOOTS_IRON", {ITEM_BOOTS_IRON, "Iron Boots"}},
        {"ITEM_BOOTS_HOVER", {ITEM_BOOTS_HOVER, "Hover Boots"}},
        {"ITEM_BULLET_BAG_30", {ITEM_BULLET_BAG_30, "Bullet Bag (30)"}},
        {"ITEM_BULLET_BAG_40", {ITEM_BULLET_BAG_40, "Bullet Bag (40)"}},
        {"ITEM_BULLET_BAG_50", {ITEM_BULLET_BAG_50 , "Bullet Bag (50)"}},
        {"ITEM_QUIVER_30", {ITEM_QUIVER_30, "Quiver (30)"}},
        {"ITEM_QUIVER_40", {ITEM_QUIVER_40, "Quiver (40)"}},
        {"ITEM_QUIVER_50", {ITEM_QUIVER_50 , "Quiver (50)"}},
        {"ITEM_BOMB_BAG_20", {ITEM_BOMB_BAG_20, "Bomb Bag (20)"}},
        {"ITEM_BOMB_BAG_30", {ITEM_BOMB_BAG_30, "Bomb Bag (30)"}},
        {"ITEM_BOMB_BAG_40", {ITEM_BOMB_BAG_40, "Bomb Bag (40)"}},
        {"ITEM_BRACELET", {ITEM_BRACELET, "Strength Bracelet"}},
        {"ITEM_GAUNTLETS_SILVER", {ITEM_GAUNTLETS_SILVER, "Silver Gauntlets"}},
        {"ITEM_GAUNTLETS_GOLD", {ITEM_GAUNTLETS_GOLD, "Golden Gauntlets"}},
        {"ITEM_SCALE_SILVER", {ITEM_SCALE_SILVER, "Silver Scale"}},
        {"ITEM_SCALE_GOLDEN", {ITEM_SCALE_GOLDEN, "Golden Scale"}},
        {"ITEM_SWORD_KNIFE", {ITEM_SWORD_KNIFE, "Big Goron Knife"}},
        {"ITEM_WALLET_ADULT", {ITEM_WALLET_ADULT, "Adult Wallet"}},
        {"ITEM_WALLET_GIANT", {ITEM_WALLET_GIANT, "Giant Wallet"}},
        {"ITEM_SEEDS", {ITEM_SEEDS, "Deku Seeds"}},
        {"ITEM_FISHING_POLE", {ITEM_FISHING_POLE, "Fishing Pole"}},
        {"ITEM_SONG_MINUET", {ITEM_SONG_MINUET, "Minuet of the Forest"}},
        {"ITEM_SONG_BOLERO", {ITEM_SONG_BOLERO, "Bolero of Fire"}},
        {"ITEM_SONG_SERENADE", {ITEM_SONG_SERENADE, "Serenade of the Seasons"}},
        {"ITEM_SONG_REQUIEM", {ITEM_SONG_REQUIEM, "Reqium of Time"}},
        {"ITEM_SONG_NOCTURNE", {ITEM_SONG_NOCTURNE, "Nocturne of Shadows"}},
        {"ITEM_SONG_PRELUDE", {ITEM_SONG_PRELUDE, "Prelude of Light"}},
        {"ITEM_SONG_LULLABY", {ITEM_SONG_LULLABY, "Zeldas Lullabye"}},
        {"ITEM_SONG_EPONA", {ITEM_SONG_EPONA , "Eponas Song"}},
        {"ITEM_SONG_SARIA", {ITEM_SONG_SARIA, "Sarias Song"}},
        {"ITEM_SONG_SUN", {ITEM_SONG_SUN, "Song of the Sun"}},
        {"ITEM_SONG_TIME", {ITEM_SONG_TIME, "Song of Time"}},
        {"ITEM_SONG_STORMS", {ITEM_SONG_STORMS, "Song of Storms"}}
};

static s8 sItemActions[] = {
        PLAYER_IA_DEKU_STICK,          // ITEM_DEKU_STICK
        PLAYER_IA_DEKU_NUT,            // ITEM_DEKU_NUT
        PLAYER_IA_BOMB,                // ITEM_BOMB
        PLAYER_IA_BOW,                 // ITEM_BOW
        PLAYER_IA_BOW_FIRE,            // ITEM_ARROW_FIRE
        PLAYER_IA_DINS_FIRE,           // ITEM_DINS_FIRE
        PLAYER_IA_SLINGSHOT,           // ITEM_SLINGSHOT
        PLAYER_IA_OCARINA_FAIRY,       // ITEM_OCARINA_FAIRY
        PLAYER_IA_OCARINA_OF_TIME,     // ITEM_OCARINA_OF_TIME
        PLAYER_IA_BOMBCHU,             // ITEM_BOMBCHU
        PLAYER_IA_HOOKSHOT,            // ITEM_HOOKSHOT
        PLAYER_IA_LONGSHOT,            // ITEM_LONGSHOT
        PLAYER_IA_BOW_ICE,             // ITEM_ARROW_ICE
        PLAYER_IA_FARORES_WIND,        // ITEM_FARORES_WIND
        PLAYER_IA_BOOMERANG,           // ITEM_BOOMERANG
        PLAYER_IA_LENS_OF_TRUTH,       // ITEM_LENS_OF_TRUTH
        PLAYER_IA_MAGIC_BEAN,          // ITEM_MAGIC_BEAN
        PLAYER_IA_HAMMER,              // ITEM_HAMMER
        PLAYER_IA_BOW_LIGHT,           // ITEM_ARROW_LIGHT
        PLAYER_IA_NAYRUS_LOVE,         // ITEM_NAYRUS_LOVE
        PLAYER_IA_BOTTLE,              // ITEM_BOTTLE_EMPTY
        PLAYER_IA_BOTTLE_POTION_RED,   // ITEM_BOTTLE_POTION_RED
        PLAYER_IA_BOTTLE_POTION_GREEN, // ITEM_BOTTLE_POTION_GREEN
        PLAYER_IA_BOTTLE_POTION_BLUE,  // ITEM_BOTTLE_POTION_BLUE
        PLAYER_IA_BOTTLE_FAIRY,        // ITEM_BOTTLE_FAIRY
        PLAYER_IA_BOTTLE_FISH,         // ITEM_BOTTLE_FISH
        PLAYER_IA_BOTTLE_MILK_FULL,    // ITEM_BOTTLE_MILK_FULL
        PLAYER_IA_BOTTLE_RUTOS_LETTER, // ITEM_BOTTLE_RUTOS_LETTER
        PLAYER_IA_BOTTLE_FIRE,         // ITEM_BOTTLE_BLUE_FIRE
        PLAYER_IA_BOTTLE_BUG,          // ITEM_BOTTLE_BUG
        PLAYER_IA_BOTTLE_BIG_POE,      // ITEM_BOTTLE_BIG_POE
        PLAYER_IA_BOTTLE_MILK_HALF,    // ITEM_BOTTLE_MILK_HALF
        PLAYER_IA_BOTTLE_POE,          // ITEM_BOTTLE_POE
        PLAYER_IA_WEIRD_EGG,           // ITEM_WEIRD_EGG
        PLAYER_IA_CHICKEN,             // ITEM_CHICKEN
        PLAYER_IA_ZELDAS_LETTER,       // ITEM_ZELDAS_LETTER
        PLAYER_IA_MASK_KEATON,         // ITEM_MASK_KEATON
        PLAYER_IA_MASK_SKULL,          // ITEM_MASK_SKULL
        PLAYER_IA_MASK_SPOOKY,         // ITEM_MASK_SPOOKY
        PLAYER_IA_MASK_BUNNY_HOOD,     // ITEM_MASK_BUNNY_HOOD
        PLAYER_IA_MASK_GORON,          // ITEM_MASK_GORON
        PLAYER_IA_MASK_ZORA,           // ITEM_MASK_ZORA
        PLAYER_IA_MASK_GERUDO,         // ITEM_MASK_GERUDO
        PLAYER_IA_MASK_TRUTH,          // ITEM_MASK_TRUTH
        PLAYER_IA_SWORD_MASTER,        // ITEM_SOLD_OUT
        PLAYER_IA_POCKET_EGG,          // ITEM_POCKET_EGG
        PLAYER_IA_POCKET_CUCCO,        // ITEM_POCKET_CUCCO
        PLAYER_IA_COJIRO,              // ITEM_COJIRO
        PLAYER_IA_ODD_MUSHROOM,        // ITEM_ODD_MUSHROOM
        PLAYER_IA_ODD_POTION,          // ITEM_ODD_POTION
        PLAYER_IA_POACHERS_SAW,        // ITEM_POACHERS_SAW
        PLAYER_IA_BROKEN_GORONS_SWORD, // ITEM_BROKEN_GORONS_SWORD
        PLAYER_IA_PRESCRIPTION,        // ITEM_PRESCRIPTION
        PLAYER_IA_FROG,                // ITEM_EYEBALL_FROG
        PLAYER_IA_EYEDROPS,            // ITEM_EYE_DROPS
        PLAYER_IA_CLAIM_CHECK,         // ITEM_CLAIM_CHECK
        PLAYER_IA_BOW_FIRE,            // ITEM_BOW_FIRE
        PLAYER_IA_BOW_ICE,             // ITEM_BOW_ICE
        PLAYER_IA_BOW_LIGHT,           // ITEM_BOW_LIGHT
        PLAYER_IA_SWORD_KOKIRI,        // ITEM_SWORD_KOKIRI
        PLAYER_IA_SWORD_MASTER,        // ITEM_SWORD_MASTER
        PLAYER_IA_SWORD_BIGGORON,      // ITEM_SWORD_BIGGORON
};


//TODO: move slot_nums back to item_id's in inventory drawing code?
//TODO: save default json inventory if it does not exist

static QuickInventory ParseInventoryJson(nlohmann::json qInventory) {
    QuickInventory ret_inv = {0, {}};

    for (const auto& inv : qInventory["inventories"].items()) {

        std::vector<QuickCategory> tmp_cats = {};
        for (const auto& cat : inv.value()["categories"].items()) {
            QuickCategory tmp_cat;

            tmp_cat.cursor = cat.value()["cursor"].get<int>();
            tmp_cat.name = cat.value()["name"].get<std::string>();
            tmp_cat.header = item_enums[cat.value()["header"].get<std::string>()].second[0];


            //map slot strings to slot ids
            auto view_tmp_slots = cat.value()["slots"].get<std::vector<std::string>>() | std::views::transform([](std::string s_slot) { return slot_enums[s_slot]; });
            for (const auto& it_tmp_cat : view_tmp_slots) {
                tmp_cat.slots.push_back(it_tmp_cat);
            }

            tmp_cats.push_back(tmp_cat);
        }

        ret_inv.inventories.push_back(tmp_cats);
    }

    return ret_inv;
}

static std::string str_json_inventory = "{\"cursor\":0,\"inventories\":[{\"categories\":[{\"cursor\":0,\"name\":\"misc\",\"header\":\"ITEM_NUT\",\"slots\":[\"SLOT_NUT\",\"SLOT_BOMB\",\"SLOT_BOMBCHU\"]},{\"cursor\":0,\"name\":\"ranged\",\"header\":\"ITEM_HOOKSHOT\",\"slots\":[\"SLOT_HOOKSHOT\",\"SLOT_LENS\",\"SLOT_HAMMER\"]},{\"cursor\":0,\"name\":\"boots\",\"header\":\"ITEM_BOOTS_IRON\",\"slots\":[\"SLOT_BOOTS_KOKIRI\",\"SLOT_BOOTS_IRON\",\"SLOT_BOOTS_HOVER\"]}]},{\"categories\":[{\"cursor\":0,\"name\":\"misc\",\"header\":\"SLOT_STICK\",\"slots\":[\"SLOT_STICK\",\"SLOT_NUT\",\"SLOT_BOMB\",\"SLOT_BOMBCHU\"]},{\"cursor\":0,\"name\":\"ranged\",\"header\":\"ITEM_BOOMERANG\",\"slots\":[\"SLOT_BOOMERANG\",\"SLOT_LENS\"]},{\"cursor\":0,\"name\":\"boots\",\"header\":\"ITEM_BOOTS_KOKIRI\",\"slots\":[\"SLOT_BOOTS_KOKIRI\"]}]}]}";

static auto inventory_filepath = Ship::Context::LocateFileAcrossAppDirs("quick_inventory.json", "soh");

static json load_jsonInventory() {
    if (!std::filesystem::exists(inventory_filepath)) {
        saveDefaultInventoryJson();
    }

    inventory_filepath = Ship::Context::LocateFileAcrossAppDirs("quick_inventory.json", "soh");
    if (!std::filesystem::exists(inventory_filepath)) {
        return NULL; //TODO: throw error here
    }

    std::ifstream file;
    file.open(inventory_filepath);

    if (file.fail()) {
        SPDLOG_INFO("gggnounc file failed to open{}", 0);
        return NULL;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();

    std::string str_json = buffer.str();

    file.close();

    return json::parse(str_json);
}

static void saveDefaultInventoryJson() {
    std::string f_path = Ship::Context::GetAppDirectoryPath() + "/quick_inventory.json";
    std::ofstream file(f_path);
    file << json::parse(str_json_inventory).dump(4);
};


static QuickInventory qi_inv = ParseInventoryJson(load_jsonInventory());

