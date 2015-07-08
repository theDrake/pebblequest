/******************************************************************************
   Filename: pebble_quest.h

     Author: David C. Drake (http://davidcdrake.com)

Description: Header file for PebbleQuest, a first-person 3D fantasy RPG
             developed for the Pebble smartwatch (SDK 3). More info online:
             http://davidcdrake.com/pebblequest
******************************************************************************/

#ifndef PEBBLE_QUEST_H_
#define PEBBLE_QUEST_H_

#include <pebble.h>

/******************************************************************************
  Enumerations
******************************************************************************/

// Windows:
enum {
  MAIN_MENU,
  INVENTORY_MENU,
  LEVEL_UP_MENU,
  LOOT_MENU,
  PEBBLE_OPTIONS_MENU,
  HEAVY_ITEMS_MENU,
  STATS_MENU,
  NARRATION_WINDOW,
  GRAPHICS_WINDOW,
  NUM_WINDOWS
};

// Narration types (ordering here matters for multi-page narrations):
enum {
  INTRO_NARRATION_1,
  INTRO_NARRATION_2,
  INTRO_NARRATION_3,
  INTRO_NARRATION_4,
  ENCUMBRANCE_NARRATION,
  DEATH_NARRATION,
  LEVEL_UP_NARRATION,
  ENDING_NARRATION,
  NUM_NARRATION_TYPES
};

// Item types:
enum {
  NONE = -1,
  PEBBLE_OF_THUNDER,
  PEBBLE_OF_FIRE,
  PEBBLE_OF_ICE,
  PEBBLE_OF_LIFE,
  PEBBLE_OF_LIGHT,
  PEBBLE_OF_SHADOW,
  PEBBLE_OF_DEATH,
  DAGGER,
  STAFF,
  SWORD,
  MACE,
  AXE,
  FLAIL,
  SHIELD,
  ROBE,
  LIGHT_ARMOR,
  HEAVY_ARMOR,
  NUM_ITEM_TYPES
};

// Cell types (for loot, an item type value is used):
enum {
  SOLID = -3,
  EMPTY,
  EXIT
};

// Equip targets (i.e., places where an item may be equipped):
enum {
  BODY,
  LEFT_HAND,
  RIGHT_HAND,
  NUM_EQUIP_TARGETS
};

// NPC types:
enum {
  BLACK_MONSTER_LARGE,
  WHITE_MONSTER_LARGE,
  BLACK_MONSTER_MEDIUM,
  WHITE_MONSTER_MEDIUM,
  BLACK_MONSTER_SMALL,
  WHITE_MONSTER_SMALL,
  DARK_OGRE,
  PALE_OGRE,
  DARK_TROLL,
  PALE_TROLL,
  DARK_GOBLIN,
  PALE_GOBLIN,
  WARRIOR_LARGE,
  WARRIOR_MEDIUM,
  WARRIOR_SMALL,
  MAGE,
  NUM_NPC_TYPES
};

// 8-bit character stats (2-8 correspond to robe/armor/shield Pebble effects):
enum {
  HEALTH = -3,
  ENERGY,
  EXPERIENCE_POINTS,
  LEVEL,
  DEPTH,
  AGILITY,
  STRENGTH,
  INTELLECT,
  HEALTH_REGEN,
  ENERGY_REGEN,
  SPELL_ABSORPTION,
  BACKLASH_DAMAGE,
  PHYSICAL_POWER,
  PHYSICAL_DEFENSE,
  MAGICAL_POWER,
  MAGICAL_DEFENSE,
  FATIGUE_RATE,
  NUM_INT8_STATS
};

// 16-bit character stats:
enum {
  CURRENT_HEALTH,
  CURRENT_ENERGY,
  MAX_HEALTH,
  MAX_ENERGY,
  NUM_INT16_STATS
};

