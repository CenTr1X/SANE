#include "measurement.h"

static measurement_task computer_tasks[] = {
    {
        0, //domain_id
        "topic0", //name
        0, //reader
        2, //delay
        1024, //max_length
        10000, //times of loop
        300, //max_packets
    },
} ;

measurement_config computer_config = {
    computer_tasks,
    sizeof(computer_tasks) / sizeof(measurement_task)
} ;

static measurement_task writer_task[] = {
    {
        0, //domain_id
        "topic0", //name
        1, //writer
        100, //delay
        1024, //max_length
        10000, //times of loop
        5000, //max_packets 
    }
};

measurement_config writer_config = {
    writer_task,
    sizeof(writer_task) / sizeof(measurement_task)
} ;

measurement_config *config[] = {
    &computer_config,
    &writer_config,
};

int number_of_config = sizeof(config) / sizeof(measurement_config *);
