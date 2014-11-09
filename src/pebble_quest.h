/******************************************************************************
   Filename: pebble_quest.h

     Author: David C. Drake (http://davidcdrake.com)

Description: Header file for the 3D, first-person, fantasy RPG PebbleQuest,
             developed for the Pebble smartwatch (SDK 2). Copyright 2014, David
             C. Drake. More info online: http://davidcdrake.com/pebblequest
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

#define DEFAULT_MAJOR_STAT_VALUE   3 // For STRENGTH, AGILITY, and INTELLECT.
#define NUM_PLAYER_ANIMATIONS      2 // Steps in the player's attack animation.
#define MIN_REGEN                  1 // Min. health/energy recovery per second.
#define MIN_DAMAGE                 2 // Min. damage per attack/spell/effect.
#define MIN_ENERGY_LOSS_PER_ACTION 2
#define MAX_NPCS_AT_ONE_TIME       2

// NPC types:
#define OOZE          0
#define WORM          1
#define WOLF          3
#define BEAR          4
#define GOBLIN        5
#define ORC           6
#define OGRE          7
#define TROLL         8
#define LIZARD_MAN    9
#define MINOTAUR      10
#define SKELETON      11
#define ZOMBIE        12
#define MUMMY         13
#define WRAITH        14
#define VAMPIRE       15
#define IMP           16
#define DRAGON        17
#define FLOATING_EYE  18
#define ELEMENTAL     19
#define THIEF         20
#define WARRIOR       21
#define MAGE          22
#define NUM_NPC_TYPES 23

// Character stats:
#define STRENGTH            0
#define AGILITY             1
#define INTELLECT           2
#define MAX_HEALTH          3
#define MAX_ENERGY          4
#define PHYSICAL_POWER      5
#define PHYSICAL_DEFENSE    6
#define MAGICAL_POWER       7
#define MAGICAL_DEFENSE     8
#define CURRENT_HEALTH      9
#define CURRENT_ENERGY      10
#define NUM_CHARACTER_STATS 11
#define NUM_MAJOR_STATS     3 // STRENGTH, AGILITY, and INTELLECT.

/******************************************************************************
  Location-related Constants
******************************************************************************/

// Cell types (for loot, an item type value is used):
#define ENTRANCE -1
#define EXIT     -2
#define EMPTY    -3
#define SOLID    -4

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

#define NARRATION_STR_LEN          100
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
#define LEVEL_UP_NARRATION  8
#define NUM_NARRATION_TYPES 9

/******************************************************************************
  Pebble- and Item-related Constants
******************************************************************************/

#define NONE                 -1
#define PEBBLE_OF_FIRE       0
#define PEBBLE_OF_ICE        1
#define PEBBLE_OF_LIGHTNING  2
#define PEBBLE_OF_LIFE       3
#define PEBBLE_OF_DEATH      4
#define PEBBLE_OF_LIGHT      5
#define PEBBLE_OF_DARKNESS   6
#define BOW                  7
#define DAGGER               8
#define STAFF                9
#define SWORD                10
#define MACE                 11
#define AXE                  12
#define FLAIL                13
#define SHIELD               14
#define ROBE                 15
#define HEAVY_ARMOR          16
#define LIGHT_ARMOR          17
#define NUM_ITEM_TYPES       18
#define NUM_PEBBLE_TYPES     7
#define NUM_HEAVY_ITEM_TYPES 11 // Excludes Pebbles.
#define FIRST_HEAVY_ITEM     BOW
#define MAX_HEAVY_ITEMS      4
#define RANDOM_ITEM          (rand() % (NUM_ITEM_TYPES - NUM_PEBBLE_TYPES) + NUM_PEBBLE_TYPES)

// Equip targets (i.e., places where an item may be equipped):
#define BODY              0
#define LEFT_HAND         1
#define RIGHT_HAND        2
#define NUM_EQUIP_TARGETS 3

// Constant status effects (via infused robes/armor/shields):
#define INCREASED_STRENGTH          0
#define INCREASED_INTELLECT         1
#define INCREASED_AGILITY           2
#define INCREASED_HEALTH_REGEN      3
#define BACKLASH_DAMAGE             4
#define INCREASED_ENERGY_REGEN      5
#define SPELL_ABSORPTION            6
#define NUM_CONSTANT_STATUS_EFFECTS 7

// Weapon infusion effects:
#define DECREASE_INTELLECT 0
#define DECREASE_AGILITY   1
#define DECREASE_STRENGTH  2
#define ABSORB_HEALTH      3
#define WOUND              4
#define STUN               5
#define INTIMIDATE         6

// Temporary status effects (via spells and infused weapons):
#define DECREASED_INTELLECT     0
#define DECREASED_STRENGTH      1
#define DECREASED_AGILITY       2
#define STUNNED                 3
#define DAMAGE_OVER_TIME        4
#define INTIMIDATED             5
#define NUM_TEMP_STATUS_EFFECTS 6

