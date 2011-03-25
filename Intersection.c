#include <ross.h>



enum events { ARIVAL, DEPARTURE, WAIT };

typedef struct {
	enum events event_type;
} Msg_Data;

typedef struct {
	int totalCarsArived;
	int roadwayFree;
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

	SV->waitingToLeave = 0;
	SV->totalCars = 0;
	SV->roadwayFree = 10;

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

		SV->roadwayFree--;

		ts = tw_rand_exponential(lp->id, R);
		CurEvent = tw_event_new(lp, ts, lp);
		NewM = (Msg_Data *)tw_event_data(CurEvent);
		NewM->event_type = WAIT;
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

	case WAIT:
		if((CV->c1 = (SV->roadwayFree > 0))){
			ts = tw_rand_exponential(lp->id, A);
			CurEvent = tw_event_new(lp, ts, lp);
			NewM = (Msg_Data *) tw_event_data(CurEvent);
			NewM->event_type = DEPARTURE;
			tw_event_send(CurEvent);
		}
		else{
			ts = tw_rand_exponential(lp->id, R);
			CurEvent = tw_event_new(lp, ts, lp);
			NewM = (Msg_Data *)tw_event_data(CurEvent);
			NewM->event_type = WAIT;
			tw_event_send(CurEvent);
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