// Temporary status effects (via spells and infused weapons):
enum {
  WEAKNESS,
  DAMAGE_OVER_TIME,
  SLOW,
  PEBBLE_OF_LIFE_STATUS_EFFECT, // Not actually used.
  INTIMIDATION,
  STUN,
  DISINTEGRATION,
  NUM_STATUS_EFFECTS
};

// Directions:
enum {
  NORTH,
  SOUTH,
  EAST,
  WEST,
  NUM_DIRECTIONS
};

/******************************************************************************
  Other Constants
******************************************************************************/

#define NUM_MAJOR_STATS                  3 // AGILITY, STRENGTH, and INTELLECT.
#define FIRST_MAJOR_STAT                 AGILITY
#define NUM_NEGATIVE_STAT_CONSTANTS      3
#define NUM_MENUS                        (STATS_MENU + 1)
#define DEFAULT_MAJOR_STAT_VALUE         1 // AGILITY, STRENGTH, and INTELLECT.
#define DEFAULT_MAX_HEALTH               10
#define DEFAULT_MAX_ENERGY               10
#define MIN_DAMAGE_TO_NPC                1
#define MIN_FATIGUE_RATE                 2
#define DEFAULT_ITEM_BONUS               3
#define MAX_NPCS_AT_ONE_TIME             2
#define MAP_WIDTH                        10
#define MAP_HEIGHT                       MAP_WIDTH
#define RANDOM_POINT_NORTH               GPoint(rand() % MAP_WIDTH, 0)
#define RANDOM_POINT_SOUTH               GPoint(rand() % MAP_WIDTH, MAP_HEIGHT - 1)
#define RANDOM_POINT_EAST                GPoint(MAP_WIDTH - 1, rand() % MAP_HEIGHT)
#define RANDOM_POINT_WEST                GPoint(0, rand() % MAP_HEIGHT)
#define NARRATION_FONT                   fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD)
#define NUM_PEBBLE_TYPES                 (PEBBLE_OF_DEATH + 1)
#define NUM_HEAVY_ITEM_TYPES             (NUM_ITEM_TYPES - NUM_PEBBLE_TYPES)
#define FIRST_HEAVY_ITEM                 DAGGER
#define MAX_HEAVY_ITEMS                  5
#define RANDOM_ITEM                      (rand() % (NUM_ITEM_TYPES - NUM_PEBBLE_TYPES) + NUM_PEBBLE_TYPES)
#define SCREEN_WIDTH                     144
#define SCREEN_HEIGHT                    168
#define SCREEN_CENTER_POINT_X            (SCREEN_WIDTH / 2)
#define SCREEN_CENTER_POINT_Y            (SCREEN_HEIGHT / 2 - STATUS_BAR_HEIGHT * 0.75)
#define SCREEN_CENTER_POINT              GPoint(SCREEN_CENTER_POINT_X, SCREEN_CENTER_POINT_Y)
#define STATUS_BAR_HEIGHT                16 // Applies to top and bottom status bars.

#ifdef PBL_COLOR
#define FULL_SCREEN_FRAME                GRect(0, STATUS_BAR_HEIGHT, SCREEN_WIDTH, SCREEN_HEIGHT - STATUS_BAR_HEIGHT)
#define GRAPHICS_FRAME                   GRect(0, STATUS_BAR_HEIGHT, GRAPHICS_FRAME_WIDTH, GRAPHICS_FRAME_HEIGHT)
#define NARRATION_TEXT_LAYER_FRAME       GRect(2, STATUS_BAR_HEIGHT, SCREEN_WIDTH - 4, SCREEN_HEIGHT)
#define NUM_BACKGROUND_COLOR_SCHEMES     8
#define NUM_BACKGROUND_COLORS_PER_SCHEME 10
#define RANDOM_COLOR                     GColorFromRGB(rand() % 256, rand() % 256, rand() % 256)
#define RANDOM_DARK_COLOR                GColorFromRGB(rand() % 128, rand() % 128, rand() % 128)
#define RANDOM_BRIGHT_COLOR              GColorFromRGB(rand() % 128 + 128, rand() % 128 + 128, rand() % 128 + 128)
#else
#define FULL_SCREEN_FRAME                GRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT - STATUS_BAR_HEIGHT)
#define GRAPHICS_FRAME                   GRect(0, 0, GRAPHICS_FRAME_WIDTH, GRAPHICS_FRAME_HEIGHT)
#define NARRATION_TEXT_LAYER_FRAME       GRect(2, 0, SCREEN_WIDTH - 4, SCREEN_HEIGHT)
#endif

