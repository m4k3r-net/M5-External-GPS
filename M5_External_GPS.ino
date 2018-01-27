#include <M5Stack.h>
#include "Free_Fonts.h"
#include <TinyGPS.h>

/* This sample code demonstrates the normal use of a TinyGPS object.
   It requires the use of SoftwareSerial, and assumes that you have a
   4800-baud serial GPS device hooked up on pins 4 (rx) and 3 (tx).
*/

TinyGPS gps;
HardwareSerial Serial2 = HardwareSerial(2);
#define ss Serial2
static void smartdelay(unsigned long ms);
static void print_float(float val, float invalid, int len, int prec);
static void print_int(unsigned long val, unsigned long invalid, int len);
static void print_date(TinyGPS &gps);
static void print_str(const char *str, int len);

void drawHeader() {
  M5.Lcd.setTextColor(TFT_BLACK, TFT_WHITE);
  M5.Lcd.fillScreen(TFT_WHITE);
  M5.Lcd.setFreeFont(FSSB12);
  // Select the font: FreeMono18pt7b – see Free_Fonts.h
  M5.Lcd.drawString(F("GPS Test"), 105, 2, GFXFF);
  M5.Lcd.setFreeFont(FSSB9);
  uint8_t pos = 33;
  M5.Lcd.drawString(F("Sats:"), 0, pos, GFXFF);
  M5.Lcd.drawString(F("hdop:"), 160, pos, GFXFF);
  pos += 22;
  M5.Lcd.drawString(F("flat:"), 0, pos, GFXFF);
  M5.Lcd.drawString(F("flon:"), 160, pos, GFXFF);
  pos += 22;
  M5.Lcd.drawString(F("Age:"), 0, pos, GFXFF);
  M5.Lcd.drawString(F("Date:"), 160, pos, GFXFF);
  pos += 22;
  M5.Lcd.drawString(F("Time:"), 0, pos, GFXFF);
  M5.Lcd.drawString(F("Alt:"), 160, pos, GFXFF);
  M5.Lcd.setFreeFont(FSS9);
}

void setup() {
  Serial.begin(115200);
  ss.begin(9600);
  delay(1000);
  M5.begin();
  drawHeader();
  Serial.print("Testing TinyGPS library v. "); Serial.println(TinyGPS::library_version());
  Serial.println();
  Serial.println("Sats HDOP Latitude  Longitude  Fix  Date       Time     Date Alt    Course Speed Card  Distance Course Card  Chars Sentences Checksum");
  Serial.println("          (deg)     (deg)      Age                      Age  (m)    --- from GPS ----  ---- to London  ----  RX    RX        Fail");
  Serial.println("-------------------------------------------------------------------------------------------------------------------------------------");
}

uint8_t iterations = 0;
unsigned short sats;
unsigned long hdop;
float flat, flon, alt;
unsigned long age, chars = 0;
unsigned short sentences = 0, failed = 0;

void loop() {
  static const double LONDON_LAT = 51.508131, LONDON_LON = -0.128002;
  String s;
  sats = gps.satellites();
  hdop = gps.hdop();
  print_int(sats, TinyGPS::GPS_INVALID_SATELLITES, 5);
  print_int(hdop, TinyGPS::GPS_INVALID_HDOP, 5);
  uint8_t pos = 33;
  if (sats == TinyGPS::GPS_INVALID_SATELLITES) s = "****";
  else s = String(sats);
  M5.Lcd.drawString(s, 60, pos, GFXFF);
  if (hdop == TinyGPS::GPS_INVALID_HDOP) s = "****";
  else s = String(hdop);
  M5.Lcd.drawString(s, 240, pos, GFXFF);
  pos += 22;

  gps.f_get_position(&flat, &flon, &age);
  print_float(flat, TinyGPS::GPS_INVALID_F_ANGLE, 10, 6);
  print_float(flon, TinyGPS::GPS_INVALID_F_ANGLE, 11, 6);
  if (flat == TinyGPS::GPS_INVALID_F_ANGLE) s = "****";
  else s = String(flat);
  M5.Lcd.drawString(s, 60, pos, GFXFF);
  if (flon == TinyGPS::GPS_INVALID_F_ANGLE) s = "****";
  else s = String(flon);
  M5.Lcd.drawString(s, 240, pos, GFXFF);
  pos += 22;
  print_int(age, TinyGPS::GPS_INVALID_AGE, 5);
  if (flon == TinyGPS::GPS_INVALID_AGE) s = "****";
  else s = String(age);
  M5.Lcd.drawString(s, 60, pos, GFXFF);
  print_date(gps, pos);
  pos += 22;
  alt = gps.f_altitude();
  print_float(alt, TinyGPS::GPS_INVALID_F_ALTITUDE, 7, 2);
  if (alt == TinyGPS::GPS_INVALID_F_ALTITUDE) s = "****";
  else s = String(alt);
  print_float(gps.f_course(), TinyGPS::GPS_INVALID_F_ANGLE, 7, 2);
  print_float(gps.f_speed_kmph(), TinyGPS::GPS_INVALID_F_SPEED, 6, 2);
  print_str(gps.f_course() == TinyGPS::GPS_INVALID_F_ANGLE ? "*** " : TinyGPS::cardinal(gps.f_course()), 6);
  print_int(flat == TinyGPS::GPS_INVALID_F_ANGLE ? 0xFFFFFFFF : (unsigned long)TinyGPS::distance_between(flat, flon, LONDON_LAT, LONDON_LON) / 1000, 0xFFFFFFFF, 9);
  print_float(flat == TinyGPS::GPS_INVALID_F_ANGLE ? TinyGPS::GPS_INVALID_F_ANGLE : TinyGPS::course_to(flat, flon, LONDON_LAT, LONDON_LON), TinyGPS::GPS_INVALID_F_ANGLE, 7, 2);
  print_str(flat == TinyGPS::GPS_INVALID_F_ANGLE ? "*** " : TinyGPS::cardinal(TinyGPS::course_to(flat, flon, LONDON_LAT, LONDON_LON)), 6);
  gps.stats(&chars, &sentences, &failed);
  print_int(chars, 0xFFFFFFFF, 6);
  print_int(sentences, 0xFFFFFFFF, 10);
  print_int(failed, 0xFFFFFFFF, 9);
  Serial.println();
  smartdelay(1000);
  iterations++;
  if (iterations == 20) {
    // Reprint header every 20 times, to make reading easier
    Serial.println("Sats HDOP Latitude  Longitude  Fix  Date       Time     Date Alt    Course Speed Card  Distance Course Card  Chars Sentences Checksum");
    Serial.println("          (deg)     (deg)      Age                      Age  (m)    --- from GPS ----  ---- to London  ----  RX    RX        Fail");
    Serial.println("-------------------------------------------------------------------------------------------------------------------------------------");
    iterations = 0;
  }
}

