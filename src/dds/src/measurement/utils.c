#include <stdlib.h>
#include <string.h>

#include "FreeRTOS.h"
#include "queue.h"
#include "dds/dds.h"
#include "measurement.h"
#include "measurement/CommonData.h"
#include "task.h"
#include "md5_copy.h"

#include "printf.h"

#define MAX_SAMPLES 1
extern measurement_config *config[];
extern int number_of_config;
int test = 0;
int zcu_index, dcu_index;
int index_of_config = -1;
dds_entity_t domainid2writer[10];
QueueHandle_t queues[10];

void make_msg(int max_length, int no, CommonData_Data *buf) {
  int length = rand() % max_length + 1;  // add a '\00'
  if(buf->context) free(buf->context);
  buf->context = malloc(length);
  memset(buf->context, 'a', length);
  buf->context[length - 1] = '\00';
  buf->no = no;
}

void handle_msg(char *content, int length, int times) {
  //do md5 calc loop
  printf("handle now, loops:%d\n", times);
  unsigned char output[16], temp[16];
  md5(content, length, temp);
  for(int i=0; i < times-1; i++) {
    md5(temp, 16, output);
    memcpy(temp, output, 16);
  }
  //printf("handle done\n");
}

void gateway_reader_task(void *arg) {
  dds_entity_t participant, topic, reader;
  measurement_task *param = (measurement_task *)arg;
  dds_qos_t *qos;
  CommonData_Data *msg;
  void *samples[MAX_SAMPLES];
  dds_sample_info_t infos[MAX_SAMPLES];
  int times;
  dds_entity_t waitSet;
  dds_attach_t wsresults[1];
  printf("gateway_reader_task %s start at:%d\n", param->name, xTaskGetTickCount());
  dds_create_domain(param->domain_id, NULL);
  participant = dds_create_participant(param->domain_id, NULL, NULL);
  printf("%s particpant create done\n", param->name);
  waitSet = dds_create_waitset (participant);

  topic = dds_create_topic(participant, &CommonData_Data_desc, param->name,
                           NULL, NULL);
  qos = dds_create_qos();
  dds_qset_reliability(qos, DDS_RELIABILITY_RELIABLE, DDS_SECS(10));
  dds_qset_history(qos, DDS_HISTORY_KEEP_ALL, 100);
  dds_qset_transport_priority(qos, param->prio);
  //dds_qset_reliability(qos, DDS_RELIABILITY_BEST_EFFORT, DDS_SECS(10));
  reader = dds_create_reader(participant, topic, qos, NULL);
  /*test*/
  samples[0] = CommonData_Data__alloc();
  dds_entity_t readcond = dds_create_readcondition (reader, DDS_NOT_READ_SAMPLE_STATE);
  dds_waitset_attach (waitSet, readcond, readcond);
  
  dds_subscription_matched_status_t matched_status;
  for (;;) {
    dds_waitset_wait (waitSet, wsresults, sizeof(wsresults)/sizeof(wsresults[0]), DDS_MSECS(1000));
    dds_get_subscription_matched_status(reader, &matched_status);
    printf("%s subscribe: matched_status.current_count = %d, matched.total_count = %d\n",param->name , matched_status.current_count, matched_status.total_count);
    dds_return_t rc =
        dds_take(reader, samples, infos, MAX_SAMPLES, MAX_SAMPLES);
    if (rc < 0) printf("dds_take: %s\n", dds_strretcode(-rc));
    if ((rc > 0) && (infos[0].valid_data)) {
      msg = (CommonData_Data *)samples[0];
      handle_msg(msg->context, strlen(msg->context), param->times_of_loop);
      vPortEnterCritical();
      if(param->delay < 0) {
        printf("\n%s recvf no.%d now : %d \n", param->name, msg->no, xTaskGetTickCount());
      } else {
        printf("\n%s recv no.%d now : %d \n", param->name, msg->no, xTaskGetTickCount());
      }
      if(param->fwd != -1) {
        xQueueSend(queues[param->domain_id], &msg->no, portMAX_DELAY);
      }
      vPortExitCritical();
    }
  }
  vTaskDelete(NULL);
}

