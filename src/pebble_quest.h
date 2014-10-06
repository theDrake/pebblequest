/******************************************************************************
   Filename: pebble_quest.h

     Author: David C. Drake (http://davidcdrake.com)

Description: Header file for the 3D, first-person, fantasy RPG PebbleQuest,
             developed for the Pebble smartwatch (SDK 2.0). Copyright 2014,
             David C. Drake. More information available online:
             http://davidcdrake.com/pebblequest
******************************************************************************/

#ifndef PEBBLE_QUEST_H_
#define PEBBLE_QUEST_H_

#include <pebble.h>

/******************************************************************************
  Game Mode Constants
******************************************************************************/

#define ACTIVE_MODE          0
#define SCROLL_MODE          1
#define MAIN_MENU_MODE       2
#define INVENTORY_MODE       3
#define PEBBLE_OPTIONS_MODE  4
#define PEBBLE_INFUSION_MODE 5
#define LOOT_MODE            6
#define REPLACE_ITEM_MODE    7
#define SHOW_STATS_MODE      8
#define LEVEL_UP_MODE        9
#define NUM_GAME_MODES       10

/******************************************************************************
  Player- and NPC-related Constants
******************************************************************************/

#define DEFAULT_BASE_STAT_VALUE         1
#define DEFAULT_PHYSICAL_POWER          10
#define DEFAULT_PHYSICAL_DEFENSE        10
#define DEFAULT_MAGICAL_POWER           10
#define DEFAULT_MAGICAL_DEFENSE         10
#define DEFAULT_SPEED                   30
#define DEFAULT_MAX_HEALTH              30
#define DEFAULT_MAX_ENERGY              30
#define DEFAULT_STAT_BOOST              5
#define NUM_PLAYER_ANIMATIONS           2 // No. of steps in the player's attack animation.
#define HEALTH_RECOVERY_RATE            1 // Health per second.
#define ENERGY_RECOVERY_RATE            1 // Energy per second.
#define MIN_DAMAGE                      2
#define MIN_ENERGY_LOSS_PER_ACTION      -2
#define MAX_NPCS_AT_ONE_TIME            2

// NPC types:
#define ARCHMAGE        0
#define MAGE            1
#define THIEF           2
#define WARRIOR         3
#define BAT             4
#define WOLF            5
#define BEAR            6
#define GOBLIN          7
#define ORC             8
#define OGRE            9
#define TROLL           10
#define SLIME           11
#define SKELETON        12
#define ZOMBIE          13
#define WRAITH          14
#define FIRE_ELEMENTAL  15
#define ICE_ELEMENTAL   16
#define STORM_ELEMENTAL 17
#define NUM_NPC_TYPES   18

// Character stats (order here matters for the stats menu):
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

// Status effects:
#define BURNED             0
#define FROZEN             1
#define SHOCKED            2
#define BLIND              3
#define SCARED             4
#define STUNNED            5
#define BLEEDING           6
#define NUM_STATUS_EFFECTS 7

/******************************************************************************
  Quest- and Map-related Constants
******************************************************************************/

// Quest types:
#define FIND_PEBBLE            0  // Find a legendary Pebble of Power!
#define FIND_ITEM              1  // Find a valuable item.
#define RECOVER_ITEM           2  // Find lost or stolen goods.
#define ESCORT                 3  // Lead someone from point A to point B.
#define RESCUE                 4  // Find captives and get them to safety.
#define ASSASSINATE            5  // Kill a specific enemy.
#define EXTERMINATE            6  // Kill all enemies.
#define ESCAPE                 7  // You've been captured. Get out alive!
#define MAIN_QUEST_1           8  // Find your first Pebble.
#define MAIN_QUEST_2           9  // Find your seventh Pebble.
#define MAIN_QUEST_3           10 // Answer the Archmage's summons.
#define NUM_QUEST_TYPES        11
#define NUM_RANDOM_QUEST_TYPES 7

// Location types:
#define DUNGEON            0
#define TUNNEL             1
#define TOWN               2
#define CASTLE             3
#define TOWER              4
#define NUM_LOCATION_TYPES 5

// Cell types (for loot, an item type value is used):
#define QUEST_ITEM  -1
#define CAPTIVE     -2
#define EMPTY       -3
#define SOLID       -4
#define CLOSED_DOOR -5
#define LOCKED_DOOR -6

// Directions:
#define NORTH          0
#define SOUTH          1
#define EAST           2
#define WEST           3
#define NUM_DIRECTIONS 4

