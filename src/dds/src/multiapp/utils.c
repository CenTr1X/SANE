#include <stdlib.h>
#include <string.h>

#include "FreeRTOS.h"
#include "dds/dds.h"
#include "measurement.h"
#include "measurement/CommonData.h"
#include "task.h"
#include "md5_copy.h"

#include "printf.h"

#define MAX_SAMPLES 1
extern measurement_config writer_config;
extern int number_of_config;
int test = 0;
int zcu_index;

typedef struct writer_param_s {
  int start;
  int end;
  int max_length;
  int delay;
  dds_entity_t writer;
} writer_param;

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
}

void writer_task(void *arg) {
  writer_param *param = (writer_param *)arg;
  CommonData_Data *msg = malloc(sizeof(CommonData_Data));
  msg->context = NULL;
  char *name = pcTaskGetName(NULL);
  printf("in task, taskname:%s start:%d end:%d\n", name, param->start, param->end);
  for (int i=param->start; i < param->end; i++) {
    vPortEnterCritical();
    printf("\n%s sent no.%d now : %d \n", name, i, xTaskGetTickCount());
    vPortExitCritical();
    make_msg(param->max_length, i,msg);
    dds_write(param->writer, msg);
    vTaskDelay(param->delay);
  }
  vTaskDelete(NULL);
}

void init_dds(measurement_task *param, int n) {
  dds_entity_t participant, topic, writer;
  dds_qos_t *qos;

  dds_create_domain(param->domain_id, NULL);
  participant = dds_create_participant(param->domain_id, NULL, NULL);

  if (participant < 0)
    printf("dds_create_participant: %s\n", dds_strretcode(-participant));

  topic = dds_create_topic(participant, &CommonData_Data_desc, param->name,
                           NULL, NULL);
  if (topic < 0) printf("dds_create_topic: %s\n", dds_strretcode(-topic));

  qos = dds_create_qos();
  dds_qset_reliability(qos, DDS_RELIABILITY_RELIABLE, DDS_SECS(10));
  //dds_qset_reliability(qos, DDS_RELIABILITY_BEST_EFFORT, DDS_SECS(10));
  dds_qset_history(qos, DDS_HISTORY_KEEP_ALL, 100);
  
  writer = dds_create_writer(participant, topic, qos, NULL);
  if (writer < 0) printf("dds_create_writer: %s\n", dds_strretcode(-writer));

  dds_return_t rc = dds_set_status_mask(writer, DDS_PUBLICATION_MATCHED_STATUS);
  uint32_t status = 0;
  if (rc != DDS_RETCODE_OK)
    printf("dds_set_status_mask: %s\n", dds_strretcode(-rc));
  while (!(status & DDS_PUBLICATION_MATCHED_STATUS)) {
    rc = dds_get_status_changes(writer, &status);
    if (rc != DDS_RETCODE_OK)
      printf("dds_get_status_changes: %s\n", dds_strretcode(-rc));
    printf("waiting domain:%d name:%s...\n", param->domain_id, param->name);
    dds_sleepfor(DDS_MSECS(20));
  }
  printf("matched!\n");
  int num_per_task = 300;
  int start = 0;
  int end = num_per_task;
  for (int i = 0; i < n; i++) {
    printf("create %d\n", i);
    writer_param *temp = pvPortMalloc(sizeof(writer_param));
    char *name = pvPortMalloc(10);
    temp->start = start;
    temp->end = end;
    temp->delay = param->delay;
    temp->max_length = param->max_length;
    temp->writer = writer;
    snprintf(name, 9, "task%d", i);
    xTaskCreate(writer_task, name, configMINIMAL_STACK_SIZE * 6,
            (void *)temp, 2, NULL);
    start += num_per_task;
    end += num_per_task;
  }

}

void parse_config(int n) {
  printf("run multiapp\n");
  init_dds(&(writer_config.tasks[0]), n);
}

