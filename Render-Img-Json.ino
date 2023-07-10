#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <TFT_eSPI.h>
#include <ArduinoJson.h>
//#include <WiFiClientSecure.h>
//#include "certs.h"
//#include "img.h"

TFT_eSPI tft = TFT_eSPI();

#define MAX_IMAGE_WDITH 100 // Adjust for your images

int16_t xpos = 0;
int16_t ypos = 0;

const char *ssid = "Guest";
const char *password = "24091995";

//const char *ssid = "SFS OFFICE";
//const char *password = "sfs#office!@";

//const char *imageUrl = "http://203.113.151.196:8080/img/avatars/imgpsh.png";
//const char *imageUrl = "http://64a756f7096b3f0fcc813ca9.mockapi.io/api/ImageArray/ImageDec/1";
//const char *imageUrl = "http://64a77ed6096b3f0fcc815dc3.mockapi.io/api/8bit/arr";
const char *imageUrl = "http://64a77ed6096b3f0fcc815dc3.mockapi.io/api/8bit/apple";

// Declare the array to store the image, image size is 100x100
uint16_t imageArray[20000] = {0};

void setup() {
    Serial.begin(115200);

    tft.begin();
    tft.setRotation(1);
    tft.fillScreen(TFT_BLACK);

    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.println("Connecting to WiFi...");
    }
    Serial.println("Connected to WiFi");

    if (downloadAndDisplayImage(imageUrl)) {
        Serial.println("Image downloaded and displayed successfully");
        WiFi.disconnect(true);
    } else {
        Serial.println("Failed to download or display the image");
    }
}

void loop() {

}

bool downloadAndDisplayImage(const char *url) {
    WiFiClient client;
    HTTPClient http;

    if (http.begin(client, url)) {
        Serial.print("[HTTP] GET...\n");
        // start connection and send HTTP header
        int httpCode = http.GET();

        // httpCode will be negative on error
        if (httpCode > 0) {
            // HTTP header has been send and Server response header has been handled
            Serial.printf("[HTTP] GET... code: %d\n", httpCode);
            tft.fillScreen(TFT_BLACK);
            tft.setTextColor(TFT_WHITE, TFT_BLACK);
            //Print http code on TFT
            tft.drawString("HTTP code: " + String(httpCode), 0, 0);
            // Waiting for 5 seconds, then make loading animation, then display the image
            delay(2000);
            tft.fillScreen(TFT_BLACK);
            // file found at server
            if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
                // get length of document (is -1 when Server sends no Content-Length header)
                int len = http.getSize();
                // Print size to Serial
                Serial.printf("Image Size: %d\n", len);
                // Get string payload, they are JSON objects
                String payload = http.getString();
                // Check if payload is empty
                if (payload.length() == 0) {
                    Serial.println("Payload is empty");
                } else {
                    Serial.println("Payload is not empty");
                    // Print payload to Serial
                    Serial.println(payload);
                    // Get size of payload
                    int payloadSize = payload.length();
                    // Print payload size to Serial
                    Serial.printf("Payload size: %d\n", payloadSize);
                }
                // Variable to count the number of bytes read
                int count = 0;
                WiFiClient *stream = http.getStreamPtr();
                // Stream& input;
                //DynamicJsonDocument doc(4096);
                // Parse JSON object
                //DeserializationError error = deserializeJson(doc, payload);
                //deserializeJson(doc, loggingStream);
                //if (error) {
                //    Serial.print(F("deserializeJson() failed: "));
                //    Serial.println(error.f_str());
                //}
                // Extract values
                //Serial.println(F("Response:"));
                //Serial.println(doc["data"].as<const char *>());
                // Buffer to read
                uint8_t buff[128] = {0};
                // Array to store continous group of 5 bytes, then we combine them to get the original byte
                uint8_t buff5[5] = {0};
                // uint16_t variable to store the original byte
                uint16_t buff16 = 0;
                // Counter for buff5
                int count5 = 0;
                // Index for imageArray to determine where to store the next byte
                int index = 0;
                // Read all data from server
                while (http.connected() && (len > 0 || len == -1)) {
                    // get available data size
                    size_t size = stream->available();
                    if (size) {
                        // Count the number of bytes read
                        count += size;
                        // Print size to Serial with line break
                        Serial.printf("Size available: %d\n", size);
                        Serial.println();
                        // read up to 128 byte
                        int c = stream->readBytes(buff, std::min((size_t) len, sizeof(buff)));
                        for (int i = 0; i < c; i++) {
                            //Serial.printf("%02X ", buff[i]);
                            // Print as char
                            //Serial.printf("%c", buff[i]);
                            // Start reading when we meet a digit
                            // Check if buff[i] is in range of 0-9, if yes, store it to buff5
                            if (buff[i] >= 48 && buff[i] <= 57) {
                                // Store buff[i] to buff5
                                buff5[count5] = buff[i];
                                // Increase count5
                                count5++;
                            } else if (buff[i] == 44) {
                                // If we meet comma
                                // Convert buff5 to uint16_t
                                buff16 = (uint16_t) strtol((char *) buff5, NULL, 10);
                                // Print buff16 to Serial as DEC
                                Serial.printf("%d ", buff16);
                                // Reset buff5
                                memset(buff5, 0, sizeof(buff5));
                                // Reset count5
                                count5 = 0;
                                // Write buff16 to imageArray
                                imageArray[index] = buff16;
                                // Increase index
                                index++;
                            }

                        }
                        if (len > 0) {
                            len -= c;
                        }
                    }
                    Serial.println();
                    delay(1);
                }
                Serial.println();
                // Show count value
                Serial.printf("Bytes count: %d\n", count);
                // Show index value
                Serial.printf("Index: %d\n", index);
                Serial.println();
                Serial.println("End of stream");
                // Print imageArray to Serial, the array is in uint16_t format
//                for (int i = 0; i < sizeof(imageArray); i++) {
//                    Serial.printf("%02X ", imageArray[i]);
//                }
                Serial.println();
                tft.setTextColor(TFT_WHITE, TFT_BLACK);
                tft.drawString("Loading image...", 0, 0);
                // Loading animation
                for (int i = 0; i < 100; i++) {
                    tft.drawPixel(i, 10, TFT_WHITE);
                    delay(10);
                }
                // Clearing the screen
                tft.fillScreen(TFT_BLACK);
                // Swap the colour byte order when rendering
                tft.setSwapBytes(true);
                // Display the image
                //tft.pushImage(0, 0, 100, 100, img);
                // Display the image
                tft.pushImage(0, 0, 100, 100, imageArray);
            }
        } else {
            Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
        }
        // Done
        Serial.println("Done");
        http.end();
        return true;
    } else {
        Serial.println("Failed to connect to the server");
        return false;
    }
}