#define STATUS_BAR_FONT                  fonts_get_system_font(FONT_KEY_GOTHIC_14)
#define STATUS_METER_PADDING             4
#define STATUS_METER_WIDTH               (GRAPHICS_FRAME_WIDTH / 2 - COMPASS_RADIUS - 2 * STATUS_METER_PADDING)
#define STATUS_METER_HEIGHT              (STATUS_BAR_HEIGHT - STATUS_METER_PADDING * 2)
#define FIRST_WALL_OFFSET                STATUS_BAR_HEIGHT
#define MIN_WALL_HEIGHT                  STATUS_BAR_HEIGHT
#define GRAPHICS_FRAME_WIDTH             SCREEN_WIDTH
#define GRAPHICS_FRAME_HEIGHT            (SCREEN_HEIGHT - 2 * STATUS_BAR_HEIGHT)
#define MAX_VISIBILITY_DEPTH             6 // Helps determine no. of cells visible in a given line of sight.
#define STRAIGHT_AHEAD                   (MAX_VISIBILITY_DEPTH - 1) // Index value for "g_back_wall_coords".
#define TOP_LEFT                         0                          // Index value for "g_back_wall_coords".
#define BOTTOM_RIGHT                     1                          // Index value for "g_back_wall_coords".
#define COMPASS_RADIUS                   5
#define NO_CORNER_RADIUS                 0
#define SMALL_CORNER_RADIUS              3
#define NINETY_DEGREES                   (TRIG_MAX_ANGLE / 4)
#define DEFAULT_ROTATION_RATE            (TRIG_MAX_ANGLE / 26) // 13.8 degrees per rotation event.
#define ELLIPSE_RADIUS_RATIO             0.4
#define HEAVY_ITEMS_MENU_HEADER_STR_LEN  16
#define ITEM_TITLE_STR_LEN               19
#define ITEM_SUBTITLE_STR_LEN            13
#define STAT_TITLE_STR_LEN               19
#define STATS_MENU_NUM_ROWS              (NUM_INT8_STATS + NUM_NEGATIVE_STAT_CONSTANTS)
#define LEVEL_UP_MENU_NUM_ROWS           NUM_MAJOR_STATS // 3
#define MAIN_MENU_NUM_ROWS               3
#define PEBBLE_OPTIONS_MENU_NUM_ROWS     2
#define LOOT_MENU_NUM_ROWS               1
#define EQUIPPED_STR                     "Equipped"
#define MULTI_CLICK_MIN                  2
#define MULTI_CLICK_MAX                  2   // We only care about double-clicks.
#define MULTI_CLICK_TIMEOUT              0   // milliseconds
#define PLAYER_ACTION_REPEAT_INTERVAL    250 // milliseconds
#define LAST_CLICK_ONLY                  true
#define DEFAULT_TIMER_DURATION           20 // milliseconds
#define DEFAULT_MAX_SMALL_INT_VALUE      100
#define MAX_SMALL_INT_DIGITS             3
#define MAX_LARGE_INT_DIGITS             5
#define MAX_DEPTH                        DEFAULT_MAX_SMALL_INT_VALUE
#define MAX_LEVEL                        DEFAULT_MAX_SMALL_INT_VALUE
#define STORAGE_KEY                      841
#define ANIMATED                         true
#define NOT_ANIMATED                     false

