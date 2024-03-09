#include "dds/dds.h"

typedef struct dds_context_s
{
    dds_entity_t participant;
    dds_entity_t topic;
    dds_entity_t reader_or_writer;
} dds_context;

typedef struct measurement_task_s
{
    dds_entity_t domain_id;
    char name[10];
    int isWriter;
    int delay;
    int max_length;
    int times_of_loop; //using md5 to loop
    int max_packet;
    int fwd; //For reader, the task should transmit fwd msg to topic. For writer the task should wait for msg from fwd.
} measurement_task;

typedef struct measurement_config_s
{
    measurement_task *tasks;
    int n;
} measurement_config;

typedef struct notification_s {
    int no;
} notification;

void parse_config(int index);