/******************************************************************************
   Filename: pebble_quest.h

     Author: David C. Drake (http://davidcdrake.com)

Description: Header file for PebbleQuest, a first-person 3D fantasy RPG
             developed for the Pebble smartwatch (SDK 2). More info online:
             http://davidcdrake.com/pebblequest
******************************************************************************/

#ifndef PEBBLE_QUEST_H_
#define PEBBLE_QUEST_H_

#include <pebble.h>

/******************************************************************************
  Window/Menu Constants
******************************************************************************/

#define MAIN_MENU           0
#define INVENTORY_MENU      1
#define LEVEL_UP_MENU       2
#define LOOT_MENU           3
#define PEBBLE_OPTIONS_MENU 4
#define HEAVY_ITEMS_MENU    5
#define NARRATION_WINDOW    6
#define GRAPHICS_WINDOW     7
#define NUM_WINDOWS         8
#define NUM_MENUS           6

/******************************************************************************
  Player- and NPC-related Constants
******************************************************************************/

#define DEFAULT_MAJOR_STAT_VALUE   1 // AGILITY, STRENGTH, and INTELLECT.
#define MIN_DAMAGE_TO_NPC          1
#define MIN_ENERGY_LOSS_PER_ACTION 3
#define MAX_NPCS_AT_ONE_TIME       2

// NPC types:
#define BLACK_BEAST_LARGE  0
#define WHITE_BEAST_LARGE  1
#define BLACK_BEAST_MEDIUM 2
#define WHITE_BEAST_MEDIUM 3
#define BLACK_BEAST_SMALL  4
#define WHITE_BEAST_SMALL  5
#define DARK_OGRE          6
#define PALE_OGRE          7
#define DARK_TROLL         8
#define PALE_TROLL         9
#define DARK_GOBLIN        10
#define PALE_GOBLIN        11
#define WARRIOR_LARGE      12
#define WARRIOR_MEDIUM     13
#define WARRIOR_SMALL      14
#define MAGE               15
#define NUM_NPC_TYPES      16

// Character stats (the first seven correspond to Pebble effects):
#define AGILITY             0
#define STRENGTH            1
#define INTELLECT           2
#define HEALTH_REGEN        3
#define BACKLASH_DAMAGE     4
#define ENERGY_REGEN        5
#define SPELL_ABSORPTION    6
#define NUM_CHARACTER_STATS 7
#define NUM_MAJOR_STATS     3 // AGILITY, STRENGTH, and INTELLECT.

/******************************************************************************
  Location-related Constants
******************************************************************************/

// Cell types (for loot, an item type value is used):
#define EXIT  -1
#define EMPTY -2
#define SOLID -3

// Directions:
#define NORTH          0
#define SOUTH          1
#define EAST           2
#define WEST           3
#define NUM_DIRECTIONS 4

// Other:
#define MAP_WIDTH          10
#define MAP_HEIGHT         MAP_WIDTH
#define RANDOM_POINT_NORTH GPoint(rand() % MAP_WIDTH, 0)
#define RANDOM_POINT_SOUTH GPoint(rand() % MAP_WIDTH, MAP_HEIGHT - 1)
#define RANDOM_POINT_EAST  GPoint(MAP_WIDTH - 1, rand() % MAP_HEIGHT)
#define RANDOM_POINT_WEST  GPoint(0, rand() % MAP_HEIGHT)

/******************************************************************************
  Narration-related Constants
******************************************************************************/

#define NARRATION_STR_LEN          103
#define NARRATION_TEXT_LAYER_FRAME GRect(2, 0, SCREEN_WIDTH - 4, SCREEN_HEIGHT)
#define NARRATION_FONT             fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD)

// Narration types (ordering matters for multi-page narrations):
#define INTRO_NARRATION_1   0
#define INTRO_NARRATION_2   1
#define INTRO_NARRATION_3   2
#define INTRO_NARRATION_4   3
#define DEATH_NARRATION     4
#define STATS_NARRATION_1   5
#define STATS_NARRATION_2   6
#define STATS_NARRATION_3   7
#define STATS_NARRATION_4   8
#define STATS_NARRATION_5   9
#define LEVEL_UP_NARRATION  10
#define ENDING_NARRATION    11
#define NUM_NARRATION_TYPES 12

/******************************************************************************
  Pebble- and Item-related Constants
******************************************************************************/

#define NONE                 -1
#define PEBBLE_OF_THUNDER    0
#define PEBBLE_OF_FIRE       1
#define PEBBLE_OF_ICE        2
#define PEBBLE_OF_LIFE       3
#define PEBBLE_OF_DEATH      4
#define PEBBLE_OF_LIGHT      5
#define PEBBLE_OF_SHADOW     6
#define DAGGER               7
#define STAFF                8
#define SWORD                9
#define MACE                 10
#define AXE                  11
#define FLAIL                12
#define SHIELD               13
#define ROBE                 14
#define LIGHT_ARMOR          15
#define HEAVY_ARMOR          16
#define NUM_ITEM_TYPES       17
#define NUM_PEBBLE_TYPES     7
#define NUM_HEAVY_ITEM_TYPES 10 // Excludes Pebbles.
#define FIRST_HEAVY_ITEM     DAGGER
#define MAX_HEAVY_ITEMS      4
#define RANDOM_ITEM          (rand() % (NUM_ITEM_TYPES - NUM_PEBBLE_TYPES) + NUM_PEBBLE_TYPES)

// Equip targets (i.e., places where an item may be equipped):
#define BODY              0
#define LEFT_HAND         1
#define RIGHT_HAND        2
#define NUM_EQUIP_TARGETS 3

