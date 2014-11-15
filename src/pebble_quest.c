/******************************************************************************
   Filename: pebble_quest.c

     Author: David C. Drake (http://davidcdrake.com)

Description: Function definitions for the 3D, first-person, fantasy RPG
             PebbleQuest, developed for the Pebble smartwatch (SDK 2).
             Copyright 2014, David C. Drake. More information available online:
             http://davidcdrake.com/pebblequest
******************************************************************************/

#include "pebble_quest.h"

/******************************************************************************
   Function: set_player_direction

Description: Sets the player's orientation to a given direction and updates the
             compass accordingly.

     Inputs: new_direction - Desired orientation.

    Outputs: None.
******************************************************************************/
void set_player_direction(const int8_t new_direction)
{
  g_player->direction = new_direction;
  switch(new_direction)
  {
    case NORTH:
      gpath_rotate_to(g_compass_path, TRIG_MAX_ANGLE / 2);
      break;
    case SOUTH:
      gpath_rotate_to(g_compass_path, 0);
      break;
    case EAST:
      gpath_rotate_to(g_compass_path, (TRIG_MAX_ANGLE * 3) / 4);
      break;
    default: // case WEST:
      gpath_rotate_to(g_compass_path, TRIG_MAX_ANGLE / 4);
      break;
  }
  layer_mark_dirty(window_get_root_layer(g_windows[GRAPHICS_WINDOW]));
}

/******************************************************************************
   Function: move_player

Description: Attempts to move the player one cell forward (or backward) in a
             given direction. If loot is present in that cell, the player does
             not move but is instead shown the loot menu. Moving onto an exit
             will take the player to a new location.

     Inputs: direction - Desired direction of movement.

    Outputs: None.
******************************************************************************/
void move_player(const int8_t direction)
{
  GPoint destination = get_cell_farther_away(g_player->position, direction, 1);

  if (occupiable(destination))
  {
    // Check for loot:
    if (get_cell_type(destination) >= 0)
    {
      g_current_selection = get_cell_type(destination);
      show_window(LOOT_MENU, NOT_ANIMATED);
      set_cell_type(destination, EMPTY);
    }

    // Check for an exit:
    else if (get_cell_type(destination) == EXIT)
    {
      init_location();
    }

    // Shift the player's position:
    else
    {
      g_player->position = destination;
    }

    layer_mark_dirty(window_get_root_layer(g_windows[GRAPHICS_WINDOW]));
  }
}

/******************************************************************************
   Function: move_npc

Description: Attempts to move a given NPC one cell forward in a given
             direction.

     Inputs: npc       - Pointer to the NPC to be moved.
             direction - Desired direction of movement.

    Outputs: None.
******************************************************************************/
void move_npc(npc_t *const npc, const int8_t direction)
{
  GPoint destination = get_cell_farther_away(npc->position, direction, 1);

  if (occupiable(destination))
  {
    npc->position = destination;
  }
}

/******************************************************************************
   Function: determine_npc_behavior

Description: Determines what a given NPC should do.

     Inputs: npc - Pointer to the NPC of interest.

    Outputs: None.
******************************************************************************/
void determine_npc_behavior(npc_t *const npc)
{
  int8_t damage;

  if (npc->status_effects[STUNNED] == 0)
  {
    if (npc->status_effects[INTIMIDATED])
    {
      move_npc(npc,
            get_opposite_direction(get_pursuit_direction(npc->position,
                                                         g_player->position)));
    }
    if (touching(npc->position, g_player->position))
    {
      damage = npc->power - npc->status_effects[FROZEN];
      if (npc->type == MAGE)
      {
        flash_screen();
        if (g_player->stats[SPELL_ABSORPTION] &&
            rand() % g_player->stats[MAGICAL_POWER] +
              g_player->stats[SPELL_ABSORPTION] > rand() % damage)
        {
          adjust_player_current_energy(damage);
        }
        else
        {
          damage_player(damage - g_player->stats[MAGICAL_DEFENSE]);
        }
      }
      else
      {
        damage_player(damage - g_player->stats[PHYSICAL_DEFENSE]);
        if (g_player->stats[BACKLASH_DAMAGE])
        {
          damage_npc(npc, damage / 2 + g_player->stats[BACKLASH_DAMAGE]);
        }
      }
    }
    else
    {
      move_npc(npc, get_pursuit_direction(npc->position, g_player->position));
    }
  }
}

/******************************************************************************
   Function: player_is_visible_from

Description: Determines whether the player is "visible" from a given position,
             where "visible" means aligned along the x- or y-axis with no wall
             or NPC in the way.

     Inputs: position - Cell coordinates from which to look for the player.

    Outputs: "True" if the player is "visible" from the given position.
******************************************************************************/
/*bool player_is_visible_from(const GPoint position)
{
  int8_t diff_x = cell.x - g_player->position.x,
         diff_y = cell.y - g_player->position.y;
  const int8_t horizontal_direction = diff_x > 0 ? WEST  : EAST,
               vertical_direction   = diff_y > 0 ? NORTH : SOUTH;

  if (diff_x > MAX_VISIBILITY_DEPTH || diff_y > MAX_VISIBILITY_DEPTH)
  {
    return false;
  }
  while (!gpoint_equal(&g_player->position, &cell))
  {
    if ((diff_x == diff_y &&
         get_cell_type(get_cell_farther_away(cell,
                                             horizontal_direction,
                                             1)) >= SOLID &&
         get_cell_type(get_cell_farther_away(cell,
                                             vertical_direction,
                                             1)) >= SOLID) ||
        (diff_x > diff_y &&
         get_cell_type(get_cell_farther_away(cell,
                                             horizontal_direction,
                                             1)) >= SOLID) ||
        (diff_x < diff_y &&
         get_cell_type(get_cell_farther_away(cell,
                                             vertical_direction,
                                             1)) >= SOLID))
    {
      return false;
    }
    shift_position(&cell, get_pursuit_direction(cell, g_player->position));
    diff_x = cell.x - g_player->position.x,
    diff_y = cell.y - g_player->position.y;
  }

  return true;
}*/

/******************************************************************************
   Function: damage_player

Description: Damages the player according to his/her defense vs. a given damage
             value.

     Inputs: damage - Potential amount of damage.

    Outputs: None.
******************************************************************************/
void damage_player(int8_t damage)
{
  if (damage < MIN_DAMAGE)
  {
    damage = MIN_DAMAGE;
  }
  vibes_short_pulse();
  adjust_player_current_health(damage * -1);
}

/******************************************************************************
   Function: damage_npc

Description: Damages a given NPC according to a given damage value. If this
             reduces the NPC's health to zero or below, the NPC's death is
             handled.

     Inputs: npc    - Pointer to the NPC to be damaged.
             damage - Amount of damage.

    Outputs: None.
******************************************************************************/
void damage_npc(npc_t *const npc, int8_t damage)
{
  if (damage < MIN_DAMAGE)
  {
    damage = MIN_DAMAGE;
  }
  npc->health -= damage;

  // Check for health absorption:
  if (npc->status_effects[DRAINED])
  {
    adjust_player_current_health(damage);
  }

  // Check for NPC death:
  if (npc->health <= 0)
  {
    npc->type             = NONE;
    g_player->exp_points += (npc->health + npc->power + npc->physical_defense +
                             npc->magical_defense) / 4;

    // If the NPC had an item, leave it behind as loot:
    if (npc->item > NONE)
    {
      set_cell_type(npc->position, npc->item);
    }
  }
}

/******************************************************************************
   Function: cast_spell_on_npc

Description: Applies the effects of a given spell, with a given potency, to a
             given NPC.

     Inputs: npc         - Pointer to the targeted NPC.
             pebble_type - Integer representing the Pebble used to cast the
                           spell.
             potency     - Amount of magical power being brought to bear.

    Outputs: None.
******************************************************************************/
void cast_spell_on_npc(npc_t *const npc,
                       const int8_t pebble_type,
                       int8_t potency)
{
  if (npc)
  {
    // First, attempt to apply a status effect:
    if (rand() % potency > rand() % npc->magical_defense)
    {
      npc->status_effects[pebble_type] += potency;
    }

    // Next, apply damage:
    potency -= (npc->magical_defense - npc->status_effects[SHOCKED]);
    if (potency < MIN_SPELL_POTENCY)
    {
      potency = MIN_SPELL_POTENCY;
    }
    damage_npc(npc, potency);
  }
}

/******************************************************************************
   Function: adjust_player_current_health

Description: Adjusts the player's current health by a given amount, which may
             be positive or negative. Health may not be increased above the
             player's max. health nor reduced below zero. If reduced to zero,
             the player character's death is handled.

     Inputs: amount - Adjustment amount (which may be positive or negative).

    Outputs: None.
******************************************************************************/
void adjust_player_current_health(const int8_t amount)
{
  g_player->stats[CURRENT_HEALTH] += amount;
  if (g_player->stats[CURRENT_HEALTH] > g_player->stats[MAX_HEALTH])
  {
    g_player->stats[CURRENT_HEALTH] = g_player->stats[MAX_HEALTH];
  }
  else if (g_player->stats[CURRENT_HEALTH] <= 0)
  {
    show_window(MAIN_MENU, NOT_ANIMATED);
    show_narration(DEATH_NARRATION);
  }
}

/******************************************************************************
   Function: adjust_player_current_energy

Description: Adjusts the player's current energy by a given amount, which may
             be positive or negative. Energy may not be increased above the
             player's max. energy value nor reduced below zero.

     Inputs: amount - Adjustment amount (which may be positive or negative).

    Outputs: None.
******************************************************************************/
void adjust_player_current_energy(const int8_t amount)
{
  g_player->stats[CURRENT_ENERGY] += amount;
  if (g_player->stats[CURRENT_ENERGY] > g_player->stats[MAX_ENERGY])
  {
    g_player->stats[CURRENT_ENERGY] = g_player->stats[MAX_ENERGY];
  }
}

/******************************************************************************
   Function: add_new_npc

Description: Initializes an NPC according to a given type and position (unless
             the position isn't occupiable or the the max. number of NPCs has
             already been reached).

     Inputs: npc_type - Desired type for the new NPC.
             position - Desired spawn point for the new NPC.

    Outputs: None.
******************************************************************************/
void add_new_npc(const int8_t npc_type, const GPoint position)
{
  int8_t i;

  if (occupiable(position))
  {
    for (i = 0; i < MAX_NPCS_AT_ONE_TIME; ++i)
    {
      if (g_location->npcs[i].type == NONE)
      {
        init_npc(&g_location->npcs[i], npc_type, position);

        return;
      }
    }
  }
}

/******************************************************************************
   Function: get_npc_spawn_point

Description: Returns a suitable NPC spawn point, outside the player's sphere of
             visibility. If the algorithm fails to find one, (-1, -1) is
             returned instead.

     Inputs: None.

    Outputs: The coordinates of a suitable NPC spawn point, or (-1, -1) if the
             algorithm fails to find one.
******************************************************************************/
GPoint get_npc_spawn_point(void)
{
  int8_t i;
  GPoint spawn_point;

  for (i = 0; i < NUM_DIRECTIONS; ++i)
  {
    spawn_point = get_cell_farther_away(g_player->position,
                                        i,
                                        MAX_VISIBILITY_DEPTH);
    if (occupiable(spawn_point))
    {
      return spawn_point;
    }
  }

  return GPoint(-1, -1);
}

/******************************************************************************
   Function: get_floor_center_point

Description: Returns the central point, with respect to the graphics layer, of
             the floor of the cell at a given visual depth and position.

     Inputs: depth    - Front-back visual depth in "g_back_wall_coords".
             position - Left-right visual position in "g_back_wall_coords".

    Outputs: GPoint coordinates of the floor's central point within the
             designated cell.
******************************************************************************/
GPoint get_floor_center_point(const int8_t depth, const int8_t position)
{
  int16_t x_midpoint1, x_midpoint2, x, y;

  x_midpoint1 = 0.5 * (g_back_wall_coords[depth][position][TOP_LEFT].x +
                       g_back_wall_coords[depth][position][BOTTOM_RIGHT].x);
  if (depth == 0)
  {
    if (position < STRAIGHT_AHEAD)      // Just to the left of the player.
    {
      x_midpoint2 = -0.5 * GRAPHICS_FRAME_WIDTH;
    }
    else if (position > STRAIGHT_AHEAD) // Just to the right of the player.
    {
      x_midpoint2 = 1.5 * GRAPHICS_FRAME_WIDTH;
    }
    else                                // Directly under the player.
    {
      x_midpoint2 = x_midpoint1;
    }
    y = GRAPHICS_FRAME_HEIGHT;
  }
  else
  {
    x_midpoint2 = 0.5 *
      (g_back_wall_coords[depth - 1][position][TOP_LEFT].x +
       g_back_wall_coords[depth - 1][position][BOTTOM_RIGHT].x);
    y = 0.5 * (g_back_wall_coords[depth][position][BOTTOM_RIGHT].y +
               g_back_wall_coords[depth - 1][position][BOTTOM_RIGHT].y);
  }
  x = 0.5 * (x_midpoint1 + x_midpoint2);

  return GPoint(x, y);
}

/******************************************************************************
   Function: get_midair_center_point

Description: Returns the midair central point, with respect to the graphics
             layer, of the cell at a given visual depth and position.

     Inputs: depth    - Front-back visual depth in "g_back_wall_coords".
             position - Left-right visual position in "g_back_wall_coords".

    Outputs: GPoint coordinates of the midair central point within the
             designated cell.
******************************************************************************/
/*GPoint get_midair_center_point(const int8_t depth, const int8_t position)
{
  uint8_t x_midpoint1, x_midpoint2, x, y;

  x_midpoint1 = 0.5 * (g_back_wall_coords[depth][position][TOP_LEFT].x +
                       g_back_wall_coords[depth][position][BOTTOM_RIGHT].x);
  if (depth == 0)
  {
    if (position < STRAIGHT_AHEAD) // Just to the left of the player.
    {
      x_midpoint2 = -0.5 * GRAPHICS_FRAME_WIDTH;
    }
    else if (position > STRAIGHT_AHEAD) // Just to the right of the player.
    {
      x_midpoint2 = 1.5 * GRAPHICS_FRAME_WIDTH;
    }
    else // Directly under the player.
    {
      x_midpoint2 = x_midpoint1;
    }
    //y = GRAPHICS_FRAME_HEIGHT;
  }
  else
  {
    x_midpoint2 = 0.5 *
      (g_back_wall_coords[depth - 1][position][TOP_LEFT].x +
       g_back_wall_coords[depth - 1][position][BOTTOM_RIGHT].x);
  }
  x = 0.5 * (x_midpoint1 + x_midpoint2);
  y = SCREEN_CENTER_POINT_Y;

  return GPoint(x, y);
}*/

/******************************************************************************
   Function: get_cell_farther_away

Description: Given a set of cell coordinates, returns new cell coordinates a
             given distance farther away in a given direction. (These may lie
             out-of-bounds.)

     Inputs: reference_point - Reference cell coordinates.
             direction       - Direction of interest.
             distance        - How far back we want to go.

    Outputs: Cell coordinates a given distance farther away from those passed
             in. (These may lie out-of-bounds.)
******************************************************************************/
GPoint get_cell_farther_away(const GPoint reference_point,
                             const int8_t direction,
                             const int8_t distance)
{
  switch (direction)
  {
    case NORTH:
      return GPoint(reference_point.x, reference_point.y - distance);
    case SOUTH:
      return GPoint(reference_point.x, reference_point.y + distance);
    case EAST:
      return GPoint(reference_point.x + distance, reference_point.y);
    default: // case WEST:
      return GPoint(reference_point.x - distance, reference_point.y);
  }
}

