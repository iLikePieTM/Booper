// Adding the OLED libraries and the bitmap images
#include <Adafruit_SSD1306.h>
#include "bitmaps.cpp"

// Setting up the SPI interface for the OLED display
#define OLED_CS     D0
#define OLED_DC     D1
#define OLED_RESET  D2
Adafruit_SSD1306 display(OLED_DC, OLED_RESET, OLED_CS);

// Device OLED screen state information.
// - 0, Standby
// - 1, Sending/Beep
// - 2, Delivered
// - 3, Receved
// - 4, Reading
int screenState, lastScreenState = 0;

// Text scrolling vars and heartAnimate counter
int location, scrolLength;
bool textLoop = true;
int breathe = 0;

// Device LED light state information
// - 0, Off
// - 1, On
int lightState = 0;

// Define light Pin
int light = D3;

// Setting up the D6 and D5 pins to be used with buttons
int beepButton = D5;
int lightButton = D6;
bool beepButtonLast, lightButtonLast = false;


void setup() {

    // Initalize display with Adafruit library
    display.begin(SSD1306_SWITCHCAPVCC);
    //display.dim(1);

    // Splash Screen
    display.setTextColor(WHITE);
    display.clearDisplay();
    display.print("Made by Aex Hoppe");
    display.display();
    delay(1750);

    // Define text settings
    display.setTextWrap(false);
    scrolLength = -500;
    display.clearDisplay();
    display.display();

    // Set button to an input and light as output
    pinMode(beepButton, INPUT);
    pinMode(lightButton, INPUT);
    pinMode(light, OUTPUT);

    // Allows the device's states to be accessed from the cloud
    Particle.variable("screenState", &screenState, INT);
    Particle.variable("lightState", &lightState, INT);
    Particle.publish("boopDeviceState","beepStandby");
    Particle.publish("boopDeviceLight","off");

    // Gets update when boop device updates state
    Particle.subscribe("beepDeviceState", stateHandler);
    Particle.subscribe("beepDeviceLight", lightHandler);

    // Manual state control from the cloud for testing
    Particle.function("deviceState", setStateOnline);

    // Turns off the onboard LED
    RGB.control(true);
}


void loop() {

    // OLED screen control
    display.clearDisplay();

    if (screenState == 1 || !textLoop) {
        // Sending a beep
        if (lastScreenState != screenState && textLoop) {
            Particle.publish("boopDeviceState","boopSent");
            lastScreenState = screenState;

            location = display.width()*3;
            display.setTextSize(4);
            textLoop = false;
        }

        display.setCursor(location/2, 12);
        display.print("Boop");

        location -= 1;

        if (location <= scrolLength) {
            location = display.width()*2;
            // textLoop ensures that the Boop splash screen can be seen
            textLoop = true;
        }

    } else if (screenState == 2) {
        // Beep has been sent
        if (lastScreenState != screenState) {
            lastScreenState = screenState;
            Particle.publish("boopDeviceState","boopDelivered");
            display.setTextSize(1);
        }

        display.setCursor(0, 56);
        display.print("Boop sent...");

        heartAnimate();

    } else if (screenState == 3) {
        // Boop has been receved
        if (lastScreenState != screenState) {
            lastScreenState = screenState;
            Particle.publish("boopDeviceState","beepReceved");

            location = display.width()*2;
            display.setTextSize(4);
        }

        display.setCursor(location/2, 12);
        display.print("Beep");

        location -= 1;

        if (location <= scrolLength) {
            location = display.width()*2;
        }

        if (wasPressed(beepButton, &beepButtonLast)) {
            screenState = 4;
        }

    } else if (screenState == 4) {
        // Boop has been read
        lastScreenState = screenState;
        Particle.publish("boopDeviceState","beepRead");

        screenState = 0;

    } else {
        // Just chilling
        if (lastScreenState != screenState) {
            lastScreenState = screenState;
            Particle.publish("boopDeviceState1298","boopStandby");
        }
        if (wasPressed(beepButton, &beepButtonLast)) {
            screenState = 1;
        }
        heartAnimate();
    }

    display.display();

    // Light controll
    if(wasPressed(lightButton, &lightButtonLast)) {
        if (lightState) {
            lightState = 0;
            Particle.publish("boopDeviceLight","off");
        } else {
            lightState = 1;
            Particle.publish("boopDeviceLight","on");
        }
    }

    if (lightState) {
        digitalWrite(light, HIGH);
    } else {
        digitalWrite(light, LOW);
    }

}


// stateHandler is called when the cloud tells us that a boop event is published.
void stateHandler(const char *event, const char *data) {
    if (!strcmp(data,"beepSent")) {
        // Incoming beep
        screenState = 3;
    } else if (!strcmp(data,"boopRead")) {
        // Our boop has been read
        screenState = 0;
    } else if (!strcmp(data,"boopReceved")) {
        // Our boop has been noted but not read
        screenState = 2;
    }
}


// light Handler is called when the cloud tells us that the beep's light is toggled
void lightHandler(const char *event, const char *data) {
    if (!strcmp(data,"on")) {
        // Beep light was turned on
        lightState = 1;
    } else if (!strcmp(data,"off")) {
        // Beep light was turned off
        lightState = 0;
    }
}


// Runs the heart animation
void heartAnimate() {

    if (breathe <= 100) {
        display.drawBitmap(38, 8, heartBitmap0, 48, 48, WHITE);
    } else if (breathe <= 200 || (breathe > 700 && breathe <= 800)) {
        display.drawBitmap(39, 9, heartBitmap1, 46, 46, WHITE);
    } else if (breathe <= 300 || (breathe > 600 && breathe <= 700)) {
        display.drawBitmap(40, 10, heartBitmap2, 44, 44, WHITE);
    } else if (breathe <= 400 || (breathe > 500 && breathe <= 600)) {
        display.drawBitmap(41, 11, heartBitmap3, 42, 42, WHITE);
    } else if (breathe <= 500) {
        display.drawBitmap(42, 12, heartBitmap4, 40, 40, WHITE);
    } else {
        display.drawBitmap(38, 8, heartBitmap0, 48, 48, WHITE);
        breathe = 0;
    }

    breathe++;
}


//Checks for button press
bool wasPressed (int button, bool *lastState) {
    if (!digitalRead(button)) {
        *lastState = false;
        return false;
    } else if (digitalRead(button) && !*lastState){
        *lastState = true;
        return true;
    } else {
        return false;
    }
}


// Manual state controll from the cloud for testing
int setStateOnline(String state) {
    if (state=="0") {
        screenState = 0;
        return 0;
    } else if (state=="1") {
        screenState = 1;
        return 1;
    } else if (state=="2") {
        screenState = 2;
        return 2;
    } else if (state=="3") {
        screenState = 3;
        return 3;
    } else if (state=="4") {
        screenState = 4;
        return 4;
    } else if (state=="5") {
        lightState = 0;
        return 0;
    } else if (state=="6") {
        lightState = 1;
        return 1;
    } else {
        return -1;
    }
}
