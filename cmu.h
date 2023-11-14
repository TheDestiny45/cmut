#ifdef CMU_IMPLEMENTATION

// scheduler

struct simulation_t;

struct task_t
{
    double time;
    int event_id;
    int entity_id;
    struct task_t *next;
};

typedef struct task_t task;

typedef struct
{
    double system_time;
    task *task_list;
} scheduler;

#define NO_ENTITY -1 // no entity past to the event function

typedef struct event_t
{
    int event_id;
    void (*event_func)(struct simulation_t *s, int entity_id);
    struct event_t *next;
} event;

typedef struct
{
    event *head;
} event_list;

typedef struct entity_element_t
{
    int id;
    struct entity_data_t *data;
    struct entity_element_t *next;
} entity_element;

typedef struct entities_set_t
{
    entity_element *head;
} entities_set;

unsigned int entities_set_id_counter = 0;

#endif // CMU_IMPLEMENTATION

#ifndef CMU_H
#define CMU_H

// Structure to specify in the main program
struct variables_t;
struct stat_indicators_t;
struct resources_t;
struct entity_data_t;

typedef struct simulation_t
{
    scheduler s;
    event_list e;
    struct variables_t *variables;
    struct stat_indicators_t *stat_indicators;
    struct resources_t *resources;
    struct entities_set_t entities;
    int start_event_id;
    int is_running;
    void (*compute_stat_indicators)(struct simulation_t *s);
} simulation;

#endif // CMU_H

#ifdef CMU_IMPLEMENTATION

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>

// functions scheduler

/**
 * initialize the scheduler
 */
void scheduler_init(scheduler *s)
{
    s->system_time = 0.0;
    s->task_list = NULL;
}

/**
 * insert a task into the scheduler in the order of time
 */
void scheduler_insert_task(scheduler *s, double delta_time, int event_id, int entity_id)
{
    task *new_task = (task *)malloc(sizeof(task));

    new_task->time = s->system_time + delta_time;
    new_task->event_id = event_id;
    new_task->entity_id = entity_id;
    new_task->next = NULL;

    if (s->task_list == NULL)
    {
        s->task_list = new_task;
    }
    else
    {
        task *current_task = s->task_list;
        task *previous_task = NULL;
        double event_time = s->system_time + delta_time;
        while (current_task != NULL && current_task->time <= event_time)
        {
            previous_task = current_task;
            current_task = current_task->next;
        }
        if (previous_task == NULL)
        {
            new_task->next = s->task_list;
            s->task_list = new_task;
        }
        else
        {
            previous_task->next = new_task;
            new_task->next = current_task;
        }
    }
}

/**
 * pull task from the scheduler
 */
task *scheduler_pull(scheduler *s)
{
    task *res = s->task_list;
    s->task_list = s->task_list->next;
    return res;
}

/**
 * print the task list
 */
void scheduler_print_task_list(const scheduler *s)
{
    task *current_task = s->task_list;
    while (current_task != NULL)
    {
        printf("time: %f, event_id: %d\n", current_task->time, current_task->event_id);
        current_task = current_task->next;
    }
}

// function event_list

/**
 * initialize the event list
 */
void event_list_init(event_list *e)
{
    e->head = NULL;
}

/**
 * insert an event into the event list
 */
void event_list_insert(event_list *e, int event_id, void (*event_func)(struct simulation_t *s, int entity_id))
{
    event *new_event = (event *)malloc(sizeof(event));
    new_event->event_id = event_id;
    new_event->event_func = event_func;
    new_event->next = NULL;

    if (e->head == NULL)
    {
        e->head = new_event;
    }
    else
    {
        event *current_event = e->head;
        while (current_event->next != NULL)
        {
            current_event = current_event->next;
        }
        current_event->next = new_event;
    }
}

/**
 * get the event from the event list
 */
event *event_list_get(event_list *e, int event_id)
{
    event *current_event = e->head;
    while (current_event != NULL)
    {
        if (current_event->event_id == event_id)
        {
            return current_event;
        }
        current_event = current_event->next;
    }
    return NULL;
}