/******************************************************************************
   Function: get_pursuit_direction

Description: Determines in which direction a character at a given position
             ought to move in order to pursue a character at another given
             position. (Simplistic: no complex path-finding.)

     Inputs: pursuer - Position of the pursuing character.
             pursuee - Position of the character being pursued.

    Outputs: Integer representing the direction in which the NPC ought to move.
******************************************************************************/
int8_t get_pursuit_direction(const GPoint pursuer, const GPoint pursuee)
{
  int8_t diff_x                     = pursuer.x - pursuee.x,
         diff_y                     = pursuer.y - pursuee.y;
  const int8_t horizontal_direction = diff_x > 0 ? WEST  : EAST,
               vertical_direction   = diff_y > 0 ? NORTH : SOUTH;
  bool checked_horizontal_direction = false,
       checked_vertical_direction   = false;

  // Check for alignment along the x-axis:
  if (diff_x == 0)
  {
    if (diff_y == 1 /* The two are already touching. */ ||
        occupiable(get_cell_farther_away(pursuer,
                                         vertical_direction,
                                         1)))
    {
      return vertical_direction;
    }
    checked_vertical_direction = true;
  }

  // Check for alignment along the y-axis:
  if (diff_y == 0)
  {
    if (diff_x == 1 /* The two are already touching. */ ||
        occupiable(get_cell_farther_away(pursuer,
                                         horizontal_direction,
                                         1)))
    {
      return horizontal_direction;
    }
    checked_horizontal_direction = true;
  }

  // If not aligned along either axis, a direction in either axis will do:
  while (!checked_horizontal_direction || !checked_vertical_direction)
  {
    if (checked_vertical_direction ||
        (!checked_horizontal_direction && rand() % 2))
    {
      if (occupiable(get_cell_farther_away(pursuer,
                                           horizontal_direction,
                                           1)))
      {
        return horizontal_direction;
      }
      checked_horizontal_direction = true;
    }
    if (!checked_vertical_direction)
    {
      if (occupiable(get_cell_farther_away(pursuer,
                                           vertical_direction,
                                           1)))
      {
        return vertical_direction;
      }
      checked_vertical_direction = true;
    }
  }

  // If we reach this point, the NPC is stuck in a corner. I'm okay with that:
  return horizontal_direction;
}

/******************************************************************************
   Function: get_direction_to_the_left

Description: Given a north/south/east/west reference direction, returns the
             direction to its left.

     Inputs: reference_direction - Direction from which to turn left.

    Outputs: Integer representing the direction to the left of the reference
             direction.
******************************************************************************/
int8_t get_direction_to_the_left(const int8_t reference_direction)
{
  switch (reference_direction)
  {
    case NORTH:
      return WEST;
    case WEST:
      return SOUTH;
    case SOUTH:
      return EAST;
    default: // case EAST:
      return NORTH;
  }
}

/******************************************************************************
   Function: get_direction_to_the_right

Description: Given a north/south/east/west reference direction, returns the
             direction to its right.

     Inputs: reference_direction - Direction from which to turn right.

    Outputs: Integer representing the direction to the right of the reference
             direction.
******************************************************************************/
int8_t get_direction_to_the_right(const int8_t reference_direction)
{
  switch (reference_direction)
  {
    case NORTH:
      return EAST;
    case EAST:
      return SOUTH;
    case SOUTH:
      return WEST;
    default: // case WEST:
      return NORTH;
  }
}

/******************************************************************************
   Function: get_opposite_direction

Description: Returns the opposite of a given direction value (i.e., given the
             argument "NORTH", "SOUTH" will be returned).

     Inputs: direction - The direction whose opposite is desired.

    Outputs: Integer representing the opposite of the given direction.
******************************************************************************/
int8_t get_opposite_direction(const int8_t direction)
{
  switch (direction)
  {
    case NORTH:
      return SOUTH;
    case SOUTH:
      return NORTH;
    case EAST:
      return WEST;
    default: // case WEST:
      return EAST;
  }
}

/******************************************************************************
   Function: get_nth_item_type

Description: Returns the type of the nth item in the player's inventory.

     Inputs: n - Integer indicating the item of interest (0th, 1st, 2nd, etc.).

    Outputs: The nth item's type.
******************************************************************************/
int8_t get_nth_item_type(const int8_t n)
{
  int8_t i, item_count = 0;

  // Search Pebbles:
  for (i = 0; i < NUM_PEBBLE_TYPES; ++i)
  {
    if (g_player->pebbles[i] > 0 && item_count++ == n)
    {
      return i;
    }
  }

  // Search heavy items:
  for (i = 0; i < MAX_HEAVY_ITEMS; ++i)
  {
    if (g_player->heavy_items[i].type > NONE && item_count++ == n)
    {
      return g_player->heavy_items[i].type;
    }
  }

  return NONE;
}

/******************************************************************************
   Function: get_num_pebble_types_owned

Description: Returns the number of types of Pebbles in the player's inventory.

     Inputs: None.

    Outputs: Number of Pebble types owned by the player.
******************************************************************************/
int8_t get_num_pebble_types_owned(void)
{
  int8_t i, num_pebble_types = 0;

  for (i = 0; i < NUM_PEBBLE_TYPES; ++i)
  {
    if (g_player->pebbles[i] > 0)
    {
      num_pebble_types++;
    }
  }

  return num_pebble_types;
}

/******************************************************************************
   Function: get_inventory_row_for_pebble

Description: Returns the row where a given Pebble type will be displayed in the
             inventory menu (which depends on how many Pebble types the player
             currently owns).

     Inputs: pebble_type - Integer representing the Pebble type of interest.

    Outputs: Integer indicating the inventory row where the Pebble type will be
             found (starting from zero).
******************************************************************************/
int8_t get_inventory_row_for_pebble(const int8_t pebble_type)
{
  int8_t i;

  for (i = 0; i < NUM_PEBBLE_TYPES; ++i)
  {
    if (get_nth_item_type(i) == pebble_type)
    {
      return i;
    }
  }

  return 0;
}

/******************************************************************************
   Function: get_num_heavy_items_owned

Description: Returns the number of heavy items in the player's inventory.

     Inputs: None.

    Outputs: Number of heavy items owned by the player.
******************************************************************************/
int8_t get_num_heavy_items_owned(void)
{
  int8_t i, num_heavy_items = 0;

  for (i = 0; i < MAX_HEAVY_ITEMS; ++i)
  {
    if (g_player->heavy_items[i].type > NONE)
    {
      num_heavy_items++;
    }
  }

  return num_heavy_items;
}

/******************************************************************************
   Function: get_heavy_item_equipped_at

Description: Given an equip target (RIGHT_HAND, LEFT_HAND, or BODY), returns
             a pointer to the heavy item equipped at that target or NULL if
             nothing (or a Pebble) is equipped there.

     Inputs: equip_target - Integer representing the equip target of interest.

    Outputs: Pointer to the item equipped at the specified target.
******************************************************************************/
heavy_item_t *get_heavy_item_equipped_at(const int8_t equip_target)
{
  int8_t i;

  for (i = 0; i < MAX_HEAVY_ITEMS; ++i)
  {
    if (g_player->heavy_items[i].equipped &&
        g_player->heavy_items[i].equip_target == equip_target)
    {
      return &g_player->heavy_items[i];
    }
  }

  return NULL;
}

/******************************************************************************
   Function: get_cell_type

Description: Returns the type of cell at a given set of coordinates.

     Inputs: cell - Coordinates of the cell of interest.

    Outputs: The indicated cell's type.
******************************************************************************/
int8_t get_cell_type(const GPoint cell)
{
  if (out_of_bounds(cell))
  {
    return SOLID;
  }

  return g_location->map[cell.x][cell.y];
}

/******************************************************************************
   Function: set_cell_type

Description: Sets the cell at a given set of coordinates to a given type.
             (Doesn't test coordinates to ensure they're in-bounds!)

     Inputs: cell - Coordinates of the cell of interest.
             type - The cell type to be assigned at those coordinates.

    Outputs: None.
******************************************************************************/
void set_cell_type(GPoint cell, const int8_t type)
{
  g_location->map[cell.x][cell.y] = type;
}

/******************************************************************************
   Function: get_npc_at

Description: Returns a pointer to the NPC occupying a given cell.

     Inputs: cell - Coordinates of the cell of interest.

    Outputs: Pointer to the NPC occupying the indicated cell, or NULL if there
             is none.
******************************************************************************/
npc_t *get_npc_at(const GPoint cell)
{
  int8_t i;

  for (i = 0; i < MAX_NPCS_AT_ONE_TIME; ++i)
  {
    if (g_location->npcs[i].type > NONE &&
        gpoint_equal(&(g_location->npcs[i].position), &cell))
    {
      return &g_location->npcs[i];
    }
  }

  return NULL;
}

/******************************************************************************
   Function: out_of_bounds

Description: Determines whether a given set of cell coordinates lies outside
             the current map boundaries.

     Inputs: cell - Coordinates of the cell of interest.

    Outputs: "True" if the cell is out of bounds.
******************************************************************************/
bool out_of_bounds(const GPoint cell)
{
  return cell.x < 0          ||
         cell.x >= MAP_WIDTH ||
         cell.y < 0          ||
         cell.y >= MAP_HEIGHT;
}

/******************************************************************************
   Function: occupiable

Description: Determines whether the cell at a given set of coordinates may be
             occupied by a game character (i.e., it's within map boundaries,
             non-solid, not already occupied by another character, etc.).

     Inputs: cell - Coordinates of the cell of interest.

    Outputs: "True" if the cell is occupiable.
******************************************************************************/
bool occupiable(const GPoint cell)
{
  return get_cell_type(cell) >= EMPTY              &&
         !gpoint_equal(&g_player->position, &cell) &&
         get_npc_at(cell) == NULL;
}

/******************************************************************************
   Function: touching

Description: Determines whether two cells are "touching," meaning they are next
             to each other (diagonal doesn't count).

     Inputs: cell   - First set of cell coordinates.
             cell_2 - Second set of cell coordinates.

    Outputs: "True" if the two cells are touching.
******************************************************************************/
bool touching(const GPoint cell, const GPoint cell_2)
{
  const int8_t diff_x = cell.x - cell_2.x,
               diff_y = cell.y - cell_2.y;

  return ((diff_x == 0 && abs(diff_y) == 1) ||
          (diff_y == 0 && abs(diff_x) == 1));
}

/******************************************************************************
   Function: show_narration

Description: Displays desired narration text via the narration window.

     Inputs: narration - Integer indicating desired narration text.

    Outputs: None.
******************************************************************************/
void show_narration(const int8_t narration)
{
  int8_t i;
  static char narration_str[NARRATION_STR_LEN + 1];

  g_current_narration = narration;
  switch (narration)
  {
    case INTRO_NARRATION_1: // Total chars: 82
      strcpy(narration_str, "The Elderstone was sundered long ago, splintering"
                            " into countless Pebbles of Power.");
      break;
    case INTRO_NARRATION_2: // Total chars: 85
      strcpy(narration_str, "You have descended into a vast dungeon where the "
                            "Pebbles are guarded by evil wizards.");
      break;
    case INTRO_NARRATION_3: // Total chars: 92
      strcpy(narration_str, "Welcome, hero, to PebbleQuest!\n\nBy David C. "
                            "Drake:\n davidcdrake.com/\n"
                            "            pebblequest");
      break;
    case INTRO_NARRATION_4: // Total chars: 93
      strcpy(narration_str, "        CONTROLS\nForward: \"Up\"\nBack: \"Down\""
                            "\nLeft: \"Up\" x 2\nRight: \"Down\" x 2\nAttack: "
                            "\"Select\"");
      break;
    case DEATH_NARRATION: // Total chars: 69
      strcpy(narration_str, "\nAlas, you have perished in the dank, dark "
                            "depths, never to be found.");
      break;
    case STATS_NARRATION_1: // Total chars: ??
      strcpy(narration_str, "Depth: ");
      strcat_int(narration_str, g_player->depth);
      strcat(narration_str, "\nExp.: ");
      strcat_int(narration_str, g_player->exp_points);
      strcat(narration_str, "\nLevel: ");
      strcat_int(narration_str, g_player->level);
      for (i = 0; i < NUM_MAJOR_STATS; ++i)
      {
        strcat(narration_str, "\n");
        strcat_stat_name(narration_str, i);
        strcat_stat_value(narration_str, i);
      }
      break;
    case STATS_NARRATION_2: // Total chars: ??
      strcpy(narration_str, "");
      for (i = PHYSICAL_POWER; i < CURRENT_HEALTH; ++i)
      {
        strcat_stat_name(narration_str, i);
        strcat_stat_value(narration_str, i);
        strcat(narration_str, "\n");
      }
      if (g_player->stats[CURRENT_HEALTH] <= 0)
      {
        init_player();
      }
      break;
    default: // case LEVEL_UP_NARRATION: // Total chars: 55
      strcpy(narration_str, "\n  You have gained"
                            "\n        a level of"
                            "\n      experience!");
      break;
  }
  text_layer_set_text(g_narration_text_layer, narration_str);
  show_window(NARRATION_WINDOW, NOT_ANIMATED);
}

/******************************************************************************
   Function: show_window

Description: Prepares and displays a given window.

     Inputs: window   - Integer representing the desired window.
             animated - If "true", the window will slide into view.

    Outputs: None.
******************************************************************************/
void show_window(const int8_t window, const bool animated)
{
  // If it's a menu, reload menu data and set to the appropriate index:
  if (window < NUM_MENUS)
  {
    menu_layer_reload_data(g_menu_layers[window]);
    menu_layer_set_selected_index(g_menu_layers[window],
                                  (MenuIndex) {0, 0},
                                  MenuRowAlignCenter,
                                  NOT_ANIMATED);
    if (window == INVENTORY_MENU)
    {
      menu_layer_set_selected_index(g_menu_layers[window],
                                    (MenuIndex) {0, g_current_selection},
                                    MenuRowAlignCenter,
                                    NOT_ANIMATED);
    }
  }

  // Show the window:
  if (!window_stack_contains_window(g_windows[window]))
  {
    window_stack_push(g_windows[window], animated);
  }
  else
  {
    while (window_stack_get_top_window() != g_windows[window])
    {
      window_stack_pop(animated);
    }
  }
  g_current_window = window;
}

/******************************************************************************
   Function: main_menu_draw_header_callback

Description: Instructions for drawing the main menu's header.

     Inputs: ctx           - Pointer to the associated context.
             cell_layer    - Pointer to the cell layer.
             section_index - Section number of the header to be drawn.
             data          - Pointer to additional data (not used).

    Outputs: None.
******************************************************************************/
static void main_menu_draw_header_callback(GContext *ctx,
                                           const Layer *cell_layer,
                                           uint16_t section_index,
                                           void *data)
{
  //Window *window = (Window *) data;

  menu_cell_basic_header_draw(ctx, cell_layer, "PebbleQuest - Menu");
}

/******************************************************************************
   Function: level_up_menu_draw_header_callback

Description: Instructions for drawing the level-up menu's header.

     Inputs: ctx           - Pointer to the associated context.
             cell_layer    - Pointer to the cell layer.
             section_index - Section number of the header to be drawn.
             data          - Pointer to additional data (not used).

    Outputs: None.
******************************************************************************/
static void level_up_menu_draw_header_callback(GContext *ctx,
                                               const Layer *cell_layer,
                                               uint16_t section_index,
                                               void *data)
{
  menu_cell_basic_header_draw(ctx, cell_layer, "Increase an Attribute!");
}

