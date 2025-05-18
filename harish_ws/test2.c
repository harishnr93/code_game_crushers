#include "../include/my_core_bot.h"
#include <stdlib.h>
#include <math.h>
#include <time.h>

void    ft_init_func(void *data);
void    ft_user_loop(void *data);

int main(int argc, char **argv)
{
    int won;
    // ft_enable_debug(); // uncomment this to show more debug information in the console when running a game
    ft_init_con("x-men", &argc, argv);
    won = ft_loop(&ft_init_func, &ft_user_loop, NULL, NULL);
    ft_close_con();
    if (won)
        return (0);
    return (1);
}

// this function is called once at the start of the game
void    ft_init_func(void *data)
{
    (void)data;

    printf("Init CORE Bot\n");
}

// Main game loop function
void ft_user_loop(void *data)
{
    (void)data;
    
    // Get essential game state information
    t_obj **units = ft_get_my_units();
    t_obj **enemy_units = ft_get_opponent_units();
    t_obj *enemy_core = ft_get_first_opponent_core();
    t_obj *my_core = ft_get_my_core();
    t_team *my_team = ft_get_my_team();
    
    // Unit counters and strategy variables
    int warrior_count = 0;
    int worker_count = 0;
    int i = 0;
    int enemy_worker_count = 0;
    int enemy_warrior_count = 0;
    
    // Advanced strategy control variables
    static int attack_phase = 0;   // 0: Building, 1: Attacking, 2: Hunter-Killer
    static int attack_timer = 0;   // Timer for coordinated attacks
    static int attack_group_size = 5;  // Optimal attack group size
    static int hunter_group_size = 3;  // Size of hunter-killer teams
    
    // Count unit types
    while (units[i])
    {
        if (units[i]->s_unit.type_id == UNIT_WARRIOR)
            warrior_count++;
        else if (units[i]->s_unit.type_id == UNIT_WORKER)
            worker_count++;
        i++;
    }
    
    // Count enemy unit types
    i = 0;
    while (enemy_units && enemy_units[i])
    {
        if (enemy_units[i]->s_unit.type_id == UNIT_WARRIOR)
            enemy_warrior_count++;
        else if (enemy_units[i]->s_unit.type_id == UNIT_WORKER)
            enemy_worker_count++;
        i++;
    }
    
    // Collect warriors in array for group tactics
    t_obj *warriors[100];
    int warrior_index = 0;
    i = 0;
    while (units[i] && warrior_index < 100)
    {
        if (units[i]->s_unit.type_id == UNIT_WARRIOR)
        {
            warriors[warrior_index] = units[i];
            warrior_index++;
        }
        i++;
    }
    
    // Economy management - ensure balanced economy
    int desired_workers = 4; // Reduced from 8 to favor more warriors
    
    // Early game: focus on economy
    if (worker_count < desired_workers && (worker_count < 3 || my_team->balance >= 100))
    {
        ft_create_unit(UNIT_WORKER);
    }
    // Mid-late game: focus on warriors
    else if (my_team->balance >= 150)
    {
        ft_create_unit(UNIT_WARRIOR);
    }
    
    // Surplus management - ensure we're not stockpiling resources
    if (my_team->balance >= 300)
    {
        ft_create_unit(UNIT_WARRIOR);
    }
    
    // Dynamic attack phase management based on game state
    attack_timer++;
    
    // Determine if we should switch to hunter-killer mode (target enemy workers)
    if (enemy_worker_count > 0 && warrior_count >= hunter_group_size && attack_phase != 2)
    {
        attack_phase = 2; // Switch to hunter-killer phase
        attack_timer = 0;
    }
    // Switch to direct core attack when we have enough warriors
    else if (warrior_count >= attack_group_size && attack_phase == 0)
    {
        attack_phase = 1; // Switch to attack phase
        attack_timer = 0;
    }
    // Reset to building phase if attack has gone on for too long without success
    else if (attack_timer > 200 && (attack_phase == 1 || attack_phase == 2))
    {
        attack_phase = 0; // Reset to building phase
        attack_timer = 0;
        // Increase attack group size if we're struggling to win
        attack_group_size = attack_group_size < 10 ? attack_group_size + 1 : attack_group_size;
    }
    
    // Worker behavior - resource gathering with improved safety
    i = 0;
    while (units[i])
    {
        if (units[i]->s_unit.type_id == UNIT_WORKER)
        {
            t_obj *resource = ft_get_nearest_resource(units[i]);
            t_obj *enemy = ft_get_nearest_opponent_unit(units[i]);
            
            // If worker is threatened, run to safety
            if (enemy && ft_distance(units[i], enemy) < 20) // Increased safety distance
            {
                double escape_angle = atan2(units[i]->y - enemy->y, units[i]->x - enemy->x);
                int escape_distance = 30;
                unsigned long escape_x = units[i]->x + (unsigned long)(escape_distance * cos(escape_angle));
                unsigned long escape_y = units[i]->y + (unsigned long)(escape_distance * sin(escape_angle));
                
                // Ensure we're not escaping off the map
                escape_x = escape_x < 10 ? 10 : escape_x;
                escape_y = escape_y < 10 ? 10 : escape_y;
                
                ft_travel_to(units[i], escape_x, escape_y);
            }
            // Otherwise gather resources
            else if (resource)
            {
                ft_travel_attack(units[i], resource);
            }
            // No resources nearby, attack enemy core if we have a significant force
            else if (warrior_count > attack_group_size * 2)
            {
                ft_travel_to_obj(units[i], enemy_core);
            }
            // Otherwise stay near our core for safety
            else
            {
                double angle = ((double)rand() / RAND_MAX) * 2 * M_PI;
                int safe_radius = 15 + (rand() % 10);
                unsigned long safe_x = my_core->x + (unsigned long)(safe_radius * cos(angle));
                unsigned long safe_y = my_core->y + (unsigned long)(safe_radius * sin(angle));
                
                ft_travel_to(units[i], safe_x, safe_y);
            }
        }
        i++;
    }
    
    // Warrior behavior - tactical combat with improved targeting
    for (i = 0; i < warrior_index; i++)
    {
        t_obj *curr = warriors[i];
        t_obj *enemy = ft_get_nearest_opponent_unit(curr);
        t_obj *enemy_worker = NULL;
        
        // Find the nearest enemy worker specifically (for hunter-killer teams)
        int j = 0;
        float min_dist = 999999.0;
        while (enemy_units && enemy_units[j])
        {
            if (enemy_units[j]->s_unit.type_id == UNIT_WORKER)
            {
                float dist = ft_distance(curr, enemy_units[j]);
                if (dist < min_dist)
                {
                    min_dist = dist;
                    enemy_worker = enemy_units[j];
                }
            }
            j++;
        }
        
        // Core defense - protect our core at all costs
        if (enemy && ft_distance(enemy, my_core) < 35) // Increased protection radius
        {
            ft_travel_attack(curr, enemy);
        }
        // Hunter-Killer mode - target enemy workers to cripple economy
        else if (attack_phase == 2 && enemy_worker && i < hunter_group_size * 2)
        {
            ft_travel_attack(curr, enemy_worker);
        }
        // Group attack strategy on enemy core
        else if (attack_phase == 1 || (attack_phase == 2 && i >= hunter_group_size * 2))
        {
            // Calculate formation position based on warrior's index for better spreading
            int row = i / 5;
            int col = i % 5;
            int offset_x = (col - 2) * 4;
            int offset_y = (row - 2) * 4;
            
            // Group attack on enemy core with improved formation
            if (enemy_core)
            {
                // Check if any enemies are near this warrior
                if (enemy && ft_distance(curr, enemy) < 15)
                {
                    // Engage nearby enemies first before continuing to core
                    ft_travel_attack(curr, enemy);
                }
                // Travel to position near core
                else if (ft_distance(curr, enemy_core) > 15)
                {
                    // Create a pseudo-random offset for each warrior to avoid clumping
                    int rand_offset_x = offset_x + ((i * 17) % 7) - 3;
                    int rand_offset_y = offset_y + ((i * 23) % 7) - 3;
                    
                    ft_travel_to(curr, enemy_core->x + rand_offset_x, enemy_core->y + rand_offset_y);
                }
                // Attack core when close
                else
                {
                    ft_attack(curr, enemy_core);
                }
            }
        }
        // Opportunistic attacks - target any close enemies
        else if (enemy && ft_distance(curr, enemy) < 25) // Increased aggressive range
        {
            ft_travel_attack(curr, enemy);
        }
        // Defensive positioning while building forces
        else
        {
            // Form defensive perimeter around core with slight randomization
            double angle = (2 * M_PI * i) / warrior_index + ((double)rand() / RAND_MAX) * 0.2 - 0.1;
            int radius = 25 + (i % 5); // Varied radius for better coverage
            unsigned long pos_x = my_core->x + (unsigned long)(radius * cos(angle));
            unsigned long pos_y = my_core->y + (unsigned long)(radius * sin(angle));
            
            ft_travel_to(curr, pos_x, pos_y);
        }
    }
    
    // Cleanup allocated memory
    free(units);
    if (enemy_units)
        free(enemy_units);
}