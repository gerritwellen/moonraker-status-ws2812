#include <Arduino.h>
#include "config.h"

DynamicJsonDocument doc(1024);

/// Pattern types supported:
enum pattern
{
    NONE,
    RAINBOW_CYCLE,
    FLASH,
    FADE
};

/// NeoPatterns Class - derived from the Adafruit_NeoPixel class
class NeoPatterns : public Adafruit_NeoPixel
{
public:
    /// Member Variables:
    pattern ActivePattern; /// which pattern is running

    unsigned long Interval;   /// milliseconds between updates
    unsigned long lastUpdate; /// last update of position

    uint32_t Color1, Color2; /// What colors are in use
    uint16_t TotalSteps;     /// total number of steps in the pattern
    uint16_t Index;          /// current step within the pattern

    /// Constructor - calls base-class constructor to initialize strip
    NeoPatterns(uint16_t pixels, uint8_t pin, uint8_t type) : Adafruit_NeoPixel(pixels, pin, type) {}
    NeoPatterns() : Adafruit_NeoPixel(1, D4, NEO_GRB + NEO_KHZ800) {}

    /// Update the pattern
    void update()
    {
        if ((millis() - lastUpdate) > Interval) /// time to update
        {
            lastUpdate = millis();
            switch (ActivePattern)
            {
            case RAINBOW_CYCLE:
                RainbowCycleUpdate();
                break;
            case FLASH:
                FlashUpdate();
                break;
            case FADE:
                FadeUpdate();
                break;
            default:
                break;
            }
        }
    }

    /// Increment the Index and reset at the end
    void Increment()
    {
        Index++;
        if (Index >= TotalSteps)
        {
            Index = 0;
        }
    }

    /// Initialize for a RainbowCycle
    void RainbowCycle(uint8_t interval)
    {
        ActivePattern = RAINBOW_CYCLE;
        Interval = interval;
        TotalSteps = 256;
        Index = 0;
    }

    /// Update the Rainbow Cycle Pattern
    void RainbowCycleUpdate()
    {
        setPixelColor(0, Wheel(Index & 255));

        show();
        Increment();
    }

    /// Initialize for a Flash
    void Flash(uint8_t interval, uint32_t c1, uint32_t c2 = Color(0, 0, 0))
    {
        ActivePattern = FLASH;
        Interval = interval;
        TotalSteps = 2;
        Index = 0;
        Color1 = c1;
        Color2 = c2;
    }

    /// Update the Flash Pattern
    void FlashUpdate()
    {
        if (Index == 0)
        {
            setPixelColor(0, Color1);
        }
        else
        {
            setPixelColor(0, Color2);
        }
        show();
        Increment();
    }

    /// Initialize for a Fade
    void Fade(uint8_t interval, uint16_t steps, uint32_t c1, uint32_t c2 = Color(0, 0, 0))
    {
        ActivePattern = FADE;
        TotalSteps = steps;
        Interval = interval;
        Index = 0;
        Color1 = c1;
        Color2 = c2;
    }

    /// Update the Fade Pattern
    void FadeUpdate()
    {

        uint16_t FadeIndex;
        uint16_t HalfSteps = TotalSteps / 2;
        if (Index >= (HalfSteps))
        {
            FadeIndex = TotalSteps - Index;
        }
        else
        {
            FadeIndex = Index;
        }

        uint8_t red = ((Red(Color1) * (HalfSteps - FadeIndex)) + (Red(Color2) * FadeIndex)) / TotalSteps;
        uint8_t green = ((Green(Color1) * (HalfSteps - FadeIndex)) + (Green(Color2) * FadeIndex)) / TotalSteps;
        uint8_t blue = ((Blue(Color1) * (HalfSteps - FadeIndex)) + (Blue(Color2) * FadeIndex)) / TotalSteps;

        ColorSet(Color(red, green, blue));

        show();
        Increment();
    }

    /// Input a value 0 to 255 to get a color value.
    /// The colours are a transition r - g - b - back to r.
    uint32_t Wheel(byte WheelPos)
    {
        WheelPos = 255 - WheelPos;
        if (WheelPos < 85)
        {
            return Color(255 - WheelPos * 3, 0, WheelPos * 3);
        }
        else if (WheelPos < 170)
        {
            WheelPos -= 85;
            return Color(0, WheelPos * 3, 255 - WheelPos * 3);
        }
        else
        {
            WheelPos -= 170;
            return Color(WheelPos * 3, 255 - WheelPos * 3, 0);
        }
    }