/******************************************************************************
   Function: inventory_menu_draw_header_callback

Description: Instructions for drawing the inventory menu's header.

     Inputs: ctx           - Pointer to the associated context.
             cell_layer    - Pointer to the cell layer.
             section_index - Section number of the header to be drawn.
             data          - Pointer to additional data (not used).

    Outputs: None.
******************************************************************************/
static void inventory_menu_draw_header_callback(GContext *ctx,
                                                const Layer *cell_layer,
                                                uint16_t section_index,
                                                void *data)
{
  menu_cell_basic_header_draw(ctx, cell_layer, "Inventory");
}

/******************************************************************************
   Function: loot_menu_draw_header_callback

Description: Instructions for drawing the loot menu's header.

     Inputs: ctx           - Pointer to the associated context.
             cell_layer    - Pointer to the cell layer.
             section_index - Section number of the header to be drawn.
             data          - Pointer to additional data (not used).

    Outputs: None.
******************************************************************************/
static void loot_menu_draw_header_callback(GContext *ctx,
                                      const Layer *cell_layer,
                                      uint16_t section_index,
                                      void *data)
{
  menu_cell_basic_header_draw(ctx, cell_layer, "Loot!");
}

/******************************************************************************
   Function: pebble_options_menu_draw_header_callback

Description: Instructions for drawing the Pebble options menu's header.

     Inputs: ctx           - Pointer to the associated context.
             cell_layer    - Pointer to the cell layer.
             section_index - Section number of the header to be drawn.
             data          - Pointer to additional data (not used).

    Outputs: None.
******************************************************************************/
static void pebble_options_menu_draw_header_callback(GContext *ctx,
                                                     const Layer *cell_layer,
                                                     uint16_t section_index,
                                                     void *data)
{
  char header_str[MENU_HEADER_STR_LEN + 1] = "";

  strcat_item_name(header_str, g_current_selection);
  menu_cell_basic_header_draw(ctx, cell_layer, header_str);
}

/******************************************************************************
   Function: heavy_items_menu_draw_header_callback

Description: Instructions for drawing the heavy items menu's header.

     Inputs: ctx           - Pointer to the associated context.
             cell_layer    - Pointer to the cell layer.
             section_index - Section number of the header to be drawn.
             data          - Pointer to additional data (not used).

    Outputs: None.
******************************************************************************/
static void heavy_items_menu_draw_header_callback(GContext *ctx,
                                                  const Layer *cell_layer,
                                                  uint16_t section_index,
                                                  void *data)
{
  char header_str[MENU_HEADER_STR_LEN + 1] = "";

  if (g_current_selection < FIRST_HEAVY_ITEM)
  {
    strcat(header_str, "Infuse");
  }
  else
  {
    strcat(header_str, "Replace");
  }
  strcat(header_str, " an item?");
  menu_cell_basic_header_draw(ctx, cell_layer, header_str);
}

/******************************************************************************
   Function: main_menu_draw_row_callback

Description: Instructions for drawing each row (cell) of the main menu.

     Inputs: ctx        - Pointer to the associated context.
             cell_layer - Pointer to the layer of the cell to be drawn.
             cell_index - Pointer to the index struct of the cell to be drawn.
             data       - Pointer to additional data (not used).

    Outputs: None.
******************************************************************************/
static void main_menu_draw_row_callback(GContext *ctx,
                                        const Layer *cell_layer,
                                        MenuIndex *cell_index,
                                        void *data)
{
  char title_str[MENU_TITLE_STR_LEN + 1]       = "",
       subtitle_str[MENU_SUBTITLE_STR_LEN + 1] = "";

  switch (cell_index->row)
  {
    case 0:
      strcat(title_str, "Play");
      strcat(subtitle_str, "Dungeon-crawl, baby!");
      break;
    case 1:
      strcat(title_str, "Inventory");
      strcat(subtitle_str, "Equip/infuse items.");
      break;
    default:
      strcat(title_str, "Character Stats");
      strcat(subtitle_str, "Strength, Agility...");
      break;
  }
  menu_cell_basic_draw(ctx, cell_layer, title_str, subtitle_str, NULL);
}

/******************************************************************************
   Function: inventory_menu_draw_row_callback

Description: Instructions for drawing each row (cell) of the inventory menu.

     Inputs: ctx        - Pointer to the associated context.
             cell_layer - Pointer to the layer of the cell to be drawn.
             cell_index - Pointer to the index struct of the cell to be drawn.
             data       - Pointer to additional data (not used).

    Outputs: None.
******************************************************************************/
static void inventory_menu_draw_row_callback(GContext *ctx,
                                             const Layer *cell_layer,
                                             MenuIndex *cell_index,
                                             void *data)
{
  char title_str[MENU_TITLE_STR_LEN + 1]       = "",
       subtitle_str[MENU_SUBTITLE_STR_LEN + 1] = "";
  heavy_item_t *item;
  int8_t item_type = get_nth_item_type(cell_index->row);

  strcat_item_name(title_str, item_type);

  // Pebbles:
  if (item_type < FIRST_HEAVY_ITEM)
  {
    strcat(subtitle_str, "(");
    strcat_int(subtitle_str, g_player->pebbles[item_type]);
    strcat(subtitle_str, ")");
    if (g_player->equipped_pebble == item_type)
    {
      strcat(subtitle_str, " Equipped");
    }
  }

  // Heavy items:
  else
  {
    item = &g_player->heavy_items[cell_index->row -
                                    get_num_pebble_types_owned()];
    if (item->infused_pebble > NONE)
    {
      strcat_magic_type(title_str, item->infused_pebble);
    }
    if (item->equipped)
    {
      strcat(subtitle_str, "Equipped");
    }
  }
  menu_cell_basic_draw(ctx, cell_layer, title_str, subtitle_str, NULL);
}

/******************************************************************************
   Function: level_up_menu_draw_row_callback

Description: Instructions for drawing each row (cell) of the level-up menu.

     Inputs: ctx        - Pointer to the associated context.
             cell_layer - Pointer to the layer of the cell to be drawn.
             cell_index - Pointer to the index struct of the cell to be drawn.
             data       - Pointer to additional data (not used).

    Outputs: None.
******************************************************************************/
static void level_up_menu_draw_row_callback(GContext *ctx,
                                            const Layer *cell_layer,
                                            MenuIndex *cell_index,
                                            void *data)
{
  char title_str[MENU_TITLE_STR_LEN + 1] = "";

  strcat_stat_name(title_str, cell_index->row);
  strcat_stat_value(title_str, cell_index->row);
  menu_cell_basic_draw(ctx, cell_layer, title_str, NULL, NULL);
}

/******************************************************************************
   Function: loot_menu_draw_row_callback

Description: Instructions for drawing each row (cell) of the loot menu.

     Inputs: ctx        - Pointer to the associated context.
             cell_layer - Pointer to the layer of the cell to be drawn.
             cell_index - Pointer to the index struct of the cell to be drawn.
             data       - Pointer to additional data (not used).

    Outputs: None.
******************************************************************************/
static void loot_menu_draw_row_callback(GContext *ctx,
                                        const Layer *cell_layer,
                                        MenuIndex *cell_index,
                                        void *data)
{
  char title_str[MENU_TITLE_STR_LEN + 1] = "";

  strcat_item_name(title_str, g_current_selection);
  menu_cell_basic_draw(ctx, cell_layer, title_str, NULL, NULL);
}

/******************************************************************************
   Function: pebble_options_menu_draw_row_callback

Description: Instructions for drawing each row (cell) of the Pebble options
             menu.

     Inputs: ctx        - Pointer to the associated context.
             cell_layer - Pointer to the layer of the cell to be drawn.
             cell_index - Pointer to the index struct of the cell to be drawn.
             data       - Pointer to additional data (not used).

    Outputs: None.
******************************************************************************/
static void pebble_options_menu_draw_row_callback(GContext *ctx,
                                                  const Layer *cell_layer,
                                                  MenuIndex *cell_index,
                                                  void *data)
{
  char title_str[MENU_TITLE_STR_LEN + 1] = "";

  switch (cell_index->row)
  {
    case 0:
      strcat(title_str, "Equip");
      break;
    default:
      strcat(title_str, "Infuse into Item");
      break;
  }
  menu_cell_basic_draw(ctx, cell_layer, title_str, NULL, NULL);
}

/******************************************************************************
   Function: heavy_items_menu_draw_row_callback

Description: Instructions for drawing the rows (cells) of each "heavy items"
             menu: the "replace item" menu and the infusion menu.

     Inputs: ctx        - Pointer to the associated context.
             cell_layer - Pointer to the layer of the cell to be drawn.
             cell_index - Pointer to the index struct of the cell to be drawn.
             data       - Pointer to additional data (not used).

    Outputs: None.
******************************************************************************/
static void heavy_items_menu_draw_row_callback(GContext *ctx,
                                               const Layer *cell_layer,
                                               MenuIndex *cell_index,
                                               void *data)
{
  char title_str[MENU_TITLE_STR_LEN + 1]       = "",
       subtitle_str[MENU_SUBTITLE_STR_LEN + 1] = "";

  strcat_item_name(title_str, g_player->heavy_items[cell_index->row].type);
  if (g_player->heavy_items[cell_index->row].infused_pebble > NONE)
  {
    strcat_magic_type(title_str,
                      g_player->heavy_items[cell_index->row].infused_pebble);
  }
  if (g_player->heavy_items[cell_index->row].equipped)
  {
    strcat(subtitle_str, "Equipped");
  }

  menu_cell_basic_draw(ctx, cell_layer, title_str, subtitle_str, NULL);
}

/******************************************************************************
   Function: menu_select_callback

Description: Called when a menu cell is selected.

     Inputs: menu_layer - Pointer to the menu layer.
             cell_index - Pointer to the index struct of the selected cell.
             data       - Pointer to additional data (not used).

    Outputs: None.
******************************************************************************/
void menu_select_callback(MenuLayer *menu_layer,
                          MenuIndex *cell_index,
                          void *data)
{
  int8_t old_item_equip_target;
  bool item_was_equipped = false;

  if (menu_layer == g_menu_layers[MAIN_MENU])
  {
    switch (cell_index->row)
    {
      case 0: // Play
        show_window(GRAPHICS_WINDOW, NOT_ANIMATED);
        if (g_player->depth == 0)
        {
          show_narration(INTRO_NARRATION_1);
          init_location();
        }
        break;
      case 1: // Inventory
        g_current_selection = 0; // To scroll menu to the top.
        show_window(INVENTORY_MENU, ANIMATED);
        break;
      default: // Character Stats
        show_narration(STATS_NARRATION_1);
        break;
    }
  }
  else if (menu_layer == g_menu_layers[LEVEL_UP_MENU])
  {
    g_player->stats[cell_index->row]++;
    adjust_minor_stats();
    g_player->stats[CURRENT_HEALTH] = g_player->stats[MAX_HEALTH];
    g_player->stats[CURRENT_ENERGY] = g_player->stats[MAX_ENERGY];
    window_stack_pop(NOT_ANIMATED);
    show_narration(STATS_NARRATION_1);
  }
  else if (menu_layer == g_menu_layers[INVENTORY_MENU])
  {
    // Pebbles:
    if (get_nth_item_type(cell_index->row) < FIRST_HEAVY_ITEM)
    {
      g_current_selection = get_nth_item_type(cell_index->row);
      show_window(PEBBLE_OPTIONS_MENU, ANIMATED);
    }

    // Heavy items:
    else
    {
      equip_heavy_item(&g_player->heavy_items[cell_index->row -
                                                get_num_pebble_types_owned()]);
      menu_layer_reload_data(g_menu_layers[INVENTORY_MENU]);
    }
  }
  else if (menu_layer == g_menu_layers[LOOT_MENU])
  {
    show_window(GRAPHICS_WINDOW, NOT_ANIMATED);
    add_current_selection_to_inventory();
  }
  else if (menu_layer == g_menu_layers[PEBBLE_OPTIONS_MENU])
  {
    switch (cell_index->row)
    {
      case 0: // Equip
        unequip_item_at(RIGHT_HAND);
        g_player->equipped_pebble = g_current_selection;
        //adjust_minor_stats();???
        g_current_selection =
          get_inventory_row_for_pebble(g_current_selection);
        show_window(INVENTORY_MENU, NOT_ANIMATED);
        break;
      default: // Infuse into Item
        show_window(HEAVY_ITEMS_MENU, ANIMATED);
        break;
    }
  }
  else // if (menu_layer == g_menu_layers[HEAVY_ITEMS_MENU])
  {
    // "Infuse item" mode:
    if (g_current_selection < FIRST_HEAVY_ITEM)
    {
      // Ensure the item isn't already infused:
      if (g_player->heavy_items[cell_index->row].infused_pebble == NONE)
      {
        // Infuse the item:
        if (g_player->heavy_items[cell_index->row].equipped)
        {
          unequip_item_at(g_player->heavy_items[cell_index->row].equip_target);
          item_was_equipped = true;
        }
        g_player->heavy_items[cell_index->row].infused_pebble =
          g_current_selection;
        if (item_was_equipped)
        {
          equip_heavy_item(&g_player->heavy_items[cell_index->row]);
        }

        // Remove the Pebble from the pool of equippable/infusable Pebbles:
        g_player->pebbles[g_current_selection]--;
        if (g_player->equipped_pebble == g_current_selection &&
            g_player->pebbles[g_current_selection] == 0)
        {
          g_player->equipped_pebble = NONE;
        }

        // Return to the inventory menu, centered on the newly-infused item:
        g_current_selection = cell_index->row + get_num_pebble_types_owned();
        show_window(INVENTORY_MENU, NOT_ANIMATED);
      }
    }

    // "Replace item" mode:
    else
    {
      // If the item to be replaced is equipped, unequip it:
      old_item_equip_target =
        g_player->heavy_items[cell_index->row].equip_target;
      if (g_player->heavy_items[cell_index->row].equipped)
      {
        unequip_item_at(old_item_equip_target);
        item_was_equipped = true;
      }

      // Reinitialize the heavy item struct:
      init_heavy_item(&g_player->heavy_items[cell_index->row],
                      g_current_selection);

      // If old item was equipped, equip new one if it has the same equip target:
      if (item_was_equipped &&
          g_player->heavy_items[cell_index->row].equip_target ==
            old_item_equip_target)
      {
        equip_heavy_item(&g_player->heavy_items[cell_index->row]);
      }

      // Show inventory menu to provide an opportunity to adjust equipment:
      window_stack_pop(NOT_ANIMATED);
      g_current_selection = cell_index->row + get_num_pebble_types_owned();
      show_window(INVENTORY_MENU, NOT_ANIMATED);
    }

    // In either mode, we want to adjust minor stats:
    adjust_minor_stats();
  }
}

/******************************************************************************
   Function: menu_get_header_height_callback

Description: Returns the section height for a given section of a given menu.

     Inputs: menu_layer    - Pointer to the menu of interest.
             section_index - Section number.
             data          - Pointer to additional data (not used).

    Outputs: The number of sections in the indicated menu.
******************************************************************************/
static int16_t menu_get_header_height_callback(MenuLayer *menu_layer,
                                               uint16_t section_index,
                                               void *data)
{
  return MENU_CELL_BASIC_HEADER_HEIGHT;
}

/******************************************************************************
   Function: menu_get_num_rows_callback

Description: Returns the number of rows in a given menu (or in a given section
             of a given menu).

     Inputs: menu_layer    - Pointer to the menu of interest.
             section_index - Section number.
             data          - Pointer to additional data (not used).

    Outputs: The number of rows in the indicated menu.
******************************************************************************/
static uint16_t menu_get_num_rows_callback(MenuLayer *menu_layer,
                                           uint16_t section_index,
                                           void *data)
{
  if (menu_layer == g_menu_layers[INVENTORY_MENU])
  {
    return get_num_pebble_types_owned() + get_num_heavy_items_owned();
  }
  else if (menu_layer == g_menu_layers[HEAVY_ITEMS_MENU])
  {
    return get_num_heavy_items_owned();
  }
  else if (menu_layer == g_menu_layers[LOOT_MENU])
  {
    return 1;
  }
  else if (menu_layer == g_menu_layers[PEBBLE_OPTIONS_MENU])
  {
    return 2;
  }
  else // MAIN_MENU or LEVEL_UP_MENU
  {
    return 3;
  }
}

