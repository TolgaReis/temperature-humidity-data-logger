/* Libraries */
#include <Adafruit_GFX.h>      // include adafruit graphics library
#include <Adafruit_PCD8544.h>  // include adafruit PCD8544 (Nokia 5110) library
#include "DHT.h"               // include DHT22 temperature and humidity sensor library
#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>
/* Libraries End */

/* Macro Constants */
#define DHTTYPE DHT22
#define READ_FAILURE 1
#define READ_SUCCESS 0
/* Macro Constants End */ 

/* Enums */
enum errorType
{
  WIFI_CONN,
  READ_DHT,
  HTTP_CONN
};
/* Enums End*/

/* Global Variables */
char* ssid = "WIFI_SSID";
char* wifiPass = "WIFI_PASS"; 
String serverPath = "SERVER_ENDPOINT";
HTTPClient http;
/* Global Variables End*/

/* DHT Sensor */
uint8_t DHTPin = D5; 
DHT dht(DHTPin, DHTTYPE);                
/* DHT Sensor End*/

/* Nokia 5110 LCD Module */
/* 
 * Nokia 5110 LCD module connections (CLK, DIN, D/C, CS, RST) 
 * D4: CLK,
 * D3: DIN,
 * D2: D/C,
 * D1: CS,
 * D0: RST
 */
Adafruit_PCD8544 display = Adafruit_PCD8544(D4, D3, D2, D1, D0);
/* Nokia 5110 LCD Module End*/

/* Function Declarations */
int readDht(float *, float *);
void displayData(float, float);
void displayErrMsg(errorType);
void displayWifiStat(bool);
/* Function Declarations End */

/* Global Variables */
int counter;
/* Global Variables End */

void setup()   {
  /* starts serial communication with card by 9600 baud rate*/
  Serial.begin(9600);

  /* DHT22 Sensor Setup */
  dht.begin();  
  /* DHT22 Sensor Setup End */
 
  /* Nokia 5110 LCD Module Setup */
  display.begin();
  display.setContrast(50);
  display.display();  
  display.clearDisplay();
  /* Nokia 5110 LCD Module Setup End */

  /* WiFi Connection Setup */
  display.setTextSize(1);
  display.setTextColor(BLACK);
  display.setCursor(0, 3);
  WiFi.begin(ssid, wifiPass);
 
  while (WiFi.status() != WL_CONNECTED) {
    displayWifiStat(false);
  }
  displayWifiStat(true);
  display.display();
  delay(1000);
  display.clearDisplay();
  /* WiFi Connection Setup End*/
  
  counter = 0;
}

void loop() {
  float temperature,
        humidity;
  switch(readDht(&temperature, &humidity))
  {
    case READ_SUCCESS:
      displayData(temperature, humidity);
      break;
    case READ_FAILURE:
      displayErrMsg(READ_DHT);
      break;
    default:
      break;
  }

  if(counter >= 60000 && WiFi.status()== WL_CONNECTED)
  {
    String requestPath = serverPath +  String("islem=data_gonder&sicaklik=") + String(temperature) + String("&nem=") + String(humidity);
    
    /* HTTP Request */
    if(http.begin(requestPath))
    {
      int httpCode = http.GET();
      String payload = http.getString();
      Serial.println(httpCode);
      Serial.println(payload); 
      http.end();  
    }
    else
    {
      displayErrMsg(HTTP_CONN);
    }
  
    counter = 0;
    /* HTTP Request End*/
  }
  else if(!(WiFi.status()== WL_CONNECTED))
  {
    while(!(WiFi.status()== WL_CONNECTED))
    {
      displayErrMsg(WIFI_CONN);
    }
  }
  
}

int readDht(float *temperature, float *humidity)
{
  *temperature = dht.readTemperature();
  *humidity = dht.readHumidity();
  if (isnan(*temperature) || isnan(*humidity)) 
    return READ_FAILURE;
  else
    return READ_SUCCESS;
}

void displayData(float temperature, float humidity)
{
  Serial.print("Sicaklik: ");
  Serial.println(temperature);
  Serial.print("Bagil nem: ");
  Serial.println(humidity);

  display.setTextSize(1);
  display.write("Sicaklik: \n\n");
  display.setTextSize(2, 4);
  display.print(temperature);
  display.setTextSize(1, 2);
  display.print(" C");
  display.display();
  delay(2000);
  display.clearDisplay();

  display.setTextSize(1);
  display.setCursor(0, 0);
  display.write("Bagil Nem: \n\n");
  display.setTextSize(2, 4);
  display.print(humidity);
  display.setTextSize(1, 2);
  display.print(" %");  
  display.display();
  delay(2000);
  display.clearDisplay();
  counter += 4000;
}

void displayErrMsg(errorType errorType)
{
  display.clearDisplay();
  switch(errorType)
  {
    case WIFI_CONN:
      Serial.println("\nHATA! WiFi \nbaglantisi!\n");
      display.write("\nHATA! WiFi \nbaglantisi!\n");   
      break;
    case HTTP_CONN:
      Serial.println("\nHATA! HTTP \nbaglantisi!\n");
      display.write("\nHATA! HTTP \nbaglantisi!\n");   
      break;      
    case READ_DHT:
      Serial.println("\nHATA! Sensor okuma!\n");
      display.write("\nHATA! Sensor okuma!\n");   
      break;
  }
  display.display();
  delay(3000);
  display.clearDisplay();
}

void displayWifiStat(bool wifiStatus)
{
  display.clearDisplay();
  switch(wifiStatus)
  {
    case false:
      Serial.println("WiFi baglantisi icin bekleniyor...");
      display.write("WiFi baglantisi icin bekleniyor...");
      break;
    case true:
      Serial.println("WiFi baglantisi saglandi!");
      display.write("WiFi baglantisi saglandi!");
      break;
    default:
      break;
  }
  display.display();
  delay(500);
  display.clearDisplay();
}
