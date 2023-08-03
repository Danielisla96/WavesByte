// ######################################################################
// Setup variables:
#define TINY_GSM_MODEM_SIM7600
#define TINY_GSM_RX_BUFFER 1024
#define SerialAT Serial1
#define DUMP_AT_COMMANDS
#define MODEM_TX            17
#define MODEM_RX            16
#define OPTION1_PIN         12
#define OPTION2_PIN         142
#define SLEEP_PIN           23
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
// ######################################################################


// ######################################################################
// Packages:
#include <WiFi.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Pangodream_18650_CL.h>
Pangodream_18650_CL BL;
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
#include <TinyGsmClient.h>
#include <SPI.h>
#include <Ticker.h>
#ifdef DUMP_AT_COMMANDS
#include <StreamDebugger.h>
StreamDebugger debugger(SerialAT, Serial);
TinyGsm modem(debugger);
#else
TinyGsm modem(SerialAT);
#endif
// ######################################################################


// ######################################################################
// Global variables:
String apn = "bam.clarochile.cl";
String apn_type = "IP";
String apn_user = "clarochile";
String apn_password = "clarochile";

String client_id = "863427043680207"; // IMEI DEL MODULO SIM (IDENTIFICADOR UNICO POR GPS) 863427043680207
String gps_raw;
String latitude = "";
String longitude = "";
String s_n = "";
String w_e = "";
String datetime_str = "";
String datetime_int = "";
String date_str = "";
String date_int = "";
String time_str = "";
String time_int = "";
String altitude = "";
String speed = "";
float speed_num = 0;
String network_raw = "";
String network_status = "";
String network_type = "";
String quality_signal_raw = "";
String quality_signal = "";
String bat_pin_value = "";
String bat_volt_value = "";
String bat_percentage_value = "";
const int pin_carga = 2;
const int pin_encendido_vehiculo = 4;
int status_charging;
int status_vehicule;
String msg;
String message_aws;
String okResponses[] = {"OK"};
String cmqttstartResponses[] = {"+CMQTTSTART: 0", "+CMQTTSTART: 23"};
String cgpsResponses[] = {"+CGPS: 0"};
String nothingResponses[] = {""};
String cmqttconnectResponses[] = {"+CMQTTCONNECT: 0,0"};
String cmqttaccqResponses[] = {"OK", "+CMQTTACCQ: 0,19"};
String cmqttdiscResponses[] = {"+CMQTTDISC: 0,0", "+CMQTTDISC: 0,11"};
String cmqttstopResponses[] = {"OK", "+CMQTTSTOP: 9"};
String cmqttsubResponses[] = {"OK", "+CMQTTSUB: 0,0"};
String cregResponses[] = {"+CREG: 0,5", "+CREG: 0,1"};
String netopenResponses[] = {"+NETOPEN: 1", "+IP ERROR: Network is already opened", "OK"};
// ######################################################################


// ######################################################################
// Function that print in LCD screen at message:
void lcd_message(String text) {
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(1, 1);
  display.println(text);
  Serial.println(text);
  display.display(); 
  delay(1);
}
// ######################################################################


// ######################################################################
// Function that define the status of charging or not:
int status_chrg() {
  if (digitalRead(pin_carga) == LOW) {
    status_charging = 0;
  }
  if (digitalRead(pin_carga) == HIGH) {
    status_charging = 1;
  }
  //Serial.println(status_charging);
  return status_charging;
}
// ######################################################################


// ######################################################################
// Function that define the status of vehicule or not:
void status_car() {
  if (digitalRead(pin_encendido_vehiculo) == LOW) {
    status_vehicule = 0;
  }
  if (digitalRead(pin_encendido_vehiculo) == HIGH) {
    status_vehicule = 1;
  }
  //Serial.println(status_charging);
}
// ######################################################################



// ######################################################################
// Function that testing modem is working OK:
void sendATAndWait(String AT, String expectedResponses[], int expectedResponsesLength, int Delay, int minLength) {
  for(int i=0; i<10; i++) {
    lcd_message("Sending an " + AT + " command...");
    SerialAT.println(AT);
  
    delay(Delay * 1000);

    String response;
  
    while (SerialAT.available()) {
      String r = SerialAT.readString();
      
      r.replace("\r", "");
      r.replace("\n", " ");
      
      response += r;
      lcd_message("message: " + r);
      
      for(int j=0; j<expectedResponsesLength; j++) {
        if (response.indexOf(expectedResponses[j]) != -1 && response.length() >= minLength) {
          lcd_message("BIEN! SIGUIENTE...");
          delay(1);
          return;
        }
      }
    }
  }
  ESP.restart();
}
// ######################################################################

