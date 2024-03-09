#include <stdlib.h>                                           
#include <string.h>                              
                                                                     
//#include "FreeRTOS.h"                                                    
#include "dds/dds.h"                                 
#include "measurement.h"                                           
#include "CommonData.h"
#include "md5_copy.h"                                             
#include "queue.h"                                      
#include <unistd.h>                                 
#include <pthread.h>

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#define MAX_SAMPLES 1                                                      
extern measurement_config *config[];    
extern int number_of_config;                                 
int zcu_index = -3;             
pthread_mutex_t mutex;
int times = 0;
int msgids[10];
                                   
void make_msg(int max_length, int no, CommonData_Data *buf) {
  srand((unsigned int)time(NULL));
  int length = rand() % max_length + 1;  // add a '\00'
  buf->context = malloc(length);
  memset(buf->context, 'a', length);
  buf->context[length - 1] = '\00';
  buf->no = no;
}                         

void handle_msg(char *content, int length, int times) {
  //do md5 calc loop
  //printf("handle now\n");
  unsigned char output[16], temp[16];
  md5(content, length, temp);
  for(int i=0; i < times-1; i++) {
    md5(temp, 16, output);
    memcpy(temp, output, 16);
  }
  //printf("handle done\n");
}

void create_msg_queue(int index) {
  key_t key = ftok(".", index);
  msgids[index] = msgget(key, IPC_CREAT | 0666);
  printf("create_msg_queue index:%d msgid:%d\n", index, msgids[index]);
}
                                                                           
void gateway_reader_task(void *arg) {
  dds_entity_t participant, topic, reader;
  measurement_task *param = (measurement_task *)arg;
  dds_qos_t *qos;
  CommonData_Data *msg;
  void *samples[MAX_SAMPLES];
  dds_sample_info_t infos[MAX_SAMPLES];
  dds_entity_t waitSet;
  dds_attach_t wsresults[1];

    
  dds_create_domain(param->domain_id, NULL);
  participant = dds_create_participant(param->domain_id, NULL, NULL);
  waitSet = dds_create_waitset (participant);

  topic = dds_create_topic(participant, &CommonData_Data_desc, param->name,
                           NULL, NULL);
  qos = dds_create_qos();
  dds_qset_reliability(qos, DDS_RELIABILITY_RELIABLE, DDS_SECS(10));
  //dds_qset_reliability(qos, DDS_RELIABILITY_BEST_EFFORT, DDS_SECS(10));
  dds_qset_history(qos, DDS_HISTORY_KEEP_ALL, 100);
  reader = dds_create_reader(participant, topic, qos, NULL);
  /*test*/
  samples[0] = CommonData_Data__alloc();
  //samples[0] = malloc(sizeof(CommonData_Data));
  dds_entity_t readcond = dds_create_readcondition (reader, DDS_NOT_READ_SAMPLE_STATE);
  dds_waitset_attach (waitSet, readcond, readcond);
  
  dds_subscription_matched_status_t matched_status;

  if (param->fwd != -1) create_msg_queue(param->domain_id);

  for (;;) {
    dds_waitset_wait (waitSet, wsresults, sizeof(wsresults)/sizeof(wsresults[0]), DDS_MSECS(1000));
    dds_get_subscription_matched_status(reader, &matched_status);
    printf("%s subscribe: matched_status.current_count = %d, matched.total_count = %d\n",param->name , matched_status.current_count, matched_status.total_count);
    dds_return_t rc =
        dds_take(reader, samples, infos, MAX_SAMPLES, MAX_SAMPLES);
    if (rc < 0) printf("dds_take: %s\n", dds_strretcode(-rc));
    //printf("%s rc:%d valid:%d\n",param->name, rc, (infos[0].valid_data));
    if ((rc > 0) && (infos[0].valid_data)) {
      msg = (CommonData_Data *)samples[0];
      //printf("=== [Subscriber] Received : ");
      pthread_mutex_lock(&mutex);
      printf("%s recv no.%d \n", param->name, msg->no);
      pthread_mutex_unlock(&mutex);
      handle_msg(msg->context, strlen(msg->context), times);
      if (param->fwd != -1) {
        notification n;
        n.no = msg->no;
        printf("%s to send queue no.%d \n", param->name, n.no);
        msgsnd(msgids[param->domain_id], &n, sizeof(notification), 0);
        printf("%s sent queue no.%d \n", param->name, n.no);
      }
    }
    //vTaskDelay(param->delay);
  }
  //vTaskDelete(NULL);
}                                                                              
                                                                                 
