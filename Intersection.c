#include <ross.h>



enum events { ARIVAL, DEPARTURE, DIRECTION_SELECT };

enum abs_directions { WEST_LEFT, WEST_STRAIGHT, WEST_RIGHT, EAST_LEFT, EAST_STRAIGHT, EAST_RIGHT, NORTH_LEFT, NORTH_STRAIGHT, NORTH_RIGHT, SOUTH_LEFT, SOUTH_STRAIGHT, SOUTH_RIGHT }; 
enum card_directions { NORTH, SOUTH, EAST, WEST };
enum relt_directions { LEFT, STRAIGHT, RIGHT };


typedef struct {
	enum events event_type;
	car a_car;
} Msg_Data;

typedef struct {
	int x_to_go;
	int y_to_go;
	int prev_intersection;
	int prev_prev_intersection;
	enum abs_directions arrived_from;
	enum card_directions card_lane;
	enum relt_directions relt_lane;
} car;

typedef struct {
	int total_cars_arrived;
	car[5] in_west_left;
	car[5] in_west_straight;
	car[5] in_west_right;
	car[5] in_north_left;
	car[5] in_north_straight;
	car[5] in_north_right;
	car[5] in_south_left;
	car[5] in_south_straight;
	car[5] in_south_right;
	car[5] in_east_left;
	car[5] in_east_straight;
	car[5] in_east_right;
	car[5] out_west_left;
	car[5] out_west_straight;
	car[5] out_west_right;
	car[5] out_north_left;
	car[5] out_north_straight;
	car[5] out_north_right;
	car[5] out_south_left;
	car[5] out_south_straight;
	car[5] out_south_right;
	car[5] out_east_left;
	car[5] out_east_straight;
	car[5] out_east_right;
	
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
		
		switch(M->car->	enum directions arrived_from){
		
		case WEST_LEFT:
			in_west_left[num_in_west_left] = M->car;
		break;
		case WEST_STRAIGHT:
		
		break;
		case WEST_RIGHT: 
		
		break;
		case EAST_LEFT: 
		
		break;
		case EAST_STRAIGHT: 
		
		break;
		case EAST_RIGHT: 
		
		break;
		case NORTH_LEFT: 
		
		break;
		case NORTH_STRAIGHT: 
		
		break;
		case NORTH_RIGHT: 
		
		break;
		case SOUTH_LEFT:
		
		break; 
		case SOUTH_STRAIGHT:
		
		break; 
		case SOUTH_RIGHT
		
		break;
		
		
		}
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

		switch( M->car->card_lane ){
			case NORTH:
				switch( M->car->relt_lane ){
					case LEFT:
						int i;
						if(abs(M->car->x_to_go) > abs(M->car->y_to_go) && M->car->y_to_go < 0 && num_out_east_right < 5){
							out_east_right[num_out_east_right] = M->car;
							num_out_east_right++;
						}
						else if(abs(M->car->x_to_go) > abs(M->car->y_to_go) && M->car->y_to_go > 0 && num_out_east_left < 5){
							out_east_left[num_out_east_left] = M->car;
							num_out_east_left++;
						}
						else if( num_out_east_straight < 5 ){
							out_east_straight[num_out_east_straight] = M->car;
							num_out_east_straight++;
						}
						else {
							out_north_left[num_out_north_left] = M->car;
							num_out_north_left++;						
						}
						for(i = 1; i < num_in_north_left; i++){
							in_north_left[i-1] = in_north_left[i];
						}
						num_in_north_left--;
					break;
					case STRAIGHT:
					
					break;
					case RIGHT:
					
					break;
				}
			
			break;
			case SOUTH:
			
			break;
			case EAST:
			
			break;
			
			case WEST:
			
			break;
		}

		break;
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