static void smartdelay(unsigned long ms) {
  unsigned long start = millis();
  do {
    while (ss.available())
      gps.encode(ss.read());
  } while (millis() - start < ms);
}

static void print_float(float val, float invalid, int len, int prec) {
  if (val == invalid) {
    while (len-- > 1)
      Serial.print('*');
    Serial.print(' ');
  } else {
    Serial.print(val, prec);
    int vi = abs((int)val);
    int flen = prec + (val < 0.0 ? 2 : 1); // . and -
    flen += vi >= 1000 ? 4 : vi >= 100 ? 3 : vi >= 10 ? 2 : 1;
    for (int i = flen; i < len; ++i)
      Serial.print(' ');
  }
  smartdelay(0);
}

static void print_int(unsigned long val, unsigned long invalid, int len) {
  char sz[32];
  if (val == invalid)
    strcpy(sz, "*******");
  else
    sprintf(sz, "%ld", val);
  sz[len] = 0;
  for (int i = strlen(sz); i < len; ++i)
    sz[i] = ' ';
  if (len > 0)
    sz[len - 1] = ' ';
  Serial.print(sz);
  smartdelay(0);
}

static void print_date(TinyGPS &gps, uint8_t pos) {
  int year;
  byte month, day, hour, minute, second, hundredths;
  unsigned long age;
  gps.crack_datetime(&year, &month, &day, &hour, &minute, &second, &hundredths, &age);
  if (age == TinyGPS::GPS_INVALID_AGE) {
    Serial.print("********** ******** ");
    M5.Lcd.drawString(F("****"), 240, pos, GFXFF);
    pos += 22;
    M5.Lcd.drawString(F("****"), 60, pos, GFXFF);
  } else {
    char sz[32];
    sprintf(sz, "%02d/%02d/%02d %02d:%02d:%02d ", month, day, year, hour, minute, second);
    Serial.print(sz);
    M5.Lcd.drawNumber(year, 240, pos, GFXFF);
    M5.Lcd.drawString("/", 260, pos, GFXFF);
    M5.Lcd.drawNumber(month, 270, pos, GFXFF);
    M5.Lcd.drawString("/", 290, pos, GFXFF);
    M5.Lcd.drawNumber(day, 400, pos, GFXFF);
    pos += 22;
    M5.Lcd.drawNumber(hour, 60, pos, GFXFF);
    M5.Lcd.drawString(":", 100, pos, GFXFF);
    M5.Lcd.drawNumber(minute, 110, pos, GFXFF);
    M5.Lcd.drawString(":", 130, pos, GFXFF);
    M5.Lcd.drawNumber(second, 140, pos, GFXFF);
  }
  print_int(age, TinyGPS::GPS_INVALID_AGE, 5);
  smartdelay(0);
}

static void print_str(const char *str, int len) {
  int slen = strlen(str);
  for (int i = 0; i < len; ++i)
    Serial.print(i < slen ? str[i] : ' ');
  smartdelay(0);
}
