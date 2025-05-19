#include "../include/my_core_bot.h"
#include "structs.h"

void	ft_init_func(void *data);
void	ft_user_loop(void *data);
void worker_combine(t_obj *worker);

int	main(int argc, char **argv)
{
	int	won;

	// ft_enable_debug();
		// uncomment this to show more debug information in the console when running a game
	t_game_data *game_data = malloc(2 * sizeof(int));
	game_data->warriors_amount = 0;
	game_data->workers_amount = 0;

	ft_init_con("x-men", &argc, argv);
	won = ft_loop(&ft_init_func, &ft_user_loop, NULL, game_data);
	// won = ft_loop(&ft_init_func, &ft_user_loop, NULL, NULL);
	ft_close_con();
	if (won)
		return (0);
	return (1);
}

// this function is called once at the start of the game
void	ft_init_func(void *data)
{
	(void)data;

	printf("Worker cost=%lu\n", ft_get_unit_config(UNIT_WORKER)->cost);
	printf("Worker hp=%lu\n", ft_get_unit_config(UNIT_WORKER)->hp);
	printf("Worker damage=%lu\n", ft_get_unit_config(UNIT_WORKER)->dmg_unit);
	printf("Warrior cost=%lu\n", ft_get_unit_config(UNIT_WARRIOR)->cost);
	printf("Warrior hp=%lu\n", ft_get_unit_config(UNIT_WARRIOR)->hp);
	printf("Warrior damage=%lu\n", ft_get_unit_config(UNIT_WARRIOR)->dmg_unit);
	ft_print_resources();
	
	// t_obj **res = game.resources;
	// res[0].
	printf("Init CORE Bot\n");
}

// this function is called every time new data is recieved
void	ft_user_loop(void *data)
{
	t_obj	**units;
	t_obj	*enemy_core;
	int		i;
	t_obj	*curr;
	int		amount_workers;
	int		amount_warriors;
	int		need_workers;
	int		need_warriors;
	int		attack_core;

	// (void)data;

	units = ft_get_my_units();

	enemy_core = ft_get_first_opponent_core();

	need_workers = 4;
	need_warriors = 4;

	i = 0;
	t_game_data *game_data = (t_game_data *)data;

	// printf("am at start =%d\n", *am);
	amount_workers = 0;
	amount_workers = 0;
	attack_core = 1;
	while (units[i]) 
	{
		curr = units[i];
		if (curr->s_unit.type_id == UNIT_WARRIOR) // if the unit is a warrior
		{
			amount_warriors++;
			t_obj *enemy = ft_get_nearest_opponent_unit(curr);
			if (enemy)
			{
				// if (attack_core)
				// {	
				// 	ft_travel_attack(curr, enemy_core);
				// 	attack_core = 0;
				// }
				// else 
					ft_travel_attack(curr, enemy);
					attack_core = -attack_core;
			}
			else
				ft_travel_attack(curr, enemy_core);
		}
		if (curr->s_unit.type_id == UNIT_WORKER)
		{
			amount_workers++;
			worker_combine(curr);
		}
		i++;
	}

	// int *k = malloc(sizeof(int));
	// if (*am != need_workers)
	// 	*am = amount_workers;

	if (amount_warriors <= need_warriors)
	{
		ft_create_unit(UNIT_WARRIOR);
		need_warriors--;
	}
	if (game_data->workers_amount < need_workers)
	{	
		// while (game_data->workers_amount < need_workers)
		// {
		// 	ft_create_unit(UNIT_WORKER);
		//  	game_data->workers_amount++;
		// 	printf("create worker =%d\n", game_data->workers_amount);
		// }
		ft_create_unit(UNIT_WORKER);
	 	game_data->workers_amount++;
		printf("create worker =%d\n", game_data->workers_amount);
		ft_create_unit(UNIT_WORKER);
		game_data->workers_amount++;
 	   	printf("create worker =%d\n", game_data->workers_amount);

	}
	int wallet = ft_get_my_team()->balance;
	if (game_data->warriors_amount == 0)
	{
		game_data->warriors_amount = 3;
		ft_create_unit(UNIT_WARRIOR);
	}
	if (game_data->warriors_amount == 3 && wallet >=  game_data->warriors_amount * 750)
	{
		ft_create_unit(UNIT_WARRIOR);
		ft_create_unit(UNIT_WARRIOR);
		ft_create_unit(UNIT_WARRIOR);
		game_data->warriors_amount = 1;
	}
	if (game_data->warriors_amount == 1)
	{
		ft_create_unit(UNIT_WARRIOR);
	}

	
	free(units);
}
