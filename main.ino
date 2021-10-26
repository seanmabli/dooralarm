#include "ESP32_MailClient.h"
#include <Keypad.h>
#include <Wire.h>

const char* ssid = "Your SSID"; // Enter Your SSID
const char* password = "Your Password"; // Enter Your Password

String emailSenderAccount  = "Your Email";
String emailSenderPassword = "Your Password";
String emailRecipient      = "Your Email";
String smtpServer          = "smtp.gmail.com"; // Change If You Are Not Sending Using Gmail
int smtpServerPort         = 465; // Change If You Are Not Sending Using Gmail
String emailSubject;
String emailText;

SMTPData smtpData;

const byte ROWS = 4;
const byte COLS = 4;

char hexaKeys[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};
byte rowPins[ROWS] = {27, 14, 12, 13};
byte colPins[COLS] = {4, 0, 2, 15}; 

Keypad customKeypad = Keypad(makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS);
String customKey;

const int MPU_addr = 0x68;
int16_t AcX;
int Avg_AcX;

String Real_Password = "1234"; // Change Your Password Here
String Entered_Password;

bool Alarm_Armed = true;
bool Alarm_Trigered = false;

int Green = 17;
int Red = 16;

void setup() {
  pinMode(Red, OUTPUT);
  pinMode(Green, OUTPUT);

  Wire.begin();
  Wire.beginTransmission(MPU_addr);
  Wire.write(0x6B);
  Wire.write(0);
  Wire.endTransmission(true);

  Serial.begin(115200);
  Serial.print("Connecting");

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(200);
  }

  Serial.println();
  Serial.println("WiFi connected.");
}

void loop() {
  Wire.beginTransmission(MPU_addr);
  Wire.write(0x3B);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU_addr, 14, true);
  AcX = Wire.read() << 8 | Wire.read();

  Avg_AcX = (Avg_AcX + AcX) / 2;
  Serial.println(Avg_AcX);

  if (Alarm_Armed == true && Alarm_Trigered == false) {
    digitalWrite(Red, LOW);
    digitalWrite(Green, LOW);
    if ((Avg_AcX + 550) < AcX || (Avg_AcX - 550) > AcX) { //Increase The 550 If The Alarm Is Triggering On Accident
      digitalWrite(Red, HIGH);
      digitalWrite(Green, LOW);
      emailSubject = "Door Opened";
      emailText = "Your Door Has Been Opened";
      Send_Email();
      Alarm_Trigered = true;
    }
  }
  if (Alarm_Armed == true && Alarm_Trigered == true) {
    customKey = customKeypad.getKey();
    Entered_Password = Entered_Password + customKey;

    if (Entered_Password.substring(Entered_Password.length() - 4) == Real_Password) {
      Serial.println("Password Correct");
      digitalWrite(Red, LOW);
      digitalWrite(Green, HIGH);
      Alarm_Armed = false;
      Alarm_Trigered = false;
      Entered_Password = "";
      emailSubject = "Passwod Entered";
      emailText = "The Password Has Been Entered";
      Send_Email(); 
    }

    digitalWrite(Red, HIGH);
    digitalWrite(Green, LOW);
  }
  if (Alarm_Armed == false) {
    customKey = customKeypad.getKey();
      
    if (customKey == "*") {
      digitalWrite(Red, LOW);
      digitalWrite(Green, LOW);
      delay(10000); //Add Time If You Want More Time To Close The Door
      Alarm_Armed = true;
      Alarm_Trigered = false;
    }

    digitalWrite(Red, LOW);
    digitalWrite(Green, HIGH);
  }
}

void Send_Email() {
  smtpData.setLogin(smtpServer, smtpServerPort, emailSenderAccount, emailSenderPassword);
  smtpData.setSender("Door Alarm", emailSenderAccount);
  smtpData.setPriority("High");
  smtpData.setSubject(emailSubject);
  smtpData.setMessage("<div style=\"color:#2f4468;\"><h3>" + emailText + "</h3></div>", true);
  smtpData.addRecipient(emailRecipient);
  if (!MailClient.sendMail(smtpData))
    Serial.println("Error sending Email, " + MailClient.smtpErrorReason());
  smtpData.empty();
}