// ######################################################################
// Function that testing modem is working OK:
void sendATAndWaitNoMsg(String AT, String expectedResponses[], int expectedResponsesLength, int Delay, int minLength) {
  for(int i=0; i<10; i++) {
    //lcd_message("Sending an " + AT + " command...");
    SerialAT.println(AT);
  
    delay(Delay * 1000);

    String response;
  
    while (SerialAT.available()) {
      String r = SerialAT.readString();
      
      r.replace("\r", "");
      r.replace("\n", " ");
      
      response += r;
      //lcd_message("message: " + r);
      
      for(int j=0; j<expectedResponsesLength; j++) {
        if (response.indexOf(expectedResponses[j]) != -1 && response.length() >= minLength) {
          //lcd_message("BIEN! SIGUIENTE...");
          delay(1000);
          return;
        }
      }
    }
  }
  ESP.restart();
}
// ######################################################################


// ######################################################################
// Function that send any AT command:
void sendAT(String AT, int Delay) {

  lcd_message("Sending an " + AT + " command...");
  SerialAT.println(AT);

  delay(Delay*100);

  while (SerialAT.available() || Serial.available()) {
    if (SerialAT.available()) {
      String r = SerialAT.readString();
      Serial.write(SerialAT.read());
      lcd_message("message: " + r);
    }
    if (Serial.available()) {
      String r = SerialAT.readString();
      SerialAT.write(Serial.read());
      lcd_message("message: " + r);
    }
  SerialAT.readString();  
  }
}
// ######################################################################


// ######################################################################
// Function that send message command:
void enviarMsg(String MSG) {
  lcd_message("Sending an " + MSG + " message...");
  SerialAT.println(MSG);
  delay(100);
  while (SerialAT.available() || Serial.available()) {
    if (SerialAT.available()) {
      String r = SerialAT.readString();
      Serial.write(SerialAT.read());
      lcd_message("message: " + r);
    }
    if (Serial.available()) {
      String r = SerialAT.readString();
      SerialAT.write(Serial.read());
      lcd_message("message: " + r);
    }
  }
  SerialAT.readString();
}
// ######################################################################


// ######################################################################
// Function that separe a String by comma:
String getValue(String data, char separator, int index) {
  int found = 0;
  int strIndex[] = {0, -1};
  int maxIndex = data.length()-1;
  for(int i=0; i<=maxIndex && found<=index; i++){
    if(data.charAt(i)==separator || i==maxIndex){
        found++;
        strIndex[0] = strIndex[1]+1;
        strIndex[1] = (i == maxIndex) ? i+1 : i;
    }
  }
  return found>index ? data.substring(strIndex[0], strIndex[1]) : "";
}
// ######################################################################


// ######################################################################
// Function that get a string between characters:
String GetStringBetweenStrings(String input, String firstdel, String enddel){
      int posfrom = input.indexOf(firstdel) + firstdel.length();
      int posto   = input.indexOf(enddel);
      return input.substring(posfrom, posto);
}
// ######################################################################


// ######################################################################
// Function that receive message from internet:
void recieveMessage() {

  if(SerialAT.available()) {
    msg = SerialAT.readString();
    msg = GetStringBetweenStrings(msg ,"{","}");
    msg.replace("\"msg\":", "");
    msg.replace("\"", "");
    if(msg == "option1_on"){
      Serial.println("OPCION 1 EJECUTANDOSE...");
      digitalWrite(OPTION1_PIN, HIGH);
    };
    if(msg == "option1_off"){
      Serial.println("OPCION 1 EJECUTANDOSE...");
      digitalWrite(OPTION1_PIN, LOW);
    };
  }
}
// ######################################################################


