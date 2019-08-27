#include "OV2640.h"
#include <WiFi.h>
#include <WebServer.h>
#include <WiFiClient.h>
#include <AutoConnect.h>

OV2640 cam;
WebServer server(80);
AutoConnect portal(server);

void handle_stream(void)
{
    WiFiClient client = server.client();
    String response = "HTTP/1.1 200 OK\r\n";
    response += "Content-Type: multipart/x-mixed-replace; boundary=frame\r\n\r\n";
    server.sendContent(response);

    while (1)
    {
        cam.run();
        if (!client.connected())
            break;
        response = "--frame\r\n";
        response += "Content-Type: image/jpeg\r\n";
        response += "Content-Length: " + String(cam.getSize()) + "\r\n\r\n";
        server.sendContent(response);

        client.write((char *)cam.getfb(), cam.getSize());
        server.sendContent("\r\n");
        if (!client.connected())
            break;
    }
}

void handleNotFound()
{
    String message = "Server is running!\n\n";
    message += "URI: ";
    message += server.uri();
    message += "\nMethod: ";
    message += (server.method() == HTTP_GET) ? "GET" : "POST";
    message += "\nArguments: ";
    message += server.args();
    message += "\n";
    server.send(200, "text/plain", message);
}

void message(String msg)
{
    Serial.println(msg);
}

void setup() {
	Serial.begin(115200);
    while (!Serial)
    {
        ;
    }

    esp32cam_aithinker_config.xclk_freq_hz = 10000000;
    esp32cam_aithinker_config.frame_size = FRAMESIZE_VGA;
    esp32cam_aithinker_config.jpeg_quality = 10;
    int camInit = cam.init(esp32cam_aithinker_config);
   
    server.on("/", HTTP_GET, handle_stream);
    server.onNotFound(handleNotFound);
	
    portal.begin();
    message(WiFi.localIP().toString());
}

void loop() {
	portal.handleClient();
}