// function simulation

void entities_set_init(entities_set *es);

/**
 * initialize the simulation
 */
void simulation_init(simulation *sim)
{
    srand(time(NULL));
    scheduler_init(&sim->s);
    event_list_init(&sim->e);
    sim->stat_indicators = NULL;
    sim->variables = NULL;
    sim->resources = NULL;
    entities_set_init(&sim->entities);
    sim->is_running = 0;
    sim->compute_stat_indicators = NULL;
}

/**
 * insert an event into the simulation
 */
void simulation_insert_event(simulation *sim, int event_id, void (*event_func)(simulation *s, int entity_id))
{
    event_list_insert(&sim->e, event_id, event_func);
}

/**
 * set the start event
 * the start event will be called when the simulation starts, it must be set in the event list
 */
void simulation_set_start_event(simulation *sim, int event_id)
{
    sim->start_event_id = event_id;
}

/**
 * set the variables
 */
void simulation_set_variables(simulation *sim, struct variables_t *variables)
{
    sim->variables = variables;
}

/**
 * set the stat indicators
 */
void simulation_set_stat_indicators(simulation *sim, struct stat_indicators_t *stat_indicators)
{
    sim->stat_indicators = stat_indicators;
}

/**
 * set the resources
 */
void simulation_set_resources(simulation *sim, struct resources_t *resources)
{
    sim->resources = resources;
}

double simulation_get_system_time(simulation *sim)
{
    return sim->s.system_time;
}

/**
 * stop the simulation
 */
void simulation_add_task(simulation *sim, double delta_time, int event_id, int entity_id)
{
    scheduler_insert_task(&sim->s, delta_time, event_id, entity_id);
}

void simulation_stop(simulation *sim)
{
    sim->is_running = 0;
}

/**
 * run the simulation
 */
void simulation_start(simulation *sim)
{
    sim->is_running = 1;
    scheduler_insert_task(&sim->s, 0, sim->start_event_id, NO_ENTITY);
    while (sim->s.task_list != NULL && sim->is_running)
    {
        task *current_task = scheduler_pull(&sim->s);
        sim->s.system_time = current_task->time;
        event *current_event = event_list_get(&sim->e, current_task->event_id);
        current_event->event_func(sim, current_task->entity_id);
        if (sim->compute_stat_indicators != NULL)
        {
            printf("%p\n", sim->compute_stat_indicators);
            sim->compute_stat_indicators(sim);
        }
        free(current_task);
    }

    sim->is_running = 0;

    // todo compute the stat indicators
    if (sim->compute_stat_indicators != NULL)
    {
        printf("%p\n", sim->compute_stat_indicators);
        sim->compute_stat_indicators(sim);
    }
}

void entities_set_clean(entities_set *es);

/**
 * free the memory allocated by the simulation
 */
void simulation_clean(simulation *sim)
{
    event *current_event = sim->e.head;
    while (current_event != NULL)
    {
        event *next_event = current_event->next;
        free(current_event);
        current_event = next_event;
    }
    sim->e.head = NULL;
    task *current_task = sim->s.task_list;
    while (current_task != NULL)
    {
        task *next_task = current_task->next;
        free(current_task);
        current_task = next_task;
    }
    entities_set_clean(&sim->entities);
}

// function entities_set

/**
 * initialize the entities set
 */
void entities_set_init(entities_set *es)
{
    es->head = NULL;
}

/**
 * insert an entity into the entities set, insert the entity and return the generated id
 */
int entities_set_insert(entities_set *es, struct entity_data_t *entity)
{
    entity_element *new_entity_element = (entity_element *)malloc(sizeof(entity_element));
    new_entity_element->id = entities_set_id_counter++;
    new_entity_element->data = entity;
    new_entity_element->next = NULL;

    if (es->head == NULL)
    {
        es->head = new_entity_element;
    }
    else
    {
        entity_element *current_entity_element = es->head;
        while (current_entity_element->next != NULL)
        {
            current_entity_element = current_entity_element->next;
        }
        current_entity_element->next = new_entity_element;
    }
    return new_entity_element->id;
}

