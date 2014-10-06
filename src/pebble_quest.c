/******************************************************************************
   Filename: pebble_quest.c

     Author: David C. Drake (http://davidcdrake.com)

Description: Function definitions for the 3D, first-person, fantasy RPG
             PebbleQuest, developed for the Pebble smartwatch (SDK 2.0).
             Copyright 2014, David C. Drake. More information available online:
             http://davidcdrake.com/pebblequest
******************************************************************************/

#include "pebble_quest.h"

/******************************************************************************
   Function: set_game_mode

Description: Sets the current game mode according to a given mode value, then
             shows the graphics window, scroll window, or menu window (with
             reloaded data) accordingly.

     Inputs: mode - Integer representing the desired game mode.

    Outputs: None.
******************************************************************************/
void set_game_mode(const int16_t mode)
{
  g_game_mode = mode;
  if (mode == ACTIVE_MODE)
  {
    show_window(g_graphics_window, NOT_ANIMATED);
  }
  else if (mode == SCROLL_MODE)
  {
    show_scroll(g_current_scroll);
  }
  else if (g_game_mode == MAIN_MENU_MODE)
  {
    show_window(g_main_menu_window, NOT_ANIMATED);
  }
  else if (g_game_mode == PEBBLE_OPTIONS_MODE)
  {
    show_window(g_options_menu_window, ANIMATED);
  }
  else if (g_game_mode == PEBBLE_INFUSION_MODE ||
           g_game_mode == REPLACE_ITEM_MODE)
  {
    show_window(g_heavy_items_menu_window, ANIMATED);
  }
  else // INVENTORY_MODE, SHOW_STATS_MODE, LOOT_MODE, or LEVEL_UP_MODE
  {
    show_window(g_ad_hoc_menu_window, ANIMATED);
  }
    /*menu_layer_reload_data(g_menu_layer);
    menu_layer_set_selected_index(g_menu_layer,
                                  (MenuIndex) {0, 0},
                                  MenuRowAlignCenter,
                                  NOT_ANIMATED);*/
}

/******************************************************************************
   Function: set_player_direction

Description: Sets the player's orientation to a given direction and updates the
             compass accordingly.

     Inputs: new_direction - Desired orientation.

    Outputs: None.
******************************************************************************/
void set_player_direction(const int16_t new_direction)
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
      gpath_rotate_to(g_compass_path, TRIG_MAX_ANGLE * 0.75);
      break;
    default: // case WEST:
      gpath_rotate_to(g_compass_path, TRIG_MAX_ANGLE / 4);
      break;
  }
  layer_mark_dirty(window_get_root_layer(g_graphics_window));
}

