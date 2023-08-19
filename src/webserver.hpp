#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <ESP8266WebServer.h>
#include <FS.h> // Include the SPIFFS library
#include "display.h"
#include "power.h"


#include "tabata.h"

#include "wifi_credentials.h"
#define AP_SSID "AakhadaTimer"
#define AP_PASS "ConnectHabibi"

ESP8266WebServer server(80); // Create a webserver object that listens for HTTP request on port 80

File fsUploadFile; // a File object to temporarily store the received file

String getContentType(String filename); // convert the file extension to the MIME type
bool handleFileRead(String path);       // send the right file to the client (if it exists)
void handleFileUpload();                // upload a new file to the SPIFFS

String uploadForm = R"(
<form method="post" enctype="multipart/form-data">
    <input type="file" name="name" multiple>
    <input class="button" type="submit" value="Upload">
</form>
)";

String getContentType(String filename)
{ // convert the file extension to the MIME type
    if (filename.endsWith(".html"))
        return "text/html";
    else if (filename.endsWith(".css"))
        return "text/css";
    else if (filename.endsWith(".js"))
        return "application/javascript";
    else if (filename.endsWith(".ico"))
        return "image/x-icon";
    else if (filename.endsWith(".gz"))
        return "application/x-gzip";
    else if (filename.endsWith(".svg"))
        return "image/svg+xml";
    return "text/plain";
}

bool handleFileRead(String path)
{ // send the right file to the client (if it exists)
    Serial.println("handleFileRead: " + path);
    if (path.endsWith("/"))
        path += "index.html";                  // If a folder is requested, send the index file
    String contentType = getContentType(path); // Get the MIME type
    String pathWithGz = path + ".gz";
    if (SPIFFS.exists(pathWithGz) || SPIFFS.exists(path))
    {                                                       // If the file exists, either as a compressed archive, or normal
        if (SPIFFS.exists(pathWithGz))                      // If there's a compressed version available
            path += ".gz";                                  // Use the compressed verion
        File file = SPIFFS.open(path, "r");                 // Open the file
        size_t sent = server.streamFile(file, contentType); // Send it to the client
        file.close();                                       // Close the file again
        Serial.println(String("\tSent file: ") + path);
        return true;
    }
    Serial.println(String("\tFile Not Found: ") + path); // If the file doesn't exist, return false
    return false;
}

void handleFileUpload()
{ // upload a new file to the SPIFFS
    HTTPUpload &upload = server.upload();
    if (upload.status == UPLOAD_FILE_START)
    {
        String filename = upload.filename;
        if (!filename.startsWith("/"))
            filename = "/" + filename;
        Serial.print("handleFileUpload Name: ");
        Serial.println(filename);
        fsUploadFile = SPIFFS.open(filename, "w"); // Open the file for writing in SPIFFS (create if it doesn't exist)
        filename = String();
    }
    else if (upload.status == UPLOAD_FILE_WRITE)
    {
        if (fsUploadFile)
            fsUploadFile.write(upload.buf, upload.currentSize); // Write the received bytes to the file
    }
    else if (upload.status == UPLOAD_FILE_END)
    {
        if (fsUploadFile)
        {                         // If the file was successfully created
            fsUploadFile.close(); // Close the file again
            Serial.print("handleFileUpload Size: ");
            Serial.println(upload.totalSize);
            server.sendHeader("Location", "/success.html"); // Redirect the client to the success page
            server.send(303);
        }
        else
        {
            server.send(500, "text/plain", "500: couldn't create file");
        }
    }
}

void handleTimerControl()
{
    String m_timer_state = server.arg("state");
    String response = "Timer state" + m_timer_state;

    if (server.hasArg("preset") && m_timer_state == "run")
    {
        startTimer(server.arg("preset").toInt());
    }
    else if (m_timer_state == "start")
    {
        playTimer();
    }
    else if (m_timer_state == "pause")
    {
        pauseTimer();
    }
    else if (m_timer_state == "stop")
    {
        stopTimer();
        sequenceStop();
    }
    else if (m_timer_state == "next")
    {
        sequenceNext();
    }
    else if (m_timer_state == "sequence")
    {
        playTimer(-1);
    }

    response = get_live_json();

    server.send(200, "application/json", response);
}

void handleSave()
{
    String response = "";
    if (server.hasArg("info"))
    {
        response = get_eeprom_json();
    }
    else
    {
        if(server.hasArg("warmUpSequence") && server.hasArg("basicSequence") && server.hasArg("regularSequence")){
            assignEepromSequence(
                server.arg("warmUpSequence"), 
                server.arg("basicSequence"), 
                server.arg("regularSequence")
                );
        }

        if(server.hasArg("initialCountdown")){
            assignEepromTimer(
                {server.arg("initialCountdown").toInt(),
                server.arg("workTime").toInt(),
                server.arg("restTime").toInt(),
                server.arg("recoveryTime").toInt(),
                server.arg("sets").toInt(),
                server.arg("cycles").toInt()},
                server.arg("timerName").toInt());
        }
        
        updateEeprom();
        response = get_eeprom_json();
        Serial.println(response);
    }

    server.send(200, "application/json", response);
}
void handleLive()
{
    String response = get_live_json();
    server.send(200, "application/json", response);
}
void handleInfo()
{
    String response;
    if (server.hasArg("info"))
    {
        response = get_stats_json();
    }
    if (server.hasArg("beepToggle")){
        beep_toggle();
    }
    if (server.hasArg("powerOff")){
        deviceSleep();        
    }
    server.send(200, "application/json", response);
}
uint8_t wifi_connect_counter = 0;
bool wifi_loop(){
    if (WiFi.status() == WL_DISCONNECTED)
    {
        DBG_LOGD(wifi_connect_counter);
        if(wifi_connect_counter == 255){
            wifi_connect_counter = 0;
            return false;
        }
        wifi_connect_counter++;
    }
    else if(WiFi.status() == WL_CONNECTED){
        return true;
    }
    DBG_LOGV(WiFi.status());
    return true;
}
void webserver_init()
{
    WiFi.mode(WIFI_AP_STA);
    WiFi.softAP(AP_SSID, AP_PASS);
    WiFi.begin(WIFI_SSID, WIFI_PASS);

    MDNS.begin("esp8266");

    SPIFFS.begin(); // Start the SPI Flash Files System

    server.on("/upload", HTTP_GET, []() {              // if the client requests the upload page
        if (!handleFileRead("/upload.html"))           // send it if it exists
            server.send(200, "text/html", uploadForm); // otherwise, respond with a 404 (Not Found) error
    });

    server.on(
        "/upload", HTTP_POST, // if the client posts to the upload page
        []()
        { server.send(200); }, // Send status 200 (OK) to tell the client we are ready to receive
        handleFileUpload       // Receive and save the file
    );

    server.onNotFound([]() {                                  // If the client requests any URI
        if (!handleFileRead(server.uri()))                    // send it if it exists
            server.send(404, "text/plain", "404: Not Found"); // otherwise, respond with a 404 (Not Found) error
    });

    server.on("/save", handleSave);
    server.on("/live", handleLive);
    server.on("/timerControl", handleTimerControl);
    server.on("/info", handleInfo);

    server.begin(); // Actually start the server
    Serial.println("HTTP server started");
}

void webserver_loop()
{
    server.handleClient();
}