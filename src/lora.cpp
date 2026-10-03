#include <Arduino.h>
#include <Adafruit_TinyUSB.h>
#include <Uart.h>
#include "lora.h"

//Configure pins and Uart for the lora module.
constexpr int loraReset = 13;
static Uart radio(NRF_UARTE1, UARTE1_IRQn, 11, 12);

//Initilizations status of the lora module. Set to true after successful initialization.
static bool loraInitialized = false;

//Command message flags. Set to true when a command is accepted or failed.
static bool commandAccepted = false;
static bool commandFailed = false;

//Stores the incoming line from the lora module.
static String incomingLine;
static bool discardLine = false;

//Stores the pending message received from the lora module including a message available flag.
static LoRaMessage pendingMessage;
static bool messageAvailable = false;

//Interrupt handler for the lora module. Calls the Uart interrupt handler to process incoming data.
extern "C" void UARTE1_IRQHandler(void)
{
    radio.IrqHandler();
}

// Handles both AT responses and received wireless messages.
static void processLine(const String& line)
{
    if (line == "+OK") {
        commandAccepted = true;
        return;
    }

    if (line.startsWith("+ERR")) {
        commandFailed = true;

        if (Serial) {
            Serial.println(line);
        }

        return;
    }

    if (!line.startsWith("+RCV=")) {
        return;
    }

    // Expected format:
    // +RCV=sender,length,payload,RSSI,SNR

    int firstComma = line.indexOf(',');
    if (firstComma < 0) return;

    int secondComma = line.indexOf(',', firstComma + 1);

    long sender = line.substring(5, firstComma).toInt();

    int payloadLength =
        line.substring(firstComma + 1, secondComma).toInt();

    if (sender < 0 || sender > 65535) return;
    if (payloadLength < 0 || payloadLength > 240) return;

    int payloadStart = secondComma + 1;
    int payloadEnd = payloadStart + payloadLength;

    if (payloadEnd >= (int)line.length()) return;
    if (line[payloadEnd] != ',') return;

    int rssiEnd = line.indexOf(',', payloadEnd + 1);
    if (rssiEnd < 0) return;

    pendingMessage.sender = (uint16_t)sender;

    pendingMessage.payload =
        line.substring(payloadStart, payloadEnd);

    pendingMessage.rssi =
        line.substring(payloadEnd + 1, rssiEnd).toInt();

    pendingMessage.snr =
        line.substring(rssiEnd + 1).toInt();

    messageAvailable = true;
}

// Updates the lora module by reading incoming data and processing complete lines.
void loraUpdate()
{
    unsigned int bytesRead = 0;

    while (radio.available() && bytesRead < 300) {
        bytesRead++;

        char c = radio.read();

        if (c == '\n') {
            if (!discardLine && incomingLine.length() > 0) {
                processLine(incomingLine);
            }

            incomingLine = "";
            discardLine = false;
        }
        else if (c != '\r' && !discardLine) {
            if (incomingLine.length() < 300) {
                incomingLine += c;
            } else {
                incomingLine = "";
                discardLine = true;
            }
        }
    }
}

// Sends a command to the lora module and waits for a response. Returns true if the command was accepted, false if it failed or timed out.
static bool loraCommand(const String& command)
{
    loraUpdate();

    commandAccepted = false;
    commandFailed = false;

    radio.print(command);
    radio.print("\r\n");

    unsigned long started = millis();

    while (millis() - started < 2000) {
        
        loraUpdate();

        if (commandFailed) {
            return false;
        }

        if (commandAccepted) {
            return true;
        }

        delay(1);
    }

    if (Serial) {
        Serial.print("LoRa command timeout: ");
        Serial.println(command);
    }

    return false;
}

// Initializes the lora module with the given address. Returns true on success, false on failure.
bool loraInit(uint16_t address)
{
    loraInitialized = false;
    messageAvailable = false;
    incomingLine = "";
    discardLine = false;

    pinMode(loraReset, OUTPUT);
    digitalWrite(loraReset, HIGH);

    radio.begin(115200);

    digitalWrite(loraReset, LOW);
    delay(150);
    digitalWrite(loraReset, HIGH);
    delay(1000);

    if (!loraCommand("AT")) return false;
    if (!loraCommand("AT+MODE=0")) return false;

    String addressCommand = "AT+ADDRESS=";
    addressCommand += address;

    if (!loraCommand(addressCommand)) return false;
    if (!loraCommand("AT+NETWORKID=18")) return false;
    if (!loraCommand("AT+PARAMETER=9,7,1,12")) return false;
    if (!loraCommand("AT+BAND=915000000,M")) return false;
    if (!loraCommand("AT+CRFOP=22")) return false;

    loraInitialized = true;
    return true;
}

bool loraIsInitialized()
{
    return loraInitialized;
}

// Sends a message to the specified destination address. Returns true if the message was accepted for sending, false otherwise.
bool loraSend(uint16_t destination, const String& payload)
{
    if (!loraInitialized) return false;

    if (payload.length() == 0 || payload.length() > 240) {
        return false;
    }

    for (size_t i = 0; i < payload.length(); i++) {
        if (payload[i] == '\r' ||
            payload[i] == '\n' ||
            payload[i] == '\0') {
            return false;
        }
    }

    String command = "AT+SEND=";
    command += destination;
    command += ",";
    command += payload.length();
    command += ",";
    command += payload;

    return loraCommand(command);
}

// Checks if a message has been received. If a message is available the function returns true. If no message is available, the function returns false.
bool loraReceive(LoRaMessage& message)
{
    if (!messageAvailable) {
        return false;
    }

    message = pendingMessage;
    messageAvailable = false;

    return true;
}