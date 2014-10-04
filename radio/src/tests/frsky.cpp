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

#include "gtests.h"

#if defined(FRSKY) && !defined(FRSKY_SPORT)
extern void frskyDProcessPacket(uint8_t *packet);
TEST(FrSky, gpsNfuel)
{
  g_model.frsky.usrProto = 1;
  frskyData.hub.gpsFix = 1;

  uint8_t pkt1[] = { 0xfd, 0x07, 0x00, 0x5e, 0x14, 0x2c, 0x00, 0x5e, 0x1c, 0x03 };
  uint8_t pkt2[] = { 0xfd, 0x07, 0x00, 0x00, 0x5e, 0x13, 0x38, 0x0c, 0x5e, 0x1b };
  uint8_t pkt3[] = { 0xfd, 0x07, 0x00, 0xc9, 0x06, 0x5e, 0x23, 0x4e, 0x00, 0x5e };
  uint8_t pkt4[] = { 0xfd, 0x07, 0x00, 0x12, 0xef, 0x2e, 0x5e, 0x1a, 0x98, 0x26 };
  uint8_t pkt5[] = { 0xfd, 0x07, 0x00, 0x5e, 0x22, 0x45, 0x00, 0x5e, 0x11, 0x02 };
  uint8_t pkt6[] = { 0xfd, 0x07, 0x00, 0x00, 0x5e, 0x19, 0x93, 0x00, 0x5e, 0x04 };
  uint8_t pkt7[] = { 0xfd, 0x03, 0x00, 0x64, 0x00, 0x5e };
  frskyDProcessPacket(pkt1);
  frskyDProcessPacket(pkt2);
  frskyDProcessPacket(pkt3);
  frskyDProcessPacket(pkt4);
  frskyDProcessPacket(pkt5);
  frskyDProcessPacket(pkt6);
  frskyDProcessPacket(pkt7);
  EXPECT_EQ(frskyData.hub.gpsCourse_bp, 44);
  EXPECT_EQ(frskyData.hub.gpsCourse_ap, 03);
  EXPECT_EQ(frskyData.hub.gpsLongitude_bp / 100, 120);
  EXPECT_EQ(frskyData.hub.gpsLongitude_bp % 100, 15);
  EXPECT_EQ(frskyData.hub.gpsLongitude_ap, 0x2698);
  EXPECT_EQ(frskyData.hub.gpsLatitudeNS, 'N');
  EXPECT_EQ(frskyData.hub.gpsLongitudeEW, 'E');
  EXPECT_EQ(frskyData.hub.fuelLevel, 100);
}

TEST(FrSky, dateNtime)
{
  uint8_t pkt1[] = { 0xfd, 0x07, 0x00, 0x5e, 0x15, 0x0f, 0x07, 0x5e, 0x16, 0x0b };
  uint8_t pkt2[] = { 0xfd, 0x07, 0x00, 0x00, 0x5e, 0x17, 0x06, 0x12, 0x5e, 0x18 };
  uint8_t pkt3[] = { 0xfd, 0x03, 0x00, 0x32, 0x00, 0x5e };
  frskyDProcessPacket(pkt1);
  frskyDProcessPacket(pkt2);
  frskyDProcessPacket(pkt3);
  EXPECT_EQ(frskyData.hub.day, 15);
  EXPECT_EQ(frskyData.hub.month, 07);
  EXPECT_EQ(frskyData.hub.year, 11);
  EXPECT_EQ(frskyData.hub.hour, 06);
  EXPECT_EQ(frskyData.hub.min, 18);
  EXPECT_EQ(frskyData.hub.sec, 50);
}
#endif

