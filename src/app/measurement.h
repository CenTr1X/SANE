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
    int fwd;
    int prio;
} measurement_task;

typedef struct measurement_config_s
{
    measurement_task *tasks;
    int n;
} measurement_config;
void parse_config(int index);
int find_config(int domain_id);
void send_from_cmd(int index, int no);