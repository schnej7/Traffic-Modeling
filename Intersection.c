#include <ross.h>

#define INTERSECTION_LPS 0

#define MAX_CARS_ON_ROAD 5

enum events { ARIVAL, DEPARTURE, DIRECTION_SELECT };

enum abs_directions { WEST_LEFT, WEST_STRAIGHT, WEST_RIGHT, EAST_LEFT, EAST_STRAIGHT, EAST_RIGHT, NORTH_LEFT, NORTH_STRAIGHT, NORTH_RIGHT, SOUTH_LEFT, SOUTH_STRAIGHT, SOUTH_RIGHT }; 
enum ariv_dept {IN, OUT};

typedef struct {
	int x_to_go;
	int y_to_go;
	int sent_back;
	enum abs_directions arrived_from;
	enum abs_directions current_lane;
	enum ariv_dept in_out;
} a_car;

typedef struct {
	enum events event_type;
	a_car car;
} Msg_Data;

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

tw_lptype mylps[] = {
  {
    (init_f) Intersection_StartUp,
    (event_f) Intersection_EventHandler,
    (revent_f) Intersection_RC_EventHandler,
    (final_f) Intersection_Statistics_CollectStats,
    (map_f) NULL,
    sizeof(Intersection_State)
    // (statecp_f) NULL
  },
  { 0 },
};

tw_stime MEAN_SERVICE=1.0;
tw_stime lookahead = 1.0;
static unsigned int stagger = 0;
static unsigned int offset_lpid = 0;
static tw_stime mult = 1.4;
static tw_stime percent_remote = 0.25;
static unsigned int ttl_lps = 0;
static unsigned int nlp_per_pe = 8;
static int g_phold_start_events = 1;
static int optimistic_memory = 100;

// rate for timestamp exponential distribution
static tw_stime mean = 1.0;

static char run_id[1024] = "undefined";
static unsigned long long totalCars=0;

int main(int argc, char * argv[]) {
	g_tw_ts_end = 300;
	g_tw_gvt_interval = 16;
	int i;

        // get rid of error if compiled w/ MEMORY queues
        g_tw_memory_nqueues=1;
	// set a min lookahead of 1.0
	lookahead = 1.0;
	//tw_opt_add(app_opt);
	tw_init(&argc, &argv);

	if( lookahead > 1.0 )
	  tw_error(TW_LOC, "Lookahead > 1.0 .. needs to be less\n");

	//reset mean based on lookahead
        mean = mean - lookahead;

        g_tw_memory_nqueues = 16; // give at least 16 memory queue event

	offset_lpid = g_tw_mynode * nlp_per_pe;
	ttl_lps = tw_nnodes() * g_tw_npe * nlp_per_pe;
	g_tw_events_per_pe = (mult * nlp_per_pe * g_phold_start_events) + 
				optimistic_memory;
	//g_tw_rng_default = TW_FALSE;
	g_tw_lookahead = lookahead;

	tw_define_lps(nlp_per_pe, sizeof(Msg_Data), 0);

	for(i = 0; i < g_tw_nlp; i++)
		tw_lp_settype(i, &mylps[0]);

        if( g_tw_mynode == 0 )
	  {
	    printf("========================================\n");
	    printf("Traffice Model Configuration..............\n");
	    printf("   Lookahead..............%lf\n", lookahead);
	    printf("   Start-events...........%u\n", g_phold_start_events);
	    printf("   stagger................%u\n", stagger);
	    printf("   Mean...................%lf\n", mean);
	    printf("   Mult...................%lf\n", mult);
	    printf("   Memory.................%u\n", optimistic_memory);
	    printf("   Remote.................%lf\n", percent_remote);
	    printf("========================================\n\n");
	  }

	tw_run();
	tw_end();

	printf("Number of Arivals: %lld\n", totalCars);

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

	for(i = 0; i < 5; i++) 
	  {
	    ts = tw_rand_exponential(lp->rng, MEAN_SERVICE );
	    CurEvent = tw_event_new(lp->gid, ts, lp);
	    NewM = (Msg_Data *)tw_event_data(CurEvent);
	    NewM->event_type = ARIVAL;
	    tw_event_send(CurEvent);
	  }
}

