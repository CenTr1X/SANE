#include "measurement.h"

static measurement_task gateway_tasks[] = {
    {
        0, //domain_id
        "topic0", //name
        0, //reader
        2, //delay
        1024, //max_length
        10000, //times of loop
        300, //max_packets
        1, //fwd
    },
    {
        1, //domain_id
        "topic1", //name
        1, //writer
        100, //delay
        1024, //max_length
        10000, //times of loop
        300, //max_packets
        0, //fwd
    },
    {
        2, //domain_id
        "topic2", //name
        0, //reader
        2, //delay
        1024, //max_length
        10000, //times of loop
        300, //max_packets
        3, //fwd
    },
    {
        3, //domain_id
        "topic3", //name
        1, //writer
        100, //delay
        1024, //max_length
        10000, //times of loop
        300, //max_packets
        2, //fwd
    },
    {
        4, //domain_id
        "topic4", //name
        0, //reader
        2, //delay
        1024, //max_length
        10000, //times of loop
        300, //max_packets
        5, //fwd
    },
    {
        5, //domain_id
        "topic5", //name
        1, //writer
        100, //delay
        1024, //max_length
        10000, //times of loop
        300, //max_packets
        4, //fwd
    },
} ;

measurement_config gateway_config = {
    gateway_tasks,
    sizeof(gateway_tasks) / sizeof(measurement_task)
} ;

static measurement_task body_cabin_dcu_task[] = {
    {
        1, //domain_id
        "topic1", //name
        0, //reader
        2, //delay
        1024, //max_length
        5000, //times of loop
        300, //max_packets
    },
    {
        2, //domain_id
        "topic2", //name
        1, //writer
        100, //delay
        1024, //max_length
        3333, //times of loop
        300, //max_packets
    },
};

measurement_config body_cabin_dcu_config = {
    body_cabin_dcu_task,
    sizeof(body_cabin_dcu_task) / sizeof(measurement_task)
} ;

static measurement_task powertrain_dcu_task[] = {
    {
        2, //domain_id
        "topic2", //name
        1, //writer
        100, //delay
        1024, //max_length
        3333, //times of loop
        300, //max_packets
    },
    {
        5, //domain_id
        "topic5", //name
        0, //reader
        2, //delay
        1024, //max_length
        5000, //times of loop
        300, //max_packets
    },
};

measurement_config powertrain_dcu_config = {
    powertrain_dcu_task,
    sizeof(powertrain_dcu_task) / sizeof(measurement_task)
};

static measurement_task motion_safety_dcu_task[] = {
    {
        0, //domain_id
        "topic0", //name
        1, //writer
        100, //delay
        1024, //max_length
        10000, //times of loop
        300, //max_packets
    },
    {
        4, //domain_id
        "topic4", //name
        1, //writer
        100, //delay
        1024, //max_length
        10000, //times of loop
        300, //max_packets
    },
        {
        5, //domain_id
        "topic5", //name
        1, //writer
        100, //delay
        1024, //max_length
        10000, //times of loop
        300, //max_packets
    },
};

measurement_config motion_safety_dcu_config = {
    motion_safety_dcu_task, 
    sizeof(motion_safety_dcu_task) / sizeof(measurement_task)
};

static measurement_task infortainment_dcu_task[] = {

};

measurement_config infortainment_dcu_config = {
    infortainment_dcu_task,
    sizeof(infortainment_dcu_task) / sizeof(measurement_task)
};

measurement_config *config[] = {
    &gateway_config,
    &body_cabin_dcu_config,
    &powertrain_dcu_config,
    &infortainment_dcu_config,
    &motion_safety_dcu_config
};

int number_of_config = sizeof(config) / sizeof(measurement_config *);