/**
 * get an entity from the entities set
 */
struct entity_data_t *entities_set_get(entities_set *es, unsigned int id)
{
    entity_element *current_entity_element = es->head;
    while (current_entity_element != NULL)
    {
        if (current_entity_element->id == id)
        {
            return current_entity_element->data;
        }
        current_entity_element = current_entity_element->next;
    }
    return NULL;
}

/**
 * remove an entity from the entities set
 */
void entities_set_remove(entities_set *es, unsigned int id)
{
    entity_element *current_entity_element = es->head;
    entity_element *previous_entity_element = NULL;
    while (current_entity_element != NULL)
    {
        if (current_entity_element->id == id)
        {
            if (previous_entity_element == NULL)
            {
                es->head = current_entity_element->next;
            }
            else
            {
                previous_entity_element->next = current_entity_element->next;
            }
            free(current_entity_element);
            return;
        }
        previous_entity_element = current_entity_element;
        current_entity_element = current_entity_element->next;
    }
}

/**
 * clean the entities set
 */
void entities_set_clean(entities_set *es)
{
    entity_element *current_entity_element = es->head;
    while (current_entity_element != NULL)
    {
        entity_element *next_entity_element = current_entity_element->next;
        free(current_entity_element->data);
        free(current_entity_element);
        current_entity_element = next_entity_element;
    }
    es->head = NULL;
}

// function generate random number by law

/**
 * generate a random number following the exponential law
 */
double rand_exp(double lambda)
{
    double u = (double)rand() / RAND_MAX;
    return -log(1 - u) / lambda;
}

/**
 * generate a random number following the uniform law
 */
double rand_unif(double a, double b)
{
    double u = (double)rand() / RAND_MAX;
    return (a + u * (b - a));
}

#define PI 3.14159265358979323846

/**
 * generate a random number following the normal law
 */
double rand_norm(double mean, double sigma) // mu = mean, sigma = standard deviation
{
    double u1 = (double)rand() / RAND_MAX;
    double u2 = (double)rand() / RAND_MAX;
    double z = sqrt(-2 * log(u1)) * cos(2 * PI * u2);
    return mean + sigma * z;
}
#endif // CMU_IMPLEMENTATION

#ifdef CMU_ADDON

struct entity_queue_elem_t
{
    unsigned int id;
    struct entity_queue_elem_t *next;
};

struct entity_queue_t
{
    struct entity_queue_elem_t *head;
    struct entity_queue_elem_t *tail;
};
typedef struct entity_queue_t entity_queue;

// function entity_queue

/**
 * initialize the entity queue
 */
void entity_queue_init(entity_queue *eq)
{
    eq->head = NULL;
    eq->tail = NULL;
}

/**
 * insert an entity into the entity queue
 */
void entity_queue_insert(entity_queue *eq, unsigned int id) // insert at the end
{
    struct entity_queue_elem_t *new_elem = (struct entity_queue_elem_t *)malloc(sizeof(struct entity_queue_elem_t));
    new_elem->id = id;
    new_elem->next = NULL;

    if (eq->head == NULL)
    {
        eq->head = new_elem;
        eq->tail = new_elem;
    }
    else
    {
        eq->tail->next = new_elem;
        eq->tail = new_elem;
    }
}

/**
 * pull an entity from the entity queue
 */
unsigned int entity_queue_pull(entity_queue *eq) // pull at the head
{
    unsigned int res = eq->head->id;
    struct entity_queue_elem_t *next_head = eq->head->next;
    free(eq->head);
    eq->head = next_head;
    return res;
}

/**
 * clean the entity queue
 */
void entity_queue_clean(entity_queue *eq)
{
    struct entity_queue_elem_t *current_elem = eq->head;
    while (current_elem != NULL)
    {
        struct entity_queue_elem_t *next_elem = current_elem->next;
        free(current_elem);
        current_elem = next_elem;
    }
    eq->head = NULL;
    eq->tail = NULL;
}

#endif // CMU_ADDON