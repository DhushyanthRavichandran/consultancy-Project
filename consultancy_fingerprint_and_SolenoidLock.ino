#include <Adafruit_Fingerprint.h>

#if defined(__AVR__) || defined(ESP8266)
// For UNO and others without hardware serial, we must use software serial...
// pin #2 is IN from sensor (GREEN wire)
// pin #3 is OUT from Arduino (WHITE wire)
// Set up the serial port to use SoftwareSerial
SoftwareSerial mySerial(2, 3);
#endif

SoftwareSerial mySerial2(12, 13);
int count = 0;

Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);

void setup()
{
  pinMode(10, OUTPUT);
  digitalWrite(10, LOW);
  Serial.begin(9600);
  while (!Serial);  // Wait for serial port to connect
  delay(100);
  mySerial2.begin(9600);
  delay(1000);
  Serial.println("\n\nAdafruit finger detect test");

  // Set the data rate for the sensor serial port
  finger.begin(57600);
  delay(5);
  if (finger.verifyPassword()) {
    Serial.println("Found fingerprint sensor!");
  } else {
    Serial.println("Did not find fingerprint sensor :(");
    while (1) { delay(1); }
  }

  Serial.println(F("Reading sensor parameters"));
  finger.getParameters();
  Serial.print(F("Status: 0x")); Serial.println(finger.status_reg, HEX);
  Serial.print(F("Sys ID: 0x")); Serial.println(finger.system_id, HEX);
  Serial.print(F("Capacity: ")); Serial.println(finger.capacity);
  Serial.print(F("Security level: ")); Serial.println(finger.security_level);
  Serial.print(F("Device address: ")); Serial.println(finger.device_addr, HEX);
  Serial.print(F("Packet len: ")); Serial.println(finger.packet_len);
  Serial.print(F("Baud rate: ")); Serial.println(finger.baud_rate);

  finger.getTemplateCount();

  if (finger.templateCount == 0) {
    Serial.print("Sensor doesn't contain any fingerprint data. Please run the 'enroll' example.");
  }
  else {
    Serial.println("Waiting for valid finger...");
    Serial.print("Sensor contains "); Serial.print(finger.templateCount); Serial.println(" templates");
  }
}
void sms(const String &msg) {
  mySerial2.println("AT+CMGF=1");    // Sets the GSM Module in Text Mode
  delay(1000);  // Delay of 1 second
  mySerial2.println("AT+CMGS=\"+919342532182\""); // Replace x with mobile number
  delay(1000);
  mySerial2.println(msg); // The SMS text you want to send
  delay(100);
  mySerial2.println((char)26); // ASCII code of CTRL+Z for saying the end of SMS to the module
  delay(1000);
  Serial.println("Message sent");

  // Debug
  while (mySerial2.available()) {
    Serial.write(mySerial2.read());
  }
  Serial.println("End of response");

  if (!mySerial2.available()) {
    Serial.println("No response from GSM module");
  }
}

void call() {
  Serial.println("Sending Message please wait….");
  mySerial2.println("AT"); // Handshake test, should return "OK" on success
  delay(1000);
  Serial.print("making call....");
  mySerial2.println("ATD+919342532182;"); // Change ZZ with the country code and xxxxxxxxxxx with the phone number to dial
  delay(20000); // Wait for 20 seconds...
  mySerial2.println("ATH"); // Hang up the call
  while (mySerial2.available()) {
    Serial.write(mySerial2.read());
  }
  Serial.println("End of response");

  if (!mySerial2.available()) {
    Serial.println("No response from GSM module");
  }
}

void loop() {
  if (count == 3) {
    call();
    count = 0;
  }
  // digitalWrite(10,LOW);
  getFingerprintID();
  delay(50); // Don't need to run this at full speed
}

uint8_t getFingerprintID() {
  uint8_t p = finger.getImage();
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image taken");
      break;
    case FINGERPRINT_NOFINGER:
      Serial.println("No finger detected");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      call();
      return p;
    case FINGERPRINT_IMAGEFAIL:
      Serial.println("Imaging error");
      return p;
    default:
      Serial.println("Unknown error");
      return p;
  }

  p = finger.image2Tz();
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image converted");
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println("Image too messy");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      call();
      return p;
    case FINGERPRINT_FEATUREFAIL:
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Could not find fingerprint features");
      return p;
    default:
      Serial.println("Unknown error");
      return p;
  }

  p = finger.fingerSearch();
  if (p == FINGERPRINT_OK) {
    Serial.println("Found a print match!");
    digitalWrite(10, HIGH);
    count = 0;
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
    call();
    return p;
  } else if (p == FINGERPRINT_NOTFOUND) {
    Serial.println("Did not find a match");
    count = count + 1;
    return p;
  } else {
    Serial.println("Unknown error");
    return p;
  }

  Serial.print("Found ID #"); Serial.print(finger.fingerID);
  Serial.print(" with confidence of "); Serial.println(finger.confidence);

  if (finger.fingerID == 1 || finger.fingerID == 2) {
    // call();
    sms("DOOR UNLOCKED !!  dhivakar accessed the door ");
    
  } else if (finger.fingerID == 3 || finger.fingerID == 4) {
    // call();
    sms("DOOR UNLOCKED !!  dhushyanth accessed the door ");
    
  }
  delay(5000);
  digitalWrite(10, LOW);
  return finger.fingerID;
}

int getFingerprintIDez() {
  uint8_t p = finger.getImage();
  if (p != FINGERPRINT_OK)  return -1;

  p = finger.image2Tz();
  if (p != FINGERPRINT_OK)  return -1;

  p = finger.fingerFastSearch();
  if (p != FINGERPRINT_OK)  return -1;

  Serial.print("Found ID #"); Serial.print(finger.fingerID);
  Serial.print(" with confidence of "); Serial.println(finger.confidence);
  count = 0;
  return finger.fingerID;
}