    /// Returns the Red component of a 32-bit color
    uint8_t Red(uint32_t color)
    {
        return (color >> 16) & 0xFF;
    }

    /// Returns the Green component of a 32-bit color
    uint8_t Green(uint32_t color)
    {
        return (color >> 8) & 0xFF;
    }

    /// Returns the Blue component of a 32-bit color
    uint8_t Blue(uint32_t color)
    {
        return color & 0xFF;
    }

    /// Set all pixels to a color (synchronously)
    void ColorSet(uint32_t color)
    {
        for (int i = 0; i < numPixels(); i++)
        {
            setPixelColor(i, color);
        }
        show();
    }
};

/// Requester Class - handles Requests to Moonraker and interprets the output
class Requester
{
private:
    NeoPatterns LED = NeoPatterns(NUMLEDS, LEDPIN, NEO_GRB + NEO_KHZ800); /// NeoPatterns Object to interact with
    const char *prefix = "http:///";                                      ///URL prefix
    const char *postfix = "/printer/objects/query?print_stats";           /// URL postfix
    String url = prefix + PRINTER_IP + postfix;                           /// URL to poll
    ESP8266WiFiMulti WiFiMulti;                                           /// Wifi Object

public:
    unsigned long pollInterval; /// milliseconds between updates
    unsigned long lastUpdate;   /// last request to Moonraker
    uint8_t modus = 7;          /// modus
    uint8_t lastModus;          /// modus before the request to detect change

    const char *URL = url.c_str(); /// Char Array of URL

    /// Constructor - initialize WIFI-Connection
    Requester()
    {

        WiFi.mode(WIFI_STA);
        WiFiMulti.addAP(SSID, WPWD);
    };

    /// Setup function to start the LED Strip
    void setup()
    {
        LED.begin();
        LED.Flash(200, LED.Color(255, 255, 255));
    }

    /// Make request to Moonraker and update the modus
    void update()
    {
        LED.update();

        if (millis() - lastUpdate > pollInterval)
        {
            lastUpdate = millis();
            lastModus = modus;

            if (WiFiMulti.run() == WL_CONNECTED)
            {
                WiFiClient c;
                HTTPClient http;
                http.begin(c, URL);
                int httpCode = http.GET();

                if (httpCode > 0)
                {
                    String payload = http.getString();
                    deserializeJson(doc, payload);
                    JsonObject payloadObject = doc.as<JsonObject>();
                    if (httpCode == HTTP_CODE_OK)
                    {
                        String state = payloadObject["result"]["status"]["print_stats"]["state"];

                        if (state == "printing")
                        {
                            modus = 3;
                        }
                        else if (state == "paused")
                        {
                            modus = 2;
                        }
                        else if (state == "error")
                        {
                            modus = 0;
                        }
                        else if (state == "standby")
                        {
                            modus = 4;
                        }
                        else if (state == "complete")
                        {
                            modus = 6;
                        }
                    }
                }
            }
        }
        if (modus != lastModus)
        {
            switch (modus)
            {
            case 0:
            {
                LED.Fade(1, 16, LED.Color(255, 0, 0), LED.Color(128, 0, 0));
                break;
            }
            case 1:
            {
                LED.Fade(1, 16, LED.Color(0, 255, 0), LED.Color(0, 128, 0));
                break;
            }

            case 2:
            {
                LED.Fade(1, 16, LED.Color(0, 0, 255), LED.Color(0, 0, 128));
                break;
            }
            case 3:
            {
                LED.Fade(1, 16, LED.Color(0, 255, 0), LED.Color(255, 255, 0));
                break;
            }
            case 4:
            {
                LED.RainbowCycle(3);
                break;
            }
            case 5:
            {
                LED.Flash(200, LED.Color(255, 0, 0), LED.Color(0, 0, 255));
                break;
            }
            case 6:
            {
                LED.Fade(1, 16, LED.Color(206, 94, 201), LED.Color(255, 117, 248));
                break;
            }
            default:
            {
                break;
            }
            }
        }
    }
};

Requester Req;
void setup();
void loop();
void setup()
{
    USE_SERIAL.begin(115200);
    USE_SERIAL.println("");
    USE_SERIAL.println("Setup start");
    Req.setup();
    USE_SERIAL.println("Setup finished");
}

void loop()
{
    Req.update();
}