#if defined(FRSKY) && defined(CPUARM)
TEST(FrSky, FrskyValueWithMinAveraging)
{
  /*
    The following expected[] array is filled
    with values that correspond to 4 elements 
    long averaging buffer.
    If length of averaging buffer is changed, this
    values must be adjusted
  */
  uint8_t expected[] = { 10, 12, 17, 25, 35, 45, 55, 65, 75, 85, 92, 97, 100, 100, 100, 100, 100};
  int testPos = 0;
  //test of averaging
  FrskyValueWithMin testVal;
  testVal.value = 0;  
  testVal.set(10);
  EXPECT_EQ(RAW_FRSKY_MINMAX(testVal), 10);
  EXPECT_EQ(testVal.value, expected[testPos++]);
  for(int n=2; n<10; ++n) {
    testVal.set(n*10);
    EXPECT_EQ(RAW_FRSKY_MINMAX(testVal), n*10);
    EXPECT_EQ(testVal.value, expected[testPos++]);
  }
  for(int n=2; n<10; ++n) {
    testVal.set(100);
    EXPECT_EQ(RAW_FRSKY_MINMAX(testVal), 100);
    EXPECT_EQ(testVal.value, expected[testPos++]);
  }
}
#endif



#if defined(FRSKY_SPORT)
extern bool checkSportPacket(uint8_t *packet);
TEST(FrSkySPORT, checkCrc)
{
  // uint8_t pkt[] = { 0x7E, 0x98, 0x10, 0x10, 0x00, 0x7D, 0x5E, 0x02, 0x00, 0x00, 0x5F };
  uint8_t pkt[] = { 0x7E, 0x98, 0x10, 0x10, 0x00, 0x7E, 0x02, 0x00, 0x00, 0x5F };
  EXPECT_EQ(checkSportPacket(pkt+1), true);
}

extern void frskySportProcessPacket(uint8_t *packet);
extern bool checkSportPacket(uint8_t *packet);
extern void frskyCalculateCellStats(void);
extern void displayVoltagesScreen();

void setSportPacketCrc(uint8_t * packet)
{
  short crc = 0;
  for (int i=1; i<FRSKY_SPORT_PACKET_SIZE-1; i++) {
    crc += packet[i]; //0-1FF
    crc += crc >> 8; //0-100
    crc &= 0x00ff;
    crc += crc >> 8; //0-0FF
    crc &= 0x00ff;
  }
  packet[FRSKY_SPORT_PACKET_SIZE-1] = 0xFF - (crc & 0x00ff);
  //TRACE("crc set: %x", packet[FRSKY_SPORT_PACKET_SIZE-1]);
}

void generateSportCellPacket(uint8_t * packet, uint8_t cells, uint8_t battnumber, uint16_t cell1, uint16_t cell2)
{
  if (battnumber < 6) {
    packet[0] = 0xA1; //DATA_ID_FLVSS;
  }
  else {
    packet[0] = 0xA1+1; //DATA_ID_FLVSS+1;
    battnumber -= 6;
  }
  packet[1] = 0x10; //DATA_FRAME
  *((uint16_t *)(packet+2)) = 0x0300; //CELLS_FIRST_ID
  uint32_t data = 0;
  data += (cells << 4) + battnumber;
  data += ((cell1 * 5) & 0xFFF) << 8;
  data += ((cell2 * 5) & 0xFFF) << 20;
  *((int32_t *)(packet+4)) = data;
  setSportPacketCrc(packet);
}

#define _V(volts)   (volts/TELEMETRY_CELL_VOLTAGE_MUTLIPLIER)

