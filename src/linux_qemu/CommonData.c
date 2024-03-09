/****************************************************************

  Generated by Eclipse Cyclone DDS IDL to C Translator
  File name: CommonData.c
  Source: /home/cyclonedds-master/examples/gateway/CommonData.idl
  Cyclone DDS: V0.11.0

*****************************************************************/
#include "CommonData.h"

static const uint32_t CommonData_Data_ops [] =
{
  /* Data */
  DDS_OP_ADR | DDS_OP_TYPE_4BY | DDS_OP_FLAG_SGN, offsetof (CommonData_Data, no),
  DDS_OP_ADR | DDS_OP_TYPE_STR, offsetof (CommonData_Data, context),
  DDS_OP_RTS
};

/* Type Information:
  [MINIMAL 46ba38848e85f1f2340c431192bd] (#deps: 0)
  [COMPLETE 2e48cfedfb8be68a081c62abc24b] (#deps: 0)
*/
#define TYPE_INFO_CDR_CommonData_Data (const unsigned char []){ \
  0x60, 0x00, 0x00, 0x00, 0x01, 0x10, 0x00, 0x40, 0x28, 0x00, 0x00, 0x00, 0x24, 0x00, 0x00, 0x00, \
  0x14, 0x00, 0x00, 0x00, 0xf1, 0x46, 0xba, 0x38, 0x84, 0x8e, 0x85, 0xf1, 0xf2, 0x34, 0x0c, 0x43, \
  0x11, 0x92, 0xbd, 0x00, 0x38, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, \
  0x00, 0x00, 0x00, 0x00, 0x02, 0x10, 0x00, 0x40, 0x28, 0x00, 0x00, 0x00, 0x24, 0x00, 0x00, 0x00, \
  0x14, 0x00, 0x00, 0x00, 0xf2, 0x2e, 0x48, 0xcf, 0xed, 0xfb, 0x8b, 0xe6, 0x8a, 0x08, 0x1c, 0x62, \
  0xab, 0xc2, 0x4b, 0x00, 0x62, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, \
  0x00, 0x00, 0x00, 0x00\
}
#define TYPE_INFO_CDR_SZ_CommonData_Data 100u
#define TYPE_MAP_CDR_CommonData_Data (const unsigned char []){ \
  0x4c, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0xf1, 0x46, 0xba, 0x38, 0x84, 0x8e, 0x85, 0xf1, \
  0xf2, 0x34, 0x0c, 0x43, 0x11, 0x92, 0xbd, 0x00, 0x34, 0x00, 0x00, 0x00, 0xf1, 0x51, 0x01, 0x00, \
  0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x24, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, \
  0x0b, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x04, 0x7f, 0xa3, 0xb7, 0x67, 0x00, \
  0x0c, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x70, 0x00, 0x5c, 0x18, 0xef, 0x72, \
  0x76, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0xf2, 0x2e, 0x48, 0xcf, 0xed, 0xfb, 0x8b, 0xe6, \
  0x8a, 0x08, 0x1c, 0x62, 0xab, 0xc2, 0x4b, 0x00, 0x5e, 0x00, 0x00, 0x00, 0xf2, 0x51, 0x01, 0x00, \
  0x19, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x11, 0x00, 0x00, 0x00, 0x43, 0x6f, 0x6d, 0x6d, \
  0x6f, 0x6e, 0x44, 0x61, 0x74, 0x61, 0x3a, 0x3a, 0x44, 0x61, 0x74, 0x61, 0x00, 0x00, 0x00, 0x00, \
  0x36, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x11, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
  0x01, 0x00, 0x04, 0x00, 0x03, 0x00, 0x00, 0x00, 0x6e, 0x6f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
  0x16, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x70, 0x00, 0x08, 0x00, 0x00, 0x00, \
  0x63, 0x6f, 0x6e, 0x74, 0x65, 0x78, 0x74, 0x00, 0x00, 0x00, 0x00, 0x00, 0x22, 0x00, 0x00, 0x00, \
  0x01, 0x00, 0x00, 0x00, 0xf2, 0x2e, 0x48, 0xcf, 0xed, 0xfb, 0x8b, 0xe6, 0x8a, 0x08, 0x1c, 0x62, \
  0xab, 0xc2, 0x4b, 0xf1, 0x46, 0xba, 0x38, 0x84, 0x8e, 0x85, 0xf1, 0xf2, 0x34, 0x0c, 0x43, 0x11, \
  0x92, 0xbd\
}
#define TYPE_MAP_CDR_SZ_CommonData_Data 242u
const dds_topic_descriptor_t CommonData_Data_desc =
{
  .m_size = sizeof (CommonData_Data),
  .m_align = dds_alignof (CommonData_Data),
  .m_flagset = DDS_TOPIC_XTYPES_METADATA,
  .m_nkeys = 0u,
  .m_typename = "CommonData::Data",
  .m_keys = NULL,
  .m_nops = 3,
  .m_ops = CommonData_Data_ops,
  .m_meta = "",
  .type_information = { .data = TYPE_INFO_CDR_CommonData_Data, .sz = TYPE_INFO_CDR_SZ_CommonData_Data },
  .type_mapping = { .data = TYPE_MAP_CDR_CommonData_Data, .sz = TYPE_MAP_CDR_SZ_CommonData_Data }
};