/******************************************************************************
   Function: draw_scene

Description: Draws a (simplistic) 3D scene based on the player's current
             position, direction, and visibility depth.

     Inputs: layer - Pointer to the relevant layer.
             ctx   - Pointer to the relevant graphics context.

    Outputs: None.
******************************************************************************/
void draw_scene(Layer *layer, GContext *ctx)
{
  int8_t i, depth;
  GPoint cell, cell_2;

  // First, draw the background, floor, and ceiling:
  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_rect(ctx,
                     layer_get_bounds(layer),
                     NO_CORNER_RADIUS,
                     GCornerNone);
  draw_floor_and_ceiling(ctx);

  // Now draw walls and cell contents:
  for (depth = MAX_VISIBILITY_DEPTH - 2; depth >= 0; --depth)
  {
    // Straight ahead at the current depth:
    cell = get_cell_farther_away(g_player->position,
                                 g_player->direction,
                                 depth);
    if (out_of_bounds(cell))
    {
      continue;
    }
    if (get_cell_type(cell) >= EMPTY)
    {
      draw_cell_walls(ctx, cell, depth, STRAIGHT_AHEAD);
      draw_cell_contents(ctx, cell, depth, STRAIGHT_AHEAD);
    }

    // To the left and right at the same depth:
    for (i = depth + 1; i > 0; --i)
    {
      cell_2 = get_cell_farther_away(cell,
                                get_direction_to_the_left(g_player->direction),
                                i);
      if (get_cell_type(cell_2) >= EMPTY)
      {
        draw_cell_walls(ctx, cell_2, depth, STRAIGHT_AHEAD - i);
        draw_cell_contents(ctx, cell_2, depth, STRAIGHT_AHEAD - i);
      }
      cell_2 = get_cell_farther_away(cell,
                               get_direction_to_the_right(g_player->direction),
                               i);
      if (get_cell_type(cell_2) >= EMPTY)
      {
        draw_cell_walls(ctx, cell_2, depth, STRAIGHT_AHEAD + i);
        draw_cell_contents(ctx, cell_2, depth, STRAIGHT_AHEAD + i);
      }
    }
  }

  // Draw the "attack slash", if applicable:
  if (g_player_is_attacking)
  {
    graphics_context_set_stroke_color(ctx, GColorWhite);
    graphics_draw_line(ctx,
                       GPoint(g_attack_slash_x1, g_attack_slash_y1),
                       GPoint(g_attack_slash_x2, g_attack_slash_y2));
    for (i = 0; i < ATTACK_SLASH_WIDTH / 2; ++i)
    {
      graphics_draw_line(ctx,
                         GPoint(g_attack_slash_x1 + i, g_attack_slash_y1),
                         GPoint(g_attack_slash_x2 + i, g_attack_slash_y2));
      graphics_draw_line(ctx,
                         GPoint(g_attack_slash_x1 - i, g_attack_slash_y1),
                         GPoint(g_attack_slash_x2 - i, g_attack_slash_y2));
    }
  }

  // Finally, draw the lower status bar:
  draw_status_bar(ctx);
}

/******************************************************************************
   Function: draw_floor_and_ceiling

Description: Draws the floor and ceiling.

     Inputs: ctx - Pointer to the relevant graphics context.

    Outputs: None.
******************************************************************************/
void draw_floor_and_ceiling(GContext *ctx)
{
  uint8_t x, y, max_y, shading_offset;

  x     = 2;
  max_y = g_back_wall_coords[MAX_VISIBILITY_DEPTH - x]
                            [STRAIGHT_AHEAD]
                            [TOP_LEFT].y;
  while (max_y > GRAPHICS_FRAME_HEIGHT / 2 - MIN_WALL_HEIGHT / 2 &&
         x <= MAX_VISIBILITY_DEPTH)
  {
    max_y = g_back_wall_coords[MAX_VISIBILITY_DEPTH - ++x]
                              [STRAIGHT_AHEAD]
                              [TOP_LEFT].y;
  }
  graphics_context_set_stroke_color(ctx, GColorWhite);
  for (y = 0; y < max_y; ++y)
  {
    // Determine horizontal distance between points:
    shading_offset = 1 + y / MAX_VISIBILITY_DEPTH;
    if (y % MAX_VISIBILITY_DEPTH >= MAX_VISIBILITY_DEPTH / 2 +
                                    MAX_VISIBILITY_DEPTH % 2)
    {
      shading_offset++;
    }
    for (x = y % 2 ? 0 : (shading_offset / 2) + (shading_offset % 2);
         x < GRAPHICS_FRAME_WIDTH;
         x += shading_offset)
    {
      // Draw a point on the ceiling:
      graphics_draw_pixel(ctx, GPoint(x, y));

      // Draw a point on the floor:
      graphics_draw_pixel(ctx, GPoint(x, GRAPHICS_FRAME_HEIGHT - y));
    }
  }
}

/******************************************************************************
   Function: draw_cell_walls

Description: Draws any walls that exist along the back and sides of a given
             cell.

     Inputs: ctx      - Pointer to the relevant graphics context.
             cell     - Coordinates of the cell of interest.
             depth    - Front-back visual depth of the cell of interest in
                        "g_back_wall_coords".
             position - Left-right visual position of the cell of interest in
                        "g_back_wall_coords".

    Outputs: None.
******************************************************************************/
void draw_cell_walls(GContext *ctx,
                     const GPoint cell,
                     const int8_t depth,
                     const int8_t position)
{
  int16_t left, right, top, bottom, y_offset;
  bool back_wall_drawn, left_wall_drawn, right_wall_drawn;
  GPoint cell_2;

  // Back wall:
  left   = g_back_wall_coords[depth][position][TOP_LEFT].x;
  right  = g_back_wall_coords[depth][position][BOTTOM_RIGHT].x;
  top    = g_back_wall_coords[depth][position][TOP_LEFT].y;
  bottom = g_back_wall_coords[depth][position][BOTTOM_RIGHT].y;
  if (bottom - top < MIN_WALL_HEIGHT)
  {
    return;
  }
  back_wall_drawn = left_wall_drawn = right_wall_drawn = false;
  cell_2          = get_cell_farther_away(cell, g_player->direction, 1);
  if (get_cell_type(cell_2) <= SOLID)
  {
    draw_shaded_quad(ctx,
                     GPoint(left, top),
                     GPoint(left, bottom),
                     GPoint(right, top),
                     GPoint(right, bottom),
                     GPoint(left, top));
    graphics_context_set_stroke_color(ctx, GColorBlack);
    graphics_draw_line(ctx, GPoint(left, top), GPoint(right, top));
    graphics_draw_line(ctx, GPoint(left, bottom), GPoint(right, bottom));

    // Ad hoc solution to a minor visual issue (remove if no longer relevant):
    if (top == g_back_wall_coords[1][0][TOP_LEFT].y)
    {
      graphics_draw_line(ctx,
                         GPoint(left, bottom + 1),
                         GPoint(right, bottom + 1));
    }

    back_wall_drawn = true;
  }

  // Left wall:
  right = left;
  if (depth == 0)
  {
    left     = 0;
    y_offset = top;
  }
  else
  {
    left     = g_back_wall_coords[depth - 1][position][TOP_LEFT].x;
    y_offset = top - g_back_wall_coords[depth - 1][position][TOP_LEFT].y;
  }
  if (position <= STRAIGHT_AHEAD)
  {
    cell_2 = get_cell_farther_away(cell,
                                get_direction_to_the_left(g_player->direction),
                                1);
    if (get_cell_type(cell_2) <= SOLID)
    {
      draw_shaded_quad(ctx,
                       GPoint(left, top - y_offset),
                       GPoint(left, bottom + y_offset),
                       GPoint(right, top),
                       GPoint(right, bottom),
                       GPoint(left, top - y_offset));
      graphics_context_set_stroke_color(ctx, GColorBlack);
      graphics_draw_line(ctx,
                         GPoint(left, top - y_offset),
                         GPoint(right, top));
      graphics_draw_line(ctx,
                         GPoint(left, bottom + y_offset),
                         GPoint(right, bottom));
      left_wall_drawn = true;
    }
  }

  // Right wall:
  left = g_back_wall_coords[depth][position][BOTTOM_RIGHT].x;
  if (depth == 0)
  {
    right = GRAPHICS_FRAME_WIDTH - 1;
  }
  else
  {
    right = g_back_wall_coords[depth - 1][position][BOTTOM_RIGHT].x;
  }
  if (position >= STRAIGHT_AHEAD)
  {
    cell_2 = get_cell_farther_away(cell,
                               get_direction_to_the_right(g_player->direction),
                               1);
    if (get_cell_type(cell_2) <= SOLID)
    {
      draw_shaded_quad(ctx,
                       GPoint(left, top),
                       GPoint(left, bottom),
                       GPoint(right, top - y_offset),
                       GPoint(right, bottom + y_offset),
                       GPoint(left, top));
      graphics_context_set_stroke_color(ctx, GColorBlack);
      graphics_draw_line(ctx,
                         GPoint(left, top),
                         GPoint(right, top - y_offset));
      graphics_draw_line(ctx,
                         GPoint(left, bottom),
                         GPoint(right, bottom + y_offset));
      right_wall_drawn = true;
    }
  }

  // Draw vertical lines at corners:
  graphics_context_set_stroke_color(ctx, GColorBlack);
  cell_2 = get_cell_farther_away(cell, g_player->direction, 1);
  if ((back_wall_drawn && (left_wall_drawn ||
       get_cell_type(get_cell_farther_away(cell_2,
                                get_direction_to_the_left(g_player->direction),
                                1)) >= EMPTY)) ||
      (left_wall_drawn &&
       get_cell_type(get_cell_farther_away(cell_2,
                                get_direction_to_the_left(g_player->direction),
                                1)) >= EMPTY))
  {
    graphics_draw_line(ctx,
                       g_back_wall_coords[depth][position][TOP_LEFT],
                       GPoint(g_back_wall_coords[depth][position][TOP_LEFT].x,
                         g_back_wall_coords[depth][position][BOTTOM_RIGHT].y));
  }
  if ((back_wall_drawn && (right_wall_drawn ||
       get_cell_type(get_cell_farther_away(cell_2,
                               get_direction_to_the_right(g_player->direction),
                               1)) >= EMPTY)) ||
      (right_wall_drawn &&
       get_cell_type(get_cell_farther_away(cell_2,
                               get_direction_to_the_right(g_player->direction),
                               1)) >= EMPTY))
  {
    graphics_draw_line(ctx,
                    g_back_wall_coords[depth][position][BOTTOM_RIGHT],
                    GPoint(g_back_wall_coords[depth][position][BOTTOM_RIGHT].x,
                           g_back_wall_coords[depth][position][TOP_LEFT].y));
  }
}