// ######################################################################
// Function that obtain GPS coordinates:
void obtainGPS() {
  lcd_message("Sending an AT+CGPSINFO command...");
  SerialAT.readString();
  SerialAT.println("AT+CGPSINFO");
  delay(100);
  gps_raw = SerialAT.readString();
  lcd_message("message: " + gps_raw);

  gps_raw.replace("\n", "");
  gps_raw.replace("+CGPSINFO","");
  gps_raw.trim();
  gps_raw.replace("AT","");
  gps_raw.trim();
  gps_raw.replace(":","");
  gps_raw.trim();
  latitude = getValue(gps_raw,',',0); //cordenada lat sin transformar
  s_n = getValue(gps_raw,',',1); // S-N (Si es S es negativo, si es N es positivo)
  longitude = getValue(gps_raw,',',2); //cordenada lng sin transformar
  w_e = getValue(gps_raw,',',3); // W-E (Si es W es negativo, si es E es positivo)
  date_str = getValue(gps_raw,',',4); // fecha (ddmmaa)
  time_str = getValue(gps_raw,',',5); // hora utc (hhmmss.s)
  altitude = getValue(gps_raw,',',6); // altitud (mts)
  speed = getValue(gps_raw,',',7); // speed (en nudos, hay que multiplicarlo por 1.852 para obtener km/h)
  datetime_str = date_str.substring(0, 2) + "-" + date_str.substring(2, 4) + "-" + String("20") + date_str.substring(4, 6) + " " + time_str.substring(0,2) + ":" + time_str.substring(2,4) + ":" + time_str.substring(4,6);
  datetime_int = String("20") + date_str.substring(4, 6) + date_str.substring(2, 4) + date_str.substring(0, 2) + time_str.substring(0,2) + time_str.substring(2,4) + time_str.substring(4,6);
  date_int = String("20") + date_str.substring(4, 6) + date_str.substring(2, 4) + date_str.substring(0, 2);
  time_int = time_str.substring(0,2) + time_str.substring(2,4) + time_str.substring(4,6);
  SerialAT.readString();
  delay(10);
}
// ######################################################################


float obtainSpeedGPS() {
  //lcd_message("Sending an AT+CGPSINFO command...");
  SerialAT.readString();
  SerialAT.println("AT+CGPSINFO");
  delay(100);
  gps_raw = SerialAT.readString();
  //lcd_message("message: " + gps_raw);

  gps_raw.replace("\n", "");
  gps_raw.replace("+CGPSINFO","");
  gps_raw.trim();
  gps_raw.replace("AT","");
  gps_raw.trim();
  gps_raw.replace(":","");
  gps_raw.trim();

  speed = getValue(gps_raw,',',7); // speed (en nudos, hay que multiplicarlo por 1.852 para obtener km/h)
  return speed.toFloat(); //la velocidad la guardamos en float para tomar decisiones.
}


// ######################################################################
// Function that obtain network connection modem:
void obtainNetworkStatus() {
  lcd_message("Sending an AT+CPSI? command...");
  SerialAT.readString();
  SerialAT.println("AT+CPSI?");
  delay(100);
  network_raw = SerialAT.readString();
  lcd_message("message: " + network_raw);

  network_raw.replace("\n", "");
  network_raw.replace("+CPSI?","");
  network_raw.trim();
  network_raw.replace("AT","");
  network_raw.trim();
  network_raw.replace(":","");
  network_raw.trim();
  network_raw.replace("+CPSI","");
  network_raw.trim();
  network_type = getValue(network_raw,',',0);
  network_status = getValue(network_raw,',',1);
  SerialAT.readString();
  delay(10);
}
// ######################################################################


// ######################################################################
// Function that obtain quality signal modem:
void obtainSignalQuality() {
  lcd_message("Sending an AT+CSQ command...");
  SerialAT.readString();
  SerialAT.println("AT+CSQ");
  delay(100);
  quality_signal_raw = SerialAT.readString();
  lcd_message("message: " + quality_signal_raw);
  quality_signal_raw.replace("\n", "");
  quality_signal_raw.replace("+CSQ","");
  quality_signal_raw.trim();
  quality_signal_raw.replace("AT","");
  quality_signal_raw.trim();
  quality_signal_raw.replace(":","");
  quality_signal_raw.trim();
  quality_signal = getValue(quality_signal_raw,',',0);
  SerialAT.readString();
  delay(10);
}
// ######################################################################

// ######################################################################
// Function that returns battery percentage:
int batteryPercentage() {
    int pinValue = BL.pinRead();
    return max(0, min(100, (pinValue - 1875) / 25 * 5));
}
// ######################################################################

