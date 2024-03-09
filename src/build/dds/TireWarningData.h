/****************************************************************

  Generated by Eclipse Cyclone DDS IDL to C Translator
  File name: TireWarningData.h
  Source: /home/centrix/Code/clean_dds/cyclonedds/build/bin/TireWarningData.idl
  Cyclone DDS: V0.11.0

*****************************************************************/
#ifndef DDSC_TIREWARNINGDATA_H_DD7873C6F45F1D0A269AFF9E9814F45D
#define DDSC_TIREWARNINGDATA_H_DD7873C6F45F1D0A269AFF9E9814F45D

#include "dds/ddsc/dds_public_impl.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct TireWarningData_Info
{
  uint32_t numberOfTire;
  char * context;
  uint32_t level;
} TireWarningData_Info;

extern const dds_topic_descriptor_t TireWarningData_Info_desc;

#define TireWarningData_Info__alloc() \
((TireWarningData_Info*) dds_alloc (sizeof (TireWarningData_Info)));

#define TireWarningData_Info_free(d,o) \
dds_sample_free ((d), &TireWarningData_Info_desc, (o))

#ifdef __cplusplus
}
#endif

#endif /* DDSC_TIREWARNINGDATA_H_DD7873C6F45F1D0A269AFF9E9814F45D */