TEST(FrSkySPORT, DISABLED_frskySetCellVoltage)
{
  uint8_t packet[FRSKY_SPORT_PACKET_SIZE];

  MODEL_RESET();
  TELEMETRY_RESET();

  // test that simulates 3 cell battery
  generateSportCellPacket(packet, 3, 0, _V(410), _V(420)); frskySportProcessPacket(packet);
  EXPECT_EQ(checkSportPacket(packet), true) << "Bad CRC generation in setSportPacketCrc()";
  generateSportCellPacket(packet, 3, 2, _V(430), _V(  0)); frskySportProcessPacket(packet);

  generateSportCellPacket(packet, 3, 0, _V(405), _V(300)); frskySportProcessPacket(packet);
  generateSportCellPacket(packet, 3, 2, _V(430), _V(  0)); frskySportProcessPacket(packet);
  
  EXPECT_EQ(frskyData.hub.cellsCount,         3);
  EXPECT_EQ(frskyData.hub.cellVolts[0], _V(405));
  EXPECT_EQ(frskyData.hub.cellVolts[1], _V(300));
  EXPECT_EQ(frskyData.hub.cellVolts[2], _V(430));
  EXPECT_EQ(frskyData.hub.cellVolts[4], _V(  0));
  EXPECT_EQ(frskyData.hub.minCellVolts, _V(300));   //current minimum cell voltage
  EXPECT_EQ(frskyData.hub.minCell,      _V(300));   //all time minimum cell voltage
  EXPECT_EQ(frskyData.hub.minCells,     _V(113));   //all time cells sum minimum
  EXPECT_EQ(frskyData.hub.cellsSum,     _V(113));   //current cells sum

  generateSportCellPacket(packet, 3, 0, _V(405), _V(250)); frskySportProcessPacket(packet);
  generateSportCellPacket(packet, 3, 2, _V(430), _V(  0)); frskySportProcessPacket(packet);
  
  generateSportCellPacket(packet, 3, 0, _V(410), _V(420)); frskySportProcessPacket(packet);
  generateSportCellPacket(packet, 3, 2, _V(430), _V(  0)); frskySportProcessPacket(packet);
  
  EXPECT_EQ(frskyData.hub.cellsCount,         3);
  EXPECT_EQ(frskyData.hub.cellVolts[0], _V(410));
  EXPECT_EQ(frskyData.hub.cellVolts[1], _V(420));
  EXPECT_EQ(frskyData.hub.cellVolts[2], _V(430));
  EXPECT_EQ(frskyData.hub.cellVolts[4], _V(  0));
  EXPECT_EQ(frskyData.hub.minCellVolts, _V(410));   //current minimum cell voltage
  EXPECT_EQ(frskyData.hub.minCell,      _V(250));   //all time minimum cell voltage
  EXPECT_EQ(frskyData.hub.minCells,     _V(108));   //all time cells sum minimum
  EXPECT_EQ(frskyData.hub.cellsSum,     _V(126));   //current cells sum

  //add another two cells - 5 cell battery
  generateSportCellPacket(packet, 5, 0, _V(418), _V(408)); frskySportProcessPacket(packet);
  generateSportCellPacket(packet, 5, 2, _V(415), _V(420)); frskySportProcessPacket(packet);
  generateSportCellPacket(packet, 5, 4, _V(410), _V(  0)); frskySportProcessPacket(packet);

  EXPECT_EQ(frskyData.hub.cellsCount,         5);
  EXPECT_EQ(frskyData.hub.cellVolts[0], _V(418));
  EXPECT_EQ(frskyData.hub.cellVolts[1], _V(408));
  EXPECT_EQ(frskyData.hub.cellVolts[2], _V(415));
  EXPECT_EQ(frskyData.hub.cellVolts[3], _V(420));
  EXPECT_EQ(frskyData.hub.cellVolts[4], _V(410));
  EXPECT_EQ(frskyData.hub.cellVolts[5], _V(  0));
  EXPECT_EQ(frskyData.hub.minCellVolts, _V(408));   //current minimum cell voltage
  EXPECT_EQ(frskyData.hub.minCell,      _V(408));   //all time minimum cell voltage
  EXPECT_EQ(frskyData.hub.minCells,     _V(207));   //all time cells sum minimum
  EXPECT_EQ(frskyData.hub.cellsSum,     _V(207));   //current cells sum

  //simulate very low voltage for cell 3
  generateSportCellPacket(packet, 5, 0, _V(418), _V(408)); frskySportProcessPacket(packet);
  generateSportCellPacket(packet, 5, 2, _V(100), _V(420)); frskySportProcessPacket(packet);
  generateSportCellPacket(packet, 5, 4, _V(410), _V(  0)); frskySportProcessPacket(packet);

  EXPECT_EQ(frskyData.hub.cellsCount,         5);
  EXPECT_EQ(frskyData.hub.cellVolts[0], _V(418));
  EXPECT_EQ(frskyData.hub.cellVolts[1], _V(408));
  EXPECT_EQ(frskyData.hub.cellVolts[2], _V(100));
  EXPECT_EQ(frskyData.hub.cellVolts[3], _V(420));
  EXPECT_EQ(frskyData.hub.cellVolts[4], _V(410));
  EXPECT_EQ(frskyData.hub.cellVolts[5], _V(  0));
  EXPECT_EQ(frskyData.hub.minCellVolts, _V(100));   //current minimum cell voltage
  EXPECT_EQ(frskyData.hub.minCell,      _V(100));   //all time minimum cell voltage
  EXPECT_EQ(frskyData.hub.minCells,     _V(175));   //all time cells sum minimum
  EXPECT_EQ(frskyData.hub.cellsSum,     _V(175));   //current cells sum

  //back to normal (but with reversed order of packets)
  generateSportCellPacket(packet, 5, 4, _V(410), _V(  0)); frskySportProcessPacket(packet);
  generateSportCellPacket(packet, 5, 0, _V(418), _V(408)); frskySportProcessPacket(packet);
  generateSportCellPacket(packet, 5, 2, _V(412), _V(420)); frskySportProcessPacket(packet);

  EXPECT_EQ(frskyData.hub.cellsCount,         5);
  EXPECT_EQ(frskyData.hub.cellVolts[0], _V(418));
  EXPECT_EQ(frskyData.hub.cellVolts[1], _V(408));
  EXPECT_EQ(frskyData.hub.cellVolts[2], _V(412));
  EXPECT_EQ(frskyData.hub.cellVolts[3], _V(420));
  EXPECT_EQ(frskyData.hub.cellVolts[4], _V(410));
  EXPECT_EQ(frskyData.hub.cellVolts[5], _V(  0));
  EXPECT_EQ(frskyData.hub.minCellVolts, _V(408));   //current minimum cell voltage
  EXPECT_EQ(frskyData.hub.minCell,      _V(100));   //all time minimum cell voltage
  EXPECT_EQ(frskyData.hub.minCells,     _V(175));   //all time cells sum minimum
  EXPECT_EQ(frskyData.hub.cellsSum,     _V(206));   //current cells sum

  //display test
  lcd_clear();
  g_model.frsky.voltsSource = FRSKY_VOLTS_SOURCE_A1;
  displayVoltagesScreen();
  EXPECT_TRUE(checkScreenshot("one_sensor_votages_screen"));
}