void gateway_writer_task(void *arg) {
  printf("gateway_writer_task\n");
  dds_entity_t participant, topic, writer;
  measurement_task *param = (measurement_task *)arg;
  dds_qos_t *qos;
  CommonData_Data *msg = malloc(sizeof(CommonData_Data));
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
  if (participant < 0)
    printf("dds_create_participant: %s\n", dds_strretcode(-participant));

  topic = dds_create_topic(participant, &CommonData_Data_desc, param->name,
                           NULL, NULL);
  if (topic < 0) printf("dds_create_topic: %s\n", dds_strretcode(-topic));

  qos = dds_create_qos();
  dds_qset_reliability(qos, DDS_RELIABILITY_RELIABLE, DDS_SECS(10));
  //dds_qset_reliability(qos, DDS_RELIABILITY_BEST_EFFORT, DDS_SECS(10));
  writer = dds_create_writer(participant, topic, qos, NULL);
  if (writer < 0) printf("dds_create_writer: %s\n", dds_strretcode(-writer));

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
   printf("%s publish matched_status.current_count = %d, matched.total_count = %d\n", param->name, matched_status.current_count, matched_status.total_count);
  
      printf("going to recv, msgid index:%d, msgid:%d\n", param->fwd, msgids[param->fwd]);
  for (int i=0; i < 300; i++) {
    notification n;
    int num;
    if (param->fwd != -1) {
      printf("%s to recv from queue no.999 \n", param->name);
      msgrcv(msgids[param->fwd], &n, sizeof(notification), 0, 0);
      printf("%s recv from queue no.%d \n", param->name, n.no);
      //free(n);
      num = n.no;
    } else {
      num = i;
    }
    make_msg(param->max_length, num,msg);
    //printf("going to send %s no.%d\n", param->name, i);
    rc = dds_write(writer, msg);
    pthread_mutex_lock(&mutex);
    printf("%s sent no.%d \n", param->name, num);
    pthread_mutex_unlock(&mutex);
    free(msg->context);
    dds_sleepfor(DDS_MSECS(param->delay));  
  }
}                                                                      
                                                                         
void init_task(measurement_task *param) {                            
  printf("init_task: %s\n", param->name);                                    
  pthread_t thread;                                                          
  if (param->isWriter) {                                                                                     
    pthread_create(&thread, NULL, gateway_writer_task, (void *)param);   
  } else {                                                                                                            
    pthread_create(&thread, NULL, gateway_reader_task, (void *)param);          
  }                                                                              
  pthread_detach(thread);                                                        
}                                                                                
                                                                                 
int main(int argc, char *argv[]) {                                                                 
  int index = 0;                                                                
  if (index > number_of_config) return;
  pthread_mutex_init(&mutex, 0);
  for (int i = 1; i < argc; i++) {
    if(strcmp(argv[i], "-p") == 0) {
      int task_index = atoi(argv[++i]);
      measurement_task *current_task = &config[index]->tasks[task_index];
      current_task->delay = atoi(argv[++i]);
      current_task->max_length = atoi(argv[++i]);
      current_task->times_of_loop = atoi(argv[++i]);
      current_task->max_packet = atoi(argv[++i]);
    } else if(strcmp(argv[i], "-t") == 0) {
      times = atoi(argv[++i]);
    }
  }                                          
  for (int i = 0; i < config[index]->n; i++) {                            
    init_task(&config[index]->tasks[i]);                           
  }                                                                              
  pthread_exit(NULL);                                                            
}      