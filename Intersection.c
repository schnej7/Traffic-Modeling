#include <ross.h>



enum events { ARIVAL, DEPARTURE, DIRECTION_SELECT };

enum abs_directions { WEST_LEFT, WEST_STRAIGHT, WEST_RIGHT, EAST_LEFT, EAST_STRAIGHT, EAST_RIGHT, NORTH_LEFT, NORTH_STRAIGHT, NORTH_RIGHT, SOUTH_LEFT, SOUTH_STRAIGHT, SOUTH_RIGHT }; 
enum ariv_dept {IN, OUT};


typedef struct {
	enum events event_type;
	car a_car;
} Msg_Data;

typedef struct {
	int x_to_go;
	int y_to_go;
	int sent_back;
	int prev_prev_intersection;
	enum abs_directions arrived_from;
	enum abs_directions current_lane;
	enum ariv_dept in_out;
} car;

typedef struct {
	int total_cars_arrived;

	int num_in_west_left;
	int num_in_west_straight;
	int num_in_west_right;
	int num_in_north_left;
	int num_in_north_straight;
	int num_in_north_right;
	int num_in_south_left;
	int num_in_south_straight;
	int num_in_south_right;
	int num_in_east_left;
	int num_in_east_straight;
	int num_in_east_right;
	int num_out_west_left;
	int num_out_west_straight;
	int num_out_west_right;
	int num_out_north_left;
	int num_out_north_straight;
	int num_out_north_right;
	int num_out_south_left;
	int num_out_south_straight;
	int num_out_south_right;
	int num_out_east_left;
	int num_out_east_straight;
	int num_out_east_right;
} Intersection_State;

void Intersection_StartUp(Intersection_State *, tw_lp *);
void Intersection_EventHandler(Intersection_State *, tw_bf *, Msg_Data *, tw_lp *);
void Intersection_RC_EventHandler(Intersection_State *, tw_bf *, Msg_Data *, tw_lp *);
void Intersection_Statistics_CollectStats(Intersection_State *, tw_lp *);

tw_lptype Intersection_lps[] = {
	{
		INTERSECTION_LP, sizeof(Intersection_State),
			(init_f) Intersection_StartUp,
			(event_f) Intersection_EventHandler,
			(revent_f) Intersection_RC_EventHandler,
			(final_f) Intersection_Statistics_CollectStats,
			(statecp_f) NULL
	},
	{ 0 },
};

int main(int argc, char * argv[]) {
	tw_lp *lp;
	tw_kp *kp;
	g_tw_ts_end = 300;
	g_tw_gvt_interval = 16;

	/* tw_lptype NumPE NumKP NumLP Message_Size*/

	tw_init(intersection_lps,1,1,1,sizeof(Msg_Data));

	lp = tw_getlp(0);
	kp = tw_getkp(0);
	tw_lp_settype(lp, INTERSECTON_LP);
	tw_lp_onkp(lp, kp);
	tw_lp_onpe(lp, tw_getpe(0));
	tw_kp_onpe(kp, tw_getpe(0));

	tw_run();

	printf("Number of Arivals: %d\n", totalCars);

	return 0;
}

