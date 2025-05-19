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

	unsigned long worker_count = 0;
	unsigned long warrior_count = 0;
	int i = 0;

	while (units[i])
	{
		t_obj *unit = units[i];
		if (unit->s_unit.type_id == UNIT_WORKER)
			worker_count++;
		else if (unit->s_unit.type_id == UNIT_WARRIOR)
			warrior_count++;
		i++;
	}

	// Early economy boost: spawn workers first
	if (worker_count < 3 && my_team->balance >= ft_get_unit_config(UNIT_WORKER)->cost)
		ft_create_unit(UNIT_WORKER);
	// Then spawn warriors for offense
	else if (my_team->balance >= ft_get_unit_config(UNIT_WARRIOR)->cost)
		ft_create_unit(UNIT_WARRIOR);

	i = 0;
	while (units[i])
	{
		t_obj *unit = units[i];
		if (unit->s_unit.type_id == UNIT_WORKER)
		{
			t_obj *res = ft_get_nearest_resource(unit);
			if (res)
				ft_travel_attack(unit, res);
		}
		else if (unit->s_unit.type_id == UNIT_WARRIOR)
		{
			t_obj *enemy = ft_get_nearest_opponent_unit(unit);
			if (enemy)
				ft_travel_attack(unit, enemy);
			else
				ft_travel_attack(unit, enemy_core);
		}
		i++;
	}
	free(units);
}