/******************************************************************************
  Graphics-related Constants
******************************************************************************/

#define SCREEN_WIDTH             144
#define SCREEN_HEIGHT            168
#define SCREEN_CENTER_POINT_X    (SCREEN_WIDTH / 2)
#define SCREEN_CENTER_POINT_Y    (SCREEN_HEIGHT / 2 - STATUS_BAR_HEIGHT * 0.75)
#define SCREEN_CENTER_POINT      GPoint(SCREEN_CENTER_POINT_X, SCREEN_CENTER_POINT_Y)
#define STATUS_BAR_HEIGHT        16 // Applies to top and bottom status bars.
#define FULL_SCREEN_FRAME        GRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT - STATUS_BAR_HEIGHT)
#define STATUS_BAR_FRAME         GRect(0, GRAPHICS_FRAME_HEIGHT, GRAPHICS_FRAME_WIDTH, STATUS_BAR_HEIGHT)
#define STATUS_BAR_FONT          fonts_get_system_font(FONT_KEY_GOTHIC_14)
#define STATUS_METER_PADDING     4
#define STATUS_METER_WIDTH       (GRAPHICS_FRAME_WIDTH / 2 - COMPASS_RADIUS - 2 * STATUS_METER_PADDING)
#define STATUS_METER_HEIGHT      (STATUS_BAR_HEIGHT - STATUS_METER_PADDING * 2)
#define FIRST_WALL_OFFSET        STATUS_BAR_HEIGHT
#define MIN_WALL_HEIGHT          STATUS_BAR_HEIGHT
#define GRAPHICS_FRAME_WIDTH     SCREEN_WIDTH
#define GRAPHICS_FRAME_HEIGHT    (SCREEN_HEIGHT - 2 * STATUS_BAR_HEIGHT)
#define GRAPHICS_FRAME           GRect(0, 0, GRAPHICS_FRAME_WIDTH, GRAPHICS_FRAME_HEIGHT)
#define MAX_VISIBILITY_DEPTH     6 // Helps determine no. of cells visible in a given line of sight.
#define STRAIGHT_AHEAD           (MAX_VISIBILITY_DEPTH - 1) // Index value for "g_back_wall_coords".
#define TOP_LEFT                 0                          // Index value for "g_back_wall_coords".
#define BOTTOM_RIGHT             1                          // Index value for "g_back_wall_coords".
#define COMPASS_RADIUS           5
#define NO_CORNER_RADIUS         0
#define SMALL_CORNER_RADIUS      3
#define NINETY_DEGREES           (TRIG_MAX_ANGLE / 4)
#define DEFAULT_ROTATION_RATE    (TRIG_MAX_ANGLE / 30) // 12 degrees per rotation event
#define ELLIPSE_RADIUS_RATIO     0.4

/******************************************************************************
  Menu-related Constants
******************************************************************************/

#define MENU_HEADER_STR_LEN   23
#define MENU_TITLE_STR_LEN    25
#define MENU_SUBTITLE_STR_LEN 20

/******************************************************************************
  Button-related Constants
******************************************************************************/

#define MULTI_CLICK_MIN            2
#define MULTI_CLICK_MAX            2   // We only care about double-clicks.
#define MULTI_CLICK_TIMEOUT        0   // milliseconds
#define MIN_ACTION_REPEAT_INTERVAL 200 // milliseconds per action
#define LAST_CLICK_ONLY            true

/******************************************************************************
  Timer-related Constants
******************************************************************************/

#define PLAYER_TIMER_DURATION 20 // milliseconds
#define FLASH_TIMER_DURATION  20 // milliseconds

/******************************************************************************
  General Constants
******************************************************************************/

#define MAX_INT_VALUE  9999
#define MAX_INT_DIGITS 4
#define STORAGE_KEY    841
#define ANIMATED       true
#define NOT_ANIMATED   false

/******************************************************************************
  Structures
******************************************************************************/

typedef struct HeavyItem {
  int16_t type, 
          infused_pebble;
} __attribute__((__packed__)) heavy_item_t;

typedef struct PlayerCharacter {
  GPoint position;
  int16_t direction,
          stats[NUM_CHARACTER_STATS],
          constant_status_effects[NUM_CONSTANT_STATUS_EFFECTS],
          temp_status_effects[NUM_TEMP_STATUS_EFFECTS],
          pebbles[NUM_PEBBLE_TYPES],
          equipped_pebble,
          exp_points,
          level,
          depth;
  heavy_item_t *heavy_items[MAX_HEAVY_ITEMS], // Clothing, armor, and weapons.
               *equipped_heavy_items[NUM_EQUIP_TARGETS];
} __attribute__((__packed__)) player_t;

