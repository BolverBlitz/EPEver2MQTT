// Settings: Stores persistant settings, loads and saves to EEPROM

#include <Arduino.h>
#include <EEPROM.h>

#define EEPROM_SIZE 1024
#define AUTO_CHARGER_SETTINGS_MAGIC 0xAC45
#define AUTO_CHARGER_PROFILE_SIZE 15
#define AUTO_CHARGER_MAX_DEVICES 6

class Settings
{
  // change eeprom config version ONLY when new parameter is added and need reset the parameter
  unsigned int configVersion = 12;

public:
  String deviceNameStr;
  struct Data
  {                              // do not re-sort this struct
    unsigned int coVers;         // config version, if changed, previus config will erased
    char deviceName[40];         // device name
    char mqttServer[40];         // mqtt Server adress
    char mqttUser[40];           // mqtt Username
    char mqttPassword[40];       // mqtt Password
    char mqttTopic[40];          // mqtt publish topic
    char mqttTriggerPath[80];    // MQTT Data Trigger Path
    unsigned int mqttPort;       // mqtt port
    unsigned int mqttRefresh;    // mqtt refresh time
    unsigned int deviceQuantity; // Quantity of Devices
    bool mqttJson;               // switch between classic mqtt and json
    bool webUIdarkmode;          // Flag for color mode in webUI
    char httpUser[40];           // http basic auth username
    char httpPass[40];           // http basic auth password
    bool haDiscovery;            // HomeAssistant Discovery switch
    char NTPTimezone[40];        // Time zone code for NTP get it from here: https://github.com/nayarsystems/posix_tz_db/blob/master/zones.csv
    char NTPServer[40];          // NTP timepool Server
    byte LEDBrightness;         // brigthness of led
    char staticIP[16];        // static IP address
    char staticGW[16];        // static gateway
    char staticSN[16];        // static subnet mask
    char staticDNS[16];       // static DNS
    uint16_t autoChargerMagic;
    bool autoChargerEnabled;
    bool autoChargerCoordinatesSet;
    double autoChargerLatitude;
    double autoChargerLongitude;
    uint16_t autoChargerLowProfile[AUTO_CHARGER_PROFILE_SIZE];
    uint16_t autoChargerFullProfile[AUTO_CHARGER_PROFILE_SIZE];
    bool autoChargerMpptEnabled[AUTO_CHARGER_MAX_DEVICES];
    uint8_t autoChargerReleasePercent[AUTO_CHARGER_MAX_DEVICES];
    uint8_t autoChargerLowProfileSet;
    uint8_t autoChargerFullProfileSet;
   } data;

  void load()
  {
    data = {}; // clear bevor load data
    EEPROM.begin(EEPROM_SIZE);
    EEPROM.get(0, data);
    EEPROM.end();
    coVersCheck();
    sanitycheck();
    deviceNameStr = data.deviceName;
  }

  void save()
  {
    sanitycheck();
    EEPROM.begin(EEPROM_SIZE);
    EEPROM.put(0, data);
    EEPROM.commit();
    EEPROM.end();
  }

  void reset()
  {
    data = {};
    save();
  }

private:
  // check the variables from eeprom