TEST(FrSkySPORT, DISABLED_StrangeCellsBug)
{
  TELEMETRY_RESET();
  uint8_t pkt[] = { 0x7E, 0x48, 0x10, 0x00, 0x03, 0x30, 0x15, 0x50, 0x81, 0xD5 };
  EXPECT_EQ(checkSportPacket(pkt+1), true);
  frskySportProcessPacket(pkt+1);
  EXPECT_EQ(frskyData.hub.cellsCount,         3);
  EXPECT_EQ(frskyData.hub.cellVolts[0], _V(000)); // now we ignore such low values 
  EXPECT_EQ(frskyData.hub.cellVolts[1], _V(413));
}

TEST(FrSkySPORT, DISABLED_frskySetCellVoltageTwoSensors)
{
  uint8_t packet[FRSKY_SPORT_PACKET_SIZE];

  MODEL_RESET();
  TELEMETRY_RESET();

  //sensor 1: 3 cell battery
  generateSportCellPacket(packet, 3, 0, _V(418), _V(416)); frskySportProcessPacket(packet);
  generateSportCellPacket(packet, 3, 2, _V(415), _V(  0)); frskySportProcessPacket(packet);

  EXPECT_EQ(frskyData.hub.cellsCount,         3);
  EXPECT_EQ(frskyData.hub.cellVolts[0], _V(418));
  EXPECT_EQ(frskyData.hub.cellVolts[1], _V(416));
  EXPECT_EQ(frskyData.hub.cellVolts[2], _V(415));
  EXPECT_EQ(frskyData.hub.cellVolts[3], _V(  0));
  EXPECT_EQ(frskyData.hub.minCellVolts, _V(415));   //current minimum cell voltage
  EXPECT_EQ(frskyData.hub.minCell,      _V(415));   //all time minimum cell voltage
  EXPECT_EQ(frskyData.hub.minCells,     _V(124));   //all time cells sum minimum
  EXPECT_EQ(frskyData.hub.cellsSum,     _V(124));   //current cells sum

  //sensor 2: 4 cell battery
  generateSportCellPacket(packet, 4, 6, _V(410), _V(420)); frskySportProcessPacket(packet);
  generateSportCellPacket(packet, 4, 8, _V(400), _V(405)); frskySportProcessPacket(packet);

  //we need to send all cells from first battery before a new calculation will be made
  generateSportCellPacket(packet, 3, 0, _V(418), _V(416)); frskySportProcessPacket(packet);
  generateSportCellPacket(packet, 3, 2, _V(415), _V(  0)); frskySportProcessPacket(packet);
  
  EXPECT_EQ(frskyData.hub.cellsCount,        7);
  EXPECT_EQ(frskyData.hub.cellVolts[0], _V(418));
  EXPECT_EQ(frskyData.hub.cellVolts[1], _V(416));
  EXPECT_EQ(frskyData.hub.cellVolts[2], _V(415));
  EXPECT_EQ(frskyData.hub.cellVolts[3], _V(410));
  EXPECT_EQ(frskyData.hub.cellVolts[4], _V(420));
  EXPECT_EQ(frskyData.hub.cellVolts[5], _V(400));
  EXPECT_EQ(frskyData.hub.cellVolts[6], _V(405));
  EXPECT_EQ(frskyData.hub.cellVolts[7], _V(  0));
  EXPECT_EQ(frskyData.hub.cellVolts[8], _V(  0));
  EXPECT_EQ(frskyData.hub.minCellVolts, _V(400));   //current minimum cell voltage
  EXPECT_EQ(frskyData.hub.minCell,      _V(400));   //all time minimum cell voltage
  EXPECT_EQ(frskyData.hub.minCells,     _V(288));   //all time cells sum minimum
  EXPECT_EQ(frskyData.hub.cellsSum,     _V(288));   //current cells sum

  //now change some voltages
  generateSportCellPacket(packet, 3, 2, _V(415), _V(  0)); frskySportProcessPacket(packet);
  generateSportCellPacket(packet, 4, 8, _V(390), _V(370)); frskySportProcessPacket(packet);
  generateSportCellPacket(packet, 3, 0, _V(420), _V(410)); frskySportProcessPacket(packet);
  generateSportCellPacket(packet, 4, 6, _V(410), _V(420)); frskySportProcessPacket(packet);

  EXPECT_EQ(frskyData.hub.cellsCount,        7);
  EXPECT_EQ(frskyData.hub.cellVolts[0], _V(420));
  EXPECT_EQ(frskyData.hub.cellVolts[1], _V(410));
  EXPECT_EQ(frskyData.hub.cellVolts[2], _V(415));
  EXPECT_EQ(frskyData.hub.cellVolts[3], _V(410));
  EXPECT_EQ(frskyData.hub.cellVolts[4], _V(420));
  EXPECT_EQ(frskyData.hub.cellVolts[5], _V(390));
  EXPECT_EQ(frskyData.hub.cellVolts[6], _V(370));
  EXPECT_EQ(frskyData.hub.cellVolts[7],_V(  0));
  EXPECT_EQ(frskyData.hub.cellVolts[8],_V(  0));
  EXPECT_EQ(frskyData.hub.minCellVolts, _V(370));   //current minimum cell voltage
  EXPECT_EQ(frskyData.hub.minCell,      _V(370));   //all time minimum cell voltage
  EXPECT_EQ(frskyData.hub.minCells,     _V(283));   //all time cells sum minimum
  EXPECT_EQ(frskyData.hub.cellsSum,     _V(283));   //current cells sum

  //display test
  lcd_clear();
  g_model.frsky.voltsSource = FRSKY_VOLTS_SOURCE_A1;
  displayVoltagesScreen();
  EXPECT_TRUE(checkScreenshot("two_sensor_votages_screen"));
}