void Intersection_EventHandler(Intersection_State *SV, tw_bf *CV, Msg_Data *M, tw_lp *lp) {
	tw_stime ts;
	tw_event *CurEvent;
	Msg_Data *NewM;
	enum abs_directions temp_direction;
	*(int *)CV = (int)0;

	switch(M->event_type) {

	case ARIVAL: 
		// Schedule a departure in the future
		SV->total_cars_arrived++;
			
		switch(M->car.current_lane){
		
		case WEST_LEFT:
			SV->num_in_east_left++;	
			M->car.current_lane = EAST_LEFT;
		break;
		case WEST_STRAIGHT:
			SV->num_in_east_straight++;	
			M->car.current_lane = EAST_STRAIGHT;
		break;
		case WEST_RIGHT: 
			SV->num_in_east_right++;
			M->car.current_lane = EAST_RIGHT;
		break;
		case EAST_LEFT: 
			SV->num_in_west_left++;
			M->car.current_lane = WEST_LEFT;
		break;
		case EAST_STRAIGHT: 
			SV->num_in_west_straight++;
			M->car.current_lane = WEST_STRAIGHT;
		break;
		case EAST_RIGHT: 
			SV->num_in_west_right++;
			M->car.current_lane = WEST_RIGHT;
		break;
		case NORTH_LEFT: 
			SV->num_in_south_left++;
			M->car.current_lane = SOUTH_LEFT;
		break;
		case NORTH_STRAIGHT: 
			SV->num_in_south_straight++;
			M->car.current_lane = SOUTH_STRAIGHT;
		break;
		case NORTH_RIGHT: 
			SV->num_in_south_right++;
			M->car.current_lane = SOUTH_RIGHT;
		break;
		case SOUTH_LEFT:
			SV->num_in_north_left++;
			M->car.current_lane = NORTH_LEFT;	
		break; 
		case SOUTH_STRAIGHT:
			SV->num_in_north_straight++;
			M->car.current_lane = NORTH_STRAIGHT;
		break; 
		case SOUTH_RIGHT:
			SV->num_in_north_right++;
			M->car.current_lane = NORTH_RIGHT;
		break;
		}

		M->car.in_out = IN;

		ts = tw_rand_exponential(lp->rng, MEAN_SERVICE);
		CurEvent = tw_event_new(lp->gid, ts, lp);
		NewM = (Msg_Data *)tw_event_data(CurEvent);
		NewM->event_type = DIRECTION_SELECT;
		tw_event_send(CurEvent);


		break;

	case DEPARTURE: 
		
		switch(M->car.current_lane){
			case WEST_LEFT:
				SV->num_out_west_left--;
			break;
			case WEST_STRAIGHT:
				SV->num_out_west_straight--;
			break;
			case WEST_RIGHT: 
				SV->num_out_west_right--;
			break;
			case EAST_LEFT:
				SV->num_out_east_left--;
			break;
			case EAST_STRAIGHT: 
				SV->num_out_east_straight--;
			break;
			case EAST_RIGHT: 
				SV->num_out_east_right--;
			break;
			case NORTH_LEFT:
				SV->num_out_north_left--;
			break;
			case NORTH_STRAIGHT:
				SV->num_out_north_straight--;
			break;
			case NORTH_RIGHT:
				SV->num_out_north_right--;
			break;
			case SOUTH_LEFT:
				SV->num_out_south_left--;
			break;
			case SOUTH_STRAIGHT:
				SV->num_out_south_straight--;
			break;
			case SOUTH_RIGHT:
				SV->num_out_south_right--;
			break;
		}

		ts = tw_rand_exponential(lp->rng, MEAN_SERVICE);
		CurEvent = tw_event_new(lp->gid, ts, lp);
		NewM = (Msg_Data *) tw_event_data(CurEvent);
		NewM->event_type = ARIVAL;
		tw_event_send(CurEvent);
		break;

	case DIRECTION_SELECT:

		
		temp_direction = M->car.current_lane;

		switch(M->car.current_lane){
			case EAST_LEFT:
				SV->num_in_east_left--;
				if(M->car.y_to_go < 0 && SV->num_out_south_straight < MAX_CARS_ON_ROAD){
					M->car.current_lane = SOUTH_STRAIGHT;
					SV->num_out_south_straight ++;
					M->car.sent_back = 0;
				}
				else if(M->car.x_to_go < 0 && SV->num_out_south_right < MAX_CARS_ON_ROAD){
					M->car.current_lane = SOUTH_RIGHT;
					SV->num_out_south_right ++;
					M->car.sent_back = 0;
				}
				else if(M->car.x_to_go > 0 && SV->num_out_south_left < MAX_CARS_ON_ROAD){
					M->car.current_lane = SOUTH_LEFT;
					SV->num_out_south_left ++;
					M->car.sent_back = 0;
				}
				else{
					if(M->car.arrived_from == SOUTH_LEFT){
						M->car.current_lane = EAST_RIGHT;
						SV->num_out_east_right++;
						M->car.sent_back++;
					}
					else if(M->car.arrived_from == EAST_STRAIGHT){
						M->car.current_lane = EAST_STRAIGHT;
						SV->num_out_east_straight++;
						M->car.sent_back++;
					}
					else if(M->car.arrived_from == NORTH_RIGHT){
						M->car.current_lane = EAST_LEFT;
						SV->num_out_east_left++;
						M->car.sent_back++;
					}
				}
			break;
			case EAST_STRAIGHT:
				SV->num_in_east_straight--;
				if(M->car.x_to_go < 0 && SV->num_out_west_straight < MAX_CARS_ON_ROAD){
					M->car.current_lane = WEST_STRAIGHT;
					SV->num_out_west_straight ++;
					M->car.sent_back = 0;
				}
				else if(M->car.y_to_go < 0 && SV->num_out_west_left < MAX_CARS_ON_ROAD){
					M->car.current_lane = WEST_LEFT;
					SV->num_out_west_left ++;
					M->car.sent_back = 0;
				}
				else if(M->car.y_to_go > 0 && SV->num_out_west_right < MAX_CARS_ON_ROAD){
					M->car.current_lane = WEST_RIGHT;
					SV->num_out_west_right ++;
					M->car.sent_back = 0;
				}
				else{
					if(M->car.arrived_from == NORTH_RIGHT){
						M->car.current_lane = EAST_LEFT;
						SV->num_out_east_left++;
						M->car.sent_back++;
					}
					else if(M->car.arrived_from == EAST_STRAIGHT){
						M->car.current_lane = EAST_STRAIGHT;
						SV->num_out_east_straight++;
						M->car.sent_back++;
					}
					else if(M->car.arrived_from == SOUTH_LEFT){
						M->car.current_lane = EAST_RIGHT;
						SV->num_out_east_right++;
						M->car.sent_back++;
					}
				}

			break;
			case EAST_RIGHT: 
			SV->num_in_east_right--;
				if(M->car.y_to_go > 0 && SV->num_out_north_straight < MAX_CARS_ON_ROAD){
					M->car.current_lane = NORTH_STRAIGHT;
					SV->num_out_north_straight ++;
					M->car.sent_back = 0;
				}
				else if(M->car.x_to_go > 0 && SV->num_out_north_right < MAX_CARS_ON_ROAD){
					M->car.current_lane = NORTH_RIGHT;
					SV->num_out_north_right ++;
					M->car.sent_back = 0;
				}
				else if(M->car.x_to_go < 0 && SV->num_out_north_left < MAX_CARS_ON_ROAD){
					M->car.current_lane = NORTH_LEFT;
					SV->num_out_north_left ++;
					M->car.sent_back = 0;
				}
				else{
					if(M->car.arrived_from == SOUTH_LEFT){
						M->car.current_lane = EAST_RIGHT;
						SV->num_out_east_right++;
						M->car.sent_back++;
					}
					else if(M->car.arrived_from == EAST_STRAIGHT){
						M->car.current_lane = EAST_STRAIGHT;
						SV->num_out_east_straight++;
						M->car.sent_back++;
					}
					else if(M->car.arrived_from == NORTH_RIGHT){
						M->car.current_lane = EAST_LEFT;
						SV->num_out_east_left++;
						M->car.sent_back++;
					}
				}
			break;
			case WEST_LEFT:
				SV->num_in_west_left--;
				if(M->car.y_to_go > 0 && SV->num_out_north_straight < MAX_CARS_ON_ROAD){
						M->car.current_lane = NORTH_STRAIGHT;
						SV->num_out_north_straight ++;
						M->car.sent_back = 0;
					}
					else if(M->car.x_to_go > 0 && SV->num_out_north_right < MAX_CARS_ON_ROAD){
						M->car.current_lane = NORTH_RIGHT;
						SV->num_out_north_right ++;
						M->car.sent_back = 0;
					}
					else if(M->car.x_to_go < 0 && SV->num_out_north_left < MAX_CARS_ON_ROAD){
						M->car.current_lane = NORTH_LEFT;
						SV->num_out_north_left ++;
						M->car.sent_back = 0;
					}
					else{
						if(M->car.arrived_from == SOUTH_RIGHT){
							M->car.current_lane = WEST_LEFT;
							SV->num_out_west_left++;
							M->car.sent_back++;
						}
						else if(M->car.arrived_from == WEST_STRAIGHT){
							M->car.current_lane = WEST_STRAIGHT;
							SV->num_out_west_straight++;							
							M->car.sent_back++;
						}
						else if(M->car.arrived_from == NORTH_LEFT){
							M->car.current_lane = WEST_RIGHT;
							SV->num_out_west_right++;
							M->car.sent_back++;
						}
					}
			break;
			case WEST_STRAIGHT: 
				SV->num_in_west_straight--;
				if(M->car.x_to_go > 0 && SV->num_out_east_straight < MAX_CARS_ON_ROAD){
					M->car.current_lane = EAST_STRAIGHT;
					SV->num_out_east_straight ++;
					M->car.sent_back = 0;
				}
				else if(M->car.y_to_go > 0 && SV->num_out_east_left < MAX_CARS_ON_ROAD){
					M->car.current_lane = EAST_LEFT;
					SV->num_out_east_left ++;
					M->car.sent_back = 0;
				}
				else if(M->car.y_to_go < 0 && SV->num_out_east_right < MAX_CARS_ON_ROAD){
					M->car.current_lane = EAST_RIGHT;
					SV->num_out_east_right ++;
					M->car.sent_back = 0;
				}
				else{
					if(M->car.arrived_from == SOUTH_RIGHT){
						M->car.current_lane = WEST_LEFT;
						SV->num_out_west_left++;
						M->car.sent_back++;
					}
					else if(M->car.arrived_from == WEST_STRAIGHT){
						M->car.current_lane = WEST_STRAIGHT;
						SV->num_out_west_straight++;
						M->car.sent_back++;
					}
					else if(M->car.arrived_from == NORTH_LEFT){
						M->car.current_lane = WEST_RIGHT;
						SV->num_out_west_right++;
						M->car.sent_back++;
					}
				}
			break;
			case WEST_RIGHT: 
				SV->num_in_west_right--;
				if(M->car.y_to_go < 0 && SV->num_out_south_straight < MAX_CARS_ON_ROAD){
					M->car.current_lane = SOUTH_STRAIGHT;
					SV->num_out_south_straight ++;
					M->car.sent_back = 0;
				}
				else if(M->car.x_to_go > 0 && SV->num_out_south_left < MAX_CARS_ON_ROAD){
					M->car.current_lane = SOUTH_LEFT;
					SV->num_out_south_left ++;
					M->car.sent_back = 0;
				}
				else if(M->car.x_to_go < 0 && SV->num_out_south_right < MAX_CARS_ON_ROAD){
					M->car.current_lane = SOUTH_RIGHT;
					SV->num_out_south_right ++;
					M->car.sent_back = 0;
				}
				else{
					if(M->car.arrived_from == SOUTH_RIGHT){
						M->car.current_lane = WEST_LEFT;
						SV->num_out_west_left++;
						M->car.sent_back++;
					}
					else if(M->car.arrived_from == WEST_STRAIGHT){
						M->car.current_lane = WEST_STRAIGHT;
						SV->num_out_west_straight++;
						M->car.sent_back++;
					}
					else if(M->car.arrived_from == NORTH_LEFT){
						M->car.current_lane = WEST_RIGHT;
						SV->num_out_west_right++;
						M->car.sent_back++;
					}
				}
			break;
			case NORTH_LEFT: 
				SV->num_in_north_left--;
				if(M->car.x_to_go > 0 && SV->num_out_east_straight < MAX_CARS_ON_ROAD){
					M->car.current_lane = EAST_STRAIGHT;
					SV->num_out_east_straight ++;
					M->car.sent_back = 0;
				}
				else if(M->car.y_to_go > 0 && SV->num_out_east_left < MAX_CARS_ON_ROAD){
					M->car.current_lane = EAST_LEFT;
					SV->num_out_east_left ++;
					M->car.sent_back = 0;
				}
				else if(M->car.y_to_go < 0 && SV->num_out_east_right < MAX_CARS_ON_ROAD){
					M->car.current_lane = EAST_RIGHT;
					SV->num_out_east_right ++;
					M->car.sent_back = 0;
				}
				else{
					if(M->car.arrived_from == WEST_RIGHT){
						M->car.current_lane = NORTH_LEFT;
						SV->num_out_north_left++;
						M->car.sent_back++;
					}
					else if(M->car.arrived_from == NORTH_STRAIGHT){
						M->car.current_lane = NORTH_STRAIGHT;
						SV->num_out_north_straight++;
						M->car.sent_back++;
					}
					else if(M->car.arrived_from == EAST_LEFT){
						M->car.current_lane = NORTH_RIGHT;
						SV->num_out_north_right++;
						M->car.sent_back++;
					}
				}

			break;
			case NORTH_STRAIGHT:
				SV->num_in_north_straight--;
				if(M->car.y_to_go < 0 && SV->num_out_south_straight < MAX_CARS_ON_ROAD){
					M->car.current_lane = SOUTH_STRAIGHT;
					SV->num_out_south_straight ++;
					M->car.sent_back = 0;
				}
				else if(M->car.x_to_go > 0 && SV->num_out_south_left < MAX_CARS_ON_ROAD){
					M->car.current_lane = SOUTH_LEFT;
					SV->num_out_south_left ++;
					M->car.sent_back = 0;
				}
				else if(M->car.x_to_go < 0 && SV->num_out_south_right < MAX_CARS_ON_ROAD){
					M->car.current_lane = SOUTH_RIGHT;
					SV->num_out_south_right ++;
					M->car.sent_back = 0;
				}
				else{
					if(M->car.arrived_from == WEST_RIGHT){
						M->car.current_lane = NORTH_LEFT;
						SV->num_out_north_left++;
						M->car.sent_back++;
					}
					else if(M->car.arrived_from == NORTH_STRAIGHT){
						M->car.current_lane = NORTH_STRAIGHT;
						SV->num_out_north_straight++;
						M->car.sent_back++;
					}
					else if(M->car.arrived_from == EAST_LEFT){
						M->car.current_lane = NORTH_RIGHT;
						SV->num_out_north_right++;
						M->car.sent_back++;
					}
				}
			break;
			case NORTH_RIGHT: 
				SV->num_in_north_right--;
				if(M->car.x_to_go < 0 && SV->num_out_west_straight < MAX_CARS_ON_ROAD){
					M->car.current_lane = WEST_STRAIGHT;
					SV->num_out_west_straight ++;
					M->car.sent_back = 0;
				}
				else if(M->car.y_to_go < 0 && SV->num_out_west_left < MAX_CARS_ON_ROAD){
					M->car.current_lane = WEST_LEFT;
					SV->num_out_west_left ++;
					M->car.sent_back = 0;
				}
				else if(M->car.y_to_go > 0 && SV->num_out_west_right < MAX_CARS_ON_ROAD){
					M->car.current_lane = WEST_RIGHT;
					SV->num_out_west_right ++;
					M->car.sent_back = 0;
				}
				else{
					if(M->car.arrived_from == WEST_RIGHT){
						M->car.current_lane = NORTH_LEFT;
						SV->num_out_north_left++;
						M->car.sent_back++;
					}
					else if(M->car.arrived_from == NORTH_STRAIGHT){
						M->car.current_lane = NORTH_STRAIGHT;
						SV->num_out_north_straight++;
						M->car.sent_back++;
					}
					else if(M->car.arrived_from == EAST_LEFT){
						M->car.current_lane = NORTH_RIGHT;
						SV->num_out_north_right++;
						M->car.sent_back++;
					}
				}
			break;
			case SOUTH_LEFT:
				SV->num_in_south_left--;
				if(M->car.x_to_go < 0 && SV->num_out_west_straight < MAX_CARS_ON_ROAD){
					M->car.current_lane = WEST_STRAIGHT;
					SV->num_out_west_straight ++;
					M->car.sent_back = 0;
				}
				else if(M->car.y_to_go < 0 && SV->num_out_west_left < MAX_CARS_ON_ROAD){
					M->car.current_lane = WEST_LEFT;
					SV->num_out_west_left ++;
					M->car.sent_back = 0;
				}
				else if(M->car.y_to_go > 0 && SV->num_out_west_right < MAX_CARS_ON_ROAD){
					M->car.current_lane = WEST_RIGHT;
					SV->num_out_west_right ++;
					M->car.sent_back = 0;
				}
				else{
					if(M->car.arrived_from == WEST_LEFT){
						M->car.current_lane = SOUTH_RIGHT;
						SV->num_out_south_right++;
						M->car.sent_back++;
					}
					else if(M->car.arrived_from == SOUTH_STRAIGHT){
						M->car.current_lane = SOUTH_STRAIGHT;
						SV->num_out_south_straight++;
						M->car.sent_back++;
					}
					else if(M->car.arrived_from == EAST_RIGHT){
						M->car.current_lane = SOUTH_LEFT;
						SV->num_out_south_left++;
						M->car.sent_back++;
					}
				}

			break; 
			case SOUTH_STRAIGHT:
				SV->num_in_south_straight--;
				if(M->car.y_to_go > 0 && SV->num_out_north_straight < MAX_CARS_ON_ROAD){
					M->car.current_lane = NORTH_STRAIGHT;
					SV->num_out_north_straight ++;
					M->car.sent_back = 0;
				}
				else if(M->car.x_to_go < 0 && SV->num_out_north_left < MAX_CARS_ON_ROAD){
					M->car.current_lane = NORTH_LEFT;
					SV->num_out_north_left ++;
					M->car.sent_back = 0;
				}
				else if(M->car.x_to_go > 0 && SV->num_out_north_right < MAX_CARS_ON_ROAD){
					M->car.current_lane = NORTH_RIGHT;
					SV->num_out_north_right ++;
					M->car.sent_back = 0;
				}
				else{
					if(M->car.arrived_from == EAST_RIGHT){
						M->car.current_lane = SOUTH_LEFT;
						SV->num_out_south_left++;
						M->car.sent_back++;
					}
					else if(M->car.arrived_from == SOUTH_STRAIGHT){
						M->car.current_lane = SOUTH_STRAIGHT;
						SV->num_out_south_straight++;
						M->car.sent_back++;
					}
					else if(M->car.arrived_from == WEST_LEFT){
						M->car.current_lane = SOUTH_RIGHT;
						SV->num_out_south_right++;
						M->car.sent_back++;
					}
				}
			break; 
			case SOUTH_RIGHT:
				SV->num_in_south_right--;
				if(M->car.x_to_go > 0 && SV->num_out_east_straight < MAX_CARS_ON_ROAD){
					M->car.current_lane = EAST_STRAIGHT;
					SV->num_out_east_straight ++;
					M->car.sent_back = 0;
				}
				else if(M->car.y_to_go > 0 && SV->num_out_east_left < MAX_CARS_ON_ROAD){
					M->car.current_lane = EAST_LEFT;
					SV->num_out_east_left ++;
					M->car.sent_back = 0;
				}
				else if(M->car.y_to_go < 0 && SV->num_out_east_right < MAX_CARS_ON_ROAD){
					M->car.current_lane = EAST_RIGHT;
					SV->num_out_east_right ++;
					M->car.sent_back = 0;
				}
				else{
					if(M->car.arrived_from == EAST_RIGHT){
						M->car.current_lane = SOUTH_LEFT;
						SV->num_out_south_left++;
						M->car.sent_back++;
					}
					else if(M->car.arrived_from == SOUTH_STRAIGHT){
						M->car.current_lane = SOUTH_STRAIGHT;
						SV->num_out_south_straight++;
						M->car.sent_back++;
					}
					else if(M->car.arrived_from == WEST_LEFT){
						M->car.current_lane = SOUTH_RIGHT;
						SV->num_out_south_right++;
						M->car.sent_back++;
					}
				}
			break;
		}

		M->car.arrived_from = temp_direction;
		M->car.in_out = OUT;
	}
}


void Intersection_RC_EventHandler(Intersection_State *SV, tw_bf *CV, Msg_Data *M, tw_lp *lp) {

	switch(M->event_type) {

	case ARIVAL: 

		break;

	case DEPARTURE:  

		break;
	

	case DIRECTION_SELECT:

		break;
	
	}
}

void Intersection_Statistics_CollectStats(Intersection_State *SV, tw_lp * lp) {
	totalCars += SV->total_cars_arrived;
}
