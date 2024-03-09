#include "measurement.h"

static measurement_task gateway_tasks[] = {
    {
        0, //domain_id
        "topic0", //name
        0, //reader
        2, //delay
        1024, //max_length
        0, //times of loop
        300, //max_packets
        -1, //fwd
    },
    {
        1, //domain_id
        "topic1", //name
        1, //writer
        100, //delay
        1024, //max_length
        0, //times of loop
        300, //max_packets
        -1, //fwd
    },
    {
        2, //domain_id
        "topic2", //name
        0, //reader
        2, //delay
        1024, //max_length
        0, //times of loop
        300, //max_packets
        -1, //fwd
    },
    {
        3, //domain_id
        "topic3", //name
        1, //writer
        100, //delay
        1024, //max_length
        0, //times of loop
        300, //max_packets
        -1, //fwd
    },
    {
        4, //domain_id
        "topic4", //name
        0, //reader
        2, //delay
        1024, //max_length
        0, //times of loop
        300, //max_packets
        -1, //fwd
    },
    {
        5, //domain_id
        "topic5", //name
        1, //writer
        100, //delay
        1024, //max_length
        0, //times of loop
        300, //max_packets
        -1, //fwd
    },
} ;

measurement_config gateway_config = {
    gateway_tasks,
    sizeof(gateway_tasks) / sizeof(measurement_task)
} ;

static measurement_task body_dcu_task[] = {
    {
        1, //domain_id
        "topic1", //name
        0, //reader
        -1, //delay
        1024, //max_length
        7500, //times of loop
        300, //max_packets
        -1, //fwd
        5, //prio
    },
};

measurement_config body_dcu_config = {
    body_dcu_task,
    sizeof(body_dcu_task) / sizeof(measurement_task)
} ;

static measurement_task powertrain_dcu_task[] = {
    {
        2, //domain_id
        "topic2", //name
        1, //writer
        150, //delay
        1024, //max_length
        2500, //times of loop
        300, //max_packets
        -1, //fwd
        1, //prio
    },
    {
        5, //domain_id
        "topic5", //name
        0, //reader
        -1, //delay
        1024, //max_length
        2500, //times of loop
        300, //max_packets
        -1, //fwd
        1, //prio
    },
};

measurement_config powertrain_dcu_config = {
    powertrain_dcu_task,
    sizeof(powertrain_dcu_task) / sizeof(measurement_task)
};

static measurement_task passive_safety_dcu_task[] = {
    {
        7, //domain_id
        "topic7", //name
        0, //reader
        150, //delay
        1024, //max_length
        2500, //times of loop
        300, //max_packets
        -1, //fwd
        10, //prio
    },
};

measurement_config motion_safety_dcu_config = {
    passive_safety_dcu_task, 
    sizeof(passive_safety_dcu_task) / sizeof(measurement_task)
};

static measurement_task adas_dcu_task[] = {
    {
        0, //domain_id
        "topic0", //name
        1, //writer
        -1, //delay
        1024, //max_length
        7500, //times of loop
        300, //max_packets
        -1, //fwd
        5, //prio
    },
    {
        3, //domain_id
        "topic3", //name
        0, //reader
        100, //delay
        1024, //max_length
        2500, //times of loop
        300, //max_packets
        4, //fwd
        1, //prio
    },
    {
        4, //domain_id
        "topic4", //name
        1, //writer
        150, //delay
        1024, //max_length
        2500, //times of loop
        300, //max_packets
        3, //fwd
        1, //prio
    },
    {
        6, //domain_id
        "topic6", //name
        1, //writer
        150, //delay
        1024, //max_length
        2500, //times of loop
        300, //max_packets
        -1, //fwd
        10, //prio
    },
};

measurement_config adas_dcu_config = {
    adas_dcu_task,
    sizeof(adas_dcu_task) / sizeof(measurement_task)
};

measurement_config *config[] = {
    &gateway_config,
    &body_dcu_config,
    &powertrain_dcu_config,
    &adas_dcu_config,
    &motion_safety_dcu_config
};

int number_of_config = sizeof(config) / sizeof(measurement_config *);
