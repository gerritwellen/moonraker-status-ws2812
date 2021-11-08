#include "config.h"

uint8_t wiCount = 0;
uint32_t previousMillis = 0;

//Animation Variables
//
uint8_t k = 0, stage = 0, modus = 6;
uint32_t last = 0;
//
//end Animaton Variables

DynamicJsonDocument doc(1024);

//Querry URL
const char *prefix = "http://";
const char *postfix = "/printer/objects/query?print_stats";
String url = prefix + PRINTER_IP + postfix;
const char *URL = url.c_str();

ESP8266WiFiMulti WiFiMulti;

//Neopixel
Adafruit_NeoPixel LED = Adafruit_NeoPixel(length, LEDPIN, NEO_GRB + NEO_KHZ800);

void setup()
{

    LED.begin();
    LED.show(); // Initialize all pixels to 'off'

    USE_SERIAL.begin(115200);
    // USE_SERIAL.setDebugOutput(true);

    USE_SERIAL.println();
    USE_SERIAL.println();
    USE_SERIAL.println("try connecting to Wifi");

    WiFi.mode(WIFI_STA);
    WiFiMulti.addAP(SSID, WPWD);
    delay(100);
}

void loop()
{
    unsigned long currentMillis = millis();

    if (currentMillis - previousMillis >= pollInterval)
    {
        previousMillis = currentMillis;

        // wait for WiFi connection
        if (WiFiMulti.run() == WL_CONNECTED)
        {
            WiFiClient c;
            HTTPClient http;
            http.begin(c, URL);
            int httpCode = http.GET();

            if (httpCode > 0)
            {
                // {
                //   "result":
                //   {
                //     "status" : {
                //       "print_stats" : {
                //         "print_duration" : 0.0,
                //         "total_duration" : 0.0,
                //         "filament_used" : 0.0,
                //         "filename" : "",
                //         "state" : "standby",
                //         "message" : ""
                //       },
                //       "virtual_sdcard" : {
                //         "progress" : 0.0,
                //         "file_position" : 0,
                //         "is_active" : false,
                //         "file_path" : null,
                //         "file_size" : 0
                //       },
                //       "webhooks" : {
                //         "state" : "ready",
                //         "state_message" : "Printer is ready"
                //       },
                //       "heater_bed" : {
                //         "temperature" : 25.436296393427405,
                //         "power" : 0.0,
                //         "target" : 0.0
                //       },
                //       "extruder" : {
                //         "pressure_advance" : 0.05,
                //         "target" : 0.0,
                //         "power" : 0.0,
                //         "can_extrude" : false,
                //         "smooth_time" : 0.04,
                //         "temperature" : 25.37958857035252
                //       }
                //     },
                //     "eventtime" : 533.171112737
                //   }
                // }
                String payload = http.getString();
                deserializeJson(doc, payload);
                JsonObject payloadObject = doc.as<JsonObject>();
                //USE_SERIAL.println(payload);
                if (httpCode == HTTP_CODE_OK)
                {
                    String state = payloadObject["result"]["status"]["print_stats"]["state"];
                    //USE_SERIAL.println(state);
                    if (state == "printing")
                    {
                        modus = 3; // Yellow, Printing
                    }
                    else if (state == "paused")
                    {
                        modus = 2; // blue
                                   // TBD: should display rainbow now and on
                    }
                    else if (state == "error")
                    {
                        modus = 0; // red
                    }
                    else if (state == "standby")
                    {
                        modus = 4;
                    }
                }
                else if (httpCode == HTTP_CODE_UNAUTHORIZED)
                {
                    // wrong api key
                    modus = 5; // Red
                    // TBD: should stop after 2min
                }
                else if (httpCode == HTTP_CODE_CONFLICT)
                {
                    // Printer is not operational
                    modus = 2; // Green
                }
            }
        }

        switch (modus)
        {
        case 0:
        {
            fade_red(&last, &k, &stage, length);
            break;
        }
        case 1:
        {
            fade_green(&last, &k, &stage, length);
            break;
        }

        case 2:
        {
            fade_blue(&last, &k, &stage, length);
            break;
        }
        case 3:
        {
            fade_yellow(&last, &k, &stage, length);
            break;
        }
        case 4:
        {
            rainbow(&last, &k, &stage, length);
            break;
        }
        case 5:
        {
            police(&last, &stage, length);
            break;
        }
        case 6:
        {
            flash(&last, &stage, length);
            break;
        }

        default:
        {

            break;
        }
        } //switch
    }
}