// Other:
#define MAP_WIDTH            15
#define MAP_HEIGHT           MAP_WIDTH
#define RANDOM_POINT_NORTH   GPoint(rand() % MAP_WIDTH, 0)
#define RANDOM_POINT_SOUTH   GPoint(rand() % MAP_WIDTH, MAP_HEIGHT - 1)
#define RANDOM_POINT_EAST    GPoint(MAP_WIDTH - 1, rand() % MAP_HEIGHT)
#define RANDOM_POINT_WEST    GPoint(0, rand() % MAP_HEIGHT)

/******************************************************************************
  Scroll-related Constants
******************************************************************************/

#define SCROLL_STR_LEN          240
#define SCROLL_TEXT_LAYER_FRAME GRect(3, 0, SCREEN_WIDTH - 6, SCROLL_STR_LEN * 4)
#define SCROLL_HEIGHT_OFFSET    10 // Ensures descenders (e.g., 'y') are fully visible.
#define SCROLL_FONT             fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD)

// Scroll types (quest scrolls are handled by quest type values):
#define FAILURE_SCROLL   NUM_QUEST_TYPES
#define VICTORY_SCROLL   (NUM_QUEST_TYPES + 1)
#define DEATH_SCROLL     (NUM_QUEST_TYPES + 2)
#define NUM_SCROLL_TYPES (NUM_QUEST_TYPES + 3)

/******************************************************************************
  Pebble- and Item-related Constants
******************************************************************************/

#define NONE                      -1
#define PEBBLE_OF_FIRE            0
#define PEBBLE_OF_ICE             1
#define PEBBLE_OF_LIGHTNING       2
#define PEBBLE_OF_LIFE            3
#define PEBBLE_OF_DEATH           4
#define PEBBLE_OF_LIGHT           5
#define PEBBLE_OF_DARKNESS        6
#define DAGGER                    7
#define STAFF                     8
#define SWORD                     9
#define MACE                      10
#define AXE                       11
#define FLAIL                     12
#define BOW                       13
#define SHIELD                    14
#define ROBE                      15
#define HEAVY_ARMOR               16
#define LIGHT_ARMOR               17
#define NUM_PEBBLE_TYPES          7
#define NUM_HEAVY_ITEM_TYPES      11 // Excludes Pebbles.
#define FIRST_HEAVY_ITEM          DAGGER
#define MAX_HEAVY_ITEMS           5

// Equip targets (i.e., places where an item may be equipped):
#define BODY              0
#define RIGHT_HAND        1
#define LEFT_HAND         2
#define NUM_EQUIP_TARGETS 3

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
#define MIN_VISIBILITY_DEPTH     2 // Min. no. of cells visible in a given line of sight.
#define DEFAULT_VISIBILITY_DEPTH (MAX_VISIBILITY_DEPTH + MIN_VISIBILITY_DEPTH) / 2 + (MAX_VISIBILITY_DEPTH + MIN_VISIBILITY_DEPTH) % 2;
#define STRAIGHT_AHEAD           (MAX_VISIBILITY_DEPTH - 1) // Index value for "g_back_wall_coords".
#define TOP_LEFT                 0                          // Index value for "g_back_wall_coords".
#define BOTTOM_RIGHT             1                          // Index value for "g_back_wall_coords".
#define COMPASS_RADIUS           5
#define NO_CORNER_RADIUS         0
#define SMALL_CORNER_RADIUS      3
#define NINETY_DEGREES           (TRIG_MAX_ANGLE / 4)
#define DEFAULT_ROTATION_RATE    (TRIG_MAX_ANGLE / 30) // 12 degrees per rotation event

/******************************************************************************
  Menu-related Constants
******************************************************************************/

#define MENU_HEADER_STR_LEN   23
#define MENU_TITLE_STR_LEN    25
#define MENU_SUBTITLE_STR_LEN 20
#define MAIN_MENU_NUM_ROWS    3

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

typedef struct NonPlayerCharacter {
  GPoint position;
  int16_t type,
          stats[NUM_CHARACTER_STATS],
          status_effects[NUM_STATUS_EFFECTS];
  struct NonPlayerCharacter *next;
} __attribute__((__packed__)) npc_t;

typedef struct PlayerCharacter {
  GPoint position;
  int16_t direction,
          stats[NUM_CHARACTER_STATS],
          status_effects[NUM_STATUS_EFFECTS],
          pebbles[NUM_PEBBLE_TYPES],
          equipped_pebble,
          num_pebbles_found,
          exp_points,
          level;
  heavy_item_t *heavy_items[MAX_HEAVY_ITEMS], // Clothing, armor, and weapons.
               *equipped_heavy_items[NUM_EQUIP_TARGETS];
} __attribute__((__packed__)) player_t;