/******************************************************************************
   Function: draw_cell_contents

Description: Draws an NPC or any other contents present in a given cell.

     Inputs: ctx      - Pointer to the relevant graphics context.
             cell     - Coordinates of the cell of interest.
             depth    - Front-back visual depth of the cell of interest in
                        "g_back_wall_coords".
             position - Left-right visual position of the cell of interest in
                        "g_back_wall_coords".

    Outputs: None.
******************************************************************************/
void draw_cell_contents(GContext *ctx,
                        const GPoint cell,
                        const int8_t depth,
                        const int8_t position)
{
  uint8_t drawing_unit; // Reference variable for drawing contents at depth.
  GPoint floor_center_point;
  npc_t *npc = get_npc_at(cell);

  // Check for a completely empty cell:
  if (get_cell_type(cell) == EMPTY && npc == NULL)
  {
    return;
  }

  // Determine the drawing unit and floor center point:
  drawing_unit = (g_back_wall_coords[depth][position][BOTTOM_RIGHT].x -
                  g_back_wall_coords[depth][position][TOP_LEFT].x) / 10;
  if ((g_back_wall_coords[depth][position][BOTTOM_RIGHT].x -
       g_back_wall_coords[depth][position][TOP_LEFT].x) % 10 >= 5)
  {
    drawing_unit++;
  }
  floor_center_point = get_floor_center_point(depth, position);

  // Draw a shadow/exit on the ground or an entrance on the ceiling:
  fill_ellipse(ctx,
               GPoint(floor_center_point.x,
                      get_cell_type(cell) == ENTRANCE                ?
                        GRAPHICS_FRAME_HEIGHT - floor_center_point.y :
                        floor_center_point.y),
               ELLIPSE_RADIUS_RATIO *
                 (g_back_wall_coords[depth][position][BOTTOM_RIGHT].x -
                  g_back_wall_coords[depth][position][TOP_LEFT].x),
               depth == 0                                               ?
                 ELLIPSE_RADIUS_RATIO *
                  (GRAPHICS_FRAME_HEIGHT -
                   g_back_wall_coords[depth][position][BOTTOM_RIGHT].y) :
                 ELLIPSE_RADIUS_RATIO *
                    (g_back_wall_coords[depth - 1][position][BOTTOM_RIGHT].y -
                     g_back_wall_coords[depth][position][BOTTOM_RIGHT].y),
               GColorBlack);

  // If there's no NPC, check for loot, then we're done:
  if (npc == NULL)
  {
    if (get_cell_type(cell) >= 0) // Loot!
    {
      graphics_context_set_fill_color(ctx, GColorWhite);
      graphics_fill_rect(ctx,
                         GRect(floor_center_point.x - drawing_unit * 2,
                               floor_center_point.y - drawing_unit * 2.5,
                               drawing_unit * 4,
                               drawing_unit * 2.5),
                         drawing_unit / 2,
                         GCornersTop);
    }
    return;
  }

  // For some NPCs, we want to increase the drawing unit:
  /*if (npc->type == BEAR  ||
      npc->type == OGRE  ||
      npc->type == TROLL ||
      npc->type == MINOTAUR)
  {
    drawing_unit += 1;
  }*/

  // Draw the NPC:
  graphics_context_set_fill_color(ctx, GColorBlack);
  /*if (npc->type == WOLF || npc->type == BEAR || npc->type == DRAGON)
  {
    // Legs:
    graphics_fill_rect(ctx,
                       GRect(floor_center_point.x - drawing_unit * 3,
                             floor_center_point.y - drawing_unit * 4,
                             drawing_unit * 2,
                             drawing_unit * 4),
                       NO_CORNER_RADIUS,
                       GCornerNone);
    graphics_fill_rect(ctx,
                       GRect(floor_center_point.x + drawing_unit,
                             floor_center_point.y - drawing_unit * 4,
                             drawing_unit * 2,
                             drawing_unit * 4),
                       NO_CORNER_RADIUS,
                       GCornerNone);

    // Body/Head:
    graphics_fill_circle(ctx,
                         GPoint(floor_center_point.x,
                                floor_center_point.y - drawing_unit * 5),
                         drawing_unit * 3);

    // Eyes:
    graphics_context_set_fill_color(ctx, GColorWhite);
    graphics_fill_rect(ctx,
                       GRect(floor_center_point.x -
                               (drawing_unit + drawing_unit / 2),
                             floor_center_point.y - drawing_unit * 7,
                             drawing_unit,
                             drawing_unit / 2),
                       drawing_unit / 4,
                       GCornersAll);
    graphics_fill_rect(ctx,
                       GRect(floor_center_point.x + drawing_unit / 2,
                             floor_center_point.y - drawing_unit * 7,
                             drawing_unit,
                             drawing_unit / 2),
                       drawing_unit / 4,
                       GCornersAll);

    // Mouth:
    graphics_fill_rect(ctx,
                       GRect(floor_center_point.x -
                               (drawing_unit + drawing_unit / 2),
                             floor_center_point.y - drawing_unit * 5,
                             drawing_unit,
                             drawing_unit + drawing_unit / 2 + (time(NULL) % 2
                               ? 0 : drawing_unit / 2)),
                       drawing_unit / 2,
                       GCornersAll);
    graphics_fill_rect(ctx,
                       GRect(floor_center_point.x - drawing_unit / 2,
                             floor_center_point.y - drawing_unit * 5,
                             drawing_unit,
                             drawing_unit + drawing_unit / 2 + (time(NULL) % 2
                               ? 0 : drawing_unit / 2)),
                       drawing_unit / 2,
                       GCornersAll);
    graphics_fill_rect(ctx,
                       GRect(floor_center_point.x + drawing_unit / 2,
                             floor_center_point.y - drawing_unit * 5,
                             drawing_unit,
                             drawing_unit + drawing_unit / 2 + (time(NULL) % 2
                               ? 0 : drawing_unit / 2)),
                       drawing_unit / 2,
                       GCornersAll);
  }
  else if (npc->type == OOZE      ||
           npc->type == WORM      ||
           npc->type == INSECT    ||
           npc->type == ELEMENTAL ||
           npc->type == FLOATING_EYE)
  {
    graphics_fill_circle(ctx,
                         GPoint(floor_center_point.x,
                                floor_center_point.y - drawing_unit * 2),
                         drawing_unit + time(NULL) % 2);
    graphics_fill_circle(ctx,
                         GPoint(floor_center_point.x - drawing_unit * 2,
                                floor_center_point.y - drawing_unit * 2),
                         drawing_unit + time(NULL) % 2);
    graphics_fill_circle(ctx,
                         GPoint(floor_center_point.x + drawing_unit * 2,
                                floor_center_point.y - drawing_unit * 3),
                         drawing_unit + time(NULL) % 2);
    graphics_fill_circle(ctx,
                         GPoint(floor_center_point.x - drawing_unit,
                                floor_center_point.y - drawing_unit * 4),
                         drawing_unit + time(NULL) % 2);
    graphics_fill_circle(ctx,
                         GPoint(floor_center_point.x,
                                floor_center_point.y - drawing_unit * 5),
                         drawing_unit + time(NULL) % 2);
  }
  else // if (npc->type == [humanoid])
  {
    // Legs:
    */draw_shaded_quad(ctx,
                     GPoint(floor_center_point.x - drawing_unit * 2,
                            floor_center_point.y - drawing_unit * 3),
                     GPoint(floor_center_point.x - drawing_unit * 2,
                            floor_center_point.y),
                     GPoint(floor_center_point.x - drawing_unit,
                            floor_center_point.y - drawing_unit * 3),
                     GPoint(floor_center_point.x - drawing_unit,
                            floor_center_point.y),
                     GPoint(g_back_wall_coords[depth][position][TOP_LEFT].x +
                              4,
                            g_back_wall_coords[depth][position][TOP_LEFT].y +
                              4));
    draw_shaded_quad(ctx,
                     GPoint(floor_center_point.x + drawing_unit,
                            floor_center_point.y - drawing_unit * 3),
                     GPoint(floor_center_point.x + drawing_unit,
                            floor_center_point.y),
                     GPoint(floor_center_point.x + drawing_unit * 2,
                            floor_center_point.y - drawing_unit * 3),
                     GPoint(floor_center_point.x + drawing_unit * 2,
                            floor_center_point.y),
                     GPoint(g_back_wall_coords[depth][position][TOP_LEFT].x +
                              4,
                            g_back_wall_coords[depth][position][TOP_LEFT].y +
                              4));

    // Waist:
    draw_shaded_quad(ctx,
                     GPoint(floor_center_point.x - drawing_unit * 2,
                            floor_center_point.y - drawing_unit * 4),
                     GPoint(floor_center_point.x - drawing_unit * 2,
                            floor_center_point.y - drawing_unit * 3),
                     GPoint(floor_center_point.x + drawing_unit * 2,
                            floor_center_point.y - drawing_unit * 4),
                     GPoint(floor_center_point.x + drawing_unit * 2,
                            floor_center_point.y - drawing_unit * 3),
                     GPoint(g_back_wall_coords[depth][position][TOP_LEFT].x +
                              4,
                            g_back_wall_coords[depth][position][TOP_LEFT].y +
                              4));

    // Torso:
    draw_shaded_quad(ctx,
                     GPoint(floor_center_point.x - drawing_unit * 2,
                            floor_center_point.y - drawing_unit * 8),
                     GPoint(floor_center_point.x - drawing_unit * 2,
                            floor_center_point.y - drawing_unit * 4),
                     GPoint(floor_center_point.x + drawing_unit * 2,
                            floor_center_point.y - drawing_unit * 8),
                     GPoint(floor_center_point.x + drawing_unit * 2,
                            floor_center_point.y - drawing_unit * 4),
                     GPoint(g_back_wall_coords[depth][position][TOP_LEFT].x -
                              10,
                            g_back_wall_coords[depth][position][TOP_LEFT].y -
                              10));
    /*graphics_fill_rect(ctx,
                       GRect(floor_center_point.x - drawing_unit * 2,
                             floor_center_point.y - drawing_unit * 8,
                             drawing_unit * 4,
                             drawing_unit * 4),
                       NO_CORNER_RADIUS,
                       GCornerNone);*/

    // Arms:
    graphics_context_set_fill_color(ctx, GColorWhite);
    graphics_fill_rect(ctx,
                       GRect(floor_center_point.x - drawing_unit * 3,
                             floor_center_point.y - drawing_unit * 8,
                             drawing_unit,
                             drawing_unit * 3),
                       drawing_unit / 2,
                       GCornersLeft);
    graphics_fill_rect(ctx,
                       GRect(floor_center_point.x + drawing_unit * 2,
                             floor_center_point.y - drawing_unit * 8,
                             drawing_unit,
                             drawing_unit * 4),
                       drawing_unit / 2,
                       GCornersRight);

    // Head:
    graphics_fill_rect(ctx,
                       GRect(floor_center_point.x - drawing_unit,
                             floor_center_point.y - drawing_unit * 10,
                             drawing_unit * 2 + 1,
                             drawing_unit * 2),
                       drawing_unit,
                       GCornersTop);

    // Eyes:
    graphics_context_set_fill_color(ctx, GColorBlack);
    graphics_fill_circle(ctx,
                         GPoint(floor_center_point.x - drawing_unit / 2,
                                floor_center_point.y - (drawing_unit * 9)),
                         drawing_unit / 4);
    graphics_fill_circle(ctx,
                         GPoint(floor_center_point.x + drawing_unit / 2,
                                floor_center_point.y - (drawing_unit * 9)),
                         drawing_unit / 4);
  //}
}

/******************************************************************************
   Function: draw_shaded_quad

Description: Draws a shaded quadrilateral according to specifications. Assumes
             the left and right sides are parallel.

     Inputs: ctx         - Pointer to the relevant graphics context.
             upper_left  - Coordinates of the upper-left point.
             lower_left  - Coordinates of the lower-left point.
             upper_right - Coordinates of the upper-right point.
             lower_right - Coordinates of the lower-right point.
             shading_ref - Reference coordinates to assist in determining
                           shading offset values for the quad's location in
                           the 3D environment. (For walls, this is the same as
                           "upper_left".)

    Outputs: None.
******************************************************************************/
void draw_shaded_quad(GContext *ctx,
                      const GPoint upper_left,
                      const GPoint lower_left,
                      const GPoint upper_right,
                      const GPoint lower_right,
                      const GPoint shading_ref)
{
  int16_t i, j, shading_offset, half_shading_offset;
  float shading_gradient = 0;

  shading_gradient = (float) (upper_right.y - upper_left.y) /
                             (upper_right.x - upper_left.x);

  for (i = upper_left.x;
       i <= upper_right.x && i < GRAPHICS_FRAME_WIDTH;
       ++i)
  {
    // Calculate a new shading offset for each "x" value:
    shading_offset = 1 + ((shading_ref.y + (i - upper_left.x) *
                           shading_gradient) / MAX_VISIBILITY_DEPTH);

    // Round up, if applicable:
    if ((int16_t) (shading_ref.y + (i - upper_left.x) *
        shading_gradient) % MAX_VISIBILITY_DEPTH >= MAX_VISIBILITY_DEPTH /
        2 + MAX_VISIBILITY_DEPTH % 2)
    {
      shading_offset++;
    }
    half_shading_offset = (shading_offset / 2) + (shading_offset % 2);

    // Now, draw points from top to bottom:
    for (j = upper_left.y + (i - upper_left.x) * shading_gradient;
         j < lower_left.y - (i - upper_left.x) * shading_gradient;
         ++j)
    {
      if ((j + (int16_t) ((i - upper_left.x) * shading_gradient) +
          (i % 2 == 0 ? 0 : half_shading_offset)) % shading_offset == 0)
      {
        graphics_context_set_stroke_color(ctx, GColorWhite);
      }
      else
      {
        graphics_context_set_stroke_color(ctx, GColorBlack);
      }
      graphics_draw_pixel(ctx, GPoint(i, j));
    }
  }
}

/******************************************************************************
   Function: draw_status_bar

Description: Draws the lower status bar.

     Inputs: ctx - Pointer to the relevant graphics context.

    Outputs: None.
******************************************************************************/
void draw_status_bar(GContext *ctx)
{
  // Health meter:
  draw_status_meter(ctx,
                    GPoint (STATUS_METER_PADDING,
                            GRAPHICS_FRAME_HEIGHT + STATUS_METER_PADDING),
                    (float) g_player->stats[CURRENT_HEALTH] /
                      g_player->stats[MAX_HEALTH]);

  // Energy meter:
  draw_status_meter(ctx,
                    GPoint (SCREEN_CENTER_POINT_X + STATUS_METER_PADDING +
                              COMPASS_RADIUS + 1,
                            GRAPHICS_FRAME_HEIGHT + STATUS_METER_PADDING),
                    (float) g_player->stats[CURRENT_ENERGY] /
                      g_player->stats[MAX_ENERGY]);

  // Compass:
  graphics_fill_circle(ctx,
                       GPoint(SCREEN_CENTER_POINT_X,
                              GRAPHICS_FRAME_HEIGHT + STATUS_BAR_HEIGHT / 2),
                       COMPASS_RADIUS);
  graphics_context_set_fill_color(ctx, GColorBlack);
  gpath_draw_outline(ctx, g_compass_path);
  gpath_draw_filled(ctx, g_compass_path);
}

/******************************************************************************
   Function: draw_status_meter

Description: Draws a "status meter" (such as a "health meter") at a given point
             according to given max. and current values of the attribute to be
             represented.

     Inputs: ctx    - Pointer to the relevant graphics context.
             origin - Top-left corner of the status meter.
             ratio  - Ratio of "current value" / "max. value" for the attribute
                      to be represented.

    Outputs: None.
******************************************************************************/
void draw_status_meter(GContext *ctx,
                       const GPoint origin,
                       const float ratio)
{
  uint8_t i, j;

  graphics_context_set_stroke_color(ctx, GColorBlack);
  graphics_context_set_fill_color(ctx, GColorWhite);

  // First, draw a "full" meter:
  graphics_fill_rect(ctx,
                     GRect(origin.x,
                           origin.y,
                           STATUS_METER_WIDTH,
                           STATUS_METER_HEIGHT),
                     SMALL_CORNER_RADIUS,
                     GCornersAll);

  // Now shade the "empty" portion:
  for (i = origin.x + STATUS_METER_WIDTH;
       i >= origin.x + (ratio * STATUS_METER_WIDTH);
       --i)
  {
    for (j = origin.y + (i % 2);
         j <= origin.y + STATUS_METER_HEIGHT;
         j += 2)
    {
      graphics_draw_pixel(ctx, GPoint(i, j));
    }
  }
}

/******************************************************************************
   Function: draw_shaded_ellipse

Description: Draws a shaded ellipse according to given specifications.

     Inputs: ctx         - Pointer to the relevant graphics context.
             center      - Central coordinates of the ellipse (with respect to
                           the graphics frame).
             h_radius    - Horizontal radius.
             v_radius    - Vertical radius.
             shading_ref - Reference coordinates to assist in determining
                           shading offset values for the quad's location in
                           the 3D environment. (For walls, this is the same as
                           "upper_left".)

    Outputs: None.
******************************************************************************/
/*void draw_shaded_ellipse(GContext *ctx,
                         const GPoint center,
                         const int8_t h_radius,
                         const int8_t v_radius,
                         const GPoint shading_ref)
{
  int32_t theta;
  int8_t i, j, shading_offset;

  // Determine the shading offset:
  shading_offset = 1 + shading_ref.y / MAX_VISIBILITY_DEPTH;
  if (shading_ref.y % MAX_VISIBILITY_DEPTH >= MAX_VISIBILITY_DEPTH / 2 +
      MAX_VISIBILITY_DEPTH % 2)
  {
    shading_offset++;
  }

  // Proceed from left to the center, and from 0 to 90 degrees:
  for (theta = 0, i = h_radius;
       theta < NINETY_DEGREES && i <= 0 && i < GRAPHICS_FRAME_WIDTH;
       theta += DEFAULT_ROTATION_RATE, ++i)
  {
    // Draw points from top to bottom:
    for (j = center.y - sin_lookup(theta) * v_radius / TRIG_MAX_RATIO;
         j < center.y + sin_lookup(theta) * v_radius / TRIG_MAX_RATIO;
         ++j)
    {
      if (j % shading_offset == 0)
      {
        graphics_context_set_stroke_color(ctx, GColorWhite);
      }
      else
      {
        graphics_context_set_stroke_color(ctx, GColorBlack);
      }
      graphics_draw_pixel(ctx, GPoint(center.x - i, j));
      graphics_draw_pixel(ctx, GPoint(center.x + i, j));
    }
  }
}*/

/******************************************************************************
   Function: draw_shaded_sphere

Description: Draws a shaded sphere according to given specifications.

     Inputs: ctx            - Pointer to the relevant graphics context.
             center         - The sphere's center point.
             radius         - The sphere's radius.
             shading_offset - Shading offset at the sphere's position relative
                              to the player.

    Outputs: "True" unless the sphere is located entirely off-screen.
******************************************************************************/
/*bool draw_shaded_sphere(GContext *ctx,
                        const GPoint center,
                        const int8_t radius,
                        int8_t shading_offset)
{
  int32_t theta;
  int8_t i, x_offset, y_offset;

  if (center.x + radius < 0 ||
      center.x - radius >= GRAPHICS_FRAME_WIDTH ||
      center.y + radius < 0 ||
      center.y - radius >= GRAPHICS_FRAME_HEIGHT)
  {
    return false;
  }

  graphics_context_set_stroke_color(ctx, GColorBlack);
  graphics_context_set_fill_color(ctx, GColorWhite);

  // First, draw a white circle:
  graphics_fill_circle(ctx, center, radius);

  // Now add shading:
  for (i = radius;
       i > radius / 2;
       i -= 2)
  {
    for (theta = i % 2 ? 0 : ((TRIG_MAX_RATIO / 360) * shading_offset) / 2;
         theta < NINETY_DEGREES;
         theta += (TRIG_MAX_RATIO / 360) * (i > radius - radius / 4 ?
                                            shading_offset :
                                            shading_offset * 2))
    {
      x_offset = cos_lookup(theta) * i / TRIG_MAX_RATIO;
      y_offset = sin_lookup(theta) * i / TRIG_MAX_RATIO;
      graphics_draw_pixel(ctx,
                          GPoint(center.x - x_offset, center.y - y_offset));
      graphics_draw_pixel(ctx,
                          GPoint(center.x + x_offset, center.y - y_offset));
      graphics_draw_pixel(ctx,
                          GPoint(center.x - x_offset, center.y + y_offset));
      graphics_draw_pixel(ctx,
                          GPoint(center.x + x_offset, center.y + y_offset));
    }
  }

  return true;
}*/

