#include "config.h"

DynamicJsonDocument doc(1024);

enum pattern
{
    NONE,
    RAINBOW_CYCLE,
    FLASH,
    FADE,
    STATIC
};

class NeoPatterns : public Adafruit_NeoPixel
{
public:
    // Member Variables:
    pattern ActivePattern; // which pattern is running

    unsigned long Interval;   // milliseconds between updates
    unsigned long lastUpdate; // last update of position

    uint32_t Color1, Color2; // What colors are in use
    uint16_t TotalSteps;     // total number of steps in the pattern
    uint16_t Index;          // current step within the pattern

    // Constructor - calls base-class constructor to initialize strip
    NeoPatterns(uint16_t pixels, uint8_t pin, uint8_t type) : Adafruit_NeoPixel(pixels, pin, type) {}
    NeoPatterns() : Adafruit_NeoPixel(1, D4, NEO_GRB + NEO_KHZ800) {}

    void update()
    {
        if ((millis() - lastUpdate) > Interval) // time to update
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
            case STATIC:
                StaticUpdate();
                break;
            default:
                break;
            }
        }
    }
    // Increment the Index and reset at the end
    void Increment()
    {
        Index++;
        if (Index >= TotalSteps)
        {
            Index = 0;
        }
    }

    // Initialize for a RainbowCycle
    void RainbowCycle(uint8_t interval)
    {
        ActivePattern = RAINBOW_CYCLE;
        Interval = interval;
        TotalSteps = 256;
        Index = 0;
    }

    // Update the Rainbow Cycle Pattern
    void RainbowCycleUpdate()
    {
        setPixelColor(0, Wheel(Index & 255));
        show();
        Increment();
    }

    // Initialize for Flash
    void Flash(uint8_t interval, uint32_t c1, uint32_t c2 = Color(0, 0, 0))
    {
        ActivePattern = FLASH;
        Interval = interval;
        TotalSteps = 2;
        Index = 0;
        Color1 = c1;
        Color2 = c2;
    }

    // Update the Flash Pattern
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

    // Initialize for a Fade
    void Fade(uint8_t interval, uint16_t steps, uint32_t c1, uint32_t c2 = Color(0, 0, 0))
    {
        ActivePattern = FADE;
        TotalSteps = steps;
        Interval = interval;
        Index = 0;
        Color1 = c1;
        Color2 = c2;
    }

    // Update the Fade Pattern
    void FadeUpdate()
    {
        // Calculate linear interpolation between Color1 and Color2
        // Optimise order of operations to minimize truncation error
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

    /// Initialize for a Static
    void Static(uint32_t c1)
    {
        ActivePattern = STATIC;
        Color1 = c1;
    }

    void StaticUpdate()
    {
        ColorSet(Color1);
        show();
    }

    // Input a value 0 to 255 to get a color value.
    // The colours are a transition r - g - b - back to r.
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

    // Returns the Red component of a 32-bit color
    uint8_t Red(uint32_t color)
    {
        return (color >> 16) & 0xFF;
    }

    // Returns the Green component of a 32-bit color
    uint8_t Green(uint32_t color)
    {
        return (color >> 8) & 0xFF;
    }

    // Returns the Blue component of a 32-bit color
    uint8_t Blue(uint32_t color)
    {
        return color & 0xFF;
    }

    // Set all pixels to a color (synchronously)
    void ColorSet(uint32_t color)
    {
        for (int i = 0; i < numPixels(); i++)
        {
            setPixelColor(i, color);
        }
        show();
    }
};

class Requester
{
public:
    unsigned long pollInterval; // milliseconds between updates
    unsigned long lastUpdate;   // last update of position
    uint8_t modus;
    uint8_t lastModus;
    uint8_t timeouts = 0;
    NeoPatterns LED = NeoPatterns(NUMLEDS, LEDPIN, NEO_GRB + NEO_KHZ800);

    //Querry URL
    const char *prefix = "http://";
    const char *postfix = "/printer/objects/query?print_stats";
    String url = prefix + PRINTER_IP + postfix;
    const char *URL = url.c_str();

    ESP8266WiFiMulti WiFiMulti;

    Requester()
    {
        // LED = l;
        WiFi.mode(WIFI_STA);
        WiFiMulti.addAP(SSID, WPWD);
    };

    void setup()
    {
        LED.begin();
        modus = 1;
    }

    /* Modes:
        0 Error Red Fade IN/Out
        1 Wifi not connected White Flash
        2 No response White Static
        3 Printing Green/Yellow Fade
        4 Standby Rainbow
        5 Paused Blue Fade IN/Out
        6 Complete  Fade Purple
    */

    void update()
    {
        LED.update();

        if (millis() - lastUpdate > pollInterval)
        {
            lastUpdate = millis();
            // wait for WiFi connection
            if (WiFiMulti.run() == WL_CONNECTED)
            {
                WiFiClient c;
                HTTPClient http;
                http.begin(c, URL);
                int httpCode = http.GET();

                if (httpCode > 0)
                {
                    timeouts = 0;
                    String payload = http.getString();
                    deserializeJson(doc, payload);
                    JsonObject payloadObject = doc.as<JsonObject>();
                    if (httpCode == HTTP_CODE_OK)
                    {
                        String state = payloadObject["result"]["status"]["print_stats"]["state"];
                        //USE_SERIAL.println(state);
                        if (state == "error")
                        {
                            modus = 0; // red
                        }
                        else if (state == "printing")
                        {
                            modus = 3; // yellow
                        }
                        else if (state == "standby")
                        {
                            modus = 4; // rainbow
                        }
                        else if (state == "paused")
                        {
                            modus = 5; // blue
                        }
                        else if (state == "complete")
                        {
                            modus = 6; // violet
                        }
                    }
                }
                else
                {
                    timeouts++;
                    if (timeouts > 10)
                    {
                        modus = 2;
                    }
                }
            }
            else
            {
                modus = 1;
            }
        }
        if (modus != lastModus)
        {
            USE_SERIAL.println(modus);
            switch (modus)
            {
            case 0:
            {
                LED.Fade(1, 16, LED.Color(255, 0, 0), LED.Color(128, 0, 0));
                break;
            }
            case 1:
            {
                LED.Flash(50, LED.Color(255, 255, 255), LED.Color(0, 0, 0));
                break;
            }
            case 2:
            {
                LED.Static(LED.Color(255, 255, 255));
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
                LED.Fade(1, 16, LED.Color(0, 0, 255), LED.Color(0, 0, 128));
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
        lastModus = modus;
    }
};

Requester Req;

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