// ######################################################################
// Function that obtains battery status:
void obtainBatteryStatus() {
    SerialAT.readString();
    lcd_message("Obtain battery status...");
    bat_pin_value = String(BL.pinRead());
    bat_volt_value = String(BL.getBatteryVolts());
    bat_percentage_value = String(batteryPercentage());
    delay(10);
}
// ######################################################################

// ######################################################################
// Function that send AWS message:
void sendMsgtoAws(String topic_aws) {
  message_aws = "";
  message_aws += { 
    String("{") + 
    String("\"client_id\":") + String("\"") + client_id + String("\"") + 
    String(", \"datetime_str\":") + String("\"") + String(datetime_str) + String("\"") + 
    String(", \"datetime_int\":") + String(datetime_int) + 
    String(", \"latitude\":") + String("\"") + latitude + String("\"") + 
    String(", \"s_n\":") + String("\"") + s_n + String("\"") + 
    String(", \"longitude\":") + String("\"") + longitude + String("\"") + 
    String(", \"w_e\":") + String("\"") + w_e + String("\"") + 
    String(", \"date_int\":") + date_int +
    String(", \"time_str\":") + String("\"") + time_int + String("\"") + 
    String(", \"altitude\":") + altitude + 
    String(", \"speed\":") + speed + 
    String(", \"network_status\":") + String("\"") + network_status + String("\"") +
    String(", \"network_type\":") + String("\"") + network_type + String("\"") + 
    String(", \"quality_signal\":") + String("\"") + quality_signal + String("\"") + 
    String(", \"bat_pin_value\":") + String("\"") + bat_pin_value + String("\"") + 
    String(", \"bat_volt_value\":") + String("\"") + bat_volt_value + String("\"") + 
    String(", \"bat_percentage_value\":") + String("\"") + bat_percentage_value + String("\"") +
    String(", \"status_charging\":") + status_charging
    + String("}")
  };
  delay(1);
  int len_topic_aws = topic_aws.length();
  int len_message = message_aws.length();
  sendAT("AT+CMQTTTOPIC=0," + String(len_topic_aws), 0);
  delay(1);
  enviarMsg(topic_aws);
  delay(1);
  sendAT("AT+CMQTTPAYLOAD=0,"+ String(len_message), 0);
  delay(1);
  enviarMsg(message_aws);
  delay(1);
  sendAT("AT+CMQTTPUB=0,1,60", 0);
  SerialAT.readString();
  delay(10);
}
// ######################################################################


// ######################################################################
// Function that send AWS message option1_on:
void sendMsgtoAws_option1_on(String topic_aws) {
  message_aws = "";
  message_aws += { 
    String("{") + 
    String("\"client_id\":") + String("\"") + client_id + String("\"") + 
    String(", \"option\":") + String("\"") + "option1_on" + String("\"") +
    String(", \"status\":") + String("\"") + "success" + String("\"") 
    + String("}")
  };
  delay(1);
  int len_topic_aws = topic_aws.length();
  int len_message = message_aws.length();
  sendAT("AT+CMQTTTOPIC=0," + String(len_topic_aws), 0);
  delay(1);
  enviarMsg(topic_aws);
  delay(1);
  sendAT("AT+CMQTTPAYLOAD=0,"+ String(len_message), 0);
  delay(1);
  enviarMsg(message_aws);
  delay(1);
  sendAT("AT+CMQTTPUB=0,1,60", 0);
  SerialAT.readString();
  delay(10);
}
// ######################################################################


// ######################################################################
// Function that send AWS message option1_off:
void sendMsgtoAws_option1_off(String topic_aws) {
  message_aws = "";
  message_aws += { 
    String("{") + 
    String("\"client_id\":") + String("\"") + client_id + String("\"") + 
    String(", \"option\":") + String("\"") + "option1_off" + String("\"") +
    String(", \"status\":") + String("\"") + "success" + String("\"") 
    + String("}")
  };
  delay(1);
  int len_topic_aws = topic_aws.length();
  int len_message = message_aws.length();
  sendAT("AT+CMQTTTOPIC=0," + String(len_topic_aws), 0);
  delay(1);
  enviarMsg(topic_aws);
  delay(1);
  sendAT("AT+CMQTTPAYLOAD=0,"+ String(len_message), 0);
  delay(1);
  enviarMsg(message_aws);
  delay(1);
  sendAT("AT+CMQTTPUB=0,1,60", 0);
  SerialAT.readString();
  delay(10);
}
// ######################################################################