/******************************************************************************
   Function: fill_ellipse

Description: Draws a filled ellipse according to given specifications.

     Inputs: ctx      - Pointer to the relevant graphics context.
             center   - Central coordinates of the ellipse (with respect to the
                        graphics frame).
             h_radius - Horizontal radius.
             v_radius - Vertical radius.
             color    - Desired color ("GColorBlack" or "GColorWhite").

    Outputs: None.
******************************************************************************/
void fill_ellipse(GContext *ctx,
                  const GPoint center,
                  const int8_t h_radius,
                  const int8_t v_radius,
                  const GColor color)
{
  int16_t theta;
  uint8_t x_offset, y_offset;

  graphics_context_set_stroke_color(ctx, color);
  for (theta = 0; theta < NINETY_DEGREES; theta += DEFAULT_ROTATION_RATE)
  {
    x_offset = cos_lookup(theta) * h_radius / TRIG_MAX_RATIO;
    y_offset = sin_lookup(theta) * v_radius / TRIG_MAX_RATIO;
    graphics_draw_line(ctx,
                       GPoint(center.x - x_offset, center.y - y_offset),
                       GPoint(center.x + x_offset, center.y - y_offset));
    graphics_draw_line(ctx,
                       GPoint(center.x - x_offset, center.y + y_offset),
                       GPoint(center.x + x_offset, center.y + y_offset));
  }
}

/******************************************************************************
   Function: draw_ellipse

Description: Draws the outline of an ellipse according to given specifications.

     Inputs: ctx      - Pointer to the relevant graphics context.
             center   - Central coordinates of the ellipse (with respect to the
                        graphics frame).
             h_radius - Horizontal radius.
             v_radius - Vertical radius.

    Outputs: None.
******************************************************************************/
/*void draw_ellipse(GContext *ctx,
                  const GPoint center,
                  const int8_t h_radius,
                  const int8_t v_radius)
{
  int32_t theta;
  int8_t x_offset, y_offset;

  for (theta = 0; theta < NINETY_DEGREES; theta += DEFAULT_ROTATION_RATE)
  {
    x_offset = cos_lookup(theta) * h_radius / TRIG_MAX_RATIO;
    y_offset = sin_lookup(theta) * v_radius / TRIG_MAX_RATIO;
    graphics_draw_pixel(ctx, GPoint(center.x - x_offset, center.y - y_offset));
    graphics_draw_pixel(ctx, GPoint(center.x + x_offset, center.y - y_offset));
    graphics_draw_pixel(ctx, GPoint(center.x - x_offset, center.y + y_offset));
    graphics_draw_pixel(ctx, GPoint(center.x + x_offset, center.y + y_offset));
  }
}*/

/******************************************************************************
   Function: flash_screen

Description: Briefly "flashes" the graphics frame by inverting all its pixels.

     Inputs: None.

    Outputs: None.
******************************************************************************/
void flash_screen(void)
{
  layer_set_hidden(inverter_layer_get_layer(g_inverter_layer), false);
  g_flash_timer = app_timer_register(DEFAULT_TIMER_DURATION,
                                     flash_timer_callback,
                                     NULL);
}

/******************************************************************************
   Function: flash_timer_callback

Description: Called when the flash timer reaches zero. Hides the inverter
             layer, ending the "flash."

     Inputs: data - Pointer to additional data (not used).

    Outputs: None.
******************************************************************************/
static void flash_timer_callback(void *data)
{
  layer_set_hidden(inverter_layer_get_layer(g_inverter_layer), true);
}

/******************************************************************************
   Function: attack_timer_callback

Description: Called when the attack timer reaches zero.

     Inputs: data - Pointer to additional data (not used).

    Outputs: None.
******************************************************************************/
static void attack_timer_callback(void *data)
{
  g_player_is_attacking = false;
  layer_mark_dirty(window_get_root_layer(g_windows[GRAPHICS_WINDOW]));
}

/******************************************************************************
   Function: graphics_window_appear

Description: Called when the graphics window appears.

     Inputs: window - Pointer to the graphics window.

    Outputs: None.
******************************************************************************/
static void graphics_window_appear(Window *window)
{
  layer_set_hidden(inverter_layer_get_layer(g_inverter_layer), true);
  g_player_is_attacking = false;
  g_current_window      = GRAPHICS_WINDOW;
}

/******************************************************************************
   Function: main_menu_appear

Description: Called when the main menu appears.

     Inputs: window - Pointer to the main menu window.

    Outputs: None.
******************************************************************************/
static void main_menu_appear(Window *window)
{
  g_current_window = MAIN_MENU;
}

/******************************************************************************
   Function: graphics_up_single_repeating_click

Description: The graphics window's single repeating click handler for the "up"
             button. Moves the player one cell forward.

     Inputs: recognizer - The click recognizer.
             context    - Pointer to the associated context.

    Outputs: None.
******************************************************************************/
void graphics_up_single_repeating_click(ClickRecognizerRef recognizer,
                                        void *context)
{
  //Window *window = (Window *) context;

  if (g_current_window == GRAPHICS_WINDOW)
  {
    move_player(g_player->direction);
  }
}

/******************************************************************************
   Function: graphics_up_multi_click

Description: The graphics window's multi-click handler for the "up" button.
             Turns the player to the left.

     Inputs: recognizer - The click recognizer.
             context    - Pointer to the associated context.

    Outputs: None.
******************************************************************************/
void graphics_up_multi_click(ClickRecognizerRef recognizer, void *context)
{
  //Window *window = (Window *) context;

  if (g_current_window == GRAPHICS_WINDOW)
  {
    set_player_direction(get_direction_to_the_left(g_player->direction));
  }
}

/******************************************************************************
   Function: graphics_down_single_repeating_click

Description: The graphics window's single repeating click handler for the
             "down" button. Moves the player one cell backward.

     Inputs: recognizer - The click recognizer.
             context    - Pointer to the associated context.

    Outputs: None.
******************************************************************************/
void graphics_down_single_repeating_click(ClickRecognizerRef recognizer,
                                          void *context)
{
  //Window *window = (Window *) context;

  if (g_current_window == GRAPHICS_WINDOW)
  {
    move_player(get_opposite_direction(g_player->direction));
  }
}

/******************************************************************************
   Function: graphics_down_multi_click

Description: The graphics window's multi-click handler for the "down" button.
             Turns the player to the right.

     Inputs: recognizer - The click recognizer.
             context    - Pointer to the associated context.

    Outputs: None.
******************************************************************************/
void graphics_down_multi_click(ClickRecognizerRef recognizer, void *context)
{
  //Window *window = (Window *) context;

  if (g_current_window == GRAPHICS_WINDOW)
  {
    set_player_direction(get_direction_to_the_right(g_player->direction));
  }
}

/******************************************************************************
   Function: graphics_select_single_repeating_click

Description: The graphics window's single repeating click handler for the
             "select" button button. Activate's the player's current attack or
             spell.

     Inputs: recognizer - The click recognizer.
             context    - Pointer to the associated context.

    Outputs: None.
******************************************************************************/
void graphics_select_single_repeating_click(ClickRecognizerRef recognizer,
                                            void *context)
{
  GPoint cell;
  npc_t *npc;
  heavy_item_t *weapon = get_heavy_item_equipped_at(RIGHT_HAND);
  //Window *window = (Window *) context;

  if (g_current_window == GRAPHICS_WINDOW &&
      g_player->stats[CURRENT_ENERGY] >= g_player->energy_loss_per_action)
  {
    adjust_player_current_energy(g_player->energy_loss_per_action * -1);

    // Check for a targeted NPC:
    cell = get_cell_farther_away(g_player->position, g_player->direction, 1);
    while (get_cell_type(cell) >= EMPTY)
    {
      npc = get_npc_at(cell);

      // If we've found an NPC or the attack isn't ranged, we're done:
      if (npc || g_player->equipped_pebble == NONE)
      {
        break;
      }
      cell = get_cell_farther_away(cell, g_player->direction, 1);
    }

    // If a Pebble is equipped, cast a spell:
    if (g_player->equipped_pebble > NONE)
    {
      flash_screen();
      cast_spell_on_npc(npc,
                        g_player->equipped_pebble,
                        g_player->stats[MAGICAL_POWER]);
    }

    // Otherwise, the player is attacking with a physical weapon:
    else
    {
      if (npc)
      {
        damage_npc(npc,
                   g_player->stats[PHYSICAL_POWER] -
                     (npc->physical_defense - npc->status_effects[BURNED]));
      }

      if (weapon)
      {
        // Check for WOUNDED/STUNNED effect from sharp/blunt weapons:
        if (npc && rand() % g_player->stats[PHYSICAL_POWER] >
                     rand() % npc->physical_defense)
        {
          npc->status_effects[weapon->type % 2 ? WOUNDED : STUNNED] +=
            g_player->stats[PHYSICAL_POWER] / 2;
        }

        // Check for an infused Pebble:
        if (weapon->infused_pebble > NONE)
        {
          flash_screen();
          cast_spell_on_npc(npc,
                            weapon->infused_pebble,
                            g_player->stats[MAGICAL_POWER] / 2);
        }
      }

      // Set up the "attack slash" graphic:
      g_player_is_attacking = true;
      g_attack_slash_x1     = rand() % (GRAPHICS_FRAME_WIDTH / 3) +
                                GRAPHICS_FRAME_WIDTH / 3;
      g_attack_slash_x2     = rand() % (GRAPHICS_FRAME_WIDTH / 3) +
                                GRAPHICS_FRAME_WIDTH / 3;
      g_attack_slash_y1     = rand() % (GRAPHICS_FRAME_HEIGHT / 3) +
                                STATUS_BAR_HEIGHT;
      g_attack_slash_y2     = GRAPHICS_FRAME_HEIGHT - STATUS_BAR_HEIGHT -
                                rand() % (GRAPHICS_FRAME_HEIGHT / 3);
      g_attack_timer        = app_timer_register(DEFAULT_TIMER_DURATION,
                                                 attack_timer_callback,
                                                 NULL);
    }

    layer_mark_dirty(window_get_root_layer(g_windows[GRAPHICS_WINDOW]));
  }
}

/******************************************************************************
   Function: graphics_click_config_provider

Description: Button-click configuration provider for the graphics window.

     Inputs: context - Pointer to the associated context.

    Outputs: None.
******************************************************************************/
void graphics_click_config_provider(void *context)
{
  //Window *window = (Window *) context;

  // "Up" button:
  window_single_repeating_click_subscribe(BUTTON_ID_UP,
                                          PLAYER_ACTION_REPEAT_INTERVAL,
                                          graphics_up_single_repeating_click);
  window_multi_click_subscribe(BUTTON_ID_UP,
                               MULTI_CLICK_MIN,
                               MULTI_CLICK_MAX,
                               MULTI_CLICK_TIMEOUT,
                               LAST_CLICK_ONLY,
                               graphics_up_multi_click);

  // "Down" button:
  window_single_repeating_click_subscribe(BUTTON_ID_DOWN,
                                         PLAYER_ACTION_REPEAT_INTERVAL,
                                         graphics_down_single_repeating_click);
  window_multi_click_subscribe(BUTTON_ID_DOWN,
                               MULTI_CLICK_MIN,
                               MULTI_CLICK_MAX,
                               MULTI_CLICK_TIMEOUT,
                               LAST_CLICK_ONLY,
                               graphics_down_multi_click);

  // "Select" button:
  window_single_repeating_click_subscribe(BUTTON_ID_SELECT,
                                       PLAYER_ACTION_REPEAT_INTERVAL,
                                       graphics_select_single_repeating_click);
}

/******************************************************************************
   Function: narration_single_click

Description: The narration window's single-click handler for all buttons. Shows
             the next narration, if any, or hides the narration window.

     Inputs: recognizer - The click recognizer.
             context    - Pointer to the associated context.

    Outputs: None.
******************************************************************************/
void narration_single_click(ClickRecognizerRef recognizer, void *context)
{
  //Window *window = (Window *) context;

  if (g_current_narration < INTRO_NARRATION_4 ||
      (g_current_narration > INTRO_NARRATION_4 &&
       g_current_narration < STATS_NARRATION_2))
  {
    show_narration(++g_current_narration);
  }
  else
  {
    window_stack_pop(NOT_ANIMATED);
  }
}

/******************************************************************************
   Function: narration_click_config_provider

Description: Button-click configurations for the narration window.

     Inputs: context - Pointer to the associated context.

    Outputs: None.
******************************************************************************/
void narration_click_config_provider(void *context)
{
  window_single_click_subscribe(BUTTON_ID_SELECT, narration_single_click);
  window_single_click_subscribe(BUTTON_ID_UP, narration_single_click);
  window_single_click_subscribe(BUTTON_ID_DOWN, narration_single_click);
  window_single_click_subscribe(BUTTON_ID_BACK, narration_single_click);
}

/******************************************************************************
   Function: tick_handler

Description: Handles changes to the game world every second while in active
             gameplay.

     Inputs: tick_time     - Pointer to the relevant time struct.
             units_changed - Indicates which time unit changed.

    Outputs: None.
******************************************************************************/
static void tick_handler(struct tm *tick_time, TimeUnits units_changed)
{
  int8_t i, j;

  if (g_current_window == GRAPHICS_WINDOW)
  {
    // Check for a "level up":
    if (g_player->exp_points / POINTS_PER_LEVEL >= g_player->level)
    {
      g_player->level++;
      show_window(LEVEL_UP_MENU, NOT_ANIMATED);
      show_narration(LEVEL_UP_NARRATION);
    }

    // Handle NPC behavior:
    for (i = 0; i < MAX_NPCS_AT_ONE_TIME; ++i)
    {
      if (time(NULL) % 2 && g_location->npcs[i].type > NONE)
      {
        determine_npc_behavior(&g_location->npcs[i]);

        // Check for player death:
        if (g_player->stats[CURRENT_HEALTH] <= 0)
        {
          return;
        }

        // Apply wounding damage:
        if (g_location->npcs[i].status_effects[WOUNDED])
        {
          damage_npc(&g_location->npcs[i],
                     g_location->npcs[i].status_effects[WOUNDED]);
        }

        // Reduce all status effects:
        for (j = 0; j < NUM_STATUS_EFFECTS; ++j)
        {
          g_location->npcs[i].status_effects[j] /= 2;
        }
      }
    }

    // Generate new NPCs periodically (does nothing if the NPC array is full):
    if (rand() % 10 == 0)
    {
      // This may create any NPC type except MAGE:
      add_new_npc(rand() % (NUM_NPC_TYPES - 1), get_npc_spawn_point());
    }

    // Handle player stat recovery:
    adjust_player_current_health(g_player->stats[HEALTH_REGEN]);
    adjust_player_current_energy(g_player->stats[ENERGY_REGEN]);

    layer_mark_dirty(window_get_root_layer(g_windows[GRAPHICS_WINDOW]));
  }
}

