#define CMU_IMPLEMENTATION
#define CMU_ADDON
#include "cmu.h"
#include <stdio.h>
#include <stdlib.h>

struct variables_t
{
    int B1;
    int B2;
    int Q1;
    int Q2;
};

struct resources_t
{
    struct entity_queue_t* queue1;
    struct entity_queue_t* queue2;
};

struct entity_data_t
{
    double ArrTime;
};

struct stat_indicators_t
{
    unsigned int NbC;
    double UseRate;
    double TailleMoyFile;
    double TpsMoyAtt;
};

typedef enum
{
    START,
    ARR_CLIENT,
    ARR_QUEUE1,
    ACC_COUNTER1,
    DEP_COUNTER1,
    ARR_QUEUE2,
    ACC_COUNTER2,
    DEP_COUNTER2,
    END,
} events;

void event_start(simulation* s, int entity_id);
void event_arr_client(simulation* s, int entity_id);
void event_arr_queue1(simulation* s, int entity_id);
void event_acc_counter1(simulation* s, int entity_id);
void event_dep_counter1(simulation* s, int entity_id);
void event_arr_queue2(simulation* s, int entity_id);
void event_acc_counter2(simulation* s, int entity_id);
void event_dep_counter2(simulation* s, int entity_id);
void event_end(simulation* s, int entity_id);
int main(void) {

    simulation sim;
    simulation_init(&sim);
    struct variables_t var = { 0 };
    struct stat_indicators_t stat = { 0 };
    struct resources_t res = { 0 };
    res.queue1 = (struct entity_queue_t*) malloc(sizeof(struct entity_queue_t));
    res.queue2 = (struct entity_queue_t*) malloc(sizeof(struct entity_queue_t));

    simulation_set_variables(&sim, &var);
    simulation_set_stat_indicators(&sim, &stat);
    simulation_set_resources(&sim, &res);

    simulation_insert_event(&sim, START, event_start);
    simulation_insert_event(&sim, ARR_CLIENT, event_arr_client);
    simulation_insert_event(&sim, ARR_QUEUE1, event_arr_queue1);
    simulation_insert_event(&sim, ACC_COUNTER1, event_acc_counter1);
    simulation_insert_event(&sim, DEP_COUNTER1, event_dep_counter1);
    simulation_insert_event(&sim, ARR_QUEUE2, event_arr_queue2);
    simulation_insert_event(&sim, ACC_COUNTER2, event_acc_counter2);
    simulation_insert_event(&sim, DEP_COUNTER2, event_dep_counter2);
    simulation_insert_event(&sim, END, event_end);

    simulation_set_start_event(&sim, START);

    simulation_start(&sim);
    simulation_clean(&sim);

    // prints all the statistics
    printf("NbC: %d\n", stat.NbC);
    printf("UseRate: %f\n", stat.UseRate);
    printf("TailleMoyFile: %f\n", stat.TailleMoyFile);
    printf("TpsMoyAtt: %f\n", stat.TpsMoyAtt);

    return EXIT_SUCCESS;
}

void event_arr_client(simulation* s, int entity_id) {

    printf("Arrivee client (time: %f)\n", simulation_get_system_time(s));

    double lambda = 0.1;
    double delta = rand_exp(lambda);
    // create a new entity with it arrival time and insert it in the entities set
    struct entity_data_t* data = malloc(sizeof(struct entity_data_t));
    data->ArrTime = simulation_get_system_time(s);
    int id = entities_set_insert(&s->entities, data);

    simulation_add_task(s, delta, ARR_QUEUE1, entity_id);
    simulation_add_task(s, delta, ARR_CLIENT, id);
    s->stat_indicators->NbC++;

}

void event_arr_queue1(simulation* s, int entity_id) {

    printf("Arrivee queue 1 (time: %f)\n", simulation_get_system_time(s));

    s->variables->Q1++;
    entity_queue_insert(s->resources->queue1, entity_id);
    if (s->variables->B1 == 0 && s->variables->Q1 == 1) {
        simulation_add_task(s, 0, ACC_COUNTER1, entity_id);
    }
}

void event_acc_counter1(simulation* s, int entity_id) {

    printf("Acces counter 1 (time: %f)\n", simulation_get_system_time(s));

    s->variables->B1 = 1;

    int id = entity_queue_pull(s->resources->queue1);

    s->variables->Q1--;

    double delta = rand_exp(0.7);
    simulation_add_task(s, delta, DEP_COUNTER1, id);
}

void event_dep_counter1(simulation* s, int entity_id) {

    printf("Depart counter 1 (time: %f)\n", simulation_get_system_time(s));

    s->variables->B1 = 0;
    simulation_add_task(s, 0, ARR_QUEUE2, entity_id);
    if (s->variables->Q1 > 0) {
        simulation_add_task(s, 0.0, ACC_COUNTER1, NO_ENTITY);
    }
}

void event_arr_queue2(simulation* s, int entity_id) {

    printf("Arrivee queue 2 (time: %f)\n", simulation_get_system_time(s));

    s->variables->Q2++;
    entity_queue_insert(s->resources->queue2, entity_id);
    if (s->variables->B2 == 0 && s->variables->Q2 == 1) {
        simulation_add_task(s, 0, ACC_COUNTER2, entity_id);
    }
}

void event_acc_counter2(simulation* s, int entity_id) {

    printf("Acces counter 2 (time: %f)\n", simulation_get_system_time(s));

    s->variables->B2 = 1;

    int id = entity_queue_pull(s->resources->queue2);

    s->variables->Q2--;

    double delta = rand_exp(0.9);
    simulation_add_task(s, delta, DEP_COUNTER2, id);
}

void event_dep_counter2(simulation* s, int entity_id) {

    printf("Depart counter 2 (time: %f)\n", simulation_get_system_time(s));

    s->variables->B2 = 0;
    if (s->variables->Q2 > 0) {
        simulation_add_task(s, 0.0, ACC_COUNTER2, NO_ENTITY);
    }
    entities_set_remove(&s->entities, entity_id);
}

void event_start(simulation* s, int entity_id) {
    printf("Start (time: %f)\n", simulation_get_system_time(s));

    s->stat_indicators->NbC = 0;
    s->stat_indicators->TailleMoyFile = 0;
    s->stat_indicators->TpsMoyAtt = 0;
    s->stat_indicators->UseRate = 0;
    s->variables->B1 = 0;
    s->variables->B2 = 0;
    s->variables->Q1 = 0;
    s->variables->Q2 = 0;

    double delta = rand_exp(1);
    simulation_add_task(s, delta, ARR_CLIENT, NO_ENTITY);
    simulation_add_task(s, 1000, END, NO_ENTITY);
}

void event_end(simulation* s, int entity_id) {
    printf("End (time: %f)\n", simulation_get_system_time(s));

    simulation_stop(s);
}