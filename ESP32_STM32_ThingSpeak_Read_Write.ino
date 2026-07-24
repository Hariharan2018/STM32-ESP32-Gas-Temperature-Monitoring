#include <WiFi.h>
#include <ThingSpeak.h>

const char* ssid = "hotspot";
const char* password = "qwerty123";

// ThingSpeak
unsigned long channelID = 3435002;
const char *writeAPIKey = "UA04BKVJUJG5JQB0";
const char *readAPIKey  = "G4QSEMWASJ5UVKDK";

WiFiClient client;

// UART2
HardwareSerial STMSerial(2);

int gasValue = 0;
int tempValue = 0;

unsigned long lastUpdate = 0;

void setup()
{
  Serial.begin(115200);

  // RX = GPIO16, TX = GPIO17
  STMSerial.begin(115200, SERIAL_8N1, 16, 17);

  WiFi.begin(ssid, password);

  Serial.print("Connecting to WiFi");

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi Connected");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());

  ThingSpeak.begin(client);
}

void loop()
{
  if (STMSerial.available())
  {
    String data = STMSerial.readStringUntil('\n');
    data.trim();

    Serial.print("Received: ");
    Serial.println(data);

    int gIndex = data.indexOf("G:");
    int tIndex = data.indexOf(",T:");

    if (gIndex != -1 && tIndex != -1)
    {
      gasValue = data.substring(gIndex + 2, tIndex).toInt();
      tempValue = data.substring(tIndex + 3).toInt();

      Serial.print("Gas = ");
      Serial.println(gasValue);

      Serial.print("Temperature = ");
      Serial.println(tempValue);
    }
  }

  if (millis() - lastUpdate >= 20000)
  {
    ThingSpeak.setField(1, gasValue);
    ThingSpeak.setField(2, tempValue);

    int writeStatus = ThingSpeak.writeFields(channelID, writeAPIKey);

    if (writeStatus == 200)
      Serial.println("ThingSpeak Write Success");
    else
    {
      Serial.print("Write Error: ");
      Serial.println(writeStatus);
    }

    delay(1000);

    int ledState = ThingSpeak.readIntField(channelID, 3, readAPIKey);

    if (ThingSpeak.getLastReadStatus() == 200)
    {
      Serial.print("LED Command = ");
      Serial.println(ledState);

      if (ledState == 1)
        STMSerial.write('1');
      else
        STMSerial.write('0');
    }

    lastUpdate = millis();
  }
}