/******************************************************************************
   Function: app_focus_handler

Description: Handles PebbleQuest going out of, or coming back into, focus
             (e.g., when a notification window temporarily hides this app).

     Inputs: in_focus - "True" if PebbleQuest is now in focus.

    Outputs: None.
******************************************************************************/
void app_focus_handler(const bool in_focus)
{
  if (!in_focus && g_current_window == GRAPHICS_WINDOW)
  {
    show_window(MAIN_MENU, NOT_ANIMATED);
  }
}

/******************************************************************************
   Function: strcat_item_name

Description: Concatenates the name of a given item onto a given string.

     Inputs: dest_str  - Pointer to the destination string.
             item_type - Integer representing the item of interest.

    Outputs: None.
******************************************************************************/
void strcat_item_name(char *const dest_str, const int8_t item_type)
{
  if (item_type < FIRST_HEAVY_ITEM)
  {
    strcat(dest_str, "P.");
    strcat_magic_type(dest_str, item_type);
  }
  else
  {
    switch(item_type)
    {
      case DAGGER:
        strcat(dest_str, "Dagger");
        break;
      case STAFF:
        strcat(dest_str, "Staff");
        break;
      case SWORD:
        strcat(dest_str, "Sword");
        break;
      case MACE:
        strcat(dest_str, "Mace");
        break;
      case AXE:
        strcat(dest_str, "Axe");
        break;
      case FLAIL:
        strcat(dest_str, "Flail");
        break;
      case SHIELD:
        strcat(dest_str, "Shield");
        break;
      case ROBE:
        strcat(dest_str, "Robe");
        break;
      case LIGHT_ARMOR:
        strcat(dest_str, "L. Armor");
        break;
      case HEAVY_ARMOR:
        strcat(dest_str, "H. Armor");
        break;
    }
  }
}

/******************************************************************************
   Function: strcat_magic_type

Description: Concatenates the name of a given magic type onto a given string.

     Inputs: dest_str   - Pointer to the destination string.
             magic_type - Integer representing the desired magic type.

    Outputs: None.
******************************************************************************/
void strcat_magic_type(char *const dest_str, const int8_t magic_type)
{
  strcat(dest_str, " of ");
  switch(magic_type)
  {
    case PEBBLE_OF_FIRE:
      strcat(dest_str, "Fire");
      break;
    case PEBBLE_OF_ICE:
      strcat(dest_str, "Ice");
      break;
    case PEBBLE_OF_LIGHTNING:
      strcat(dest_str, "Lightning");
      break;
    case PEBBLE_OF_LIFE:
      strcat(dest_str, "Life");
      break;
    case PEBBLE_OF_DEATH:
      strcat(dest_str, "Death");
      break;
    case PEBBLE_OF_LIGHT:
      strcat(dest_str, "Light");
      break;
    case PEBBLE_OF_DARKNESS:
      strcat(dest_str, "Darkness");
      break;
  }
}

/******************************************************************************
   Function: strcat_stat_name

Description: Concatenates the name of a given stat onto a given string.

     Inputs: dest_str - Pointer to the destination string.
             stat     - Integer representing the stat of interest.

    Outputs: None.
******************************************************************************/
void strcat_stat_name(char *const dest_str, const int8_t stat)
{
  switch(stat)
  {
    case STRENGTH:
      strcat(dest_str, "Strength");
      break;
    case AGILITY:
      strcat(dest_str, "Agility");
      break;
    case INTELLECT:
      strcat(dest_str, "Intellect");
      break;
    case MAX_HEALTH:
      strcat(dest_str, "Health");
      break;
    case MAX_ENERGY:
      strcat(dest_str, "Energy");
      break;
    case PHYSICAL_POWER:
      strcat(dest_str, "Ph. Power");
      break;
    case PHYSICAL_DEFENSE:
      strcat(dest_str, "Ph. Defense");
      break;
    case MAGICAL_POWER:
      strcat(dest_str, "M. Power");
      break;
    case MAGICAL_DEFENSE:
      strcat(dest_str, "M. Defense");
      break;
  }
  strcat(dest_str, ": ");
}

/******************************************************************************
   Function: strcat_stat_value

Description: Concatenates the value of a given stat onto a given string. (This
             function was created because of the anomalous cases of MAX_HEALTH
             and MAX_ENERGY, wherein the current health/energy and a "slash"
             are also concatenated.)

     Inputs: dest_str - Pointer to the destination string.
             stat     - Integer representing the stat of interest.

    Outputs: None.
******************************************************************************/
void strcat_stat_value(char *const dest_str, const int8_t stat)
{
  if (stat == MAX_HEALTH || stat == MAX_ENERGY)
  {
    strcat_int(dest_str,
               g_player->stats[stat + (CURRENT_HEALTH - MAX_HEALTH)]);
    strcat(dest_str, "/");
  }
  strcat_int(dest_str, g_player->stats[stat]);
}

/******************************************************************************
   Function: strcat_int

Description: Concatenates a "small" integer value onto the end of a string. The
             absolute value of the integer may not exceed MAX_INT_VALUE (if it
             does, MAX_INT_VALUE will be used in its place). If the integer is
             negative, a minus sign will be included.

     Inputs: dest_str - Pointer to the destination string.
             integer  - Integer value to be converted to characters and
                        appended to the string.

    Outputs: None.
******************************************************************************/
void strcat_int(char *const dest_str, int16_t integer)
{
  int8_t i, j;
  static char int_str[MAX_INT_DIGITS + 1];
  bool negative = false;

  int_str[0] = '\0';
  if (integer < 0)
  {
    negative = true;
    integer  *= -1;
  }
  if (integer > MAX_INT_VALUE)
  {
    integer = MAX_INT_VALUE;
  }
  if (integer == 0)
  {
    strcpy(int_str, "0");
  }
  else
  {
    for (i = 0; integer != 0; integer /= 10)
    {
      j            = integer % 10;
      int_str[i++] = '0' + j;
    }
    int_str[i] = '\0';
  }

  i = strlen(dest_str) + strlen(int_str);
  if (negative)
  {
    ++i;
  }
  dest_str[i--] = '\0';
  j             = 0;
  while (int_str[j] != '\0')
  {
    dest_str[i--] = int_str[j++];
  }
  if (negative)
  {
    dest_str[i--] = '-';
  }
}

/******************************************************************************
   Function: add_current_selection_to_inventory

Description: Adds an item of type "g_current_selection" to the player's
             inventory then displays the inventory menu to provide an
             opportunity to infuse items and adjust equipment. If it's a heavy
             item and there's no more room for heavy items, the "replace item"
             menu will be shown.

     Inputs: None.

    Outputs: None.
******************************************************************************/
void add_current_selection_to_inventory(void)
{
  int8_t i;

  // Pebbles:
  if (g_current_selection < FIRST_HEAVY_ITEM)
  {
    g_player->pebbles[g_current_selection]++;
    g_current_selection = get_inventory_row_for_pebble(g_current_selection);
    show_window(INVENTORY_MENU, NOT_ANIMATED);
  }

  // Heavy items:
  else
  {
    for (i = 0; i < MAX_HEAVY_ITEMS; ++i)
    {
      if (g_player->heavy_items[i].type == NONE)
      {
        init_heavy_item(&g_player->heavy_items[i], g_current_selection);
        g_current_selection = i + get_num_pebble_types_owned();
        show_window(INVENTORY_MENU, NOT_ANIMATED);

        return;
      }
    }

    // If we reach this point, the player's heavy item array is full:
    show_window(HEAVY_ITEMS_MENU, ANIMATED);
  }
}

/******************************************************************************
   Function: equip_heavy_item

Description: Equips a given heavy item to its appropriate equip target,
             unequipping the previously equipped item (if any), then adjusts
             constant status effects and minor stats accordingly.

     Inputs: heavy_item - Pointer to the heavy item to be equipped.

    Outputs: None.
******************************************************************************/
void equip_heavy_item(heavy_item_t *const heavy_item)
{
  unequip_item_at(heavy_item->equip_target);
  heavy_item->equipped = true;
  if (heavy_item->equip_target < RIGHT_HAND &&
      heavy_item->infused_pebble > NONE)
  {
    g_player->stats[heavy_item->infused_pebble]++;
  }
  adjust_minor_stats();
}

/******************************************************************************
   Function: unequip_item_at

Description: Unequips the equipped item (if any) at a given equip target, then
             adjusts constant status effects and minor stats accordingly.

     Inputs: equip_target - Integer representing the equip target (BODY,
                            LEFT_HAND, or RIGHT_HAND) that is to be emptied.

    Outputs: None.
******************************************************************************/
void unequip_item_at(const int8_t equip_target)
{
  heavy_item_t *heavy_item = get_heavy_item_equipped_at(equip_target);

  if (equip_target == RIGHT_HAND)
  {
    g_player->equipped_pebble = NONE;
  }
  if (heavy_item)
  {
    heavy_item->equipped = false;
    if (heavy_item->equip_target < RIGHT_HAND &&
        heavy_item->infused_pebble > NONE)
    {
      g_player->stats[heavy_item->infused_pebble]--;
    }
  }
  adjust_minor_stats();
}

/******************************************************************************
   Function: adjust_minor_stats

Description: Assigns values to the player's minor stats according to major stat
             values (STRENGTH, AGILITY, and INTELLECT) then adjusts them
             according to equipped items and status effects.

     Inputs: None.

    Outputs: None.
******************************************************************************/
void adjust_minor_stats(void)
{
  int8_t i;

  // Set minor stats according to major stats:
  g_player->stats[MAX_HEALTH]       = g_player->stats[STRENGTH]  * 10;
  g_player->stats[MAX_ENERGY]       = g_player->stats[AGILITY] * 4 +
                                        g_player->stats[STRENGTH] * 3 +
                                        g_player->stats[INTELLECT] * 3;
  g_player->stats[PHYSICAL_POWER]   = g_player->stats[STRENGTH] +
                                        g_player->stats[AGILITY] / 2;
  g_player->stats[PHYSICAL_DEFENSE] = g_player->stats[AGILITY] +
                                        g_player->stats[STRENGTH] / 2;
  g_player->stats[MAGICAL_POWER]    = g_player->stats[INTELLECT] +
                                        g_player->stats[AGILITY] / 2;
  g_player->stats[MAGICAL_DEFENSE]  = g_player->stats[AGILITY] +
                                        g_player->stats[INTELLECT] / 2;
  g_player->energy_loss_per_action  = MIN_ENERGY_LOSS_PER_ACTION;

  // Weapon (or equipped Pebble):
  if (get_heavy_item_equipped_at(RIGHT_HAND))
  {
    // Weapons:
    for (i = DAGGER;
         i <= get_heavy_item_equipped_at(RIGHT_HAND)->type;
         i += 2)
    {
      g_player->stats[PHYSICAL_POWER]++;
      g_player->energy_loss_per_action++;
    }
  }
  else if (g_player->equipped_pebble > NONE)
  {
    g_player->energy_loss_per_action += g_player->stats[INTELLECT] / 3;
  }

  // Armor:
  if (get_heavy_item_equipped_at(BODY))
  {
    if (get_heavy_item_equipped_at(BODY)->type == LIGHT_ARMOR)
    {
      g_player->stats[PHYSICAL_DEFENSE]++;
      g_player->stats[MAGICAL_POWER]--;
    }
    else if (get_heavy_item_equipped_at(BODY)->type == HEAVY_ARMOR)
    {
      g_player->stats[PHYSICAL_DEFENSE] += 2;
      g_player->stats[MAGICAL_POWER]    -= 2;
      g_player->energy_loss_per_action++;
    }
  }

  // Shield:
  if (get_heavy_item_equipped_at(LEFT_HAND))
  {
    g_player->stats[PHYSICAL_DEFENSE]++;
    g_player->stats[MAGICAL_POWER]--;
  }
}

/******************************************************************************
   Function: init_player

Description: Initializes the global player character struct according to
             default values.

     Inputs: None.

    Outputs: None.
******************************************************************************/
void init_player(void)
{
  int8_t i;

  g_player->level           = 1;
  g_player->exp_points      = 0;
  g_player->depth           = 0;
  g_player->equipped_pebble = NONE;
  for (i = 0; i < NUM_PEBBLE_TYPES; ++i)
  {
    g_player->pebbles[i] = 0;
  }
  for (i = 0; i < NUM_MAJOR_STATS; ++i)
  {
    g_player->stats[i] = DEFAULT_MAJOR_STAT_VALUE;
  }
  g_player->stats[HEALTH_REGEN]    = g_player->stats[ENERGY_REGEN]     = 1;
  g_player->stats[BACKLASH_DAMAGE] = g_player->stats[SPELL_ABSORPTION] = 0;

  // Add starting inventory items:
  init_heavy_item(&g_player->heavy_items[0], DAGGER);
  init_heavy_item(&g_player->heavy_items[1], STAFF);
  init_heavy_item(&g_player->heavy_items[2], ROBE);
  equip_heavy_item(&g_player->heavy_items[0]);
  equip_heavy_item(&g_player->heavy_items[2]);

  // Assign minor stats, then ensure health and energy are at 100%:
  adjust_minor_stats();
  g_player->stats[CURRENT_HEALTH] = g_player->stats[MAX_HEALTH];
  g_player->stats[CURRENT_ENERGY] = g_player->stats[MAX_ENERGY];
}

/******************************************************************************
   Function: init_npc

Description: Initializes a non-player character (NPC) struct according to a
             given NPC type.

     Inputs: npc      - Pointer to the NPC struct to be initialized.
             type     - Integer indicating the desired NPC type.
             position - The NPC's starting position.

    Outputs: None.
******************************************************************************/
void init_npc(npc_t *const npc, const int8_t type, const GPoint position)
{
  int8_t i;

  npc->type     = type;
  npc->position = position;
  npc->item     = NONE;
  for (i = 0; i < NUM_STATUS_EFFECTS; ++i)
  {
    npc->status_effects[i] = 0;
  }

  // Set major stats according to current dungeon depth:
  npc->health = npc->power = npc->physical_defense = npc->magical_defense =
    g_player->depth * 2;

  // Check for increased health:
  /*if (type == ORC        ||
      type == WARRIOR    ||
      type == BEAR       ||
      type == TROLL      ||
      type == OOZE ||
      type == MINOTAUR   ||
      type == MUMMY      ||
      type == DRAGON     ||
      type == VAMPIRE)
  {
    npc->health *= 2;
  }

  // Check for increased power:
  if (type == MAGE       ||
      type == ORC        ||
      type == WARRIOR    ||
      type == BEAR       ||
      type == TROLL      ||
      type == LIZARD_MAN ||
      type == MINOTAUR   ||
      type == MUMMY      ||
      type == DRAGON     ||
      type == VAMPIRE)
  {
    npc->power *= 2;
  }

  // Check for increased phys. defense (via armor or evasion):
  if (type == WARRIOR  ||
      type == THIEF   ||
      type == LIZARD_MAN ||
      type == IMP     ||
      type == DRAGON  ||
      type == VAMPIRE)
  {
    npc->physical_defense *= 2;
  }

  // Check for increased mag. defense:
  if (type == MAGE    ||
      type == VAMPIRE ||
      type == DRAGON  ||
      type == IMP     ||
      type == ELEMENTAL)
  {
    npc->magical_defense *= 2;
  }*/

  // Some NPCs may carry a random item:
  if (type == ORC        ||
      type == GOBLIN     ||
      type == LIZARD_MAN ||
      type == VAMPIRE    ||
      type == WARRIOR    ||
      type == THIEF)
  {
    npc->item = rand() % 2 ? NONE : RANDOM_ITEM; // Excludes Pebbles.
  }

  // Mages are the only source of Pebbles:
  else if (type == MAGE)
  {
    npc->item = rand() % NUM_PEBBLE_TYPES;
  }
}

