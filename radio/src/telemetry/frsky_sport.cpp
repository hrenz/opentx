/*
 * Authors (alphabetical order)
 * - Andre Bernet <bernet.andre@gmail.com>
 * - Andreas Weitl
 * - Bertrand Songis <bsongis@gmail.com>
 * - Bryan J. Rentoul (Gruvin) <gruvin@gmail.com>
 * - Cameron Weeks <th9xer@gmail.com>
 * - Erez Raviv
 * - Gabriel Birkus
 * - Jean-Pierre Parisy
 * - Karl Szmutny
 * - Michael Blandford
 * - Michal Hlavinka
 * - Pat Mackenzie
 * - Philip Moss
 * - Rob Thomson
 * - Romolo Manfredini <romolo.manfredini@gmail.com>
 * - Thomas Husterer
 *
 * opentx is based on code named
 * gruvin9x by Bryan J. Rentoul: http://code.google.com/p/gruvin9x/,
 * er9x by Erez Raviv: http://code.google.com/p/er9x/,
 * and the original (and ongoing) project by
 * Thomas Husterer, th9x: http://code.google.com/p/th9x/
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include "../opentx.h"

// FrSky PRIM IDs (1 byte)
#define DATA_FRAME              0x10

// FrSky old DATA IDs (1 byte)
#define GPS_ALT_BP_ID           0x01
#define TEMP1_ID                0x02
#define RPM_ID                  0x03
#define FUEL_ID                 0x04
#define TEMP2_ID                0x05
#define VOLTS_ID                0x06
#define GPS_ALT_AP_ID           0x09
#define BARO_ALT_BP_ID          0x10
#define GPS_SPEED_BP_ID         0x11
#define GPS_LONG_BP_ID          0x12
#define GPS_LAT_BP_ID           0x13
#define GPS_COURS_BP_ID         0x14
#define GPS_DAY_MONTH_ID        0x15
#define GPS_YEAR_ID             0x16
#define GPS_HOUR_MIN_ID         0x17
#define GPS_SEC_ID              0x18
#define GPS_SPEED_AP_ID         0x19
#define GPS_LONG_AP_ID          0x1A
#define GPS_LAT_AP_ID           0x1B
#define GPS_COURS_AP_ID         0x1C
#define BARO_ALT_AP_ID          0x21
#define GPS_LONG_EW_ID          0x22
#define GPS_LAT_NS_ID           0x23
#define ACCEL_X_ID              0x24
#define ACCEL_Y_ID              0x25
#define ACCEL_Z_ID              0x26
#define CURRENT_ID              0x28
#define VARIO_ID                0x30
#define VFAS_ID                 0x39
#define VOLTS_BP_ID             0x3A
#define VOLTS_AP_ID             0x3B
#define FRSKY_LAST_ID           0x3F

// FrSky new DATA IDs (2 bytes)
#define ALT_FIRST_ID            0x0100
#define ALT_LAST_ID             0x010f
#define VARIO_FIRST_ID          0x0110
#define VARIO_LAST_ID           0x011f
#define CURR_FIRST_ID           0x0200
#define CURR_LAST_ID            0x020f
#define VFAS_FIRST_ID           0x0210
#define VFAS_LAST_ID            0x021f
#define CELLS_FIRST_ID          0x0300
#define CELLS_LAST_ID           0x030f
#define T1_FIRST_ID             0x0400
#define T1_LAST_ID              0x040f
#define T2_FIRST_ID             0x0410
#define T2_LAST_ID              0x041f
#define RPM_FIRST_ID            0x0500
#define RPM_LAST_ID             0x050f
#define FUEL_FIRST_ID           0x0600
#define FUEL_LAST_ID            0x060f
#define ACCX_FIRST_ID           0x0700
#define ACCX_LAST_ID            0x070f
#define ACCY_FIRST_ID           0x0710
#define ACCY_LAST_ID            0x071f
#define ACCZ_FIRST_ID           0x0720
#define ACCZ_LAST_ID            0x072f
#define GPS_LONG_LATI_FIRST_ID  0x0800
#define GPS_LONG_LATI_LAST_ID   0x080f
#define GPS_ALT_FIRST_ID        0x0820
#define GPS_ALT_LAST_ID         0x082f
#define GPS_SPEED_FIRST_ID      0x0830
#define GPS_SPEED_LAST_ID       0x083f
#define GPS_COURS_FIRST_ID      0x0840
#define GPS_COURS_LAST_ID       0x084f
#define GPS_TIME_DATE_FIRST_ID  0x0850
#define GPS_TIME_DATE_LAST_ID   0x085f
#define A3_FIRST_ID             0x0900
#define A3_LAST_ID              0x090f
#define A4_FIRST_ID             0x0910
#define A4_LAST_ID              0x091f
#define AIR_SPEED_FIRST_ID      0x0a00
#define AIR_SPEED_LAST_ID       0x0a0f
#define RSSI_ID                 0xf101
#define ADC1_ID                 0xf102
#define ADC2_ID                 0xf103
#define BATT_ID                 0xf104
#define SWR_ID                  0xf105

// Default sensor data IDs (Physical IDs + CRC)
#define DATA_ID_VARIO            0x00 // 0
#define DATA_ID_FLVSS            0xA1 // 1
#define DATA_ID_FAS              0x22 // 2
#define DATA_ID_GPS              0x83 // 3
#define DATA_ID_RPM              0xE4 // 4
#define DATA_ID_SP2UH            0x45 // 5
#define DATA_ID_SP2UR            0xC6 // 6

struct SportSensor {
  const uint16_t firstId;
  const uint16_t lastId;
  const char * name;
  const TelemetryUnit unit;
  const uint8_t prec;
};

const SportSensor sportSensors[] = {
  { RSSI_ID, RSSI_ID, ZSTR_RSSI, UNIT_RAW, 0 },
  { T1_FIRST_ID, T2_LAST_ID, ZSTR_TEMP, UNIT_CELSIUS, 0 },
  { RPM_FIRST_ID, RPM_LAST_ID, ZSTR_RPM, UNIT_RPMS, 0 },
  { FUEL_FIRST_ID, FUEL_LAST_ID, ZSTR_FUEL, UNIT_PERCENT, 0 },
  { ALT_FIRST_ID, ALT_LAST_ID, ZSTR_ALT, UNIT_METERS, 2 }, // TODO autoOffset
  { VARIO_FIRST_ID, VARIO_LAST_ID, ZSTR_VSPD, UNIT_METERS_PER_SECOND, 2 },
  { ACCX_FIRST_ID, ACCX_LAST_ID, ZSTR_ACCX, UNIT_G, 2 },
  { ACCY_FIRST_ID, ACCY_LAST_ID, ZSTR_ACCY, UNIT_G, 2 },
  { ACCZ_FIRST_ID, ACCZ_LAST_ID, ZSTR_ACCZ, UNIT_G, 2 },
  { CURR_FIRST_ID, CURR_LAST_ID, ZSTR_CURR, UNIT_AMPS, 1 },
  { VFAS_FIRST_ID, VFAS_LAST_ID, ZSTR_VFAS, UNIT_VOLTS, 2 },
  { AIR_SPEED_FIRST_ID, AIR_SPEED_LAST_ID, ZSTR_ASPD, UNIT_METERS_PER_SECOND, 1 },
  { GPS_SPEED_FIRST_ID, GPS_SPEED_LAST_ID, ZSTR_GSPD, UNIT_KTS, 3 }, // TODO precision 3!!!
  { CELLS_FIRST_ID, CELLS_LAST_ID, ZSTR_CELLS, UNIT_CELLS, 2 },
  { GPS_ALT_FIRST_ID, GPS_ALT_LAST_ID, ZSTR_GPSALT, UNIT_METERS, 2 },
  { GPS_TIME_DATE_FIRST_ID, GPS_TIME_DATE_LAST_ID, ZSTR_GPSDATETIME, UNIT_DATETIME, 0 },
  { GPS_LONG_LATI_FIRST_ID, GPS_LONG_LATI_LAST_ID, ZSTR_GPS, UNIT_GPS, 0 },
  { 0, 0, NULL, UNIT_RAW, 0 } // sentinel
};

const SportSensor * getSportSensor(uint16_t id)
{
  const SportSensor * result = NULL;
  for (const SportSensor * sensor = sportSensors; sensor->firstId; sensor++) {
    if (id >= sensor->firstId && id <= sensor->lastId) {
      result = sensor;
      break;
    }
  }
  return result;
}

void setBaroAltitude(int32_t baroAltitude)
{
  // First received barometer altitude => Altitude offset
  if (!frskyData.hub.baroAltitudeOffset)
    frskyData.hub.baroAltitudeOffset = -baroAltitude;

  baroAltitude += frskyData.hub.baroAltitudeOffset;
  frskyData.hub.baroAltitude = baroAltitude;

  baroAltitude /= 100;
  if (baroAltitude > frskyData.hub.maxAltitude)
    frskyData.hub.maxAltitude = baroAltitude;
  if (baroAltitude < frskyData.hub.minAltitude)
    frskyData.hub.minAltitude = baroAltitude;
}

void processHubPacket(uint8_t id, uint16_t value)
{
  if (id > FRSKY_LAST_ID)
    return;

  if (id == GPS_LAT_BP_ID) {
    if (value)
      frskyData.hub.gpsFix = 1;
    else if (frskyData.hub.gpsFix > 0 && frskyData.hub.gpsLatitude_bp > 1)
      frskyData.hub.gpsFix = 0;
  }
  else if (id == GPS_LONG_BP_ID) {
    if (value)
      frskyData.hub.gpsFix = 1;
    else if (frskyData.hub.gpsFix > 0 && frskyData.hub.gpsLongitude_bp > 1)
      frskyData.hub.gpsFix = 0;
  }

  if (id == GPS_ALT_BP_ID || (id >= GPS_ALT_AP_ID && id <= GPS_LAT_NS_ID && id != BARO_ALT_BP_ID && id != BARO_ALT_AP_ID)) {
    // if we don't have a fix, we may discard the value
    if (frskyData.hub.gpsFix <= 0)
      return;
  }

  ((uint16_t*)&frskyData.hub)[id] = value;

  switch (id) {

    case RPM_ID:
      frskyData.hub.rpm *= (uint8_t)60/(g_model.frsky.blades+2);
      if (frskyData.hub.rpm > frskyData.hub.maxRpm)
        frskyData.hub.maxRpm = frskyData.hub.rpm;
      break;
      
    case TEMP1_ID:
      if (frskyData.hub.temperature1 > frskyData.hub.maxTemperature1)
        frskyData.hub.maxTemperature1 = frskyData.hub.temperature1;
      break;

    case TEMP2_ID:
      if (frskyData.hub.temperature2 > frskyData.hub.maxTemperature2)
        frskyData.hub.maxTemperature2 = frskyData.hub.temperature2;
      break;

    case CURRENT_ID:
      if ((int16_t)frskyData.hub.current > 0 && ((int16_t)frskyData.hub.current + g_model.frsky.fasOffset) > 0)
        frskyData.hub.current += g_model.frsky.fasOffset;
      else
        frskyData.hub.current = 0;
      if (frskyData.hub.current > frskyData.hub.maxCurrent)
        frskyData.hub.maxCurrent = frskyData.hub.current;
      break;

    case VOLTS_AP_ID:
#if defined(FAS_BSS)
      frskyData.hub.vfas = (frskyData.hub.volts_bp * 10 + frskyData.hub.volts_ap);
#else
      frskyData.hub.vfas = ((frskyData.hub.volts_bp * 100 + frskyData.hub.volts_ap * 10) * 21) / 110;
#endif
      if (!frskyData.hub.minVfas || frskyData.hub.vfas < frskyData.hub.minVfas)
        frskyData.hub.minVfas = frskyData.hub.vfas;
      break;

    case BARO_ALT_AP_ID:
      if (frskyData.hub.baroAltitude_ap > 9)
        frskyData.hub.varioHighPrecision = true;
      if (!frskyData.hub.varioHighPrecision)
        frskyData.hub.baroAltitude_ap *= 10;
      setBaroAltitude((int32_t)100 * frskyData.hub.baroAltitude_bp + (frskyData.hub.baroAltitude_bp >= 0 ? frskyData.hub.baroAltitude_ap : -frskyData.hub.baroAltitude_ap));
      break;

    case GPS_ALT_AP_ID:
    {
      frskyData.hub.gpsAltitude = (frskyData.hub.gpsAltitude_bp * 100) + frskyData.hub.gpsAltitude_ap;
      if (!frskyData.hub.gpsAltitudeOffset)
        frskyData.hub.gpsAltitudeOffset = -frskyData.hub.gpsAltitude;
      if (!frskyData.hub.baroAltitudeOffset) {
        int altitude = TELEMETRY_RELATIVE_GPS_ALT_BP;
        if (altitude > frskyData.hub.maxAltitude)
          frskyData.hub.maxAltitude = altitude;
        if (altitude < frskyData.hub.minAltitude)
          frskyData.hub.minAltitude = altitude;
      }
      if (!frskyData.hub.pilotLatitude && !frskyData.hub.pilotLongitude) {
        // First received GPS position => Pilot GPS position
        getGpsPilotPosition();
      }
      else if (frskyData.hub.gpsDistNeeded || g_menuStack[g_menuStackPtr] == menuTelemetryFrsky) {
        getGpsDistance();
      }
      break;
    }

    case GPS_SPEED_BP_ID:
      // Speed => Max speed
      if (frskyData.hub.gpsSpeed_bp > frskyData.hub.maxGpsSpeed)
        frskyData.hub.maxGpsSpeed = frskyData.hub.gpsSpeed_bp;
      break;

    case VOLTS_ID:
      frskyUpdateCells();
      break;

    case GPS_HOUR_MIN_ID:
      frskyData.hub.hour = ((uint8_t)(frskyData.hub.hour + g_eeGeneral.timezone + 24)) % 24;
      break;

    case ACCEL_X_ID:
    case ACCEL_Y_ID:
    case ACCEL_Z_ID:
      ((int16_t*)(&frskyData.hub))[id] /= 10;
      break;

  }
}

bool checkSportPacket(uint8_t *packet)
{
  short crc = 0;
  for (int i=1; i<FRSKY_SPORT_PACKET_SIZE; i++) {
    crc += packet[i]; //0-1FF
    crc += crc >> 8; //0-100
    crc &= 0x00ff;
    crc += crc >> 8; //0-0FF
    crc &= 0x00ff;
  }
  return (crc == 0x00ff);
}

#define SPORT_DATA_U8(packet)   (packet[4])
#define SPORT_DATA_S32(packet)  (*((int32_t *)(packet+4)))
#define SPORT_DATA_U32(packet)  (*((uint32_t *)(packet+4)))
#define HUB_DATA_U16(packet)    (*((uint16_t *)(packet+4)))

void frskySportProcessPacket(uint8_t *packet)
{
  uint8_t  dataId = packet[0];
  uint8_t  prim   = packet[1];
  uint16_t appId  = *((uint16_t *)(packet+2));

  if (!checkSportPacket(packet)) {
    return;
  }

  if (prim == DATA_FRAME)  {
    uint32_t data = SPORT_DATA_S32(packet);

    if (appId == RSSI_ID) {
      frskyStreaming = FRSKY_TIMEOUT10ms; // reset counter only if valid frsky packets are being detected
      data = SPORT_DATA_U8(packet);
      frskyData.rssi.set(data);
    }

    else if (appId == SWR_ID) {
#if defined(PCBTARANIS) && defined(REVPLUS)
      // SWR skipped (not reliable on Taranis+)
      return;
#else
      data = SPORT_DATA_U8(packet);
      frskyData.swr.set(data);
#endif
    }

    if (TELEMETRY_STREAMING()/* because when Rx is OFF it happens that some old A1/A2 values are sent from the XJT module*/) {
      if (appId == ADC1_ID || appId == ADC2_ID) {
        // A1/A2 of DxR receivers
        uint8_t idx = appId - ADC1_ID;
        frskyData.analog[idx].set(SPORT_DATA_U8(packet), g_model.frsky.channels[idx].type);
#if defined(VARIO)
        uint8_t varioSource = g_model.frsky.varioSource - VARIO_SOURCE_A1;
        if (varioSource == appId-ADC1_ID) {
          frskyData.hub.varioSpeed = applyChannelRatio(varioSource, frskyData.analog[varioSource].value);
        }
#endif
      }
      else if (appId == BATT_ID) {
        frskyData.analog[TELEM_ANA_A1].set(SPORT_DATA_U8(packet), UNIT_VOLTS);
      }
      else if ((appId >> 8) == 0) {
        // The old FrSky IDs
        uint8_t  id = (uint8_t)appId;
        uint16_t value = HUB_DATA_U16(packet);
        processHubPacket(id, value);
      }
      else {
        const SportSensor * sensor = getSportSensor(appId);
        TelemetryUnit unit = UNIT_RAW;
        uint8_t precision = 0;
        if (sensor) {
          unit = sensor->unit;
          precision = sensor->prec;
        }
        setTelemetryValue(TELEM_PROTO_FRSKY_SPORT, appId, dataId, data, unit, precision);
      }
    }
  }
}

void frskySportSetDefault(int index, uint16_t id, uint8_t instance)
{
  TelemetrySensor & telemetrySensor = g_model.telemetrySensors[index];

  telemetrySensor.id = id;
  telemetrySensor.instance = instance;

  const SportSensor * sensor = getSportSensor(id);
  if (sensor) {
    TelemetryUnit unit = sensor->unit;
    if (unit == UNIT_CELLS)
      unit = UNIT_VOLTS;
    telemetrySensor.init(sensor->name, unit, sensor->prec);
  }
  else {
    char label[4];
    label[0] = hex2zchar((id & 0xf000) >> 12);
    label[1] = hex2zchar((id & 0x0f00) >> 8);
    label[2] = hex2zchar((id & 0x00f0) >> 4);
    label[3] = hex2zchar((id & 0x000f) >> 0);
    telemetrySensor.init(label);
  }

  eeDirty(EE_MODEL);
}
