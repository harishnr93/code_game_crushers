void ft_user_loop(void *data)
{
    (void)data;
    t_obj **units = ft_get_my_units();
    t_team *my_team = ft_get_my_team();
    t_obj *enemy_core = ft_get_first_opponent_core();
    int warrior_count = 0;
    int worker_count = 0;
    t_obj *leader = NULL;
    int i = 0;

    // Count units and pick oldest warrior as leader
    while (units[i])
    {
        if (units[i]->s_unit.type_id == UNIT_WARRIOR)
        {
            if (!leader || units[i]->id < leader->id)
                leader = units[i];
            warrior_count++;
        }
        else if (units[i]->s_unit.type_id == UNIT_WORKER)
            worker_count++;
        i++;
    }

    // Spawn logic
    if (worker_count < 3)
        ft_create_unit(UNIT_WORKER);  // Emergency worker replacement
    else if (worker_count < 4)
        ft_create_unit(UNIT_WORKER);  // Early game worker setup
    else if (my_team->balance >= 100)
        ft_create_unit(UNIT_WARRIOR); // Continuous warrior spawn

    // Unit behavior loop
    i = 0;
    while (units[i])
    {
        t_obj *curr = units[i];

        if (curr->s_unit.type_id == UNIT_WORKER)
        {
            t_obj *resource = ft_get_nearest_resource(curr);
            if (resource)
                ft_travel_attack(curr, resource);
            else
                ft_travel_attack(curr, enemy_core);
        }
        else if (curr->s_unit.type_id == UNIT_WARRIOR)
        {
            t_obj *enemy = ft_get_nearest_opponent_unit(curr);
            if (enemy && ft_distance(curr, enemy) < 15)
                ft_travel_attack(curr, enemy);
            else if (curr == leader)
                ft_travel_attack(curr, enemy_core);
            else
                ft_travel_to_obj(curr, leader);
        }
        i++;
    }
    free(units);
}


/******************************* */

#define GROUP_SIZE 3
#define SCATTER_RADIUS 10
#define REGROUP_DISTANCE 10

#include <stdlib.h>

void ft_user_loop(void *data)
{
    (void)data;
    t_obj **units = ft_get_my_units();
    t_team *my_team = ft_get_my_team();
    t_obj *enemy_core = ft_get_first_opponent_core();
    t_obj *my_core = ft_get_my_core();
    unsigned long balance = my_team->balance;

    int worker_count = 0;
    t_obj *warriors[100];
    int warrior_count = 0;
    int i = 0;

    // Count and classify units
    while (units[i])
    {
        if (units[i]->s_unit.type_id == UNIT_WORKER)
            worker_count++;
        else if (units[i]->s_unit.type_id == UNIT_WARRIOR && warrior_count < 100)
            warriors[warrior_count++] = units[i];
        i++;
    }

    // Maintain 4 workers
    while (worker_count < 4 && balance >= 100)
    {
        ft_create_unit(UNIT_WORKER);
        balance -= 100;
        worker_count++;
    }

    // Spawn warriors with remaining balance
    while (balance >= 100)
    {
        ft_create_unit(UNIT_WARRIOR);
        balance -= 100;
    }

    // Worker logic
    i = 0;
    while (units[i])
    {
        if (units[i]->s_unit.type_id == UNIT_WORKER)
        {
            t_obj *resource = ft_get_nearest_resource(units[i]);
            if (resource)
                ft_travel_attack(units[i], resource);
            else
                ft_travel_to_obj(units[i], my_core); // fallback
        }
        i++;
    }

    // Warrior logic: scatter and regroup
    for (i = 0; i < warrior_count; i++)
    {
        t_obj *curr = warriors[i];
        t_obj *enemy = ft_get_nearest_opponent_unit(curr);

        if (enemy && ft_distance(curr, enemy) < 15)
        {
            ft_travel_attack(curr, enemy); // priority: fight nearby threat
            continue;
        }

        // Phase 1: scatter to a random rally point near own core
        if (curr->data == NULL)
        {
            unsigned long rx = my_core->x + (rand() % (SCATTER_RADIUS * 2)) - SCATTER_RADIUS;
            unsigned long ry = my_core->y + (rand() % (SCATTER_RADIUS * 2)) - SCATTER_RADIUS;
            ft_travel_to(curr, rx, ry);
            curr->data = (void *)1; // Mark as scattered
            continue;
        }

        // Phase 2: regroup and attack in squads
        if (i % GROUP_SIZE == 0)
        {
            // Squad leader goes for the core
            ft_travel_attack(curr, enemy_core);
        }
        else
        {
            // Follow the previous squad member
            ft_travel_to_obj(curr, warriors[i - 1]);
        }
    }

    free(units);
}

/******** */


#include "../include/my_core_bot.h"

void	ft_init_func(void *data);
void	ft_user_loop(void *data);

int	main(int argc, char **argv)
{
	int	won;
	// ft_enable_debug(); // uncomment this to show more debug information in the console when running a game
	ft_init_con("x-men", &argc, argv);
	won = ft_loop(&ft_init_func, &ft_user_loop, NULL, NULL);
	ft_close_con();
	if (won)
		return (0);
	return (1);
}

// this function is called once at the start of the game
void	ft_init_func(void *data)
{
	(void)data;

	printf("Init CORE Bot\n");
}

void ft_user_loop(void *data)
{
    (void)data;
    t_obj **units = ft_get_my_units();
    t_obj *enemy_core = ft_get_first_opponent_core();
    t_team *my_team = ft_get_my_team();
    t_obj *resource = NULL;
    int warrior_count = 0;
    int worker_count = 0;
    int i = 0;

    while (units[i])
    {
        if (units[i]->s_unit.type_id == UNIT_WARRIOR)
            warrior_count++;
        else if (units[i]->s_unit.type_id == UNIT_WORKER)
            worker_count++;
        i++;
    }

    // Focus on warrior production with minimal workers for economy
    if (worker_count < 3 && my_team->balance >= 100)
        ft_create_unit(UNIT_WORKER);
    else if (my_team->balance >= 150)
        ft_create_unit(UNIT_WARRIOR);

    i = 0;
    while (units[i])
    {
        t_obj *curr = units[i];
        if (curr->s_unit.type_id == UNIT_WARRIOR)
        {
            t_obj *enemy = ft_get_nearest_opponent_unit(curr);
            // Attack core if close, otherwise prioritize nearby units
            if (ft_distance(curr, enemy_core) < 20 || !enemy)
                ft_travel_attack(curr, enemy_core);
            else
                ft_travel_attack(curr, enemy);
        }
        else if (curr->s_unit.type_id == UNIT_WORKER)
        {
            resource = ft_get_nearest_resource(curr);
            // Workers gather resources, but join attack if enough workers exist
            if (resource && worker_count <= 5)
                ft_travel_attack(curr, resource);
            else
                ft_travel_attack(curr, enemy_core);
        }
        i++;
    }
    free(units);
}