/******************************************************************************
   Function: init_heavy_item

Description: Initializes a new heavy item struct according to a given type.

     Inputs: item - Pointer to the heavy item struct.
             type - The type of heavy item to be initialized.

    Outputs: None.
******************************************************************************/
void init_heavy_item(heavy_item_t *const item, const int8_t type)
{
  item->type           = type;
  item->infused_pebble = NONE;
  item->equipped       = false;
  if (type < SHIELD)
  {
    item->equip_target = RIGHT_HAND;
  }
  else if (type == SHIELD)
  {
    item->equip_target = LEFT_HAND;
  }
  else
  {
    item->equip_target = BODY;
  }
}

/******************************************************************************
   Function: init_wall_coords

Description: Initializes the global "back_wall_coords" array so that it
             contains the top-left and bottom-right coordinates for every
             potential back wall location on the screen. (This establishes the
             field of view and sense of perspective while also facilitating
             convenient drawing of the 3D environment.)

     Inputs: None.

    Outputs: None.
******************************************************************************/
void init_wall_coords(void)
{
  uint8_t i, j, wall_width;
  const float perspective_modifier = 2.0; // Helps determine FOV, etc.

  for (i = 0; i < MAX_VISIBILITY_DEPTH - 1; ++i)
  {
    for (j = 0; j < (STRAIGHT_AHEAD * 2) + 1; ++j)
    {
      g_back_wall_coords[i][j][TOP_LEFT]     = GPoint(0, 0);
      g_back_wall_coords[i][j][BOTTOM_RIGHT] = GPoint(0, 0);
    }
  }
  for (i = 0; i < MAX_VISIBILITY_DEPTH - 1; ++i)
  {
    g_back_wall_coords[i][STRAIGHT_AHEAD][TOP_LEFT] =
      GPoint(FIRST_WALL_OFFSET - i * perspective_modifier,
             FIRST_WALL_OFFSET - i * perspective_modifier);
    if (i > 0)
    {
      g_back_wall_coords[i][STRAIGHT_AHEAD][TOP_LEFT].x +=
        g_back_wall_coords[i - 1][STRAIGHT_AHEAD][TOP_LEFT].x;
      g_back_wall_coords[i][STRAIGHT_AHEAD][TOP_LEFT].y +=
        g_back_wall_coords[i - 1][STRAIGHT_AHEAD][TOP_LEFT].y;
    }
    g_back_wall_coords[i][STRAIGHT_AHEAD][BOTTOM_RIGHT].x =
      GRAPHICS_FRAME_WIDTH - g_back_wall_coords[i][STRAIGHT_AHEAD][TOP_LEFT].x;
    g_back_wall_coords[i][STRAIGHT_AHEAD][BOTTOM_RIGHT].y =
      GRAPHICS_FRAME_HEIGHT -
        g_back_wall_coords[i][STRAIGHT_AHEAD][TOP_LEFT].y;
    wall_width = g_back_wall_coords[i][STRAIGHT_AHEAD][BOTTOM_RIGHT].x -
                   g_back_wall_coords[i][STRAIGHT_AHEAD][TOP_LEFT].x;
    for (j = 1; j <= STRAIGHT_AHEAD; ++j)
    {
      g_back_wall_coords[i][STRAIGHT_AHEAD - j][TOP_LEFT]       =
        g_back_wall_coords[i][STRAIGHT_AHEAD][TOP_LEFT];
      g_back_wall_coords[i][STRAIGHT_AHEAD - j][TOP_LEFT].x     -= wall_width *
                                                                     j;
      g_back_wall_coords[i][STRAIGHT_AHEAD - j][BOTTOM_RIGHT]   =
        g_back_wall_coords[i][STRAIGHT_AHEAD][BOTTOM_RIGHT];
      g_back_wall_coords[i][STRAIGHT_AHEAD - j][BOTTOM_RIGHT].x -= wall_width *
                                                                     j;
      g_back_wall_coords[i][STRAIGHT_AHEAD + j][TOP_LEFT]       =
        g_back_wall_coords[i][STRAIGHT_AHEAD][TOP_LEFT];
      g_back_wall_coords[i][STRAIGHT_AHEAD + j][TOP_LEFT].x     += wall_width *
                                                                     j;
      g_back_wall_coords[i][STRAIGHT_AHEAD + j][BOTTOM_RIGHT]   =
        g_back_wall_coords[i][STRAIGHT_AHEAD][BOTTOM_RIGHT];
      g_back_wall_coords[i][STRAIGHT_AHEAD + j][BOTTOM_RIGHT].x += wall_width *
                                                                     j;
    }
  }
}

/******************************************************************************
   Function: init_location

Description: Initializes the global location struct, setting up a new location
             with an entrance, an exit, and a single NPC of type "MAGE".

     Inputs: None.

    Outputs: None.
******************************************************************************/
void init_location(void)
{
  int8_t i, j, builder_direction;
  GPoint builder_position;

  // Remove any preexisting NPCs:
  for (i = 0; i < MAX_NPCS_AT_ONE_TIME; ++i)
  {
    g_location->npcs[i].type = NONE;
  }

  // Now set each cell to solid:
  for (i = 0; i < MAP_WIDTH; ++i)
  {
    for (j = 0; j < MAP_HEIGHT; ++j)
    {
      g_location->map[i][j] = SOLID;
    }
  }

  // Next, set entrance and exit points:
  switch (builder_direction = rand() % NUM_DIRECTIONS)
  {
    case NORTH:
      builder_position = RANDOM_POINT_NORTH;
      set_cell_type(RANDOM_POINT_SOUTH, EXIT);
      break;
    case SOUTH:
      builder_position = RANDOM_POINT_SOUTH;
      set_cell_type(RANDOM_POINT_NORTH, EXIT);
      break;
    case EAST:
      builder_position = RANDOM_POINT_EAST;
      set_cell_type(RANDOM_POINT_WEST, EXIT);
      break;
    default: // case WEST:
      builder_position = RANDOM_POINT_WEST;
      set_cell_type(RANDOM_POINT_EAST, EXIT);
      break;
  }
  g_player->position = GPoint(builder_position.x, builder_position.y);
  set_player_direction(builder_direction);

  // Now carve a path between the entrance and exit points:
  while (get_cell_type(builder_position) != EXIT)
  {
    // Add random loot or simply make the cell EMPTY:
    if (rand() % 50 == 0)
    {
      set_cell_type(builder_position, RANDOM_ITEM); // Excludes Pebbles.
    }
    else
    {
      set_cell_type(builder_position, EMPTY);
    }

    // Move the builder:
    switch (builder_direction)
    {
      case NORTH:
        if (builder_position.y > 0)
        {
          builder_position.y--;
        }
        break;
      case SOUTH:
        if (builder_position.y < MAP_HEIGHT - 1)
        {
          builder_position.y++;
        }
        break;
      case EAST:
        if (builder_position.x < MAP_WIDTH - 1)
        {
          builder_position.x++;
        }
        break;
      default: // case WEST:
        if (builder_position.x > 0)
        {
          builder_position.x--;
        }
        break;
    }

    // 25% chance of turning:
    if (rand() % NUM_DIRECTIONS == 0)
    {
      builder_direction = rand() % NUM_DIRECTIONS;
    }
  }

  // Add an evil wizard at the exit point:
  add_new_npc(MAGE, builder_position);

  // Finally, set the entrance to ENTRANCE and increment the player's depth:
  set_cell_type(g_player->position, ENTRANCE);
  g_player->depth++;
}

/******************************************************************************
   Function: init_window

Description: Initializes the window at a given index of the "g_windows" array.

     Inputs: window_index - Integer indicating which window to initialize.

    Outputs: None.
******************************************************************************/
void init_window(const int8_t window_index)
{
  g_windows[window_index] = window_create();

  // Menu windows:
  if (window_index < NUM_MENUS)
  {
    g_menu_layers[window_index] = menu_layer_create(FULL_SCREEN_FRAME);
    layer_add_child(window_get_root_layer(g_windows[window_index]),
                    menu_layer_get_layer(g_menu_layers[window_index]));
    menu_layer_set_click_config_onto_window(g_menu_layers[window_index],
                                            g_windows[window_index]);

    // Main menu:
    if (window_index == MAIN_MENU)
    {
      window_set_window_handlers(g_windows[window_index], (WindowHandlers)
      {
        .appear = main_menu_appear,
      });
      menu_layer_set_callbacks(g_menu_layers[window_index],
                               NULL,
                               (MenuLayerCallbacks)
      {
        .get_header_height = menu_get_header_height_callback,
        .draw_header       = main_menu_draw_header_callback,
        .get_num_rows      = menu_get_num_rows_callback,
        .draw_row          = main_menu_draw_row_callback,
        .select_click      = menu_select_callback,
      });
    }

    // Inventory menu:
    else if (window_index == INVENTORY_MENU)
    {
      menu_layer_set_callbacks(g_menu_layers[window_index],
                               NULL,
                               (MenuLayerCallbacks)
      {
        .get_header_height = menu_get_header_height_callback,
        .draw_header       = inventory_menu_draw_header_callback,
        .get_num_rows      = menu_get_num_rows_callback,
        .draw_row          = inventory_menu_draw_row_callback,
        .select_click      = menu_select_callback,
      });
    }

    // Level-up menu:
    else if (window_index == LEVEL_UP_MENU)
    {
      menu_layer_set_callbacks(g_menu_layers[window_index],
                               NULL,
                               (MenuLayerCallbacks)
      {
        .get_header_height = menu_get_header_height_callback,
        .draw_header       = level_up_menu_draw_header_callback,
        .get_num_rows      = menu_get_num_rows_callback,
        .draw_row          = level_up_menu_draw_row_callback,
        .select_click      = menu_select_callback,
      });
    }

    // Loot menu:
    else if (window_index == LOOT_MENU)
    {
      menu_layer_set_callbacks(g_menu_layers[window_index],
                               NULL,
                               (MenuLayerCallbacks)
      {
        .get_header_height = menu_get_header_height_callback,
        .draw_header       = loot_menu_draw_header_callback,
        .get_num_rows      = menu_get_num_rows_callback,
        .draw_row          = loot_menu_draw_row_callback,
        .select_click      = menu_select_callback,
      });
    }

    // Pebble options menu:
    else if (window_index == PEBBLE_OPTIONS_MENU)
    {
      menu_layer_set_callbacks(g_menu_layers[window_index],
                               NULL,
                               (MenuLayerCallbacks)
      {
        .get_header_height = menu_get_header_height_callback,
        .draw_header       = pebble_options_menu_draw_header_callback,
        .get_num_rows      = menu_get_num_rows_callback,
        .draw_row          = pebble_options_menu_draw_row_callback,
        .select_click      = menu_select_callback,
      });
    }

    // Heavy items menu:
    else // if (window_index == HEAVY_ITEMS_MENU)
    {
      menu_layer_set_callbacks(g_menu_layers[window_index],
                               NULL,
                               (MenuLayerCallbacks)
      {
        .get_header_height = menu_get_header_height_callback,
        .draw_header       = heavy_items_menu_draw_header_callback,
        .get_num_rows      = menu_get_num_rows_callback,
        .draw_row          = heavy_items_menu_draw_row_callback,
        .select_click      = menu_select_callback,
      });
    }
  }

  // Narration window:
  else if (window_index == NARRATION_WINDOW)
  {
    window_set_background_color(g_windows[window_index], GColorBlack);
    window_set_click_config_provider(g_windows[window_index],
                                     narration_click_config_provider);
    g_narration_text_layer = text_layer_create(NARRATION_TEXT_LAYER_FRAME);
    text_layer_set_background_color(g_narration_text_layer, GColorBlack);
    text_layer_set_text_color(g_narration_text_layer, GColorWhite);
    text_layer_set_font(g_narration_text_layer, NARRATION_FONT);
    text_layer_set_text_alignment(g_narration_text_layer, GTextAlignmentLeft);
    layer_add_child(window_get_root_layer(g_windows[window_index]),
                    text_layer_get_layer(g_narration_text_layer));
  }

  // Graphics window:
  else // if (window_index == GRAPHICS_WINDOW)
  {
    window_set_background_color(g_windows[window_index], GColorBlack);
    window_set_window_handlers(g_windows[window_index], (WindowHandlers)
    {
      .appear = graphics_window_appear,
    });
    window_set_click_config_provider(g_windows[window_index],
                                     (ClickConfigProvider)
                                       graphics_click_config_provider);
    layer_set_update_proc(window_get_root_layer(g_windows[window_index]),
                          draw_scene);

    // Graphics frame inverter (for the "flash" effect):
    g_inverter_layer = inverter_layer_create(GRAPHICS_FRAME);
    layer_add_child(window_get_root_layer(g_windows[window_index]),
                    inverter_layer_get_layer(g_inverter_layer));
  }
}

/******************************************************************************
   Function: deinit_window

Description: Deinitializes the window at a given index of the "g_windows"
             array.

     Inputs: window_index - Integer indicating which window to deinitialize.

    Outputs: None.
******************************************************************************/
void deinit_window(const int8_t window_index)
{
  if (window_index < NUM_MENUS)
  {
    menu_layer_destroy(g_menu_layers[window_index]);
  }
  else if (window_index == NARRATION_WINDOW)
  {
    text_layer_destroy(g_narration_text_layer);
  }
  else // if (window_index == GRAPHICS_WINDOW)
  {
    inverter_layer_destroy(g_inverter_layer);
  }
  window_destroy(g_windows[window_index]);
}

/******************************************************************************
   Function: init

Description: Initializes the PebbleQuest app.

     Inputs: None.

    Outputs: None.
******************************************************************************/
void init(void)
{
  int8_t i;

  srand(time(NULL));
  g_current_window = MAIN_MENU;

  // Set up graphics window and graphics-related variables:
  init_window(GRAPHICS_WINDOW);
  init_wall_coords();
  g_compass_path = gpath_create(&COMPASS_PATH_INFO);
  gpath_move_to(g_compass_path, GPoint(SCREEN_CENTER_POINT_X,
                                       GRAPHICS_FRAME_HEIGHT +
                                         STATUS_BAR_HEIGHT / 2));
  g_player_is_attacking = false;

  // Load saved data or initialize a brand new player struct:
  g_player   = malloc(sizeof(player_t));
  g_location = malloc(sizeof(location_t));
  if (persist_exists(STORAGE_KEY))
  {
    persist_read_data(STORAGE_KEY, g_player, sizeof(player_t));
    persist_read_data(STORAGE_KEY + 1, g_location, sizeof(location_t));
  }
  else
  {
    init_player();
  }

  // Initialize all other windows and display the main menu:
  for (i = 0; i < GRAPHICS_WINDOW; ++i)
  {
    init_window(i);
  }
  show_window(MAIN_MENU, ANIMATED);

  // Subscribe to relevant services:
  app_focus_service_subscribe(app_focus_handler);
  tick_timer_service_subscribe(SECOND_UNIT, tick_handler);
}

/******************************************************************************
   Function: deinit

Description: Deinitializes the PebbleQuest app.

     Inputs: None.

    Outputs: None.
******************************************************************************/
void deinit(void)
{
  int8_t i;

  tick_timer_service_unsubscribe();
  app_focus_service_unsubscribe();
  persist_write_data(STORAGE_KEY, g_player, sizeof(player_t));
  persist_write_data(STORAGE_KEY + 1, g_location, sizeof(location_t));
  free(g_player);
  free(g_location);
  for (i = 0; i < NUM_WINDOWS; ++i)
  {
    deinit_window(i);
  }
}

/******************************************************************************
   Function: main

Description: Main function for the PebbleQuest app.

     Inputs: None.

    Outputs: Number of errors encountered.
******************************************************************************/
int main(void)
{
  init();
  app_event_loop();
  deinit();

  return 0;
}