// Temporary status effects (via spells and infused weapons):
#define WEAKNESS           0
#define DAMAGE_OVER_TIME   1
#define SLOW               2
//                         3 Pebble of Life doesn't cause a "status effect".
#define DISINTEGRATION     4
#define INTIMIDATION       5
#define STUN               6
#define NUM_STATUS_EFFECTS 7

/******************************************************************************
  Graphics-related Constants
******************************************************************************/

#define SCREEN_WIDTH          144
#define SCREEN_HEIGHT         168
#define SCREEN_CENTER_POINT_X (SCREEN_WIDTH / 2)
#define SCREEN_CENTER_POINT_Y (SCREEN_HEIGHT / 2 - STATUS_BAR_HEIGHT * 0.75)
#define SCREEN_CENTER_POINT   GPoint(SCREEN_CENTER_POINT_X, SCREEN_CENTER_POINT_Y)
#define STATUS_BAR_HEIGHT     16 // Applies to top and bottom status bars.
#define FULL_SCREEN_FRAME     GRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT - STATUS_BAR_HEIGHT)
#define STATUS_BAR_FONT       fonts_get_system_font(FONT_KEY_GOTHIC_14)
#define STATUS_METER_PADDING  4
#define STATUS_METER_WIDTH    (GRAPHICS_FRAME_WIDTH / 2 - COMPASS_RADIUS - 2 * STATUS_METER_PADDING)
#define STATUS_METER_HEIGHT   (STATUS_BAR_HEIGHT - STATUS_METER_PADDING * 2)
#define FIRST_WALL_OFFSET     STATUS_BAR_HEIGHT
#define MIN_WALL_HEIGHT       STATUS_BAR_HEIGHT
#define GRAPHICS_FRAME_WIDTH  SCREEN_WIDTH
#define GRAPHICS_FRAME_HEIGHT (SCREEN_HEIGHT - 2 * STATUS_BAR_HEIGHT)
#define GRAPHICS_FRAME        GRect(0, 0, GRAPHICS_FRAME_WIDTH, GRAPHICS_FRAME_HEIGHT)
#define MAX_VISIBILITY_DEPTH  6 // Helps determine no. of cells visible in a given line of sight.
#define STRAIGHT_AHEAD        (MAX_VISIBILITY_DEPTH - 1) // Index value for "g_back_wall_coords".
#define TOP_LEFT              0                          // Index value for "g_back_wall_coords".
#define BOTTOM_RIGHT          1                          // Index value for "g_back_wall_coords".
#define COMPASS_RADIUS        5
#define NO_CORNER_RADIUS      0
#define SMALL_CORNER_RADIUS   3
#define NINETY_DEGREES        (TRIG_MAX_ANGLE / 4)
#define DEFAULT_ROTATION_RATE (TRIG_MAX_ANGLE / 30) // 12 degrees per rotation event.
#define ELLIPSE_RADIUS_RATIO  0.4

/******************************************************************************
  Menu-related Constants
******************************************************************************/

#define HEAVY_ITEMS_MENU_HEADER_STR_LEN 16
#define MENU_TITLE_STR_LEN              25
#define MENU_SUBTITLE_STR_LEN           20

/******************************************************************************
  Button-related Constants
******************************************************************************/

#define MULTI_CLICK_MIN               2
#define MULTI_CLICK_MAX               2   // We only care about double-clicks.
#define MULTI_CLICK_TIMEOUT           0   // milliseconds
#define PLAYER_ACTION_REPEAT_INTERVAL 250 // milliseconds
#define LAST_CLICK_ONLY               true

/******************************************************************************
  General Constants
******************************************************************************/

#define DEFAULT_TIMER_DURATION      20 // milliseconds
#define DEFAULT_MAX_SMALL_INT_VALUE 100
#define MAX_SMALL_INT_DIGITS        3
#define MAX_DEPTH                   DEFAULT_MAX_SMALL_INT_VALUE
#define MAX_LEVEL                   DEFAULT_MAX_SMALL_INT_VALUE
#define STORAGE_KEY                 841
#define ANIMATED                    true
#define NOT_ANIMATED                false

/******************************************************************************
  Structures
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
         stats[NUM_CHARACTER_STATS],
         physical_power,
         physical_defense,
         magical_power,
         magical_defense,
         pebbles[NUM_PEBBLE_TYPES],
         equipped_pebble,
         energy_loss_per_action,
         level,
         depth;
  int16_t health,
          energy,
          max_health,
          max_energy;
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
  int8_t map[MAP_WIDTH][MAP_HEIGHT];
  GPoint entrance;
  npc_t npcs[MAX_NPCS_AT_ONE_TIME];
} __attribute__((__packed__)) location_t;

/******************************************************************************
  Global Variables
******************************************************************************/

Window *g_windows[NUM_WINDOWS];
MenuLayer *g_menu_layers[NUM_MENUS];
InverterLayer *g_inverter_layer;
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

static const GPathInfo COMPASS_PATH_INFO = {
  .num_points = 4,
  .points = (GPoint []) {{-3, -3},
                         {3, -3},
                         {0, 6},
                         {-3, -3}}
};

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
bool occupiable(const GPoint cell);
int8_t show_narration(const int8_t narration);
int8_t show_window(const int8_t window, const bool animated);
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
void draw_status_bar(GContext *ctx);
void draw_status_meter(GContext *ctx,
                       const GPoint origin,
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
void strcat_item_name(char *const dest_str, const int8_t item_type);
void strcat_magic_type(char *const dest_str, const int8_t magic_type);
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