typedef struct Quest {
  int16_t type,
          map[MAP_WIDTH][MAP_HEIGHT],
          entrance_direction,
          exit_direction,
          primary_npc_type,
          num_npcs,
          kills;
  GPoint starting_point,
         ending_point;
  npc_t *npcs;
  bool completed;
} __attribute__((__packed__)) quest_t;

/******************************************************************************
  Global Variables
******************************************************************************/

Window *g_menu_window,
       *g_scroll_window,
       *g_graphics_window;
InverterLayer *g_inverter_layer;
ScrollLayer *g_scroll_scroll_layer;
MenuLayer *g_menu_layer;
TextLayer *g_scroll_text_layer;
AppTimer *g_player_timer,
         *g_flash_timer;
GPoint g_back_wall_coords[MAX_VISIBILITY_DEPTH - 1]
                         [(STRAIGHT_AHEAD * 2) + 1]
                         [2];
int16_t g_game_mode,
        g_current_scroll,
        g_current_selection,
        g_player_animation_mode;
GPath *g_compass_path;
quest_t *g_quest;
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

void set_game_mode(const int16_t mode);
void set_player_direction(const int16_t new_direction);
void move_player(const int16_t direction);
void move_npc(npc_t *npc, const int16_t direction);
void determine_npc_behavior(npc_t *npc);
void damage_player(int16_t damage);
void damage_npc(npc_t *npc, const int16_t damage);
void remove_npc(npc_t *npc);
void adjust_player_current_health(const int16_t amount);
void adjust_player_current_mp(const int16_t amount);
void end_quest(void);
void add_new_npc(const int16_t npc_type, const GPoint position);
int16_t get_random_npc_type(void);
GPoint get_npc_spawn_point(void);
GPoint get_floor_center_point(const int16_t depth, const int16_t position);
GPoint get_cell_farther_away(const GPoint reference_point,
                             const int16_t direction,
                             const int16_t distance);
int16_t get_pursuit_direction(const GPoint pursuer, const GPoint pursuee);
int16_t get_direction_to_the_left(const int16_t reference_direction);
int16_t get_direction_to_the_right(const int16_t reference_direction);
int16_t get_opposite_direction(const int16_t direction);
int16_t get_boosted_stat_value(const int16_t stat_index,
                               const int16_t boost_amount);
int16_t get_nth_item_type(const int16_t n);
heavy_item_t *get_pointer_to_nth_item(const int16_t n);
int16_t get_num_pebble_types_owned(void);
int16_t get_num_heavy_items_owned(void);
int16_t get_equip_target(const int16_t item_type);
int16_t get_cell_type(const GPoint cell);
void set_cell_type(GPoint cell, const int16_t type);
npc_t *get_npc_at(const GPoint cell);
bool out_of_bounds(const GPoint cell);
bool occupiable(const GPoint cell);
bool touching(const GPoint cell, const GPoint cell_2);
void show_scroll(const int16_t scroll);
void show_window(Window *window, bool animated);
static void menu_draw_header_callback(GContext* ctx,
                                      const Layer *cell_layer,
                                      uint16_t section_index,
                                      void *data);
static void menu_draw_row_callback(GContext *ctx,
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
void scroll_select_single_click(ClickRecognizerRef recognizer, void *context);
void scroll_click_config_provider(void *context);
void app_focus_handler(const bool in_focus);
void strcat_item_name(char *dest_str, const int16_t item_type);
void strcat_magic_type(char *dest_str, const int16_t magic_type);
void strcat_stat_name(char *dest_str, const int16_t stat);
void strcat_stat_value(char *dest_str, const int16_t stat);
void strcat_int(char *dest_str, int16_t integer);
void assign_minor_stats(int16_t *stats_array);
void add_item_to_inventory(const int16_t item_type);
void equip_heavy_item(heavy_item_t *const item);
void init_player(void);
void deinit_player(void);
void init_npc(npc_t *npc, const int16_t type, const GPoint position);
void init_heavy_item(heavy_item_t *item, const int16_t n);
void init_wall_coords(void);
void init_quest(const int16_t type);
void init_quest_map(void);
void deinit_quest(void);
void init_scroll(void);
void deinit_scroll(void);
void init_graphics(void);
void deinit_graphics(void);
void init_menu(void);
void deinit_menu(void);
void init(void);
void deinit(void);
int main(void);

#endif // PEBBLE_QUEST_H_