typedef struct NonPlayerCharacter {
  GPoint position;
  int16_t type,
          stats[NUM_CHARACTER_STATS],
          temp_status_effects[NUM_TEMP_STATUS_EFFECTS],
          item;
} __attribute__((__packed__)) npc_t;

typedef struct Location {
  int16_t map[MAP_WIDTH][MAP_HEIGHT];
  npc_t *npcs[MAX_NPCS_AT_ONE_TIME];
} __attribute__((__packed__)) location_t;

/******************************************************************************
  Global Variables
******************************************************************************/

Window *g_windows[NUM_WINDOWS];
MenuLayer *g_menu_layers[NUM_MENUS];
InverterLayer *g_inverter_layer;
TextLayer *g_narration_text_layer;
AppTimer *g_player_timer,
         *g_flash_timer;
GPoint g_back_wall_coords[MAX_VISIBILITY_DEPTH - 1]
                         [(STRAIGHT_AHEAD * 2) + 1]
                         [2];
int16_t g_current_window,
        g_current_narration,
        g_current_selection,
        g_player_current_animation;
GPath *g_compass_path;
location_t *g_location;
player_t *g_player;

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

void set_player_direction(const int16_t new_direction);
void move_player(const int16_t direction);
void move_npc(npc_t *npc, const int16_t direction);
void determine_npc_behavior(npc_t *npc);
void damage_player(int16_t damage);
void damage_npc(npc_t *npc, int16_t damage);
void adjust_player_current_health(const int16_t amount);
void adjust_player_current_mp(const int16_t amount);
void add_new_npc(const int16_t npc_type, const GPoint position);
GPoint get_npc_spawn_point(void);
GPoint get_floor_center_point(const int16_t depth, const int16_t position);
GPoint get_cell_farther_away(const GPoint reference_point,
                             const int16_t direction,
                             const int16_t distance);
int16_t get_pursuit_direction(const GPoint pursuer, const GPoint pursuee);
int16_t get_direction_to_the_left(const int16_t reference_direction);
int16_t get_direction_to_the_right(const int16_t reference_direction);
int16_t get_opposite_direction(const int16_t direction);
int16_t get_nth_item_type(const int16_t n);
int16_t get_num_pebble_types_owned(void);
int16_t get_num_heavy_items_owned(void);
int16_t get_equip_target(const int16_t item_type);
int16_t get_cell_type(const GPoint cell);
void set_cell_type(GPoint cell, const int16_t type);
npc_t *get_npc_at(const GPoint cell);
bool out_of_bounds(const GPoint cell);
bool occupiable(const GPoint cell);
bool touching(const GPoint cell, const GPoint cell_2);
void show_narration(const int16_t narration);
void show_window(const int16_t window, const bool animated);
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
void draw_player_action(GContext *ctx);
void draw_floor_and_ceiling(GContext *ctx);
void draw_cell_walls(GContext *ctx,
                     const GPoint cell,
                     const int16_t depth,
                     const int16_t position);
void draw_cell_contents(GContext *ctx,
                        const GPoint cell,
                        const int16_t depth,
                        const int16_t position);
void draw_shaded_quad(GContext *ctx,
                      const GPoint upper_left,
                      const GPoint lower_left,
                      const GPoint upper_right,
                      const GPoint lower_right,
                      const GPoint shading_ref);
void fill_quad(GContext *ctx,
               const GPoint upper_left,
               const GPoint lower_left,
               const GPoint upper_right,
               const GPoint lower_right,
               const GColor color);
void draw_status_bar(GContext *ctx);
void draw_status_meter(GContext *ctx,
                       const GPoint origin,
                       const float ratio);
void fill_ellipse(GContext *ctx,
                  const GPoint center,
                  const int16_t h_radius,
                  const int16_t v_radius,
                  const GColor color);
void flash_screen(void);
static void flash_timer_callback(void *data);
static void player_timer_callback(void *data);
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
void strcat_item_name(char *dest_str, const int16_t item_type);
void strcat_magic_type(char *dest_str, const int16_t magic_type);
void strcat_stat_name(char *dest_str, const int16_t stat);
void strcat_stat_value(char *dest_str, const int16_t stat);
void strcat_int(char *dest_str, int16_t integer);
void add_current_selection_to_inventory(void);
void equip_heavy_item(heavy_item_t *const item);
void unequip_item_at(int16_t equip_target);
bool player_is_using_magic_type(int16_t magic_type);
void assign_minor_stats(int16_t *stats, heavy_item_t **equipped_items);
void init_player(void);
void deinit_player(void);
void init_npc(npc_t *npc, const int16_t type, const GPoint position);
void init_heavy_item(heavy_item_t *const item, const int16_t n);
void init_wall_coords(void);
void init_location(void);
void deinit_location(void);
void init_window(const int16_t window_index);
void deinit_window(const int16_t window_index);
void init(void);
void deinit(void);
int main(void);

#endif // PEBBLE_QUEST_H_
