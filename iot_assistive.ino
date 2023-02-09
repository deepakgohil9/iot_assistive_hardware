#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecureBearSSL.h>
#include <Notecard.h>
#include <DHT.h>
#include <Servo.h>

#define txRxPinsSerial Serial
#define productUID "<product_uid>"

#define LDRPIN D1
#define FLAMEPIN D2
#define DHTPIN D3
#define GASPIN D4
#define RELAY1 D7
#define RELAY2 D8
#define SERVOPIN D6

#define DHTTYPE DHT11

const char *ssid = "<ssid_name>";
const char *password = "<password>";

Notecard notecard;
DHT dht(DHTPIN, DHTTYPE);
Servo servo;

void setup()
{

    notecard.begin(txRxPinsSerial, 9600);

    J *req = notecard.newRequest("hub.set");
    JAddStringToObject(req, "product", productUID);
    JAddStringToObject(req, "mode", "continuous");
    bool stat = notecard.sendRequest(req);
    Serial.println(stat);

    pinMode(LDRPIN, INPUT);
    pinMode(GASPIN, INPUT);
    pinMode(FLAMEPIN, INPUT);
    pinMode(RELAY1, OUTPUT);
    pinMode(RELAY2, OUTPUT);
    servo.attach(SERVOPIN);
    delay(500);
    dht.begin();
    delay(2000);

    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    Serial.print("Connecting to WiFi ..");
    while (WiFi.status() != WL_CONNECTED)
    {
        Serial.print('.');
        delay(1000);
    }
}

void loop()
{
    if ((WiFi.status() == WL_CONNECTED))
    {

        std::unique_ptr<BearSSL::WiFiClientSecure> client(new BearSSL::WiFiClientSecure);

        client->setInsecure();

        HTTPClient https;

        if (https.begin(*client, "<base_url>/control/get/1"))
        {
            int httpCode = https.GET();
            if (httpCode > 0)
            {
                if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY)
                {
                    String payload = https.getString();
                    if (payload == "true")
                        digitalWrite(RELAY1, HIGH);
                    else
                        digitalWrite(RELAY1, LOW);

                    Serial.println(payload);
                }
            }
            else
            {
                Serial.printf("[HTTPS] GET... failed, error: %s\n", https.errorToString(httpCode).c_str());
            }
            https.end();
        }
        else
        {
            Serial.printf("[HTTPS] Unable to connect\n");
        }

        if (https.begin(*client, "<base_url>/control/get/2"))
        {
            int httpCode = https.GET();
            if (httpCode > 0)
            {
                if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY)
                {
                    String payload = https.getString();
                    if (payload == "true")
                        digitalWrite(RELAY2, HIGH);
                    else
                        digitalWrite(RELAY2, LOW);

                    Serial.println(payload);
                }
            }
            else
            {
                Serial.printf("[HTTPS] GET... failed, error: %s\n", https.errorToString(httpCode).c_str());
            }
            https.end();
        }
        else
        {
            Serial.printf("[HTTPS] Unable to connect\n");
        }

        if (https.begin(*client, "<base_url>/control/get/3"))
        {
            int httpCode = https.GET();
            if (httpCode > 0)
            {
                if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY)
                {
                    String payload = https.getString();
                    if (payload == "true")
                        servo.write(180);
                    else
                        servo.write(0);

                    Serial.println(payload);
                }
            }
            else
            {
                Serial.printf("[HTTPS] GET... failed, error: %s\n", https.errorToString(httpCode).c_str());
            }
            https.end();
        }
        else
        {
            Serial.printf("[HTTPS] Unable to connect\n");
        }
    }

    float h = dht.readHumidity();
    Serial.print("Humidity Level     : ");
    Serial.print(h);
    Serial.println(" %");

    float t = dht.readTemperature();
    Serial.print("Temperature Level  : ");
    Serial.print(t);
    Serial.println(" C");

    int l = digitalRead(LDRPIN);
    Serial.print("Light  : ");
    Serial.println(l);

    int g = digitalRead(GASPIN);
    Serial.print("GAS  : ");
    Serial.println(g);

    int f = digitalRead(FLAMEPIN);
    Serial.print("FLAME  : ");
    Serial.println(f);

    J *req = notecard.newRequest("note.add");
    if (req != NULL)
    {
        JAddStringToObject(req, "file", "mlh_htf.qo");
        JAddBoolToObject(req, "sync", true);

        J *body = JCreateObject();
        if (body != NULL)
        {
            JAddNumberToObject(body, "temperature", t);
            JAddNumberToObject(body, "humidity", h);
            JAddNumberToObject(body, "light", l);
            JAddNumberToObject(body, "gas", g);
            JAddNumberToObject(body, "flame", f);
            JAddItemToObject(req, "body", body);
        }
        bool stat = notecard.sendRequest(req);
        Serial.println(stat);
    }
    Serial.println("------------------------------------------------");
    delay(30000);
}