void  Intersection_StartUp(Intersection_State *SV, tw_lp * lp) {
	int i;
	tw_event *CurEvent;
	tw_stime ts;
	Msg_Data *NewM;

	SV->total_cars_arrived = 0;
	SV->num_in_west_left = 0;
	SV->num_in_west_straight = 0;
	SV->num_in_west_right = 0;
	SV->num_in_north_left = 0;
	SV->num_in_north_straight = 0;
	SV->num_in_north_right = 0;
	SV->num_in_south_left = 0;
	SV->num_in_south_straight = 0;
	SV->num_in_south_right = 0;
	SV->num_in_east_left = 0;
	SV->num_in_east_straight = 0;
	SV->num_in_east_right = 0;
	SV->num_out_west_left = 0;
	SV->num_out_west_straight = 0;
	SV->num_out_west_right = 0;
	SV->num_out_north_left = 0;
	SV->num_out_north_straight = 0;
	SV->num_out_north_right = 0;
	SV->num_out_south_left = 0;
	SV->num_out_south_straight = 0;
	SV->num_out_south_right = 0;
	SV->num_out_east_left = 0;
	SV->num_out_east_straight = 0;
	SV->num_out_east_right = 0;

	for(i = 0; i < 5; i++) {
		ts = tw_rand_exponential(lp->id, A);
		CurEvent = tw_event_new(lp, ts, lp);
		NewM = (Msg_Data *)tw_event_data(CurEvent);
		NewM->event_type = ARRIVAL;
		tw_event_send(CurEvent);
	}
}

