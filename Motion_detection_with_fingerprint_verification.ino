#include <WiFi.h>
#include <ESP_Mail_Client.h>
#include <Adafruit_Fingerprint.h>

HardwareSerial myserial(2);

Adafruit_Fingerprint finger = Adafruit_Fingerprint(&myserial);   //link between fingerprint sensor and Serial 

String SMTP_HOST = "smtp.gmail.com";
int SMTP_PORT = 465;
String AUTHOR_EMAIL = "saisuddhir28@gmail.com";
String AUTHOR_PASS = "wpwcpkgmoxngvlzi";
String Rec_EMAIL = "suddhirsai@gmail.com";
String SSID = "FTTH-BSNL-BHUVANA";
String pass = "eswari@bsnl889";
SMTPSession smtp;
int trigpin = 2;
int echopin = 4;
long duration;
int distance;
int led = 26;

void smtpCallback(SMTP_Status status) {
  Serial.println(status.info());
  if (status.success()) {
    Serial.println("--------------------");
    ESP_MAIL_PRINTF("Message sent success: %d\n", status.completedCount());
    ESP_MAIL_PRINTF("Message sent failed: %d\n", status.failedCount());
    Serial.println("-------------\n");
    smtp.sendingResult.clear();
  }
}

uint8_t getFingerprintID();

void setup() {
  pinMode(led, OUTPUT);
  WiFi.begin(SSID, pass);
  Serial.begin(115200);
  Serial.println("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.println(".");
    delay(200);
  }
  Serial.println("Status : Connected");
  Serial.print("IP :");
  Serial.println(WiFi.localIP());

  pinMode(trigpin, OUTPUT);
  pinMode(echopin, INPUT);
  digitalWrite(trigpin, LOW);

  //fingerprint part
  Serial.println("Fingerprint Verification");

  finger.begin(57600);

  while (true) {
    if (finger.verifyPassword()) {
      Serial.println("Found fingerprint sensor!");
      break;  // Break the while loop if the condition is met
    } else {
      Serial.println("Did not find fingerprint sensor :(");
    }
    delay(1000);
  }

  Serial.println(F("Reading sensor parameters"));
  finger.getParameters();
  Serial.print(F("Status: 0x"));
  Serial.println(finger.status_reg, HEX);
  Serial.print(F("Sys ID: 0x"));
  Serial.println(finger.system_id, HEX);
  Serial.print(F("Capacity: "));
  Serial.println(finger.capacity);
  Serial.print(F("Security level: "));
  Serial.println(finger.security_level);
  Serial.print(F("Device address: "));
  Serial.println(finger.device_addr, HEX);
  Serial.print(F("Packet len: "));
  Serial.println(finger.packet_len);
  Serial.print(F("Baud rate: "));
  Serial.println(finger.baud_rate);

  finger.getTemplateCount();

  if (finger.templateCount == 0) {
    Serial.print("Sensor doesn't contain any fingerprint data. Please run the 'enroll' example.");
  }
  else {
    Serial.println("Waiting for valid finger...");
    Serial.print("Sensor contains "); Serial.print(finger.templateCount); Serial.println(" templates");
  }
}

void loop() {
  delay(1000);
  //ultrasonic sensor part
  digitalWrite(trigpin, HIGH);
  delayMicroseconds(5);
  digitalWrite(trigpin, LOW);
  duration = pulseIn(echopin, HIGH);
  distance = duration * 0.017; //put your main code here, to run repeatedly:
  Serial.print("distance = ");
  Serial.print(distance);
  Serial.println("cm");

  if (distance < 150) {
  
    digitalWrite(led, HIGH);
    //fingerprint part
    getFingerprintID();
    delay(50);
    int p=-1;
    int x = finger.fingerID;
    String text;
    if(x==1){
       text = "Hello Sir! Nana is waiting outside your door";
    } 
    else if (x == 2) 
    {
      text = "Hello Sir! Mummy is waiting outside your door";
    }
    else if(x==3)
    {
      text="This is your fingerprint";

    }
    else {
      text="Hello Sir! There is someone unidentified waiting outside your door";
    }

    //SMTP part
    MailClient.networkReconnect(true);
    smtp.debug(1);
    smtp.callback(smtpCallback);
    Session_Config config;

    config.server.host_name = SMTP_HOST;
    config.server.port = SMTP_PORT;
    config.login.email = AUTHOR_EMAIL;
    config.login.password = AUTHOR_PASS;

    config.time.ntp_server = F("pool.ntp.org,time.nist.gov");
    config.time.gmt_offset = 3;
    config.time.day_light_offset = 0;

    SMTP_Message message;
    message.sender.name = "JARVIS";
    message.sender.email = AUTHOR_EMAIL;
    message.subject = "ROOM";
    message.addRecipient("SAI", Rec_EMAIL);
    message.text.content = text;
    message.text.charSet = "us-ascii";
    message.text.transfer_encoding = Content_Transfer_Encoding::enc_7bit;

    if (!smtp.connect(&config)) {
      ESP_MAIL_PRINTF("Connection error, Status Code: %d, Error Code: %d, Reason: %s", smtp.statusCode(), smtp.errorCode(), smtp.errorReason().c_str());
      return;
    }
    if (!smtp.isLoggedIn()) {
      Serial.println("\nNot yet logged in.");
    }
    else {
      if (smtp.isAuthenticated())
        Serial.println("\nSuccessfully logged in.");
      else
        Serial.println("\nConnected with no Auth.");
    }

    if (!MailClient.sendMail(&smtp, &message))
      ESP_MAIL_PRINTF("Error, Status Code: %d, Error Code: %d, Reason: %s", smtp.statusCode(), smtp.errorCode(), smtp.errorReason().c_str());
  }
  else {
    digitalWrite(led, LOW);
  }

  delay(10000);
}

uint8_t getFingerprintID() {

  int p = -1;
  Serial.println("Place your finger");
  while (p != FINGERPRINT_OK)
  {
    p = finger.getImage();
    switch (p)
    {
    case FINGERPRINT_OK:
      Serial.println("Image taken");
      break;
    case FINGERPRINT_NOFINGER:
      Serial.println(".");
      break;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      break;
    case FINGERPRINT_IMAGEFAIL:
      Serial.println("Imaging error");
      break;
    default:
      Serial.println("Unknown error");
      break;
    }
  }

  // OK success!

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
      return p;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Could not find fingerprint features");
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Could not find fingerprint features");
      return p;
    default:
      Serial.println("Unknown error");
      return p;
  }

  // OK converted!
  p = finger.fingerSearch();
  if (p == FINGERPRINT_OK) {
    Serial.println("Found a print match!");
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
    return p;
  } else if (p == FINGERPRINT_NOTFOUND) {
    Serial.println("Did not find a match");
    return p;
  } else {
    Serial.println("Unknown error");
    return p;
  }

  // found a match!
  Serial.print("Found ID #"); Serial.print(finger.fingerID);
  Serial.print(" with confidence of "); Serial.println(finger.confidence);

  return finger.fingerID;
}