static const GPathInfo COMPASS_PATH_INFO = {
  .num_points = 4,
  .points = (GPoint []) {{-3, -3},
                         {3, -3},
                         {0, 6},
                         {-3, -3}}
};

static const char *const g_narration_strings[] = {
  "Evil wizards stole the Elderstone and sundered it, creating a hundred Pebbles of Power.",
  "You have entered the wizards' vast underground lair to recover the Pebbles and save the realm.",
  "Welcome, hero, to PebbleQuest!\n\nBy David C. Drake:\ndavidcdrake.com/\n            pebblequest",
  "       CONTROLS\nForward: \"Up\"\nBack: \"Down\"\nLeft: \"Up\" x 2\nRight: \"Down\" x 2\nAttack: \"Select\"",
  "You're at your maximum weight capacity. Drop an old item if you're sure you want to keep this new one.",
  "Alas, another hero has perished in the dank, dark depths. A new champion must arise to save humanity!",
  "\n  You have gained\n        a level of\n      experience!",
  "Congratulations, hero of the realm! You've vanquished the evil mages and restored peace and order. Huzzah!",
};

static const char *const g_main_menu_strings[] = {
  "Play",
  "Inventory",
  "Character Stats",
  "Dungeon-crawl, baby!",
  "Equip/infuse items.",
  "Health, Energy...",
};

static const char *const g_pebble_options_menu_strings[] = {
  "Equip",
  "Infuse into Item",
  "Cast ranged spells.",
  "Enchant a weapon, etc.",
};

static const char *const g_stat_names[] = {
  "Health",
  "Energy",
  "XP",
  "Level",
  "Depth",
  "Agility",
  "Strength",
  "Intellect",
  "Health Regen.",
  "Energy Regen.",
  "Spell Absorption",
  "Backlash Dmg.",
  "Phys. Power",
  "Phys. Def.",
  "Mag. Power",
  "Mag. Def.",
  "Fatigue Rate",
};

static const char *const g_item_names[] = {
  "Pebble of Thunder",
  "Pebble of Fire",
  "Pebble of Ice",
  "Pebble of Life",
  "Pebble of Light",
  "Pebble of Shadow",
  "Pebble of Death",
  "Dagger",
  "Staff",
  "Sword",
  "Mace",
  "Axe",
  "Flail",
  "Shield",
  "Robe",
  "L. Armor",
  "H. Armor",
};

static const char *const g_magic_type_names[] = {
  "",
  " of Thunder",
  " of Fire",
  " of Ice",
  " of Life",
  " of Light",
  " of Shadow",
  " of Death",
};

/******************************************************************************
  Structure Definitions
******************************************************************************/

typedef struct HeavyItem {
  int8_t type,
         infused_pebble,
         equip_target;
  bool equipped;
} __attribute__((__packed__)) heavy_item_t;

typedef struct PlayerCharacter {
  GPoint position;
  int8_t direction,
         int8_stats[NUM_INT8_STATS],
         pebbles[NUM_PEBBLE_TYPES],
         equipped_pebble;
  int16_t int16_stats[NUM_INT16_STATS];
  uint16_t exp_points;
  heavy_item_t heavy_items[MAX_HEAVY_ITEMS]; // Clothing, armor, and weapons.
} __attribute__((__packed__)) player_t;

typedef struct NonPlayerCharacter {
  GPoint position;
  int8_t type,
         item,
         health,
         power,
         physical_defense,
         magical_defense;
  uint8_t status_effects[NUM_STATUS_EFFECTS];
} __attribute__((__packed__)) npc_t;

typedef struct Location {
#ifdef PBL_COLOR
  int8_t map[MAP_WIDTH][MAP_HEIGHT],
         floor_color_scheme,
         wall_color_scheme;
#else
  int8_t map[MAP_WIDTH][MAP_HEIGHT];
#endif
  GPoint entrance;
  npc_t npcs[MAX_NPCS_AT_ONE_TIME];
} __attribute__((__packed__)) location_t;