void generateSportFasVoltagePacket(uint8_t * packet, uint32_t voltage)
{
  packet[0] = 0x22; //DATA_ID_FAS
  packet[1] = 0x10; //DATA_FRAME
  *((uint16_t *)(packet+2)) = 0x0210; //VFAS_FIRST_ID
  *((int32_t *)(packet+4)) = voltage;  // unit 10mV
  setSportPacketCrc(packet);
}

TEST(FrSkySPORT, DISABLED_frskyVfas)
{
  uint8_t packet[FRSKY_SPORT_PACKET_SIZE];

  //telemetryReset();
  TELEMETRY_RESET();

  // tests for Vfas
  generateSportFasVoltagePacket(packet, 5000); frskySportProcessPacket(packet);
  EXPECT_EQ(frskyData.hub.vfas,    500);
  EXPECT_EQ(frskyData.hub.minVfas, 500);

  generateSportFasVoltagePacket(packet, 6524); frskySportProcessPacket(packet);
  EXPECT_EQ(frskyData.hub.vfas,    652);
  EXPECT_EQ(frskyData.hub.minVfas, 500);

  generateSportFasVoltagePacket(packet, 1248); frskySportProcessPacket(packet);
  EXPECT_EQ(frskyData.hub.vfas,    124);
  EXPECT_EQ(frskyData.hub.minVfas, 124);

  generateSportFasVoltagePacket(packet, 2248); frskySportProcessPacket(packet);
  EXPECT_EQ(frskyData.hub.vfas,    224);
  EXPECT_EQ(frskyData.hub.minVfas, 124);
}