//Animation Functions
void rainbow(uint32_t *lastmillis, uint8_t *k, uint8_t *stage, uint8_t length)
{

    unsigned long currentmillis = millis();

    if ((currentmillis - *lastmillis) >= 20)
    {
        *lastmillis = millis();
        switch (*stage)
        {
        case 0:
        {
            for (int i = 0; i < length; i++)
            {
                LED.setPixelColor(i, 255 - *k, 0, *k);
            }
            break;
        }

        case 1:
        {
            for (int i = 0; i < length; i++)
            {
                LED.setPixelColor(i, 0, *k, 255 - *k);
            }
            break;
        }

        case 2:
        {
            for (int i = 0; i < length; i++)
            {
                LED.setPixelColor(i, *k, 255 - *k, 0);
            }
            break;
        }
        }
        LED.show();
        if (*k == 254)
        {
            *stage = (*stage + 1) % 3;
        }
        *k = (*k + 1) % 255;
    }
}

void fade_red(uint32_t *lastmillis, uint8_t *k, uint8_t *stage, uint8_t length)
{

    unsigned long currentmillis = millis();

    if ((currentmillis - *lastmillis) >= 20)
    {
        *lastmillis = millis();
        switch (*stage)
        {
        case 0:
        {
            for (int i = 0; i < length; i++)
            {
                LED.setPixelColor(i, 0, 63 + *k, 0);
            }
            break;
        }

        case 1:
        {
            for (int i = 0; i < length; i++)
            {
                LED.setPixelColor(i, 0, 255 - *k, 0);
            }
            break;
        }
        }
        LED.show();
        if (*k == 191)
        {
            *stage = (*stage + 1) % 2;
        }
        *k = (*k + 1) % 192;
    }
}

void fade_green(uint32_t *lastmillis, uint8_t *k, uint8_t *stage, uint8_t length)
{

    unsigned long currentmillis = millis();

    if ((currentmillis - *lastmillis) >= 20)
    {
        *lastmillis = millis();
        switch (*stage)
        {
        case 0:
        {
            for (int i = 0; i < length; i++)
            {
                LED.setPixelColor(i, 0, 63 + *k, 0);
            }
            break;
        }

        case 1:
        {
            for (int i = 0; i < length; i++)
            {
                LED.setPixelColor(i, 0, 255 - *k, 0);
            }
            break;
        }
        }
        LED.show();
        if (*k == 191)
        {
            *stage = (*stage + 1) % 2;
        }
        *k = (*k + 1) % 192;
    }
}

void fade_blue(uint32_t *lastmillis, uint8_t *k, uint8_t *stage, uint8_t length)
{

    unsigned long currentmillis = millis();

    if ((currentmillis - *lastmillis) >= 20)
    {
        *lastmillis = millis();
        switch (*stage)
        {
        case 0:
        {
            for (int i = 0; i < length; i++)
            {
                LED.setPixelColor(i, 0, 0, 63 + *k);
            }
            break;
        }

        case 1:
        {
            for (int i = 0; i < length; i++)
            {
                LED.setPixelColor(i, 0, 0, 255 - *k);
            }
            break;
        }
        }
        LED.show();
        if (*k == 191)
        {
            *stage = (*stage + 1) % 2;
        }
        *k = (*k + 1) % 192;
    }
}

void fade_yellow(uint32_t *lastmillis, uint8_t *k, uint8_t *stage, uint8_t length)
{

    unsigned long currentmillis = millis();

    if ((currentmillis - *lastmillis) >= 20)
    {
        *lastmillis = millis();
        switch (*stage)
        {
        case 0:
        {
            for (int i = 0; i < length; i++)
            {
                LED.setPixelColor(i, 63 + *k, 63 + *k, 0);
            }
            break;
        }

        case 1:
        {
            for (int i = 0; i < length; i++)
            {
                LED.setPixelColor(i, 255 - *k, 255 - *k, 0);
            }
            break;
        }
        }
        LED.show();
        if (*k == 191)
        {
            *stage = (*stage + 1) % 2;
        }
        *k = (*k + 1) % 192;
    }
}

void police(uint32_t *lastmillis, uint8_t *stage, uint8_t length)
{

    unsigned long currentmillis = millis();

    if ((currentmillis - *lastmillis) >= 400)
    {
        *lastmillis = millis();
        if (*stage == 0)
        {
            for (int i = 0; i < length; i++)
            {
                LED.setPixelColor(i, 255, 0, 0);
            }
            LED.show();
            *stage = 1;
        }
        else
        {
            if (*stage == 1)
            {
                for (int i = 0; i < length; i++)
                {
                    LED.setPixelColor(i, 0, 0, 255);
                }
                LED.show();
                *stage = 0;
            }
        }
    }
}

void flash(uint32_t *lastmillis, uint8_t *stage, uint8_t length)
{

    unsigned long currentmillis = millis();

    if ((currentmillis - *lastmillis) >= 40)
    {
        *lastmillis = millis();
        if (*stage == 0)
        {
            for (int i = 0; i < length; i++)
            {
                LED.setPixelColor(i, 255, 255, 255);
            }
            LED.show();
            *stage = 1;
        }
        else
        {
            if (*stage == 1)
            {
                for (int i = 0; i < length; i++)
                {
                    LED.setPixelColor(i, 0, 0, 0);
                }
                LED.show();
                *stage = 0;
            }
        }
    }
}