/******************************************************************************
  Global Variables
******************************************************************************/

Window *g_windows[NUM_WINDOWS];
MenuLayer *g_menu_layers[NUM_MENUS];
TextLayer *g_narration_text_layer;
AppTimer *g_flash_timer,
         *g_attack_timer;
GPoint g_back_wall_coords[MAX_VISIBILITY_DEPTH - 1]
                         [(STRAIGHT_AHEAD * 2) + 1]
                         [2];
uint8_t g_current_window,
        g_current_narration,
        g_current_selection,
        g_attack_slash_x1,
        g_attack_slash_x2,
        g_attack_slash_y1,
        g_attack_slash_y2;
bool g_player_is_attacking;
GPath *g_compass_path;
player_t *g_player;
location_t *g_location;

#ifdef PBL_COLOR
GColor g_magic_type_colors[NUM_PEBBLE_TYPES][2],
       g_background_colors[NUM_BACKGROUND_COLOR_SCHEMES]
                          [NUM_BACKGROUND_COLORS_PER_SCHEME];
StatusBarLayer *g_status_bars[NUM_WINDOWS];
#else
InverterLayer *g_inverter_layer;
#endif

/******************************************************************************
  Function Declarations
******************************************************************************/

int8_t set_player_direction(const int8_t new_direction);
bool move_player(const int8_t direction);
void move_npc(npc_t *const npc, const int8_t direction);
int8_t damage_player(int8_t damage);
int8_t damage_npc(npc_t *const npc, int8_t damage);
int8_t cast_spell_on_npc(npc_t *const npc,
                         const int8_t magic_type,
                         const int8_t max_potency);
int8_t adjust_player_current_health(const int8_t amount);
int8_t adjust_player_current_energy(const int8_t amount);
bool add_new_npc(const int8_t npc_type, const GPoint position);
GPoint get_cell_farther_away(const GPoint reference_point,
                             const int8_t direction,
                             const int8_t distance);
int8_t get_pursuit_direction(const GPoint pursuer, const GPoint pursuee);
int8_t get_direction_to_the_left(const int8_t reference_direction);
int8_t get_direction_to_the_right(const int8_t reference_direction);
int8_t get_opposite_direction(const int8_t direction);
int8_t get_nth_item_type(const int8_t n);
int8_t get_num_pebble_types_owned(void);
int8_t get_inventory_row_for_pebble(const int8_t pebble_type);
heavy_item_t *get_heavy_item_equipped_at(const int8_t equip_target);
int8_t get_cell_type(const GPoint cell);
void set_cell_type(GPoint cell, const int8_t type);
npc_t *get_npc_at(const GPoint cell);
char *get_stat_title_str(const int8_t stat_index);
bool occupiable(const GPoint cell);
int8_t show_narration(const int8_t narration);
int8_t show_window(const int8_t window_index, const bool animated);
static void main_menu_draw_header_callback(GContext *ctx,
                                           const Layer *cell_layer,
                                           uint16_t section_index,
                                           void *data);
static void inventory_menu_draw_header_callback(GContext *ctx,
                                                const Layer *cell_layer,
                                                uint16_t section_index,
                                                void *data);
static void level_up_menu_draw_header_callback(GContext *ctx,
                                               const Layer *cell_layer,
                                               uint16_t section_index,
                                               void *data);
static void loot_menu_draw_header_callback(GContext *ctx,
                                           const Layer *cell_layer,
                                           uint16_t section_index,
                                           void *data);
static void pebble_options_menu_draw_header_callback(GContext *ctx,
                                                     const Layer *cell_layer,
                                                     uint16_t section_index,
                                                     void *data);
static void heavy_items_menu_draw_header_callback(GContext *ctx,
                                                  const Layer *cell_layer,
                                                  uint16_t section_index,
                                                  void *data);
