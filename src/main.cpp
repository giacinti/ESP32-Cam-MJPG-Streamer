#include "OV2640.h"
#include <WiFi.h>
#include <WebServer.h>
#include <WiFiClient.h>
#include <AutoConnect.h>

#define BOUNDARY String("--boundarydonotcross")

OV2640 cam;
WebServer server(80);
AutoConnect portal(server);

void message(String msg)
{
    Serial.println(msg);
}

void handle_stream(void)
{
	#if 1
	String args=server.arg(0);
	message("arg, name="+server.argName(0)+" value="+server.arg(0));
	#endif
	
    WiFiClient client = server.client();
    String response = "HTTP/1.1 200 OK\r\n";
    response += "Content-Type: multipart/x-mixed-replace; boundary="+BOUNDARY+"\r\n\r\n";
    server.sendContent(response);

    while (1)
    {
        cam.run();
        if (!client.connected())
            break;
        response = BOUNDARY+"\r\n";
        response += "Content-Type: image/jpeg\r\n";
        response += "Content-Length: " + String(cam.getSize()) + "\r\n\r\n";
        server.sendContent(response);

        client.write((char *)cam.getfb(), cam.getSize());
        server.sendContent("\r\n");
        if (!client.connected())
            break;
    }
}

void handle_image(void)
{
	WiFiClient client = server.client();

    cam.run();
    if (!client.connected())
        return;
    String response = "HTTP/1.1 200 OK\r\n";
    response += "Content-disposition: inline\r\n";
    response += "Content-type: image/jpeg\r\n";
    response += "Content-Length: " + String(cam.getSize()) + "\r\n\r\n";
    server.sendContent(response);
    client.write((char *)cam.getfb(), cam.getSize());
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


void setup() {
	Serial.begin(115200);
    while (!Serial)
    {
        ;
    }

    esp32cam_aithinker_config.xclk_freq_hz = 10000000;
    esp32cam_aithinker_config.frame_size = FRAMESIZE_VGA;
    esp32cam_aithinker_config.jpeg_quality = 10;
    /*int camInit = */cam.init(esp32cam_aithinker_config);
   
    //server.on("/", HTTP_GET, handle_stream);
	server.on("/image", HTTP_GET, handle_image);
    server.onNotFound(handleNotFound);
	
    portal.begin();
    message(WiFi.localIP().toString());
}

void loop() {
	portal.handleClient();
}