  void sanitycheck()
  {
    if (strlen(data.deviceName) == 0 || strlen(data.deviceName) >= 40)
    {
      strcpy(data.deviceName, "EPEver2MQTT");
    }
    if (strlen(data.mqttServer) == 0 || strlen(data.mqttServer) >= 40)
    {
      strcpy(data.mqttServer, "");
    }
    if (strlen(data.mqttUser) == 0 || strlen(data.mqttUser) >= 40)
    {
      strcpy(data.mqttUser, "");
    }
    if (strlen(data.mqttPassword) == 0 || strlen(data.mqttPassword) >= 40)
    {
      strcpy(data.mqttPassword, "");
    }
    if (strlen(data.mqttTopic) == 0 || strlen(data.mqttTopic) >= 40)
    {
      strcpy(data.mqttTopic, "EPEver");
    }
    if (data.mqttPort <= 0 || data.mqttPort >= 65530)
    {
      data.mqttPort = 0;
    }
    if (data.mqttRefresh <= 1 || data.mqttRefresh >= 65530)
    {
      data.mqttRefresh = 0;
    }
    if (data.mqttJson && !data.mqttJson)
    {
      data.mqttJson = false;
    }
    if (data.deviceQuantity < 1 || data.deviceQuantity >= 10)
    {
      data.deviceQuantity = 1;
    }
    if (strlen(data.mqttTriggerPath) == 0 || strlen(data.mqttTriggerPath) >= 80)
    {
      strcpy(data.mqttTriggerPath, "");
    }
    if (data.webUIdarkmode && !data.webUIdarkmode)
    {
      data.webUIdarkmode = false;
    }
    if (strlen(data.httpUser) == 0 || strlen(data.httpUser) >= 40)
    {
      strcpy(data.httpUser, "");
    }
    if (strlen(data.httpPass) == 0 || strlen(data.httpPass) >= 40)
    {
      strcpy(data.httpPass, "");
    }
    if (data.haDiscovery && !data.haDiscovery)
    {
      data.haDiscovery = false;
    }
    if (strlen(data.NTPTimezone) == 0 || strlen(data.NTPTimezone) >= 40)
    {
      strcpy(data.NTPTimezone, "");
    }
    if (strlen(data.NTPServer) == 0 || strlen(data.NTPServer) >= 40)
    {
      strcpy(data.NTPServer, "pool.ntp.org");
    }
    if (data.LEDBrightness && !data.LEDBrightness)
    {
      data.LEDBrightness = 127;
    }
    if (strlen(data.staticIP) == 0 || strlen(data.staticIP) >= 16)
    {
      strcpy(data.staticIP, "");
    }
    if (strlen(data.staticGW) == 0 || strlen(data.staticGW) >= 16)
    {
      strcpy(data.staticGW, "");
    }
    if (strlen(data.staticSN) == 0 || strlen(data.staticSN) >= 16)
    {
      strcpy(data.staticSN, "");
    }
    if (strlen(data.staticDNS) == 0 || strlen(data.staticDNS) >= 16)
    {
      strcpy(data.staticDNS, "");
    }
    if (data.autoChargerMagic != AUTO_CHARGER_SETTINGS_MAGIC)
    {
      initializeAutoCharger();
    }
    if (!isfinite(data.autoChargerLatitude) || data.autoChargerLatitude < -90.0 || data.autoChargerLatitude > 90.0)
    {
      data.autoChargerLatitude = 0.0;
    }
    if (!isfinite(data.autoChargerLongitude) || data.autoChargerLongitude < -180.0 || data.autoChargerLongitude > 180.0)
    {
      data.autoChargerLongitude = 0.0;
    }
    for (uint8_t device = 0; device < AUTO_CHARGER_MAX_DEVICES; device++)
    {
      if (data.autoChargerReleasePercent[device] > 100)
      {
        data.autoChargerReleasePercent[device] = 60;
      }
    }
    if (data.autoChargerLowProfileSet > 1)
    {
      data.autoChargerLowProfileSet = 0;
    }
    if (data.autoChargerFullProfileSet > 1)
    {
      data.autoChargerFullProfileSet = 0;
    }
    if (!data.autoChargerLowProfileSet || !data.autoChargerFullProfileSet)
    {
      data.autoChargerEnabled = false;
    }
  }
  void initializeAutoCharger()
  {
    data.autoChargerMagic = AUTO_CHARGER_SETTINGS_MAGIC;
    data.autoChargerEnabled = false;
    data.autoChargerCoordinatesSet = false;
    data.autoChargerLatitude = 0.0;
    data.autoChargerLongitude = 0.0;
    memset(data.autoChargerLowProfile, 0, sizeof(data.autoChargerLowProfile));
    memset(data.autoChargerFullProfile, 0, sizeof(data.autoChargerFullProfile));
    memset(data.autoChargerMpptEnabled, 0, sizeof(data.autoChargerMpptEnabled));
    data.autoChargerLowProfileSet = 0;
    data.autoChargerFullProfileSet = 0;
    for (uint8_t device = 0; device < AUTO_CHARGER_MAX_DEVICES; device++)
    {
      data.autoChargerReleasePercent[device] = 60;
    }
  }
  void coVersCheck()
  {
    if (data.coVers != configVersion)
    {
      data.coVers = configVersion;
      strcpy(data.deviceName, "EPEver2MQTT");
      strcpy(data.mqttServer, "");
      strcpy(data.mqttUser, "");
      strcpy(data.mqttPassword, "");
      strcpy(data.mqttTopic, "EPEver");
      strcpy(data.mqttTriggerPath, "");
      data.deviceQuantity = 1;
      data.mqttPort = 0;
      data.mqttRefresh = 300;
      data.mqttJson = false;
      data.webUIdarkmode = false;
      strcpy(data.httpUser, "");
      strcpy(data.httpPass, "");
      data.haDiscovery = false;
      data.LEDBrightness = 127;
      strcpy(data.staticIP, "");
      strcpy(data.staticGW, "");
      strcpy(data.staticSN, "");
      strcpy(data.staticDNS, "");
      save();
      load();
    }
  }
};