void Intersection_EventHandler(Intersection_State *SV, tw_bf *CV, Msg_Data *M, tw_lp *lp) {
	tw_stime ts;
	tw_event *CurEvent;
	Msg_Data *NewM;

	*(int *)CV = (int)0;

	switch(M->event_type) {

	case ARRIVAL: 
		// Schedule a departure in the future
		SV->totalCarsArived++;
		
		switch(M->car->current_lane){
		
		case WEST_LEFT:
			num_in_east_left++;	
			M->car->current_lane = EAST_LEFT;
		break;
		case WEST_STRAIGHT:
			num_in_east_straight++;	
			M->car->current_lane = EAST_STRAIGHT;
		break;
		case WEST_RIGHT: 
			num_in_east_right++;
			M->car->current_lane = EAST_RIGHT;
		break;
		case EAST_LEFT: 
			num_in_west_left++;
			M->car->current_lane = WEST_LEFT;
		break;
		case EAST_STRAIGHT: 
			num_in_west_straight++;
			M->car->current_lane = WEST_STRAIGHT;
		break;
		case EAST_RIGHT: 
			num_in_west_right++;
			M->car->current_lane = WEST_RIGHT;
		break;
		case NORTH_LEFT: 
			num_in_south_left++;
			M->car->current_lane = SOUTH_LEFT;
		break;
		case NORTH_STRAIGHT: 
			num_in_south_straight++;
			M->car->current_lane = SOUTH_STRAIGHT;
		break;
		case NORTH_RIGHT: 
			num_in_south_right++;
			M->car->current_lane = SOUTH_RIGHT;
		break;
		case SOUTH_LEFT:
			num_in_north_left++;
			M->car->current_lane = NORTH_LEFT;	
		break; 
		case SOUTH_STRAIGHT:
			num_in_north_straight++;
			M->car->current_lane = NORTH_STRAIGHT;
		break; 
		case SOUTH_RIGHT
			num_in_north_right++;
			M->car->current_lane = NORTH_RIGHT;
		break;
		}

		M->car->in_out = IN;

		ts = tw_rand_exponential(lp->id, R);
		CurEvent = tw_event_new(lp, ts, lp);
		NewM = (Msg_Data *)tw_event_data(CurEvent);
		NewM->event_type = DIRECTION_SELECT;
		tw_event_send(CurEvent);


		break;

	case DEPARTURE: 
		SV->roadwayFree++;

		ts = tw_rand_exponential(lp->id, A);
		CurEvent = tw_event_new(lp, ts, lp);
		NewM = (Msg_Data *) tw_event_data(CurEvent);
		NewM->event_type = ARRIVAL;
		tw_event_send(CurEvent);
		break;

	case DIRECTION_SELECT:

		enum abs_directions temp_direction = M->car->current_lane;

		switch(M->car->current_lane){
			case EAST_LEFT:
				num_in_east_left--;
				if(M->car->y_to_go < 0 && num_out_south_straight < MAX_CARS_ON_ROAD){
					M->car->current_lane = SOUTH_STRAIGHT;
					num_out_south_straight ++;
					M->car->sent_back = 0;
				}
				else if(M->car->x_to_go < 0 && num_out_south_right < MAX_CARS_ON_ROAD){
					M->car->current_lane = SOUTH_RIGHT;
					num_out_south_right ++;
					M->car->sent_back = 0;
				}
				else if(M->car->x_to_go > 0 && num_out_south_left < MAX_CARS_ON_ROAD){
					M->car->current_lane = SOUTH_LEFT;
					num_out_south_left ++;
					M->car->sent_back = 0;
				}
				else{
					if(M->car->arrived_from == SOUTH_LEFT){
						M->car->current_lane = EAST_RIGHT;
						num_out_east_right++;
						M->car->sent_back++;
					}
					else if(M->car->arrived_from == EAST_STRAIGHT){
						M->car->current_lane = EAST_STRAIGHT;
						num_out_east_straight++;
						M->car->sent_back++;
					}
					else if(M->car->arrived_from == NORTH_RIGHT){
						M->car->current_lane = EAST_LEFT;
						num_out_east_left++;
						M->car->sent_back++;
					}
				}
			break;
			case EAST_STRAIGHT:
				num_in_east_straight--;
				if(M->car->x_to_go < 0 && num_out_west_straight < MAX_CARS_ON_ROAD){
					M->car->current_lane = WEST_STRAIGHT;
					num_out_west_straight ++;
					M->car->sent_back = 0;
				}
				else if(M->car->y_to_go < 0 && num_out_west_left < MAX_CARS_ON_ROAD){
					M->car->current_lane = WEST_LEFT;
					num_out_west_left ++;
					M->car->sent_back = 0;
				}
				else if(M->car->y_to_go > 0 && num_out_west_right < MAX_CARS_ON_ROAD){
					M->car->current_lane = WEST_RIGHT;
					num_out_west_right ++;
					M->car->sent_back = 0;
				}
				else{
					if(M->car->arrived_from == NORTH_RIGHT){
						M->car->current_lane = EAST_LEFT;
						num_out_east_left++;
						M->car->sent_back++;
					}
					else if(M->car->arrived_from == EAST_STRAIGHT){
						M->car->current_lane = EAST_STRAIGHT;
						num_out_east_straight++;
						M->car->sent_back++;
					}
					else if(M->car->arrived_from == SOUTH_LEFT){
						M->car->current_lane = EAST_RIGHT;
						num_out_east_right++;
						M->car->sent_back++;
					}
				}

			break;
			case EAST_RIGHT: 


			break;
			case WEST_LEFT:
				num_in_west_left--;
				if(M->car->y_to_go > 0 && num_out_north_straight < MAX_CARS_ON_ROAD){
						M->car->current_lane = NORTH_STRAIGHT;
						num_out_north_straight ++;
						M->car->sent_back = 0;
					}
					else if(M->car->x_to_go > 0 && num_out_north_right < MAX_CARS_ON_ROAD){
						M->car->current_lane = NORTH_RIGHT;
						num_out_north_right ++;
						M->car->sent_back = 0;
					}
					else if(M->car->x_to_go < 0 && num_out_north_left < MAX_CARS_ON_ROAD){
						M->car->current_lane = NORTH_LEFT;
						num_out_north_left ++;
						M->car->sent_back = 0;
					}
					else{
						if(M->car->arrived_from == SOUTH_RIGHT){
							M->car->current_lane = WEST_LEFT;
							num_out_west_left++;
							M->car->sent_back++;
						}
						else if(M->car->arrived_from == WEST_STRAIGHT){
							M->car->current_lane = WEST_STRAIGHT;
							num_out_west_straight++;							
							M->car->sent_back++;
						}
						else if(M->car->arrived_from == NORTH_LEFT){
							M->car->current_lane = WEST_RIGHT;
							num_out_west_right++;
							M->car->sent_back++;
						}
					}
			break;
			case WEST_STRAIGHT: 
				num_in_west_straight--;
				if(M->car->x_to_go > 0 && num_out_east_straight < MAX_CARS_ON_ROAD){
					M->car->current_lane = EAST_STRAIGHT;
					num_out_east_straight ++;
					M->car->sent_back = 0;
				}
				else if(M->car->y_to_go > 0 && num_out_east_left < MAX_CARS_ON_ROAD){
					M->car->current_lane = EAST_LEFT;
					num_out_east_left ++;
					M->car->sent_back = 0;
				}
				else if(M->car->y_to_go < 0 && num_out_east_right < MAX_CARS_ON_ROAD){
					M->car->current_lane = EAST_RIGHT;
					num_out_east_right ++;
					M->car->sent_back = 0;
				}
				else{
					if(M->car->arrived_from == SOUTH_RIGHT){
						M->car->current_lane = WEST_LEFT;
						num_out_west_left++;
						M->car->sent_back++;
					}
					else if(M->car->arrived_from == WEST_STRAIGHT){
						M->car->current_lane = WEST_STRAIGHT;
						num_out_west_straight++;
						M->car->sent_back++;
					}
					else if(M->car->arrived_from == NORTH_LEFT){
						M->car->current_lane = WEST_RIGHT;
						num_out_west_right++;
						M->car->sent_back++;
					}
				}
			break;
			case WEST_RIGHT: 


			break;
			case NORTH_LEFT: 
				num_in_north_left--;
				if(M->car->x_to_go > 0 && num_out_east_straight < MAX_CARS_ON_ROAD){
					M->car->current_lane = EAST_STRAIGHT;
					num_out_east_straight ++;
					M->car->sent_back = 0;
				}
				else if(M->car->y_to_go > 0 && num_out_east_left < MAX_CARS_ON_ROAD){
					M->car->current_lane = EAST_LEFT;
					num_out_east_left ++;
					M->car->sent_back = 0;
				}
				else if(M->car->y_to_go < 0 && num_out_east_right < MAX_CARS_ON_ROAD){
					M->car->current_lane = EAST_RIGHT;
					num_out_east_right ++;
					M->car->sent_back = 0;
				}
				else{
					if(M->car->arrived_from == WEST_RIGHT){
						M->car->current_lane = NORTH_LEFT;
						num_out_north_left++;
						M->car->sent_back++;
					}
					else if(M->car->arrived_from == NORTH_STRAIGHT){
						M->car->current_lane = NORTH_STRAIGHT;
						num_out_north_straight++;
						M->car->sent_back++;
					}
					else if(M->car->arrived_from == EAST_LEFT){
						M->car->current_lane = NORTH_RIGHT;
						num_out_north_right++;
						M->car->sent_back++;
					}
				}

			break;
			case NORTH_STRAIGHT:
				num_in_north_straight--;
				if(M->car->y_to_go < 0 && num_out_south_straight < MAX_CARS_ON_ROAD){
					M->car->current_lane = SOUTH_STRAIGHT;
					num_out_south_straight ++;
					M->car->sent_back = 0;
				}
				else if(M->car->x_to_go > 0 && num_out_south_left < MAX_CARS_ON_ROAD){
					M->car->current_lane = SOUTH_LEFT;
					num_out_south_left ++;
					M->car->sent_back = 0;
				}
				else if(M->car->x_to_go < 0 && num_out_south_right < MAX_CARS_ON_ROAD){
					M->car->current_lane = SOUTH_RIGHT;
					num_out_south_right ++;
					M->car->sent_back = 0;
				}
				else{
					if(M->car->arrived_from == WEST_RIGHT){
						M->car->current_lane = NORTH_LEFT;
						num_out_north_left++;
						M->car->sent_back++;
					}
					else if(M->car->arrived_from == NORTH_STRAIGHT){
						M->car->current_lane = NORTH_STRAIGHT;
						num_out_north_straight++;
						M->car->sent_back++;
					}
					else if(M->car->arrived_from == EAST_LEFT){
						M->car->current_lane = NORTH_RIGHT;
						num_out_north_right++;
						M->car->sent_back++;
					}
				}
			break;
			case NORTH_RIGHT: 


			break;
			case SOUTH_LEFT:
				num_in_south_left--;
				if(M->car->x_to_go < 0 && num_out_west_straight < MAX_CARS_ON_ROAD){
					M->car->current_lane = WEST_STRAIGHT;
					num_out_west_straight ++;
					M->car->sent_back = 0;
				}
				else if(M->car->y_to_go < 0 && num_out_west_left < MAX_CARS_ON_ROAD){
					M->car->current_lane = WEST_LEFT;
					num_out_west_left ++;
					M->car->sent_back = 0;
				}
				else if(M->car->y_to_go > 0 && num_out_west_right < MAX_CARS_ON_ROAD){
					M->car->current_lane = WEST_RIGHT;
					num_out_west_right ++;
					M->car->sent_back = 0;
				}
				else{
					if(M->car->arrived_from == WEST_LEFT){
						M->car->current_lane = SOUTH_RIGHT;
						num_out_south_right++;
						M->car->sent_back++;
					}
					else if(M->car->arrived_from == SOUTH_STRAIGHT){
						M->car->current_lane = SOUTH_STRAIGHT;
						num_out_south_straight++;
						M->car->sent_back++;
					}
					else if(M->car->arrived_from == EAST_RIGHT){
						M->car->current_lane = SOUTH_LEFT;
						num_out_south_left++;
						M->car->sent_back++;
					}
				}

			break; 
			case SOUTH_STRAIGHT:
				num_in_south_straight--;
				if(M->car->y_to_go > 0 && num_out_north_straight < MAX_CARS_ON_ROAD){
					M->car->current_lane = NORTH_STRAIGHT;
					num_out_north_straight ++;
					M->car->sent_back = 0;
				}
				else if(M->car->x_to_go < 0 && num_out_north_left < MAX_CARS_ON_ROAD){
					M->car->current_lane = NORTH_LEFT;
					num_out_north_left ++;
					M->car->sent_back = 0;
				}
				else if(M->car->x_to_go > 0 && num_out_north_right < MAX_CARS_ON_ROAD){
					M->car->current_lane = NORTH_RIGHT;
					num_out_north_right ++;
					M->car->sent_back = 0;
				}
				else{
					if(M->car->arrived_from == EAST_RIGHT){
						M->car->current_lane = SOUTH_LEFT;
						num_out_south_left++;
						M->car->sent_back++;
					}
					else if(M->car->arrived_from == SOUTH_STRAIGHT){
						M->car->current_lane = SOUTH_STRAIGHT;
						num_out_south_straight++;
						M->car->sent_back++;
					}
					else if(M->car->arrived_from == WEST_LEFT){
						M->car->current_lane = SOUTH_RIGHT;
						num_out_south_right++;
						M->car->sent_back++;
					}
				}
			break; 
			case SOUTH_RIGHT


			break;
		}

		M->car->arrived_from = temp_direction;
		M->car->in_out = OUT;
	}
}


void Interseciton_RC_EventHandler(Intersection_State *SV, tw_bf *CV, Msg_Data *M, tw_lp *lp) {

	switch(M->event_type) {

	case ARRIVAL: 
		SV->totalCarsArived--;
		SV->roadwayFree++;

		break;

	case DEPARTURE:  
		SV->roadwayFree--;
		tw_rand_reverse_unif(lp->id);
		break;
	}

	case WAIT:
		if(CV->c1){
			tw_rand_reverse_unif(lp->id);
		}
		else{

		}

}

void Airport_Statistics_CollectStats(Airport_State *SV, tw_lp * lp) {
	NumLanded += SV->NumLanded;
}