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
void	ft_user_loop(void *data)
{
	(void)data;

	t_team	*my_team = ft_get_my_team();
	t_obj	**units = ft_get_my_units();
	t_obj	*enemy_core = ft_get_first_opponent_core();
	t_obj	*my_core = ft_get_my_core();

	unsigned long balance = my_team->balance;
	t_unit_config *worker_cfg = ft_get_unit_config(UNIT_WORKER);
	t_unit_config *warrior_cfg = ft_get_unit_config(UNIT_WARRIOR);

	int	worker_count = 0;
	int	warrior_count = 0;
	int	i = 0;

	while (units[i])
	{
		if (units[i]->s_unit.type_id == UNIT_WORKER)
			worker_count++;
		else if (units[i]->s_unit.type_id == UNIT_WARRIOR)
			warrior_count++;
		i++;
	}

	// Dynamic unit production strategy
	if (worker_count < 3 && balance >= worker_cfg->cost)
		ft_create_unit(UNIT_WORKER);
	else if (balance >= warrior_cfg->cost)
		ft_create_unit(UNIT_WARRIOR);

	i = 0;
	while (units[i])
	{
		t_obj *unit = units[i];
		if (unit->state != STATE_ALIVE)
		{
			i++;
			continue;
		}

		if (unit->s_unit.type_id == UNIT_WORKER)
		{
			t_obj *res = ft_get_nearest_resource(unit);
			if (res && res->state == STATE_ALIVE)
				ft_travel_attack(unit, res); // Efficient gathering
		}
		else if (unit->s_unit.type_id == UNIT_WARRIOR)
		{
			// First protect core if enemy is near
			t_obj *near_threat = ft_get_nearest_opponent_unit(my_core);
			if (near_threat && ft_distance(my_core, near_threat) < 10)
				ft_travel_attack(unit, near_threat);
			else
			{
				t_obj *enemy_unit = ft_get_nearest_opponent_unit(unit);
				if (enemy_unit && ft_distance(unit, enemy_unit) <= warrior_cfg->max_range + 2)
					ft_travel_attack(unit, enemy_unit);
				else if (enemy_core)
					ft_travel_attack(unit, enemy_core);
			}
		}
		i++;
	}
	free(units);
}
