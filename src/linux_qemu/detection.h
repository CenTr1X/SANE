typedef enum tire_status_e {
    GOOD,
    FLAT_TIRE,
    TOO_LOW
} tire_status;

typedef enum waning_level_e {
    LOW,
    MEDIUM,
    HIGH,
    CRITICAL
} warning_level;

#define DDS_TIRE_DOMAIN ((uint32_t) 1)
#define DDS_HUD_DOMAIN ((uint32_t) 2)

void start_handler();
void start_receiver();
void publish_warning(int numberOfTire, char description[], warning_level level);