static void stats_menu_draw_header_callback(GContext *ctx,
                                            const Layer *cell_layer,
                                            uint16_t section_index,
                                            void *data);
static void main_menu_draw_row_callback(GContext *ctx,
                                        const Layer *cell_layer,
                                        MenuIndex *cell_index,
                                        void *data);
static void inventory_menu_draw_row_callback(GContext *ctx,
                                             const Layer *cell_layer,
                                             MenuIndex *cell_index,
                                             void *data);
static void level_up_menu_draw_row_callback(GContext *ctx,
                                            const Layer *cell_layer,
                                            MenuIndex *cell_index,
                                            void *data);
static void loot_menu_draw_row_callback(GContext *ctx,
                                        const Layer *cell_layer,
                                        MenuIndex *cell_index,
                                        void *data);
static void pebble_options_menu_draw_row_callback(GContext *ctx,
                                                  const Layer *cell_layer,
                                                  MenuIndex *cell_index,
                                                  void *data);
static void heavy_items_menu_draw_row_callback(GContext *ctx,
                                               const Layer *cell_layer,
                                               MenuIndex *cell_index,
                                               void *data);
static void stats_menu_draw_row_callback(GContext *ctx,
                                         const Layer *cell_layer,
                                         MenuIndex *cell_index,
                                         void *data);
void menu_select_callback(MenuLayer *menu_layer,
                          MenuIndex *cell_index,
                          void *data);
static int16_t menu_get_header_height_callback(MenuLayer *menu_layer,
                                               uint16_t section_index,
                                               void *data);
static uint16_t menu_get_num_rows_callback(MenuLayer *menu_layer,
                                           uint16_t section_index,
                                           void *data);
void draw_scene(Layer *layer, GContext *ctx);
void draw_floor_and_ceiling(GContext *ctx);
void draw_cell_walls(GContext *ctx,
                     const GPoint cell,
                     const int8_t depth,
                     const int8_t position);
void draw_cell_contents(GContext *ctx,
                        const GPoint cell,
                        const int8_t depth,
                        const int8_t position);
void draw_shaded_quad(GContext *ctx,
                      const GPoint upper_left,
                      const GPoint lower_left,
                      const GPoint upper_right,
                      const GPoint lower_right,
                      const GPoint shading_ref);
void draw_status_meter(GContext *ctx,
                       GPoint origin,
                       const float ratio);
void fill_ellipse(GContext *ctx,
                  const GPoint center,
                  const int8_t h_radius,
                  const int8_t v_radius,
                  const GColor color);
void flash_screen(void);
static void flash_timer_callback(void *data);
static void attack_timer_callback(void *data);
static void graphics_window_appear(Window *window);
void graphics_up_single_repeating_click(ClickRecognizerRef recognizer,
                                        void *context);
void graphics_up_multi_click(ClickRecognizerRef recognizer, void *context);
void graphics_down_single_repeating_click(ClickRecognizerRef recognizer,
                                          void *context);
void graphics_down_multi_click(ClickRecognizerRef recognizer, void *context);
void graphics_select_single_repeating_click(ClickRecognizerRef recognizer,
                                            void *context);
void graphics_click_config_provider(void *context);
void narration_single_click(ClickRecognizerRef recognizer, void *context);
void narration_click_config_provider(void *context);
void app_focus_handler(const bool in_focus);
void equip_heavy_item(heavy_item_t *const item);
void unequip_heavy_item(heavy_item_t *const heavy_item);
void unequip_item_at(const int8_t equip_target);
void set_player_minor_stats(void);
void init_player(void);
void init_npc(npc_t *const npc, const int8_t type, const GPoint position);
void init_heavy_item(heavy_item_t *const item, const int8_t n);
void init_wall_coords(void);
void init_location(void);
void init_window(const int8_t window_index);
void deinit_window(const int8_t window_index);
void init(void);
void deinit(void);
int main(void);

#endif // PEBBLE_QUEST_H_
