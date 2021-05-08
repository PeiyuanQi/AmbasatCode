/*******************************************************************************
* AmbaSat-1 TTN
* 20th May 2020
* Version 1.0
* Filename: main.h
*
* Copyright (c) 2020 AmbaSat Ltd
* https://ambasat.com
*
* licensed under Creative Commons Attribution-ShareAlike 3.0
* ******************************************************************************/

#include <avr/sleep.h>
#include <avr/wdt.h>
#include <lmic.h>
#include <hal/hal.h>
#include <SPI.h>
#include "LowPower.h"
#include "i2c.h"
#include <Arduino.h>
#include <LoraMessage.h>
#include <settings.h>
#include "Adafruit_SHT31.h"
#include <Wire.h>

#define addr 0x4A
#define LedPin 9
#define ADR_MODE 1

int sleepcycles = 130;
bool joined = false;
bool sleeping = false;

bool enableHeater = false;
uint8_t loopCnt = 0;

Adafruit_SHT31 sht31 = Adafruit_SHT31();
// -----------------------------------------------------------------------------
// Gyro/Magno/Accel structure
// -----------------------------------------------------------------------------
struct threeDData 
{
  	uint8_t x;
	uint8_t y;
	uint8_t z;
};

// -----------------------------------------------------------------------------
// TTN payload data structure - see https://www.thethingsnetwork.org/docs/devices/bytes.html
// -----------------------------------------------------------------------------
struct
{
  uint8_t sensorType;
	uint8_t field1; 
  float field2; 
	uint16_t voltage;
} packetData;

// Set the sensor type 
static const uint8_t sensorType = SENSOR_01_DHT30; // for test purposes

// TTN *****************************  IMPORTANT 
// 
// Set the following three values to match your unique AmbaSat-1 satellite   
// 
// The Network Session Key  F2 A5 E5 6B 0C 7E E3 94 9C 4D 41 5E 93 8C 33 97
static const PROGMEM u1_t NWKSKEY[16] = {0xF2,0xA5,0xE5,0x6B,0x0C,0x7E,0xE3,0x94,0x9C,0x4D,0x41,0x5E,0x93,0x8C,0x33,0x97}; //<< CHANGE

// LoRaWAN AppSKey, application session key 77 3A 01 85 EC FC 04 42 89 69 88 16 31 1D 63 7C
static const u1_t PROGMEM APPSKEY[16] = {0x77,0x3A,0x01,0x85,0xEC,0xFC,0x04,0x42,0x89,0x69,0x88,0x16,0x31,0x1D,0x63,0x7C}; //<< CHANGE

// LoRaWAN end-device address (DevAddr) 2601104B
static const u4_t DEVADDR = 0x2601104B ;  //<< CHANGE

/***********************************  IMPORTANT */

// These callbacks are only used in over-the-air activation, so they are
// left empty here (cannot be left out completely unless
// DISABLE_JOIN is set in config.h, otherwise the linker will complain).
void os_getArtEui (u1_t* buf) { }
void os_getDevEui (u1_t* buf) { }
void os_getDevKey (u1_t* buf) { }

static osjob_t sendjob;
static osjob_t initjob;

// Pin mapping
const lmic_pinmap lmic_pins = {
  .nss = 10,
  .rxtx = LMIC_UNUSED_PIN,
  .rst = 7,
  .dio = {2, A2, LMIC_UNUSED_PIN},
};