/******************************************************************************
   Function: move_player

Description: Attempts to move the player one cell forward in a given direction.

     Inputs: direction - Desired direction of movement.

    Outputs: None.
******************************************************************************/
void move_player(const int16_t direction)
{
  GPoint destination = get_cell_farther_away(g_player->position, direction, 1);

  // Check for movement into the entrance/exit:
  if (gpoint_equal(&g_player->position, &g_quest->starting_point) &&
      g_player->direction == g_quest->entrance_direction)
  {
    end_quest();
  }
  else if (occupiable(destination))
  {
    // Shift the player's position:
    g_player->position = destination;

    // Check for loot:
    if (get_cell_type(destination) > EMPTY)
    {
      set_game_mode(LOOT_MODE);
    }
    /*if (get_cell_type(destination) == QUEST_ITEM)
    {
      set_cell_type(destination, EMPTY);
      g_quest->completed = true;
    }*/
    layer_mark_dirty(window_get_root_layer(g_graphics_window));
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
void move_npc(npc_t *npc, const int16_t direction)
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
void determine_npc_behavior(npc_t *npc)
{
  if (touching(npc->position, g_player->position))
  {
    damage_player(npc->stats[PHYSICAL_POWER]);
  }
  else
  {
    move_npc(npc, get_pursuit_direction(npc->position, g_player->position));
  }
}

/******************************************************************************
   Function: player_is_visible_from

Description: Determines whether the player is (more or less) visible from a
             given set of cell coordinates.

     Inputs: cell - Cell coordinates from which to look for the player.

    Outputs: "True" if the player is (more or less) visible from the given
             coordinates.
******************************************************************************/
/*bool player_is_visible_from(GPoint cell)
{
  int16_t diff_x = cell.x - g_player->position.x,
          diff_y = cell.y - g_player->position.y;
  const int16_t horizontal_direction = diff_x > 0 ? WEST  : EAST,
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
void damage_player(int16_t damage)
{
  damage -= g_player->stats[PHYSICAL_DEFENSE] / 2;
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
void damage_npc(npc_t *npc, const int16_t damage)
{
  npc->stats[CURRENT_HEALTH] -= damage;
  if (npc->stats[CURRENT_HEALTH] <= 0)
  {
    g_quest->kills++;
    //g_player->exp_points += get_exp_bonus(npc->type);
    if (g_quest->type == MAIN_QUEST_3 && npc->type == ARCHMAGE)
    {
      g_quest->completed = true;
    }
    remove_npc(npc);
  }
}

/******************************************************************************
   Function: remove_npc

Description: Handles the removal of a given NPC from memory and from the
             current quest's list of NPCs.

     Inputs: npc - Pointer to the NPC to be killed.

    Outputs: None.
******************************************************************************/
void remove_npc(npc_t *npc)
{
  npc_t *npc_pointer, *npc_pointer_2;

  npc_pointer = npc_pointer_2 = g_quest->npcs;
  while (npc_pointer != NULL)
  {
    if (npc_pointer == npc)
    {
      if (npc_pointer->next != NULL)
      {
        if (npc_pointer == g_quest->npcs)
        {
          g_quest->npcs = npc_pointer->next;
        }
        else
        {
          npc_pointer_2->next = npc_pointer->next;
        }
      }
      else if (npc_pointer == g_quest->npcs)
      {
        g_quest->npcs = NULL;
      }
      else
      {
        npc_pointer_2->next = NULL;
      }
      free(npc_pointer);

      return;
    }
    npc_pointer_2 = npc_pointer;
    npc_pointer = npc_pointer->next;
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
void adjust_player_current_health(const int16_t amount)
{
  g_player->stats[CURRENT_HEALTH] += amount;
  if (g_player->stats[CURRENT_HEALTH] > g_player->stats[MAX_HEALTH])
  {
    g_player->stats[CURRENT_HEALTH] = g_player->stats[MAX_HEALTH];
  }
  else if (g_player->stats[CURRENT_HEALTH] <= 0)
  {
    //show_window(g_menu_window, NOT_ANIMATED);
    show_scroll(DEATH_SCROLL);
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
void adjust_player_current_energy(const int16_t amount)
{
  g_player->stats[CURRENT_ENERGY] += amount;
  if (g_player->stats[CURRENT_ENERGY] > g_player->stats[MAX_ENERGY])
  {
    g_player->stats[CURRENT_ENERGY] = g_player->stats[MAX_ENERGY];
  }
}

/******************************************************************************
   Function: adjust_visibility_depth

Description: Adjusts the player's visibility depth by a given amount, which may
             be positive or negative.

     Inputs: amount - Adjustment amount (which may be positive or negative).

    Outputs: None.
******************************************************************************/
/*void adjust_visibility_depth(int16_t amount)
{
  g_player->visibility_depth += amount;
  if (g_player->visibility_depth > MAX_VISIBILITY_DEPTH)
  {
    g_player->visibility_depth = MAX_VISIBILITY_DEPTH;
  }
  else if (g_player->visibility_depth < MIN_VISIBILITY_DEPTH)
  {
    g_player->visibility_depth = MIN_VISIBILITY_DEPTH;
  }
}*/

/******************************************************************************
   Function: end_quest

Description: Called when the player walks into the exit, ending the current
             quest.

     Inputs: None.

    Outputs: None.
******************************************************************************/
void end_quest(void)
{
  if (g_quest->completed)
  {
    g_current_scroll = VICTORY_SCROLL;
  }
  else
  {
    g_current_scroll = FAILURE_SCROLL;
  }
  set_game_mode(SCROLL_MODE);
}

/******************************************************************************
   Function: add_new_npc

Description: Creates an NPC of a given type at a given location and adds it to
             the current quest's linked list of NPCs. (If this would exceed the
             max. number of NPCs at one time, or if the given position isn't
             occupiable, a new NPC is not created.)

     Inputs: npc_type - Desired type for the new NPC.
             position - Desired spawn point for the new NPC.

    Outputs: None.
******************************************************************************/
void add_new_npc(const int16_t npc_type, const GPoint position)
{
  npc_t *npc_pointer = g_quest->npcs;

  if (occupiable(position))
  {
    if (npc_pointer == NULL)
    {
      g_quest->npcs = malloc(sizeof(npc_t));
      init_npc(g_quest->npcs, npc_type, position);

      return;
    }
    while (npc_pointer->next != NULL)
    {
      npc_pointer = npc_pointer->next;
    }
    npc_pointer->next = malloc(sizeof(npc_t));
    init_npc(npc_pointer->next, npc_type, position);
  }
}

/******************************************************************************
   Function: get_random_npc_type

Description: Returns a random NPC type.

     Inputs: None.

    Outputs: Integer representing an NPC type.
******************************************************************************/
int16_t get_random_npc_type(void)
{
  return rand() % NUM_NPC_TYPES;
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
  int16_t i, j, direction;
  bool checked_left, checked_right;
  GPoint spawn_point, spawn_point2;

  for (i = 0, direction = rand() % NUM_DIRECTIONS;
       i < NUM_DIRECTIONS;
       ++i, direction = (direction + 1 == NUM_DIRECTIONS ? NORTH :
                                                           direction + 1))
  {
    spawn_point = get_cell_farther_away(g_player->position,
                                        direction,
                                        MAX_VISIBILITY_DEPTH);
    if (out_of_bounds(spawn_point))
    {
      continue;
    }
    if (occupiable(spawn_point))
    {
      return spawn_point;
    }
    for (j = 1; j < MAX_VISIBILITY_DEPTH - 1; ++j)
    {
      checked_left = checked_right = false;
      do
      {
        // Check to the left:
        if (checked_right || rand() % 2)
        {
          spawn_point2 = get_cell_farther_away(spawn_point,
                                          get_direction_to_the_left(direction),
                                          j);
          checked_left = true;
        }
        // Check to the right:
        else if (!checked_right)
        {
          spawn_point2 = get_cell_farther_away(spawn_point,
                                         get_direction_to_the_right(direction),
                                         j);
          checked_right = true;
        }
        if (occupiable(spawn_point2))
        {
          return spawn_point2;
        }
      } while (!checked_left || !checked_right);
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
GPoint get_floor_center_point(const int16_t depth, const int16_t position)
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
/*GPoint get_midair_center_point(const int16_t depth, const int16_t position)
{
  int16_t x_midpoint1, x_midpoint2, x, y;

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
                             const int16_t direction,
                             const int16_t distance)
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
int16_t get_pursuit_direction(const GPoint pursuer, const GPoint pursuee)
{
  int16_t diff_x                     = pursuer.x - pursuee.x,
          diff_y                     = pursuer.y - pursuee.y;
  const int16_t horizontal_direction = diff_x > 0 ? WEST  : EAST,
                vertical_direction   = diff_y > 0 ? NORTH : SOUTH;
  bool checked_horizontal_direction  = false,
       checked_vertical_direction    = false;

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
int16_t get_direction_to_the_left(const int16_t reference_direction)
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
int16_t get_direction_to_the_right(const int16_t reference_direction)
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
int16_t get_opposite_direction(const int16_t direction)
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
   Function: get_boosted_stat_value

Description: Determines what value a given stat will be raised to if boosted by
             a given amount, which is either the sum of those values or
             MAX_INT_VALUE.

     Inputs: stat_index   - Index value of the stat of interest.
             boost_amount - Desired boost amount.

    Outputs: The new value the stat will have if it is boosted.
******************************************************************************/
int16_t get_boosted_stat_value(const int16_t stat_index,
                               const int16_t boost_amount)
{
  int16_t boosted_stat_value = g_player->stats[stat_index] + boost_amount;

  if (boosted_stat_value >= MAX_INT_VALUE)
  {
    return MAX_INT_VALUE;
  }

  return boosted_stat_value;
}

/******************************************************************************
   Function: get_nth_item_type

Description: Returns the type of the nth item in the player's inventory.

     Inputs: n - Integer indicating the item of interest (1st, 2nd, 3rd, etc.).

    Outputs: The nth item's type.
******************************************************************************/
int16_t get_nth_item_type(const int16_t n)
{
  int16_t i, item_count = 0;

  // Search Pebbles:
  for (i = 0; i < NUM_PEBBLE_TYPES; ++i)
  {
    if (g_player->pebbles[i] > 0 && ++item_count == n)
    {
      return i;
    }
  }

  // Search heavy items:
  for (i = 0; i < MAX_HEAVY_ITEMS; ++i)
  {
    if (g_player->heavy_items[i]->type != NONE && ++item_count == n)
    {
      return g_player->heavy_items[i]->type;
    }
  }

  return NONE;
}

/******************************************************************************
   Function: get_pointer_to_nth_item

Description: Returns a pointer to the nth item in the player's inventory, which
             is assumed to be a heavy item.

     Inputs: n - Integer indicating the item of interest (1st, 2nd, 3rd, etc.).

    Outputs: Pointer to the nth item (which should be a heavy item).
******************************************************************************/
heavy_item_t *get_pointer_to_nth_item(const int16_t n)
{
  int16_t i, item_count = get_num_pebble_types_owned();

  for (i = 0; i < MAX_HEAVY_ITEMS; ++i)
  {
    if (g_player->heavy_items[i]->type != NONE && ++item_count == n)
    {
      break;
    }
  }

  return g_player->heavy_items[i];
}

/******************************************************************************
   Function: get_num_pebble_types_owned

Description: Returns the number of types of Pebbles in the player's inventory.

     Inputs: None.

    Outputs: Number of Pebble types owned by the player.
******************************************************************************/
int16_t get_num_pebble_types_owned(void)
{
  int16_t i, num_pebble_types = 0;

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
   Function: get_num_heavy_items_owned

Description: Returns the number of heavy items in the player's inventory.

     Inputs: None.

    Outputs: Number of heavy items owned by the player.
******************************************************************************/
int16_t get_num_heavy_items_owned(void)
{
  int16_t i, num_heavy_items = 0;

  for (i = 0; i < MAX_HEAVY_ITEMS; ++i)
  {
    if (g_player->heavy_items[i]->type != NONE)
    {
      num_heavy_items++;
    }
  }

  return num_heavy_items;
}

/******************************************************************************
   Function: get_equip_target

Description: Given an item type, returns that item's equip target (RIGHT_HAND,
             LEFT_HAND, or BODY).

     Inputs: item_type - Integer representing the item type of interest.

    Outputs: Integer representing the item's equip target.
******************************************************************************/
int16_t get_equip_target(const int16_t item_type)
{
  if (item_type < SHIELD)
  {
    return RIGHT_HAND;
  }
  else if (item_type == SHIELD)
  {
    return LEFT_HAND;
  }

  return BODY;
}

/******************************************************************************
   Function: get_cell_type

Description: Returns the type of cell at a given set of coordinates.

     Inputs: cell - Coordinates of the cell of interest.

    Outputs: The indicated cell's type.
******************************************************************************/
int16_t get_cell_type(const GPoint cell)
{
  if (out_of_bounds(cell))
  {
    return SOLID;
  }

  return g_quest->map[cell.x][cell.y];
}

/******************************************************************************
   Function: set_cell_type

Description: Sets the cell at a given set of coordinates to a given type.
             (Doesn't test coordinates to ensure they're in-bounds!)

     Inputs: cell - Coordinates of the cell of interest.
             type - The cell type to be assigned at those coordinates.

    Outputs: None.
******************************************************************************/
void set_cell_type(GPoint cell, const int16_t type)
{
  g_quest->map[cell.x][cell.y] = type;
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
  npc_t *npc = g_quest->npcs;

  while (npc != NULL)
  {
    if (gpoint_equal(&npc->position, &cell))
    {
      return npc;
    }
    npc = npc->next;
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
  const int16_t diff_x = cell.x - cell_2.x,
                diff_y = cell.y - cell_2.y;

  return ((diff_x == 0 && abs(diff_y) == 1) ||
          (diff_y == 0 && abs(diff_x) == 1));
}

/******************************************************************************
   Function: show_scroll

Description: Displays desired scroll text via the scroll window.

     Inputs: scroll - Integer indicating desired scroll text.

    Outputs: None.
******************************************************************************/
void show_scroll(const int16_t scroll)
{
  static char scroll_str[SCROLL_STR_LEN + 1]; // To store scroll text.
  GSize content_size;                         // To adjust scroll size.

  switch (scroll)
  {
    strcpy(scroll_str, "Adventurer,\n\n ");
    //strcpy(scroll_str, "Hero of the Realm,\n\n  ");
    case MAIN_QUEST_1:
      strcat(scroll_str,
"Seek ye the fabled Pebbles of Power, remnants of the sundered Elderstone, for the good of the Realm. The Archmage hath scried the location of a cave where thy search may begin. May the Gods be with thee!\n\n--King Lannus");
      break;
    case MAIN_QUEST_2:
      strcat(scroll_str,
"Thy skill in finding and recovering Pebbles of Power is astounding! Bring them to me that I may attempt to reunite them, to forge anew a shard of the legendary Elderstone.\n\n--Archmage Dreyan");
      break;
    case MAIN_QUEST_3:
      strcat(scroll_str,
"Thou hast revealed the Archmage's treachery and bravely defeated him, saving the Realm from a fate most dire! I name thee Hero of the Realm and offer my wealth to assist thee in thy future adventures.\n\n--King Lannus");
      break;
    case FIND_PEBBLE:
      strcat(scroll_str, "Find a Pebble...");
      break;
    case FIND_ITEM:
      strcat(scroll_str, "Find an artifact...");
      break;
    case RECOVER_ITEM:
      strcat(scroll_str, "Find my stuff!");
      break;
    case ESCORT:
      strcat(scroll_str, "Escort me to...");
      break;
    case RESCUE:
      strcat(scroll_str, "Please rescue my grandson!");
      break;
    case ASSASSINATE:
      strcat(scroll_str, "Kill their leader.");
      break;
    case EXTERMINATE:
      strcat(scroll_str, "Wipe them out. All of them.");
      break;
    case ESCAPE:
      strcat(scroll_str, "Escape!");
      break;
    case FAILURE_SCROLL:
      strcat(scroll_str, "Alas, thou hast failed.");
      deinit_quest();
      break;
    case VICTORY_SCROLL:
      strcat(scroll_str, "Thou art victorious!");
      deinit_quest();
      break;
    default: // case DEATH_SCROLL:
      strcat(scroll_str, "Alas, our hero hath perished.");
      deinit_quest();
      break;
  }
  text_layer_set_text(g_scroll_text_layer, scroll_str);
  content_size = text_layer_get_content_size(g_scroll_text_layer);
  content_size.h += SCROLL_HEIGHT_OFFSET;
  text_layer_set_size(g_scroll_text_layer, content_size);
  scroll_layer_set_content_size(g_scroll_scroll_layer, content_size);
  scroll_layer_set_content_offset(g_scroll_scroll_layer,
                                  GPointZero,
                                  NOT_ANIMATED);
  show_window(g_scroll_window, ANIMATED);
}

/******************************************************************************
   Function: show_window

Description: Displays a given window. (Assumes that window has already been
             initialized.)

     Inputs: window   - Pointer to the desired window.
             animated - If "true", the window will slide into view.

    Outputs: None.
******************************************************************************/
void show_window(Window *window, bool animated)
{
  if (!window_stack_contains_window(window))
  {
    window_stack_push(window, animated);
  }
  else
  {
    while (window_stack_get_top_window() != window)
    {
      window_stack_pop(animated);
    }
  }
}

/******************************************************************************
   Function: menu_draw_header_callback

Description: Instructions for drawing each menu's header.

     Inputs: ctx           - Pointer to the associated context.
             cell_layer    - Pointer to the cell layer.
             section_index - Section number of the header to be drawn.
             data          - Pointer to additional data (not used).

    Outputs: None.
******************************************************************************/
static void menu_draw_header_callback(GContext *ctx,
                                      const Layer *cell_layer,
                                      uint16_t section_index,
                                      void *data)
{
  char header_str[MENU_HEADER_STR_LEN + 1] = "";

  if (g_game_mode == MAIN_MENU_MODE)
  {
    strcat(header_str, "Main Menu");
  }
  else if (g_game_mode == INVENTORY_MODE)
  {
    strcat(header_str, "Inventory");
  }
  else if (g_game_mode == PEBBLE_OPTIONS_MODE)
  {
    strcat_item_name(header_str, g_current_selection);
  }
  else if (g_game_mode == PEBBLE_INFUSION_MODE)
  {
    strcat(header_str, "Which item?");
  }
  else if (g_game_mode == LOOT_MODE)
  {
    strcat(header_str, "Loot");
  }
  else if (g_game_mode == REPLACE_ITEM_MODE)
  {
    strcat(header_str, "Replace an item?");
  }
  else if (g_game_mode == SHOW_STATS_MODE)
  {
    strcat(header_str, "Stats");
  }
  else // if (g_game_mode == LEVEL_UP_MODE)
  {
    strcat(header_str, "Level ");
    strcat_int(header_str, g_player->level);
    strcat(header_str, " reached!");
  }
  menu_cell_basic_header_draw(ctx, cell_layer, header_str);
}

/******************************************************************************
   Function: menu_draw_row_callback

Description: Instructions for drawing the rows (cells) of each menu.

     Inputs: ctx        - Pointer to the associated context.
             cell_layer - Pointer to the layer of the cell to be drawn.
             cell_index - Pointer to the index struct of the cell to be drawn.
             data       - Pointer to additional data (not used).

    Outputs: None.
******************************************************************************/
static void menu_draw_row_callback(GContext *ctx,
                                   const Layer *cell_layer,
                                   MenuIndex *cell_index,
                                   void *data)
{
  int16_t item_type;
  char title_str[MENU_TITLE_STR_LEN + 1]       = "",
       subtitle_str[MENU_SUBTITLE_STR_LEN + 1] = "";
  heavy_item_t *item;

  if (g_game_mode == MAIN_MENU_MODE)
  {
    switch (cell_index->row)
    {
      case 0:
        strcat(title_str, g_quest == NULL ? "New Quest" : "Continue");
        strcat(subtitle_str, "Enter the fray!");
        break;
      case 1:
        strcat(title_str, "Character Stats");
        strcat(subtitle_str, "Strength, Agility...");
        break;
      case 2:
        strcat(title_str, "Inventory");
        strcat(subtitle_str, "Equip/infuse items.");
        break;
    }
  }
  else if (g_game_mode == SHOW_STATS_MODE || g_game_mode == LEVEL_UP_MODE)
  {
    strcat_stat_name(title_str, cell_index->row);
    strcat_stat_value(title_str, cell_index->row);
    if (g_game_mode == LEVEL_UP_MODE)
    {
      strcat(title_str, "->");
      strcat_int(title_str, get_boosted_stat_value(cell_index->row, 1));
    }
  }
  else if (g_game_mode == PEBBLE_OPTIONS_MODE)
  {
    switch (cell_index->row)
    {
      case 0:
        strcat(title_str, "Equip");
        break;
      default:
        strcat(title_str, "Infuse into Item");
        break;
    }
  }
  else if (g_game_mode == LOOT_MODE)
  {
    strcat_item_name(title_str, get_cell_type(g_player->position));
  }
  else // INVENTORY_MODE, PEBBLE_INFUSION_MODE, or REPLACE_ITEM_MODE
  {
    item_type = get_nth_item_type(cell_index->row -
                                  (g_game_mode != INVENTORY_MODE ?
                                   get_num_pebble_types_owned()  :
                                   0));
    strcat_item_name(title_str, item_type);

    // Pebbles (only relevant in INVENTORY_MODE):
    if (item_type < FIRST_HEAVY_ITEM)
    {
      strcat(title_str, " (");
      strcat_int(title_str, g_player->pebbles[item_type]);
      strcat(title_str, ")");
      if (g_player->equipped_pebble == item_type)
      {
        strcat(subtitle_str, "Equipped");
      }
    }

    // Heavy items:
    else
    {
      item = g_player->heavy_items[cell_index->row -
                                   (g_game_mode == INVENTORY_MODE ?
                                    get_num_pebble_types_owned()  :
                                    0)];
      if (item->infused_pebble != NONE)
      {
        strcat_magic_type(title_str, item->infused_pebble);
      }
      if (g_player->equipped_heavy_items[get_equip_target(item->type)] == item)
      {
        strcat(subtitle_str, "Equipped");
      }
    }
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
  int16_t equip_target;

  if (g_game_mode == MAIN_MENU_MODE)
  {
    switch (cell_index->row)
    {
      case 0: // New Quest / Continue
        if (g_quest == NULL)
        {
          g_quest = malloc(sizeof(quest_t));
          if (g_player->num_pebbles_found == 0)
          {
            init_quest(MAIN_QUEST_1);
          }
          else if (g_player->num_pebbles_found == NUM_PEBBLE_TYPES - 1)
          {
            init_quest(MAIN_QUEST_2);
          }
          else if (g_player->num_pebbles_found == NUM_PEBBLE_TYPES)
          {
            init_quest(MAIN_QUEST_3);
          }
          else
          {
            init_quest(rand() % NUM_RANDOM_QUEST_TYPES);
          }
          set_game_mode(ACTIVE_MODE);
          g_current_scroll = g_quest->type;
          set_game_mode(SCROLL_MODE);
        }
        else
        {
          set_game_mode(ACTIVE_MODE);
        }
        break;
      case 1: // Character Stats
        set_game_mode(SHOW_STATS_MODE);
        break;
      case 2: // Inventory
        set_game_mode(INVENTORY_MODE);
        break;
    }
  }
  else if (g_game_mode == LEVEL_UP_MODE)
  {
    if (g_player->stats[cell_index->row] < MAX_INT_VALUE)
    {
      g_player->stats[cell_index->row] =
        get_boosted_stat_value(cell_index->row, 1);
      g_quest == NULL ? set_game_mode(MAIN_MENU_MODE) :
                        set_game_mode(ACTIVE_MODE);
    }
  }
  else if (g_game_mode == LOOT_MODE)
  {
    add_item_to_inventory(get_cell_type(g_player->position));
    set_cell_type(g_player->position, EMPTY);
  }
  else if (g_game_mode == INVENTORY_MODE)
  {
    if (cell_index->row < FIRST_HEAVY_ITEM)
    {
      g_current_selection = cell_index->row;
      set_game_mode(PEBBLE_OPTIONS_MODE);
    }
    else
    {
      equip_heavy_item(g_player->heavy_items[cell_index->row -
                                               NUM_PEBBLE_TYPES]);
    }
  }
  else if (g_game_mode == PEBBLE_OPTIONS_MODE)
  {
    switch (cell_index->row)
    {
      case 0: // Equip
        g_player->equipped_heavy_items[RIGHT_HAND] = NULL;
        g_player->equipped_pebble                  = g_current_selection;
        break;
      default: // Infuse into Item
        set_game_mode(PEBBLE_INFUSION_MODE);
        break;
    }
  }
  else if (g_game_mode == PEBBLE_INFUSION_MODE)
  {
    if (g_player->heavy_items[cell_index->row]->infused_pebble == NONE)
    {
      g_player->heavy_items[cell_index->row]->infused_pebble =
        g_current_selection;
      set_game_mode(INVENTORY_MODE);
    }
  }
  else // if (g_game_mode == REPLACE_ITEM_MODE)
  {
    // Unequip the old item, unless the new item has the same equip target:
    equip_target =
      get_equip_target(g_player->heavy_items[cell_index->row]->type);
    if (g_player->equipped_heavy_items[equip_target] ==
          g_player->heavy_items[cell_index->row] &&
        equip_target != get_equip_target(g_current_selection))
    {
      g_player->equipped_heavy_items[equip_target] = NULL;
    }

    // Reinitialize the heavy item struct, then return to gameplay:
    init_heavy_item(g_player->heavy_items[cell_index->row],
                    g_current_selection);
    set_game_mode(ACTIVE_MODE);
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
  if (g_game_mode == MAIN_MENU_MODE)
  {
    return MAIN_MENU_NUM_ROWS;
  }
  else if (g_game_mode == LOOT_MODE)
  {
    return 1;
  }
  else if (g_game_mode == PEBBLE_OPTIONS_MODE)
  {
    return 2;
  }
  else if (g_game_mode == LEVEL_UP_MODE)
  {
    return 3;
  }
  else if (g_game_mode == INVENTORY_MODE)
  {
    return get_num_pebble_types_owned() + get_num_heavy_items_owned();
  }
  else // PEBBLE_INFUSION_MODE or REPLACE_ITEM_MODE
  {
    return get_num_heavy_items_owned();
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
  int16_t i, depth;
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

  // Draw applicable weapon fire:
  if (g_player_animation_mode > 0)
  {
    draw_player_action(ctx);
  }

  // Finally, draw the lower status bar:
  draw_status_bar(ctx);
}

/******************************************************************************
   Function: draw_player_action

Description: Draws the player's attack/spell onto the screen.

     Inputs: ctx - Pointer to the relevant graphics context.

    Outputs: None.
******************************************************************************/
void draw_player_action(GContext *ctx)
{
  //int16_t i;

  /*graphics_context_set_stroke_color(ctx, GColorWhite);
  graphics_draw_line(ctx,
                     GPoint(SCREEN_CENTER_POINT_X, GRAPHICS_FRAME_HEIGHT),
                     SCREEN_CENTER_POINT);
  for (i = 0; i <= g_lightning_base_width / 2; ++i)
  {
    if (i == g_lightning_base_width / 2)
    {
      graphics_context_set_stroke_color(ctx, GColorBlack);
    }
    graphics_draw_line(ctx,
                      GPoint(SCREEN_CENTER_POINT_X - i, GRAPHICS_FRAME_HEIGHT),
                      GPoint(SCREEN_CENTER_POINT_X - i / 3,
                             SCREEN_CENTER_POINT_Y));
    graphics_draw_line(ctx,
                      GPoint(SCREEN_CENTER_POINT_X + i, GRAPHICS_FRAME_HEIGHT),
                      GPoint(SCREEN_CENTER_POINT_X + i / 3,
                             SCREEN_CENTER_POINT_Y));
  }*/
}

/******************************************************************************
   Function: draw_floor_and_ceiling

Description: Draws the floor and ceiling.

     Inputs: ctx - Pointer to the relevant graphics context.

    Outputs: None.
******************************************************************************/
void draw_floor_and_ceiling(GContext *ctx)
{
  int16_t x, y, max_y, shading_offset;

  x = 2;
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
                     const int16_t depth,
                     const int16_t position)
{
  int16_t left, right, top, bottom, y_offset, exit_offset_x, exit_offset_y;
  bool back_wall_drawn, left_wall_drawn, right_wall_drawn, exit_present;
  GPoint cell_2;

  // Back wall:
  left          = g_back_wall_coords[depth][position][TOP_LEFT].x;
  right         = g_back_wall_coords[depth][position][BOTTOM_RIGHT].x;
  top           = g_back_wall_coords[depth][position][TOP_LEFT].y;
  bottom        = g_back_wall_coords[depth][position][BOTTOM_RIGHT].y;
  exit_present  = gpoint_equal(&cell, &g_quest->starting_point);
  exit_offset_y = (right - left) / 4;
  if (bottom - top < MIN_WALL_HEIGHT)
  {
    return;
  }
  back_wall_drawn = left_wall_drawn = right_wall_drawn = false;
  cell_2 = get_cell_farther_away(cell, g_player->direction, 1);
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

    // Entrance/exit:
    if (exit_present && g_player->direction == g_quest->entrance_direction)
    {
      graphics_context_set_fill_color(ctx, GColorBlack);
      exit_offset_x = (right - left) / 3;
      graphics_fill_rect(ctx,
                         GRect(left + exit_offset_x,
                               top + exit_offset_y,
                               exit_offset_x,
                               bottom - top - exit_offset_y),
                         NO_CORNER_RADIUS,
                         GCornerNone);
    }

    back_wall_drawn = true;
  }

  // Left wall:
  right = left;
  if (depth == 0)
  {
    left = 0;
    y_offset = top;
  }
  else
  {
    left = g_back_wall_coords[depth - 1][position][TOP_LEFT].x;
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

      // Entrance/exit:
      if (exit_present && get_direction_to_the_left(g_player->direction) ==
                          g_quest->entrance_direction)
      {
        exit_offset_x = (right - left) / 3;
        fill_quad(ctx,
                  GPoint(depth == 0 ? 0 : left + exit_offset_x,
                         top - (depth == 0 ? y_offset - 4: y_offset / 3) +
                           exit_offset_y),
                  GPoint(depth == 0 ? 0 : left + exit_offset_x,
                         bottom + (depth == 0 ? y_offset : y_offset / 3)),
                  GPoint(right - exit_offset_x, top + exit_offset_y),
                  GPoint(right - exit_offset_x, bottom + 3),
                  GColorBlack);
      }

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

      // Entrance/exit:
      if (exit_present && get_direction_to_the_right(g_player->direction) ==
                          g_quest->entrance_direction)
      {
        exit_offset_x = (right - left) / 3;
        fill_quad(ctx,
                  GPoint(left + exit_offset_x, top + exit_offset_y),
                  GPoint(left + exit_offset_x, bottom + 4),
                  GPoint(depth == 0 ? GRAPHICS_FRAME_WIDTH :
                                      right - exit_offset_x,
                         top - (depth == 0 ? y_offset - 5 : y_offset / 3) +
                           exit_offset_y),
                  GPoint(depth == 0 ? GRAPHICS_FRAME_WIDTH :
                                      right - exit_offset_x,
                         bottom + (depth == 0 ? y_offset : y_offset / 3)),
                  GColorBlack);
      }

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
                        const int16_t depth,
                        const int16_t position)
{
  int16_t drawing_unit; // Reference variable for drawing contents at depth.
  npc_t *npc = get_npc_at(cell);
  GPoint floor_center_point;

  // Check for a completely empty cell:
  if (get_cell_type(cell) == EMPTY && npc == NULL)
  {
    return;
  }

  // Determine the drawing unit:
  drawing_unit = (g_back_wall_coords[depth][position][BOTTOM_RIGHT].x -
                  g_back_wall_coords[depth][position][TOP_LEFT].x) / 10;
  if ((g_back_wall_coords[depth][position][BOTTOM_RIGHT].x -
       g_back_wall_coords[depth][position][TOP_LEFT].x) % 10 >= 5)
  {
    drawing_unit++;
  }
  floor_center_point = get_floor_center_point(depth, position);

  // Draw a shadow on the ground:
  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_rect(ctx,
                     GRect(floor_center_point.x - drawing_unit * 4,
                           floor_center_point.y - drawing_unit / 2,
                           drawing_unit * 8,
                           drawing_unit),
                     drawing_unit / 2,
                     GCornersAll);

  // Draw the character (or a treasure chest for loot):
  if (npc == NULL)
  {
    if (get_cell_type(cell) == CAPTIVE)
    {
      // Legs:
      graphics_fill_rect(ctx,
                         GRect(floor_center_point.x -
                                 (drawing_unit + drawing_unit / 2),
                               floor_center_point.y - drawing_unit * 3,
                               drawing_unit,
                               drawing_unit * 3),
                         NO_CORNER_RADIUS,
                         GCornerNone);
      graphics_fill_rect(ctx,
                         GRect(floor_center_point.x + drawing_unit / 2,
                               floor_center_point.y - drawing_unit * 3,
                               drawing_unit,
                               drawing_unit * 3),
                         NO_CORNER_RADIUS,
                         GCornerNone);

      // Waist:
      graphics_fill_rect(ctx,
                         GRect(floor_center_point.x -
                                 (drawing_unit + drawing_unit / 2),
                               floor_center_point.y - drawing_unit * 4,
                               drawing_unit * 3,
                               drawing_unit),
                         NO_CORNER_RADIUS,
                         GCornerNone);

      // Torso:
      draw_shaded_quad(ctx,
                       GPoint(floor_center_point.x -
                                (drawing_unit + drawing_unit / 2),
                              floor_center_point.y - drawing_unit * 8),
                       GPoint(floor_center_point.x -
                                (drawing_unit + drawing_unit / 2),
                              floor_center_point.y - drawing_unit * 4),
                       GPoint(floor_center_point.x +
                                (drawing_unit + drawing_unit / 2),
                              floor_center_point.y - drawing_unit * 8),
                       GPoint(floor_center_point.x +
                                (drawing_unit + drawing_unit / 2),
                              floor_center_point.y - drawing_unit * 4),
                       GPoint(g_back_wall_coords[depth][position][TOP_LEFT].x -
                                20,
                              g_back_wall_coords[depth][position][TOP_LEFT].y -
                                20));

      // Arms:
      graphics_context_set_fill_color(ctx, GColorWhite);
      graphics_fill_rect(ctx,
                         GRect(floor_center_point.x - drawing_unit * 2,
                               floor_center_point.y - drawing_unit * 8,
                               drawing_unit / 2,
                               drawing_unit * 4),
                         drawing_unit / 4,
                         GCornersLeft);
      graphics_fill_rect(ctx,
                         GRect(floor_center_point.x + (drawing_unit +
                                 drawing_unit / 2),
                               floor_center_point.y - drawing_unit * 8,
                               drawing_unit / 2,
                               drawing_unit * 4),
                         drawing_unit / 4,
                         GCornersRight);

      // Head:
      graphics_fill_rect(ctx,
                         GRect(floor_center_point.x - drawing_unit / 2,
                               floor_center_point.y - drawing_unit * 10,
                               drawing_unit + 1,
                               drawing_unit * 2),
                         drawing_unit / 2,
                         GCornersAll);

      // Hair:
      draw_shaded_quad(ctx,
                       GPoint(floor_center_point.x - drawing_unit / 2,
                              floor_center_point.y - drawing_unit * 10),
                       GPoint(floor_center_point.x - drawing_unit / 2,
                              floor_center_point.y -
                                (drawing_unit * 9 + drawing_unit / 3)),
                       GPoint(floor_center_point.x + drawing_unit / 2,
                              floor_center_point.y - drawing_unit * 10),
                       GPoint(floor_center_point.x + drawing_unit / 2,
                              floor_center_point.y -
                                (drawing_unit * 9 + drawing_unit / 3)),
                       GPoint(g_back_wall_coords[depth][position][TOP_LEFT].x -
                                10,
                              g_back_wall_coords[depth][position][TOP_LEFT].y -
                                10));

      // Eyes:
      graphics_context_set_fill_color(ctx, GColorBlack);
      graphics_fill_circle(ctx,
                           GPoint(floor_center_point.x - drawing_unit / 4,
                                  floor_center_point.y - (drawing_unit * 9)),
                           drawing_unit / 6);
      graphics_fill_circle(ctx,
                           GPoint(floor_center_point.x + drawing_unit / 4,
                                  floor_center_point.y - (drawing_unit * 9)),
                           drawing_unit / 6);
    }
    else if (get_cell_type(cell) > EMPTY) // Loot!
    {
      graphics_context_set_fill_color(ctx, GColorWhite);
      graphics_fill_rect(ctx,
                         GRect(floor_center_point.x - drawing_unit * 2,
                               floor_center_point.y - drawing_unit * 4,
                               drawing_unit * 4,
                               drawing_unit * 4),
                         drawing_unit / 2,
                         GCornersTop);
    }
  }
  else if (npc->type == ORC)
  {
    // Legs:
    draw_shaded_quad(ctx,
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
  }
  else if (npc->type == WOLF)
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
  else if (npc->type == SLIME)
  {
    // Body:
    graphics_fill_circle(ctx,
                         GPoint(floor_center_point.x,
                                floor_center_point.y - drawing_unit * 2),
                         drawing_unit * 2);

    // Head:
    graphics_fill_circle(ctx,
                         GPoint(floor_center_point.x,
                                floor_center_point.y - drawing_unit * 6),
                         drawing_unit * 4);

    // Eyes:
    graphics_context_set_fill_color(ctx, GColorWhite);
    graphics_fill_rect(ctx,
                       GRect(floor_center_point.x - drawing_unit * 3,
                             floor_center_point.y - drawing_unit * 7,
                             drawing_unit * 2,
                             drawing_unit),
                       drawing_unit / 2,
                       GCornersAll);
    graphics_fill_rect(ctx,
                       GRect(floor_center_point.x + drawing_unit,
                             floor_center_point.y - drawing_unit * 7,
                             drawing_unit * 2,
                             drawing_unit),
                       drawing_unit / 2,
                       GCornersAll);
  }
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
   Function: fill_quad

Description: Draws a filled quadrilateral according to specifications. Assumes
             the left and right sides are parallel.

     Inputs: ctx         - Pointer to the relevant graphics context.
             upper_left  - Coordinates of the upper-left point.
             lower_left  - Coordinates of the lower-left point.
             upper_right - Coordinates of the upper-right point.
             lower_right - Coordinates of the lower-right point.
             color       - Desired color.

    Outputs: None.
******************************************************************************/
void fill_quad(GContext *ctx,
               const GPoint upper_left,
               const GPoint lower_left,
               const GPoint upper_right,
               const GPoint lower_right,
               const GColor color)
{
  int16_t i;
  float dy_over_width = (float) (upper_right.y - upper_left.y) /
                                (upper_right.x - upper_left.x);

  graphics_context_set_stroke_color(ctx, color);
  for (i = upper_left.x;
       i <= upper_right.x && i < GRAPHICS_FRAME_WIDTH;
       ++i)
  {
    graphics_draw_line(ctx,
                       GPoint(i, upper_left.y + (i - upper_left.x) *
                                 dy_over_width),
                       GPoint(i, lower_left.y - (i - upper_left.x) *
                                 dy_over_width));
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
  int16_t i, j;

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

Description: Draws a filled ellipse according to given specifications.

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
                         const int16_t h_radius,
                         const int16_t v_radius,
                         const GPoint shading_ref)
{
  int32_t theta;
  int16_t i, j, shading_offset;

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
                        const int16_t radius,
                        int16_t shading_offset)
{
  int32_t theta;
  int16_t i, x_offset, y_offset;

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
/*void fill_ellipse(GContext *ctx,
                  const GPoint center,
                  const int16_t h_radius,
                  const int16_t v_radius,
                  const GColor color)
{
  int32_t theta;
  int16_t x_offset, y_offset;

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
}*/

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
                  const int16_t h_radius,
                  const int16_t v_radius)
{
  int32_t theta;
  int16_t x_offset, y_offset;

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
  g_flash_timer = app_timer_register(FLASH_TIMER_DURATION,
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
   Function: player_timer_callback

Description: Called when the player timer reaches zero.

     Inputs: data - Pointer to additional data (not used).

    Outputs: None.
******************************************************************************/
static void player_timer_callback(void *data)
{
  if (g_graphics_window == NULL)
  {
    return;
  }
  else if (--g_player_animation_mode > 0)
  {
    g_player_timer = app_timer_register(PLAYER_TIMER_DURATION,
                                        player_timer_callback,
                                        NULL);
  }
  layer_mark_dirty(window_get_root_layer(g_graphics_window));
}

/******************************************************************************
   Function: graphics_window_appear

Description: Called when the graphics window appears.

     Inputs: window - Pointer to the graphics window.

    Outputs: None.
******************************************************************************/
static void graphics_window_appear(Window *window)
{
  g_game_mode             = ACTIVE_MODE;
  g_player_animation_mode = 0;
  layer_set_hidden(inverter_layer_get_layer(g_inverter_layer), true);
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
  if (g_game_mode == ACTIVE_MODE)
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
  if (g_game_mode == ACTIVE_MODE)
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
  if (g_game_mode == ACTIVE_MODE)
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
  if (g_game_mode == ACTIVE_MODE)
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
  //Window *window = (Window *) context;

  if (g_game_mode == ACTIVE_MODE)
  {
    // Check for a targeted NPC:
    cell = get_cell_farther_away(g_player->position, g_player->direction, 1);
    while (get_cell_type(cell) >= EMPTY)
    {
      npc = get_npc_at(cell);

      // If we've found an NPC or the attack isn't ranged, we're done:
      if (npc != NULL ||
          (g_player->equipped_pebble == NONE &&
           g_player->equipped_heavy_items[RIGHT_HAND]->type != BOW))
      {
        break;
      }
      cell = get_cell_farther_away(cell, g_player->direction, 1);
    }

    // If a Pebble is equipped, cast a spell:
    if (g_player->stats[CURRENT_ENERGY] >= MIN_ENERGY_LOSS_PER_ACTION)
    {
      if (g_player->equipped_pebble != NONE)
      {
        flash_screen();
        adjust_player_current_energy(MIN_ENERGY_LOSS_PER_ACTION);
        if (npc != NULL)
        {
          damage_npc(npc,
                     g_player->stats[MAGICAL_POWER] -
                       npc->stats[MAGICAL_DEFENSE]);
        }
      }

      // Otherwise, the player is attacking with a physical weapon:
      else if (g_player->stats[CURRENT_ENERGY] >= MIN_ENERGY_LOSS_PER_ACTION)
      {
        adjust_player_current_energy(MIN_ENERGY_LOSS_PER_ACTION);
        g_player_timer = app_timer_register(PLAYER_TIMER_DURATION,
                                            player_timer_callback,
                                            NULL);
        if (npc != NULL)
        {
          damage_npc(npc,
                     g_player->stats[PHYSICAL_POWER] -
                       npc->stats[PHYSICAL_DEFENSE]);
        }
      }
    }

    layer_mark_dirty(window_get_root_layer(g_graphics_window));
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
  // "Up" button:
  window_single_repeating_click_subscribe(BUTTON_ID_UP,
                                          MIN_ACTION_REPEAT_INTERVAL,
                                          graphics_up_single_repeating_click);
  window_multi_click_subscribe(BUTTON_ID_UP,
                               MULTI_CLICK_MIN,
                               MULTI_CLICK_MAX,
                               MULTI_CLICK_TIMEOUT,
                               LAST_CLICK_ONLY,
                               graphics_up_multi_click);

  // "Down" button:
  window_single_repeating_click_subscribe(BUTTON_ID_DOWN,
                                         MIN_ACTION_REPEAT_INTERVAL,
                                         graphics_down_single_repeating_click);
  window_multi_click_subscribe(BUTTON_ID_DOWN,
                               MULTI_CLICK_MIN,
                               MULTI_CLICK_MAX,
                               MULTI_CLICK_TIMEOUT,
                               LAST_CLICK_ONLY,
                               graphics_down_multi_click);

  // "Select" button:
  window_single_repeating_click_subscribe(BUTTON_ID_SELECT,
                                       MIN_ACTION_REPEAT_INTERVAL,
                                       graphics_select_single_repeating_click);
}

/******************************************************************************
   Function: scroll_select_single_click

Description: The scroll window's single-click handler for the "select" button.
             Hides the scroll window.

     Inputs: recognizer - The click recognizer.
             context    - Pointer to the associated context.

    Outputs: None.
******************************************************************************/
void scroll_select_single_click(ClickRecognizerRef recognizer, void *context)
{
  window_stack_pop(NOT_ANIMATED);
}

/******************************************************************************
   Function: scroll_click_config_provider

Description: Button-click configurations for the scroll window.

     Inputs: context - Pointer to the associated context.

    Outputs: None.
******************************************************************************/
void scroll_click_config_provider(void *context)
{
  window_single_click_subscribe(BUTTON_ID_SELECT, scroll_select_single_click);
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
  int16_t current_num_npcs = 0;
  npc_t *npc_pointer;

  if (g_game_mode == ACTIVE_MODE)
  {
    // Handle NPC behavior:
    npc_pointer = g_quest->npcs;
    while (npc_pointer != NULL)
    {
      determine_npc_behavior(npc_pointer);
      if (g_player->stats[CURRENT_HEALTH] <= 0)
      {
        return;
      }
      current_num_npcs++;
      npc_pointer = npc_pointer->next;
    }

    // Determine whether a new NPC should be generated:
    if (current_num_npcs < MAX_NPCS_AT_ONE_TIME               &&
        g_quest->kills + current_num_npcs < g_quest->num_npcs &&
        rand() % 4 == 0)
    {
      add_new_npc(get_random_npc_type(), get_npc_spawn_point());
    }

    // Handle player stat recovery:
    adjust_player_current_health(HEALTH_RECOVERY_RATE);
    adjust_player_current_energy(ENERGY_RECOVERY_RATE);

    layer_mark_dirty(window_get_root_layer(g_graphics_window));
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
  if (!in_focus && g_game_mode == ACTIVE_MODE)
  {
    set_game_mode(MAIN_MENU_MODE);
  }
}

/******************************************************************************
   Function: strcat_item_name

Description: Concatenates the name of a given item onto a given string.

     Inputs: dest_str  - Pointer to the destination string.
             item_type - Integer representing the item of interest.

    Outputs: None.
******************************************************************************/
void strcat_item_name(char *dest_str, const int16_t item_type)
{
  if (item_type < FIRST_HEAVY_ITEM)
  {
    strcat(dest_str, "Pebble");
    strcat_magic_type(dest_str, item_type);
  }
  else
  {
    switch(item_type)
    {
      /*case KEY:
        strcat(dest_str, "Key");
        break;
      case QUEST_ITEM:
        g_quest->type == RESCUE ? strcat(dest_str, "Captive") :
                                  strcat(dest_str, "Artifact");
        break;*/
      case ROBE:
        strcat(dest_str, "Robe");
        break;
      case LIGHT_ARMOR:
        strcat(dest_str, "L. Armor");
        break;
      case HEAVY_ARMOR:
        strcat(dest_str, "H. Armor");
        break;
      case SHIELD:
        strcat(dest_str, "Shield");
        break;
      case DAGGER:
        strcat(dest_str, "Dagger");
        break;
      case SWORD:
        strcat(dest_str, "Sword");
        break;
      case AXE:
        strcat(dest_str, "Axe");
        break;
      case STAFF:
        strcat(dest_str, "Staff");
        break;
      case MACE:
        strcat(dest_str, "Mace");
        break;
      case FLAIL:
        strcat(dest_str, "Flail");
        break;
      case BOW:
        strcat(dest_str, "Bow");
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
void strcat_magic_type(char *dest_str, const int16_t magic_type)
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
void strcat_stat_name(char *dest_str, const int16_t stat)
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
      strcat(dest_str, "Phys. Power");
      break;
    case PHYSICAL_DEFENSE:
      strcat(dest_str, "Phys. Defense");
      break;
    case MAGICAL_POWER:
      strcat(dest_str, "Mag. Power");
      break;
    case MAGICAL_DEFENSE:
      strcat(dest_str, "Mag. Defense");
      break;
  }
  strcat(dest_str, " ");
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
void strcat_stat_value(char *dest_str, const int16_t stat)
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
void strcat_int(char *dest_str, int16_t integer)
{
  int16_t i, j;
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
   Function: assign_minor_stats

Description: Assigns values to the minor stats of a given stats array according
             to its major stat values (STRENGTH, AGILITY, and INTELLECT), which
             must already be assigned.

     Inputs: stats_array - Stats array of the character of interest.

    Outputs: None.
******************************************************************************/
void assign_minor_stats(int16_t *stats_array)
{
  stats_array[MAX_HEALTH]       = stats_array[STRENGTH * 10];
  stats_array[MAX_ENERGY]       = stats_array[INTELLECT * 10];
  stats_array[PHYSICAL_POWER]   = stats_array[STRENGTH * 2] +
                                    stats_array[AGILITY];
  stats_array[PHYSICAL_DEFENSE] = stats_array[STRENGTH] +
                                    stats_array[AGILITY * 2];
  stats_array[MAGICAL_POWER]    = stats_array[INTELLECT * 2] +
                                    stats_array[AGILITY];
  stats_array[MAGICAL_DEFENSE]  = stats_array[INTELLECT] +
                                    stats_array[AGILITY * 2];
}

/******************************************************************************
   Function: add_item_to_inventory

Description: Adds an item of a given type to the player's inventory. If it's a
             heavy item and there's no more room for heavy items, the "replace
             item" menu will be shown.

     Inputs: item_type - The type of item to be added.

    Outputs: None.
******************************************************************************/
void add_item_to_inventory(const int16_t item_type)
{
  int16_t i;

  if (item_type < FIRST_HEAVY_ITEM)
  {
    g_player->pebbles[item_type]++;
  }
  else
  {
    for (i = 0; i < MAX_HEAVY_ITEMS; ++i)
    {
      if (g_player->heavy_items[i]->type == NONE)
      {
        init_heavy_item(g_player->heavy_items[i], item_type);

        return;
      }
    }

    // If we reach this point, the player's heavy item array is full:
    g_current_selection = item_type;
    set_game_mode(REPLACE_ITEM_MODE);
  }
}

/******************************************************************************
   Function: equip_heavy_item

Description: Equips a given heavy item to its appropriate equip target.

     Inputs: item - Pointer to the heavy item to be equipped.

    Outputs: None.
******************************************************************************/
void equip_heavy_item(heavy_item_t *const item)
{
  int16_t equip_target = get_equip_target(item->type);

  // If the item equips to the right hand, unequip any equipped Pebble:
  if (equip_target == RIGHT_HAND)
  {
    g_player->equipped_pebble = NONE;
  }

  // Equip the heavy item:
  g_player->equipped_heavy_items[equip_target] = item;
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
  int i;

  g_player->level             = 1;
  g_player->exp_points        = 0;
  g_player->num_pebbles_found = 0;
  g_player->equipped_pebble   = NONE;
  g_player->stats[STRENGTH]   = DEFAULT_BASE_STAT_VALUE;
  g_player->stats[AGILITY]    = DEFAULT_BASE_STAT_VALUE;
  g_player->stats[INTELLECT]  = DEFAULT_BASE_STAT_VALUE;
  assign_minor_stats(g_player->stats);
  for (i = 0; i < NUM_PEBBLE_TYPES; ++i)
  {
    g_player->pebbles[i] = 0;
  }
  for (i = 0; i < MAX_HEAVY_ITEMS; ++i)
  {
    init_heavy_item(g_player->heavy_items[i], NONE);
  }
  for (i = 0; i < NUM_EQUIP_TARGETS; ++i)
  {
    g_player->equipped_heavy_items[i] = NULL;
  }
  add_item_to_inventory(DAGGER);
  add_item_to_inventory(ROBE);
  equip_heavy_item(g_player->heavy_items[0]);
  equip_heavy_item(g_player->heavy_items[1]);
}

/******************************************************************************
   Function: deinit_player

Description: Deinitializes the global player character struct, freeing
             associated memory.

     Inputs: None.

    Outputs: None.
******************************************************************************/
void deinit_player(void)
{
  int16_t i;

  if (g_player != NULL)
  {
    for (i = 0; i < MAX_HEAVY_ITEMS; ++i)
    {
      free(g_player->heavy_items[i]);
    }
    free(g_player);
    g_player = NULL;
  }
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
void init_npc(npc_t *npc, const int16_t type, const GPoint position)
{
  int16_t i;

  npc->type             = type;
  npc->position         = position;
  npc->next             = NULL;
  npc->stats[STRENGTH]  = g_player->stats[STRENGTH]  / 5;
  npc->stats[AGILITY]   = g_player->stats[AGILITY]   / 5;
  npc->stats[INTELLECT] = g_player->stats[INTELLECT] / 5;
  for (i = 0; i < NUM_STATUS_EFFECTS; ++i)
  {
    npc->status_effects[i] = 0;
  }

  // Adjust for each NPC type's weaknesses/strengths:
  if (type == ORC     ||
      type == WARRIOR ||
      type == BEAR    ||
      type == OGRE    ||
      type == TROLL)
  {
    npc->stats[STRENGTH] *= 2;
  }
  if (type == THIEF   ||
      type == WARRIOR ||
      type == GOBLIN  ||
      type == ORC)
  {
    npc->stats[AGILITY] *= 2;
  }
  if (type == ARCHMAGE ||
      type == MAGE)
  {
    npc->stats[INTELLECT] *= 2;
  }
  assign_minor_stats(npc->stats);
}

/******************************************************************************
   Function: init_heavy_item

Description: Initializes a new heavy item struct according to a given type.

     Inputs: item - Pointer to the heavy item struct.
             type - The type of heavy item to be initialized.

    Outputs: None.
******************************************************************************/
void init_heavy_item(heavy_item_t *item, const int16_t type)
{
  item->type           = type;
  item->infused_pebble = NONE;
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
  int16_t i, j, wall_width;
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
   Function: init_quest

Description: Initializes the global quest struct according to a given quest
             type.

     Inputs: type - The type of quest to initialize.

    Outputs: None.
******************************************************************************/
void init_quest(const int16_t type)
{
  g_quest->type             = type;
  g_quest->completed        = false;
  g_quest->kills            = 0;
  g_quest->npcs             = NULL;
  g_quest->primary_npc_type = GOBLIN;
  g_quest->num_npcs         = 5 * (rand() % 5 + 2);  // 10-30 enemies.
  //g_quest->exp_reward       = 5 * g_quest->num_npcs; // 50-150 exp. points.
  init_quest_map();

  // Move and orient the player:
  g_player->position = g_quest->starting_point;
  set_player_direction(get_opposite_direction(g_quest->entrance_direction));
}

/******************************************************************************
   Function: init_quest_map

Description: Initializes the current quest's map (i.e., its 2D array of cells).

     Inputs: None.

    Outputs: None.
******************************************************************************/
void init_quest_map(void)
{
  int16_t i, j, builder_direction;
  GPoint builder_position;

  // First, set each cell to solid:
  for (i = 0; i < MAP_WIDTH; ++i)
  {
    for (j = 0; j < MAP_HEIGHT; ++j)
    {
      g_quest->map[i][j] = SOLID;
    }
  }

  // Next, set starting and exit points:
  switch (g_quest->entrance_direction = rand() % NUM_DIRECTIONS)
  {
    case NORTH:
      g_quest->starting_point = RANDOM_POINT_NORTH;
      g_quest->ending_point   = RANDOM_POINT_SOUTH;
      break;
    case SOUTH:
      g_quest->starting_point = RANDOM_POINT_SOUTH;
      g_quest->ending_point   = RANDOM_POINT_NORTH;
      break;
    case EAST:
      g_quest->starting_point = RANDOM_POINT_EAST;
      g_quest->ending_point   = RANDOM_POINT_WEST;
      break;
    default: // case WEST:
      g_quest->starting_point = RANDOM_POINT_WEST;
      g_quest->ending_point   = RANDOM_POINT_EAST;
      break;
  }

  // Now, carve a path between the starting and end points:
  builder_position  = g_quest->starting_point;
  builder_direction = get_opposite_direction(g_quest->entrance_direction);
  while (!gpoint_equal(&builder_position, &g_quest->ending_point))
  {
    set_cell_type(builder_position, EMPTY);
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
    g_quest->exit_direction = builder_direction;
    if (rand() % NUM_DIRECTIONS == 0) // 25% chance of turning.
    {
      builder_direction = rand() % NUM_DIRECTIONS;
    }
  }
  set_cell_type(builder_position, EMPTY);

  // Finally, add special NPCs/objects, if applicable:
  if (g_quest->type == MAIN_QUEST_3)
  {
    add_new_npc(ARCHMAGE, g_quest->ending_point);
  }
  /*else if (g_quest->type == RECOVER_ITEM || g_quest->type == RESCUE)
  {
    set_cell_type(g_quest->ending_point, QUEST_ITEM);
  }*/
}

/******************************************************************************
   Function: deinit_quest

Description: Deinitializes the global quest struct, freeing associated memory.
             Also resets relevant player struct data members, including health,
             energy, status effects, keys, and quest items.

     Inputs: None.

    Outputs: None.
******************************************************************************/
void deinit_quest(void)
{
  int16_t i;

  if (g_quest != NULL)
  {
    // Reset relevant player data:
    //g_player->inventory[QUEST_ITEM]->n = 0;
    //g_player->inventory[KEY]->n        = 0;
    g_player->stats[CURRENT_HEALTH]    = g_player->stats[MAX_HEALTH];
    g_player->stats[CURRENT_ENERGY]    = g_player->stats[MAX_ENERGY];
    for (i = 0; i < NUM_STATUS_EFFECTS; ++i)
    {
      g_player->status_effects[i] = 0;
    }

    // Remove NPCs and the quest struct itself from memory:
    while (g_quest->npcs != NULL)
    {
      remove_npc(g_quest->npcs);
    }
    free(g_quest);
    g_quest = NULL;
  }
}

/******************************************************************************
   Function: init_scroll

Description: Initializes the scroll window.

     Inputs: None.

    Outputs: None.
******************************************************************************/
void init_scroll(void)
{
  g_scroll_window       = window_create();
  g_scroll_scroll_layer = scroll_layer_create(FULL_SCREEN_FRAME);
  scroll_layer_set_click_config_onto_window(g_scroll_scroll_layer,
                                            g_scroll_window);
  window_set_click_config_provider(g_scroll_window,
                                   (ClickConfigProvider)
                                     scroll_click_config_provider);
  layer_add_child(window_get_root_layer(g_scroll_window),
                  scroll_layer_get_layer(g_scroll_scroll_layer));
  g_scroll_text_layer = text_layer_create(SCROLL_TEXT_LAYER_FRAME);
  text_layer_set_background_color(g_scroll_text_layer, GColorWhite);
  text_layer_set_text_color(g_scroll_text_layer, GColorBlack);
  text_layer_set_font(g_scroll_text_layer, SCROLL_FONT);
  text_layer_set_text_alignment(g_scroll_text_layer, GTextAlignmentLeft);
  scroll_layer_add_child(g_scroll_scroll_layer,
                         text_layer_get_layer(g_scroll_text_layer));
}

/******************************************************************************
   Function: deinit_scroll

Description: Deinitializes the scroll window.

     Inputs: None.

    Outputs: None.
******************************************************************************/
void deinit_scroll(void)
{
  text_layer_destroy(g_scroll_text_layer);
  scroll_layer_destroy(g_scroll_scroll_layer);
  window_destroy(g_scroll_window);
}

/******************************************************************************
   Function: init_graphics

Description: Initializes the graphics window.

     Inputs: None.

    Outputs: None.
******************************************************************************/
void init_graphics(void)
{
  // Graphics window:
  g_graphics_window = window_create();
  window_set_background_color(g_graphics_window, GColorBlack);
  window_set_window_handlers(g_graphics_window, (WindowHandlers)
  {
    .appear    = graphics_window_appear,
  });
  window_set_click_config_provider(g_graphics_window,
                                   (ClickConfigProvider)
                                     graphics_click_config_provider);
  layer_set_update_proc(window_get_root_layer(g_graphics_window), draw_scene);

  // Graphics frame inverter (for the "flash" effect):
  g_inverter_layer = inverter_layer_create(GRAPHICS_FRAME);
  layer_add_child(window_get_root_layer(g_graphics_window),
                  inverter_layer_get_layer(g_inverter_layer));

  // Tick timer subscription:
  tick_timer_service_subscribe(SECOND_UNIT, tick_handler);
}

/******************************************************************************
   Function: deinit_graphics

Description: Deinitializes the graphics window.

     Inputs: None.

    Outputs: None.
******************************************************************************/
void deinit_graphics(void)
{
  inverter_layer_destroy(g_inverter_layer);
  window_destroy(g_graphics_window);
}

/******************************************************************************
   Function: init_menu_windows

Description: Initializes the menu windows.

     Inputs: None.

    Outputs: None.
******************************************************************************/
void init_menu_windows(void)
{
  // Main menu window:
  g_main_menu_window     = window_create();
  g_main_menu_menu_layer = menu_layer_create(FULL_SCREEN_FRAME);
  menu_layer_set_callbacks(g_main_menu_menu_layer, NULL, (MenuLayerCallbacks)
  {
    .get_header_height = menu_get_header_height_callback,
    .draw_header       = menu_draw_header_callback,
    .get_num_rows      = menu_get_num_rows_callback,
    .draw_row          = menu_draw_row_callback,
    .select_click      = menu_select_callback,
  });
  layer_add_child(window_get_root_layer(g_main_menu_window),
                  menu_layer_get_layer(g_main_menu_menu_layer));
  menu_layer_set_click_config_onto_window(g_main_menu_menu_layer,
                                          g_main_menu_window);

  // Ad hoc menu window:
  g_ad_hoc_menu_window     = window_create();
  g_ad_hoc_menu_menu_layer = menu_layer_create(FULL_SCREEN_FRAME);
  menu_layer_set_callbacks(g_ad_hoc_menu_menu_layer, NULL, (MenuLayerCallbacks)
  {
    .get_header_height = menu_get_header_height_callback,
    .draw_header       = menu_draw_header_callback,
    .get_num_rows      = menu_get_num_rows_callback,
    .draw_row          = menu_draw_row_callback,
    .select_click      = menu_select_callback,
  });
  layer_add_child(window_get_root_layer(g_ad_hoc_menu_window),
                  menu_layer_get_layer(g_ad_hoc_menu_menu_layer));
  menu_layer_set_click_config_onto_window(g_ad_hoc_menu_menu_layer,
                                          g_ad_hoc_menu_window);

  // Options menu window:
  g_options_menu_window     = window_create();
  g_options_menu_menu_layer = menu_layer_create(FULL_SCREEN_FRAME);
  menu_layer_set_callbacks(g_options_menu_menu_layer,
                           NULL,
                           (MenuLayerCallbacks)
  {
    .get_header_height = menu_get_header_height_callback,
    .draw_header       = menu_draw_header_callback,
    .get_num_rows      = menu_get_num_rows_callback,
    .draw_row          = menu_draw_row_callback,
    .select_click      = menu_select_callback,
  });
  layer_add_child(window_get_root_layer(g_options_menu_window),
                  menu_layer_get_layer(g_options_menu_menu_layer));
  menu_layer_set_click_config_onto_window(g_options_menu_menu_layer,
                                          g_options_menu_window);

  // Heavy items menu window:
  g_heavy_items_menu_window     = window_create();
  g_heavy_items_menu_menu_layer = menu_layer_create(FULL_SCREEN_FRAME);
  menu_layer_set_callbacks(g_heavy_items_menu_menu_layer,
                           NULL,
                           (MenuLayerCallbacks)
  {
    .get_header_height = menu_get_header_height_callback,
    .draw_header       = menu_draw_header_callback,
    .get_num_rows      = menu_get_num_rows_callback,
    .draw_row          = menu_draw_row_callback,
    .select_click      = menu_select_callback,
  });
  layer_add_child(window_get_root_layer(g_heavy_items_menu_window),
                  menu_layer_get_layer(g_heavy_items_menu_menu_layer));
  menu_layer_set_click_config_onto_window(g_heavy_items_menu_menu_layer,
                                          g_heavy_items_menu_window);
}

/******************************************************************************
   Function: deinit_menu_windows

Description: Deinitializes the menu windows.

     Inputs: None.

    Outputs: None.
******************************************************************************/
void deinit_menu_windows(void)
{
  menu_layer_destroy(g_main_menu_menu_layer);
  window_destroy(g_main_menu_window);
  menu_layer_destroy(g_ad_hoc_menu_menu_layer);
  window_destroy(g_ad_hoc_menu_window);
  menu_layer_destroy(g_options_menu_menu_layer);
  window_destroy(g_options_menu_window);
  menu_layer_destroy(g_heavy_items_menu_menu_layer);
  window_destroy(g_heavy_items_menu_window);
}

/******************************************************************************
   Function: init

Description: Initializes the PebbleQuest app.

     Inputs: None.

    Outputs: None.
******************************************************************************/
void init(void)
{
  int16_t i;

  srand(time(NULL));
  g_game_mode = MAIN_MENU_MODE;
  g_quest     = NULL;
  init_menu_windows();
  init_scroll();
  init_graphics();
  init_wall_coords();
  g_compass_path = gpath_create(&COMPASS_PATH_INFO);
  gpath_move_to(g_compass_path, GPoint(SCREEN_CENTER_POINT_X,
                                       GRAPHICS_FRAME_HEIGHT +
                                         STATUS_BAR_HEIGHT / 2));
  app_focus_service_subscribe(app_focus_handler);

  // Check for saved data and initialize the player struct:
  g_player = malloc(sizeof(player_t));
  for (i = 0; i < MAX_HEAVY_ITEMS; ++i)
  {
    g_player->heavy_items[i] = malloc(sizeof(heavy_item_t));
  }
  if (persist_exists(STORAGE_KEY))
  {
    for (i = 0; i < MAX_HEAVY_ITEMS; ++i)
    {
      persist_read_data(STORAGE_KEY + i,
                        g_player->heavy_items[i],
                        sizeof(heavy_item_t));
    }
    persist_read_data(STORAGE_KEY + MAX_HEAVY_ITEMS,
                      g_player,
                      sizeof(player_t));
  }
  else
  {
    init_player();
  }
  show_window(g_main_menu_window, NOT_ANIMATED);
}

/******************************************************************************
   Function: deinit

Description: Deinitializes the PebbleQuest app.

     Inputs: None.

    Outputs: None.
******************************************************************************/
void deinit(void)
{
  int16_t i;

  for (i = 0; i < MAX_HEAVY_ITEMS; ++i)
  {
    persist_write_data(STORAGE_KEY + i,
                       g_player->heavy_items[i],
                       sizeof(heavy_item_t));
  }
  persist_write_data(STORAGE_KEY + MAX_HEAVY_ITEMS,
                     g_player,
                     sizeof(player_t));
  deinit_scroll();
  deinit_graphics();
  deinit_menu_windows();
  deinit_quest();
  deinit_player();
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
