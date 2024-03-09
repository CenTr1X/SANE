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
    {
        1, //domain_id
        "topic1", //name
        1, //writer
        100, //delay
        1024, //max_length
        10000, //times of loop
        300, //max_packets
    },
    {
        2, //domain_id
        "topic2", //name
        1, //writer
        2, //delay
        1024, //max_length
        10000, //times of loop
        300, //max_packets
    },
    {
        3, //domain_id
        "topic3", //name
        0, //reader
        2, //delay
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
} ;

measurement_config computer_config = {
    computer_tasks,
    sizeof(computer_tasks) / sizeof(measurement_task)
} ;

static measurement_task lidar_task[] = {
    {
        0, //domain_id
        "topic0", //name
        1, //writer
        100, //delay
        1024, //max_length
        0, //times of loop
        300, //max_packets
        -1, //fwd
        5, //prio
    }
};

measurement_config lidar_config = {
    lidar_task,
    sizeof(lidar_task) / sizeof(measurement_task)
} ;

static measurement_task monitor_task[] = {
    {
        1, //domain_id
        "topic1", //name
        0, //reader
        2, //delay
        1024, //max_length
        0, //times of loop
        300, //max_packets
        -1, //fwd
        5, //prio
    },
};

measurement_config monitor_config = {
    monitor_task,
    sizeof(monitor_task) / sizeof(measurement_task)
};

static measurement_task front_left_zcu_task[] = {
    {
        2, //domain_id
        "topic2", //name
        1, //writer
        200, //delay
        1024, //max_length
        0, //times of loop
        300, //max_packets
        -1, //fwd
        10, //prio
    },
    {
        3, //domain_id
        "topic3", //name
        1, //writer
        200, //delay
        1024, //max_length
        0, //times of loop
        300, //max_packets
        -1, //fwd
        1, //prio
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
        1, //prio
    },
    {
        5, //domain_id
        "topic5", //name
        0, //reader
        2, //delay
        1024, //max_length
        0, //times of loop
        300, //max_packets
        -1, //fwd
        10, //prio
    },
};

measurement_config front_left_zcu_config = {
    front_left_zcu_task, 
    sizeof(front_left_zcu_task) / sizeof(measurement_task)
};

static measurement_task front_right_zcu_task[] = {
    {
        2, //domain_id
        "topic2", //name
        1, //writer
        200, //delay
        1024, //max_length
        0, //times of loop
        300, //max_packets
        -1, //fwd
        10, //prio
    },
    {
        3, //domain_id
        "topic3", //name
        1, //writer
        200, //delay
        1024, //max_length
        0, //times of loop
        300, //max_packets
        -1, //fwd
        1, //prio
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
        1, //prio
    },
    {
        5, //domain_id
        "topic5", //name
        0, //reader
        2, //delay
        1024, //max_length
        0, //times of loop
        300, //max_packets
        -1, //fwd
        10, //prio
    },
};

measurement_config front_right_zcu_config = {
    front_right_zcu_task, 
    sizeof(front_right_zcu_task) / sizeof(measurement_task)
};

static measurement_task rear_left_zcu_task[] = {
    {
        2, //domain_id
        "topic2", //name
        1, //writer
        200, //delay
        1024, //max_length
        0, //times of loop
        300, //max_packets
        -1, //fwd
        10, //prio
    },
    {
        3, //domain_id
        "topic3", //name
        1, //writer
        200, //delay
        1024, //max_length
        0, //times of loop
        300, //max_packets
        -1, //fwd
        1, //prio
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
        1, //prio
    },
    {
        5, //domain_id
        "topic5", //name
        0, //reader
        2, //delay
        1024, //max_length
        0, //times of loop
        300, //max_packets
        -1, //fwd
        10, //prio
    },
};

measurement_config rear_left_zcu_config = {
    rear_left_zcu_task, 
    sizeof(rear_left_zcu_task) / sizeof(measurement_task)
};

static measurement_task rear_right_zcu_task[] = {
    {
        2, //domain_id
        "topic2", //name
        1, //writer
        200, //delay
        1024, //max_length
        0, //times of loop
        300, //max_packets
        -1, //fwd
        10, //prio
    },
    {
        3, //domain_id
        "topic3", //name
        1, //writer
        200, //delay
        1024, //max_length
        0, //times of loop
        300, //max_packets
        -1, //fwd
        1, //prio
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
        1, //prio
    },
    {
        5, //domain_id
        "topic5", //name
        0, //reader
        2, //delay
        1024, //max_length
        0, //times of loop
        300, //max_packets
        -1, //fwd
        10, //prio
    },
};

measurement_config rear_right_zcu_config = {
    rear_right_zcu_task, 
    sizeof(rear_right_zcu_task) / sizeof(measurement_task)
};

measurement_config *config[] = {
    &computer_config,
    &lidar_config,
    &monitor_config,
    &front_left_zcu_config,
    &front_right_zcu_config,
    &rear_left_zcu_config,
    &rear_right_zcu_config
};

int number_of_config = sizeof(config) / sizeof(measurement_config *);
