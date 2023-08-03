
#define TINY_GSM_MODEM_SIM7600
#define SerialMon Serial
#define SerialAT Serial1
#define TINY_GSM_DEBUG SerialMon


#include <TinyGsmClient.h>
#ifdef DUMP_AT_COMMANDS
#include <StreamDebugger.h>
StreamDebugger debugger(SerialAT, SerialMon);
TinyGsm modem(debugger);
#else
TinyGsm modem(SerialAT);
#endif

#define PIN_TX                  17
#define PIN_RX                  16
#define UART_BAUD               115200
#define PWR_PIN                 27
#define LED_PIN                 12
#define SLEEP_PIN               23
#define MODEM_DTR               22

bool reply = false;

void modem_on() {
    // Set-up modem  power pin
    Serial.println("\nStarting Up Modem...");
    pinMode(PWR_PIN, OUTPUT);
    digitalWrite(PWR_PIN, HIGH);
    delay(300);
    //digitalWrite(PWR_PIN, LOW);
    delay(300);


    pinMode(SLEEP_PIN, OUTPUT); //sleep pin in output
    digitalWrite(SLEEP_PIN, HIGH); //power on module
    delay(10000); // 10 seconds to reset the module

    int i = 10;
    Serial.println("\nTesting Modem Response...\n");
    Serial.println("****");
    while (i) {
        SerialAT.println("AT");
        delay(500);
        if (SerialAT.available()) {
            String r = SerialAT.readString();
            Serial.println(r);
            if ( r.indexOf("OK") >= 0 ) {
                reply = true;
                break;;
            }
        }
        delay(500);
        i--;
    }
    Serial.println("****\n");
}


void enviarAT(String AT, int Delay) {

  Serial.println("\nSending an " + AT + " command...\n");
  SerialAT.println(AT);

  delay(Delay*1000);

  while (SerialAT.available() || Serial.available()) {
    if (SerialAT.available()) {
      Serial.write(SerialAT.read());
    }
    
    if (Serial.available()) {
      SerialAT.write(Serial.read());
    }
  }

}


void setup() {
  SerialMon.begin(115200);

  SerialAT.begin(UART_BAUD, SERIAL_8N1, PIN_RX, PIN_TX);

  DBG("Wait...");
  int retry = 5;
  while (!reply && retry--){
      modem_on();
  }

  if (reply) {
      Serial.println(F("***********************************************************"));
      Serial.println(F(" You can now send AT commands"));
      Serial.println(F(" Enter \"AT\" (without quotes), and you should see \"OK\""));
      Serial.println(F(" If it doesn't work, select \"Both NL & CR\" in Serial Monitor"));
      Serial.println(F(" DISCLAIMER: Entering AT commands without knowing what they do"));
      Serial.println(F(" can have undesired consiquinces..."));
      Serial.println(F("***********************************************************\n"));
  } else {
      Serial.println(F("***********************************************************"));
      Serial.println(F(" Failed to connect to the modem! Check the baud and try again."));
      Serial.println(F("***********************************************************\n"));
      ESP.restart();
  }

}

void loop() {

    while (SerialAT.available()) {
        Serial.write(SerialAT.read());
    }

    while (Serial.available()) {
        SerialAT.write(Serial.read());
    }

}