void gateway_writer_task(void *arg) {
  dds_entity_t participant, topic, writer;
  measurement_task *param = (measurement_task *)arg;
  dds_qos_t *qos;
  CommonData_Data *msg = malloc(sizeof(CommonData_Data));
  printf("gateway_writer_task %s start at:%d\n", param->name, xTaskGetTickCount());
  msg->context = NULL;
  int start, end;
  #if defined(USE_ZONE)
    if(zcu_index >= 0) { //zcu index >= 0 means it is a zcu
      start = zcu_index * (int)(param->max_packet / 4);
      end = (zcu_index + 1) * (int)(param->max_packet / 4);
    } else { //zcu index < 0 means it is computer or something else
      start = 0;
      end = param->max_packet;
    }

  #elif defined(USE_DOMAIN)
    start = 0;
    end = param->max_packet;
  #endif


  dds_create_domain(param->domain_id, NULL);
  participant = dds_create_participant(param->domain_id, NULL, NULL);
  printf("%s particpant create done\n", param->name);
  if (participant < 0)
    printf("dds_create_participant: %s\n", dds_strretcode(-participant));

  topic = dds_create_topic(participant, &CommonData_Data_desc, param->name,
                           NULL, NULL);
  if (topic < 0) printf("dds_create_topic: %s\n", dds_strretcode(-topic));

  qos = dds_create_qos();
  dds_qset_reliability(qos, DDS_RELIABILITY_RELIABLE, DDS_SECS(10));
  //dds_qset_reliability(qos, DDS_RELIABILITY_BEST_EFFORT, DDS_SECS(10));
  dds_qset_history(qos, DDS_HISTORY_KEEP_ALL, 100);
  dds_qset_transport_priority(qos, param->prio);
  writer = dds_create_writer(participant, topic, qos, NULL);
  if (writer < 0) printf("dds_create_writer: %s\n", dds_strretcode(-writer));
  domainid2writer[param->domain_id] = writer;
  printf("create writer:%d", writer);
  dds_return_t rc = dds_set_status_mask(writer, DDS_PUBLICATION_MATCHED_STATUS);
  uint32_t status = 0;
  if (rc != DDS_RETCODE_OK)
    printf("dds_set_status_mask: %s\n", dds_strretcode(-rc));
  while (!(status & DDS_PUBLICATION_MATCHED_STATUS)) {
    rc = dds_get_status_changes(writer, &status);
    //printf("%s waiting...\n", param->name);
    if (rc != DDS_RETCODE_OK)
      printf("dds_get_status_changes: %s\n", dds_strretcode(-rc));

    dds_sleepfor(DDS_MSECS(20));
  }
  dds_publication_matched_status_t matched_status;
    dds_get_publication_matched_status(writer, &matched_status);
   printf("%s publish matched_status.current_count = %d, matched.total_count = %d, now = %d\n", param->name, matched_status.current_count, matched_status.total_count, xTaskGetTickCount());
  if(param->delay > 0) {
    if(param->fwd == -1) {
      for (int i=start; i < end; i++) {
        vPortEnterCritical();
        printf("\n%s sent no.%d now : %d \n", param->name, i, xTaskGetTickCount());
        vPortExitCritical();
        handle_msg("abcdef", 6, param->times_of_loop);
        make_msg(param->max_length, i,msg);
        rc = dds_write(writer, msg);
        vTaskDelay(param->delay);
      }
    } else {
      int32_t no, count = 0;
      while(count < end-start) {
        vPortEnterCritical();
        xQueueReceive(queues[param->fwd], &no, portMAX_DELAY);
        printf("\n%s sent no.%d now : %d \n", param->name, no, xTaskGetTickCount());
        vPortExitCritical();
        handle_msg("abcdef", 6, param->times_of_loop);
        make_msg(param->max_length, no, msg);
        rc = dds_write(writer, msg);
        count++;
      }

    }

  }

  vTaskDelete(NULL);
}

void init_queue(int domain_id) {
  QueueHandle_t xQueue = xQueueCreate(50, sizeof(int32_t));
  queues[domain_id] = xQueue;
}

void init_task(measurement_task *param) {
  printf("init_task: %s\n", param->name);
  if(param->fwd != -1 && param->isWriter == 0) init_queue(param->domain_id);
  if (param->isWriter) {
    xTaskCreate(gateway_writer_task, param->name, configMINIMAL_STACK_SIZE * 6,
                (void *)param, 2, NULL);
  } else {
    xTaskCreate(gateway_reader_task, param->name, configMINIMAL_STACK_SIZE * 6,
                (void *)param, 1, NULL);
  }
}

int find_config(int domain_id) {
  printf("domainid:%d\n", domain_id);
  for(int i = 0; i < config[index_of_config]->n; i++) {
    printf("domain id:%d\n", config[index_of_config]->tasks[i].domain_id);
    if(domain_id == config[index_of_config]->tasks[i].domain_id) {
      printf("found index:%d\n", i);
      return i;
    }
  }
  return -1;
}

void send_from_cmd(int index, int no) {
  if(config[index_of_config]->tasks[index].isWriter == 0) {
    printf("not writer!\n");
    return;
  }
  uint32_t status = 0;
  dds_entity_t writer = domainid2writer[config[index_of_config]->tasks[index].domain_id];
  CommonData_Data *msg = malloc(sizeof(CommonData_Data));
  msg->context = NULL;
  printf("index:%d, no:%d\n", index, no);
  vPortEnterCritical();
  printf("\n%s sent no.%d now : %d \n", config[index_of_config]->tasks[index].name, no, xTaskGetTickCount());
  vPortExitCritical();
  handle_msg("abcdef", 6, config[index_of_config]->tasks[index].times_of_loop);
  make_msg(config[index_of_config]->tasks[index].max_length, no, msg);
  dds_write(writer, msg);
}

void parse_config(int index) {
  printf("%d %d", index, number_of_config);
  srand((unsigned int)xTaskGetTickCount());
  #if defined(USE_ZONE)
    zcu_index = index - 3;
  #elif defined(USE_DOMAIN)
    dcu_index = index - 1;
  #endif
  if (index >= number_of_config) return;
  index_of_config = index;
  for (int i = 0; i < config[index]->n; i++) {
    init_task(&config[index]->tasks[i]);
  }
}

