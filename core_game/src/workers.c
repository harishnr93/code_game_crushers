#include "../include/my_core_bot.h"

void worker_combine(t_obj *worker)
{
	// t_obj *enemy = ft_get_nearest_opponent_unit(worker);
	// 		if (enemy)
	// 			ft_travel_attack(worker, enemy);
	t_obj *resource = ft_get_nearest_resource(worker);
	if (resource)
		ft_travel_attack(worker, resource);
	else
		ft_travel_attack(worker, ft_get_first_opponent_core());
		// sft_travel_to_obj(worker, resource);

}