void generateSportFasCurrentPacket(uint8_t * packet, uint32_t current)
{
  packet[0] = 0x22; //DATA_ID_FAS
  packet[1] = 0x10; //DATA_FRAME
  *((uint16_t *)(packet+2)) = 0x0200; //CURR_FIRST_ID
  *((int32_t *)(packet+4)) = current;
  setSportPacketCrc(packet);
}

TEST(FrSkySPORT, DISABLED_frskyCurrent)
{
  uint8_t packet[FRSKY_SPORT_PACKET_SIZE];

  TELEMETRY_RESET();
  g_model.frsky.fasOffset = -5;  /* unit: 1/10 amps */

  // tests for Curr
  generateSportFasCurrentPacket(packet, 0); frskySportProcessPacket(packet);
  EXPECT_EQ(frskyData.hub.current,      0);
  EXPECT_EQ(frskyData.hub.maxCurrent,   0);

  // measured current less then offset - value should be zero
  generateSportFasCurrentPacket(packet, 4); frskySportProcessPacket(packet);
  EXPECT_EQ(frskyData.hub.current,      0);
  EXPECT_EQ(frskyData.hub.maxCurrent,   0);

  generateSportFasCurrentPacket(packet, 10); frskySportProcessPacket(packet);
  EXPECT_EQ(frskyData.hub.current,      5);
  EXPECT_EQ(frskyData.hub.maxCurrent,   5);

  generateSportFasCurrentPacket(packet, 500); frskySportProcessPacket(packet);
  EXPECT_EQ(frskyData.hub.current,      495);
  EXPECT_EQ(frskyData.hub.maxCurrent,   495);

  generateSportFasCurrentPacket(packet, 200); frskySportProcessPacket(packet);
  EXPECT_EQ(frskyData.hub.current,      195);
  EXPECT_EQ(frskyData.hub.maxCurrent,   495);

  // test with positive offset
  TELEMETRY_RESET();
  g_model.frsky.fasOffset = 5;  /* unit: 1/10 amps */

  generateSportFasCurrentPacket(packet, 0); frskySportProcessPacket(packet);
  EXPECT_EQ(frskyData.hub.current,      5);
  EXPECT_EQ(frskyData.hub.maxCurrent,   5);

  generateSportFasCurrentPacket(packet, 500); frskySportProcessPacket(packet);
  EXPECT_EQ(frskyData.hub.current,      505);
  EXPECT_EQ(frskyData.hub.maxCurrent,   505);

  generateSportFasCurrentPacket(packet, 200); frskySportProcessPacket(packet);
  EXPECT_EQ(frskyData.hub.current,      205);
  EXPECT_EQ(frskyData.hub.maxCurrent,   505);
}
#endif  //#if defined(FRSKY_SPORT)