void SleepPinModemOn() {
  lcd_message("Sleep PIN Modem ON");
  delay(2000);
  pinMode(SLEEP_PIN, OUTPUT);
  digitalWrite(SLEEP_PIN, HIGH);
}

void SleepPinModemOff() {
  lcd_message("Sleep PIN Modem OFF");
  delay(2000);
  pinMode(SLEEP_PIN, OUTPUT);
  digitalWrite(SLEEP_PIN, LOW);
}

void SleepEsp(int minSleep) {
  lcd_message("Sleep Esp32" + String(minSleep) + " minutes... (Wake up charging...)");
  esp_sleep_enable_timer_wakeup(minSleep * 60 * 1000000);
  esp_sleep_enable_ext0_wakeup(gpio_num_t(pin_carga), HIGH);
  esp_light_sleep_start();
}

void SendGpsMessageAws() {
  status_car();
  status_chrg();
  obtainNetworkStatus();
  obtainSignalQuality();
  obtainBatteryStatus();
  obtainGPS();
  sendMsgtoAws("GPS_" + String(client_id) + "/data");
};

void ReconnectAws() {
  sendATAndWait("AT+CGDCONT=1,\"" + apn_type + "\",\"" + apn + "\"", okResponses, 1, 1, 2);
  sendATAndWait("AT+CMQTTDISC=0,120", cmqttdiscResponses, 1, 1, 2);
  sendATAndWait("AT+CMQTTACCQ=0,\"" + String(client_id) + "\",1", cmqttaccqResponses, 2, 1, 2);
  sendATAndWait("AT+CMQTTSSLCFG=0,0", okResponses, 1, 1, 2);
  sendATAndWait("AT+CMQTTCONNECT=0,\"tcp://a3kl3w0pwjown7-ats.iot.us-east-1.amazonaws.com:8883\",60,1", cmqttconnectResponses, 1, 1, 5);
  sendAT("AT+CMQTTSUBTOPIC=0,26,1", 1);
  enviarMsg("GPS_" + String(client_id) + "/action");
  sendATAndWait("AT+CMQTTSUB=0", cmqttsubResponses, 2, 1, 2);
};

// Mandamos un ping a google para ver si hay internet.
bool checkInternetConnection() {
  //lcd_message("Checking internet connection...");
  sendATAndWaitNoMsg("AT+CDNSGIP=\"www.google.com\"", okResponses, 1, 1, 2);

  return true;
};


