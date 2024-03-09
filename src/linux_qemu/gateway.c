#include "dds/dds.h"
#include "detection.h"
#include "DetectionData.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

/* An array of one message (aka sample in dds terms) will be used. */
#define MAX_SAMPLES 1


void handle_warning(dds_entity_t writer,int numberOfTire, char *description, int size, warning_level level);


int main()
{
  dds_entity_t tire_participant;
  dds_entity_t tire_topic;
  dds_entity_t hud_participant;
  dds_entity_t hud_topic;
  dds_entity_t reader;
  dds_entity_t writer;
  TireWarningData_Info *info;
  void *samples[MAX_SAMPLES];
  dds_sample_info_t infos[MAX_SAMPLES];
  dds_return_t rc;
  dds_qos_t *qos;
  dds_create_domain(DDS_TIRE_DOMAIN , NULL);
  dds_create_domain(DDS_HUD_DOMAIN , NULL);
  tire_participant = dds_create_participant (DDS_TIRE_DOMAIN, NULL, NULL);
  if (tire_participant < 0)
    DDS_FATAL("dds_create_participant: %s\n", dds_strretcode(-tire_participant));

  tire_topic = dds_create_topic (
    tire_participant, &TireWarningData_Info_desc, "TireWarningData", NULL, NULL);
  if (tire_topic < 0)
    DDS_FATAL("dds_create_topic: %s\n", dds_strretcode(-tire_topic));

  hud_participant = dds_create_participant (DDS_HUD_DOMAIN, NULL, NULL);
  if (hud_participant < 0)
    DDS_FATAL("dds_create_participant: %s\n", dds_strretcode(-hud_participant));

  hud_topic = dds_create_topic (
    hud_participant, &HudWarningData_Info_desc, "HudWarningData", NULL, NULL);
  if (hud_topic < 0)
    DDS_FATAL("dds_create_topic: %s\n", dds_strretcode(-hud_topic));
  
  printf ("\n=== [Subscriber] Waiting for a sample...\n");
  /* Create a reliable Reader. */
  qos = dds_create_qos ();
  dds_qset_reliability (qos, DDS_RELIABILITY_RELIABLE, DDS_SECS (10));
  reader = dds_create_reader (tire_participant, tire_topic, qos, NULL);
  if (reader < 0)
    DDS_FATAL("dds_create_reader: %s\n", dds_strretcode(-reader));

  writer = dds_create_writer (hud_participant, hud_topic, qos, NULL);
  if (writer < 0)
    DDS_FATAL("dds_create_writer: %s\n", dds_strretcode(-writer));
  dds_delete_qos(qos);

  printf ("\n=== [Subscriber] Waiting for a 222 ...\n");

  /* Initialize sample buffer, by pointing the void pointer within
   * the buffer array to a valid sample memory location. */
  samples[0] = TireWarningData_Info__alloc ();
  //handle_warning(writer, 1, "test", 4, 1);
  //return;
  /* Poll until data has been read. */
  while (true)
  {
    /* Do the actual read.
     * The return value contains the number of read samples. */
    rc = dds_take (reader, samples, infos, MAX_SAMPLES, MAX_SAMPLES);
    if (rc < 0)
      DDS_FATAL("dds_take: %s\n", dds_strretcode(-rc));

    /* Check if we read some data and it is valid. */
    if ((rc > 0) && (infos[0].valid_data))
    {
      printf("recv:");
      /* Print Message. */
      //msg = (HelloWorldData_Msg*) samples[0];
      info = (TireWarningData_Info*) samples[0];
      handle_warning(writer, info->numberOfTire, info->context, sizeof(info->context), (warning_level)info->level);
      printf("this sample has been read?: %d", infos[0].sample_state);
    }
    else
    {
      /* Polling sleep. */
      dds_sleepfor (DDS_MSECS (20));
    }
  }

  /* Free the data location. */
  TireWarningData_Info_free (samples[0], DDS_FREE_ALL);

  /* Deleting the participant will delete all its children recursively as well. */
  rc = dds_delete (tire_participant);
  if (rc != DDS_RETCODE_OK)
    DDS_FATAL("dds_delete: %s\n", dds_strretcode(-rc));

  rc = dds_delete (hud_participant);
  if (rc != DDS_RETCODE_OK)
    DDS_FATAL("dds_delete: %s\n", dds_strretcode(-rc));

}

void handle_warning(dds_entity_t writer,int numberOfTire, char *description, int size, warning_level level)
{
  dds_return_t rc = dds_set_status_mask(writer, DDS_PUBLICATION_MATCHED_STATUS);
  uint32_t status = 0;
  HudWarningData_Info info;

  printf("!!!!!numberOfTire:%d description:%s warning_level:%d!!!!!!!!!!!!!\n", numberOfTire, description, level);

  while(!(status & DDS_PUBLICATION_MATCHED_STATUS))
  {
    rc = dds_get_status_changes (writer, &status);
    //printf("status:%d\n", status);
    if (rc != DDS_RETCODE_OK)
      DDS_FATAL("dds_get_status_changes: %s\n", dds_strretcode(-rc));
    //printf("wating...\n");    
    dds_sleepfor (DDS_MSECS (20));
  }
  info.level = level;
  info.context = malloc(size);
  strcpy(info.context, description);
  //info.context = "testtest123";
  switch (numberOfTire)
  {
    case 0:
      info.location = "Front Left Tire";
      break;
    case 1:
      info.location = "Front Right Tire";
      break;
    case 2:
      info.location = "Rear Left Tire";
      break;
    case 3:
      info.location = "Rear Right Tire";
      break;
    default:
      info.location = "Unknown Location";
  }
  printf("sent:");
  fflush(stdout);
  rc = dds_write (writer, &info);
  if (rc != DDS_RETCODE_OK)
    DDS_FATAL("dds_write: %s\n", dds_strretcode(-rc));
}