void setup() {
  delay(1000);
  lcd_message("WELCOME - GPS DANIEL ISLA");

  pinMode(pin_carga, INPUT_PULLUP);

  //pinMode(OPTION1_PIN, OUTPUT);
  //pinMode(OPTION2_PIN, OUTPUT);

  pinMode(SLEEP_PIN, OUTPUT);
  delay(200);
  digitalWrite(SLEEP_PIN, HIGH);

  //SleepPinModemOn();
  
  Serial.begin(115200);
  SerialAT.begin(115200, SERIAL_8N1, MODEM_RX, MODEM_TX);
  Serial.setTimeout(1000);
  SerialAT.setTimeout(10);

  const int delayInSeconds = 40; // 15 sec

  // esperamos 1 minuto para que inicie bien el modulo.
  for (int i = delayInSeconds; i > 0; --i) {
      lcd_message("GPS INICIANDO... Espera " + String(i) + " segundos para que inicie el modulo.");
      delay(1000); // delay for 1 second
  }



  sendATAndWait("AT", okResponses, 1, 1, 2); // command, response, n_expected_response, delay, length
  sendATAndWait("AT+CREG?", cregResponses, 2, 2, 2);
  sendATAndWait("AT+CGACT=1,1", okResponses, 2, 1, 2);
  sendATAndWait("AT+CCLK?", okResponses, 1, 1, 2);
  sendATAndWait("AT+CGPS=0", okResponses, 1, 1, 2);
  sendATAndWait("AT+CGPS=1", okResponses, 1, 1, 2);
  sendATAndWait("AT+CGDCONT=1,\"" + apn_type + "\",\"" + apn + "\"", okResponses, 1, 1, 2);
  sendATAndWait("AT+CGAUTH=1,1,\"" + apn_user + "\",\"" + apn_password + "\"", okResponses, 1, 1, 2); 
  sendATAndWait("AT+NETOPEN", netopenResponses, 2, 1, 2);
  sendATAndWait("AT+NETOPEN?", netopenResponses, 3, 1, 2);
  sendATAndWait("AT+CGPADDR=1", okResponses, 1, 1, 2);
  sendATAndWait("AT+CMQTTDISC=0,120", cmqttdiscResponses, 2, 1, 2);
  sendATAndWait("AT+CMQTTREL=0", okResponses, 1, 1, 2);
  sendATAndWait("AT+CMQTTSTOP", cmqttstopResponses, 2, 1, 2);
  sendATAndWait("AT+CSSLCFG=\"authmode\",0,2", okResponses, 1, 1, 2);
  sendATAndWait("AT+CSSLCFG=\"cacert\",0,\"cacert.pem\"", okResponses, 1, 1, 2);
  sendATAndWait("AT+CSSLCFG=\"clientcert\",0,\"clientcert.pem\"", okResponses, 1, 1, 2);
  sendATAndWait("AT+CSSLCFG=\"clientkey\",0,\"clientkey.pem\"", okResponses, 1, 1, 2);
  sendATAndWait("AT+CSSLCFG?", nothingResponses, 1, 1, 2);
  sendATAndWait("AT+CMQTTSTART", cmqttstartResponses, 2, 0, 2);
  sendATAndWait("AT+CMQTTACCQ=0,\"" + String(client_id) + "\",1", cmqttaccqResponses, 2, 1, 2);
  sendATAndWait("AT+CMQTTSSLCFG=0,0", okResponses, 1, 1, 2);
  sendATAndWait("AT+CMQTTCONNECT=0,\"tcp://a3kl3w0pwjown7-ats.iot.us-east-1.amazonaws.com:8883\",60,1", cmqttconnectResponses, 1, 5, 5);
  sendAT("AT+CMQTTSUBTOPIC=0,26,1", 2);
  enviarMsg("GPS_" + String(client_id) + "/action");
  sendATAndWait("AT+CMQTTSUB=0", cmqttsubResponses, 2, 1, 2);
  sendATAndWait("AT+CGPSINFO", okResponses, 1, 3, 80);

  SendGpsMessageAws();

  lcd_message("LISTO - COMIENZA EL LOOP EN 1 SEGUNDO");
  delay(1000);

}

unsigned long previousMillisObtainGPS = 0;
const long intervalObtainGPS = 5000;

unsigned long previousMillisSendMessageFast = 0;
const long intervalSendMessageFast = 5000;

unsigned long previousMillisSendMessageSlow = 0;
const long intervalSendMessageSlow = 60000;

unsigned long previousMillisReconnectAws = 0;
const long intervalReconnectAws = 20 * 60 * 1000;

void loop() {
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillisReconnectAws >= intervalReconnectAws) {
    previousMillisReconnectAws = currentMillis;
    ReconnectAws();
  }

  if (checkInternetConnection()) {
    if (status_chrg() == 1) {
      if (obtainSpeedGPS() > 0) {
        if (currentMillis - previousMillisSendMessageFast >= intervalSendMessageFast) {
          previousMillisSendMessageFast = currentMillis;
          SendGpsMessageAws();
        } else {
          long remainingTimeFast = intervalSendMessageFast - (currentMillis - previousMillisSendMessageFast);
          lcd_message("Next message in " + String(remainingTimeFast / 1000.0) + " seconds (FAST MODE)");
        }
      } else if (obtainSpeedGPS() == 0) {
        if (currentMillis - previousMillisSendMessageSlow >= intervalSendMessageSlow) {
          previousMillisSendMessageSlow = currentMillis;
          SendGpsMessageAws();
        } else {
          long remainingTimeSlow = intervalSendMessageSlow - (currentMillis - previousMillisSendMessageSlow);
          lcd_message("Next message in " + String(remainingTimeSlow / 1000.0) + " seconds (SLOW MODE)");
        }
      }
    } else if (status_chrg() == 0) {
      SendGpsMessageAws();
      SleepPinModemOff();
      SleepEsp(10);
    }
  }
}
