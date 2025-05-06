#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <SoftwareSerial.h>

SoftwareSerial NodeMCU(D2, D3); // RX = D2, TX = D3

const char* ssid = "Sk";
const char* password = "shiv76543";

ESP8266WebServer server(80);
int mode = 1;

void setup() {
  Serial.begin(9600);
  NodeMCU.begin(9600);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nConnected to WiFi");
  Serial.print("ESP IP: ");
  Serial.println(WiFi.localIP());

  server.on("/", handleRoot);
  server.on("/control", HTTP_GET, handleControl);
  server.begin();
}

void loop() {
  server.handleClient();
  if (NodeMCU.available()) {
    char c = NodeMCU.read();
    Serial.print("From Arduino: ");
    Serial.println(c);
  }
}

void handleRoot() {
  String html = "<html><head><style>";
  html += "body {font-family: Arial, sans-serif; text-align: center; background-color: #f4f4f4; padding: 20px;}";
  html += "h1 {color: #333;}";
  html += "button {padding: 15px; margin: 10px; font-size: 18px; border: none; border-radius: 8px; cursor: pointer; width: 180px;}";
  html += ".mode-btn {background-color: #007BFF; color: white;}";
  html += ".mode-btn:hover {background-color: #0056b3;}";
  html += ".control-btn {background-color: #28a745; color: white;}";
  html += ".control-btn:hover {background-color: #218838;}";
  html += ".stop-btn {background-color: #dc3545; color: white;}";
  html += ".stop-btn:hover {background-color: #bd2130;}";
  html += "#speedSlider {width: 200px;}";
  html += "#speedValue {font-size: 18px; font-weight: bold; margin-left: 10px;}";
  html += "</style></head><body>";
  
  html += "<h1>ESP8266 Robot Control</h1>";
  html += "<p><b>Current Mode:</b> " + String(mode == 1 ? "Autonomous" : "Web Control") + "</p>";
  html += "<button class='mode-btn' onclick=\"sendRequest('1')\">Autonomous Mode</button>";
  html += "<button class='mode-btn' onclick=\"sendRequest('2')\">Web Control Mode</button><br><br>";

  if (mode == 2) {
    html += "<button class='control-btn' onclick=\"sendRequest('F')\">Forward</button><br>";
    html += "<button class='control-btn' onclick=\"sendRequest('L')\">Left</button>";
    html += "<button class='stop-btn' onclick=\"sendRequest('S')\">Stop</button>";
    html += "<button class='control-btn' onclick=\"sendRequest('R')\">Right</button><br>";
    html += "<button class='control-btn' onclick=\"sendRequest('B')\">Backward</button><br><br>";
    
    // Speed Control Slider
    html += "<label><b>Speed:</b></label><br>";
    html += "<input type='range' min='0' max='255' value='255' id='speedSlider' oninput='updateSpeed(this.value)'>";
    html += "<span id='speedValue'>255</span>";
  }

  html += "<script>";
  html += "function sendRequest(cmd){fetch('/control?cmd='+cmd);}";
  html += "function updateSpeed(val){";
  html += "document.getElementById('speedValue').innerText = val;";
  html += "fetch('/control?cmd=SPEED'+val);";
  html += "}";
  html += "</script></body></html>";

  server.send(200, "text/html", html);
}

void handleControl() {
  if (server.hasArg("cmd")) {
    String cmd = server.arg("cmd");
    
    if (cmd.startsWith("SPEED")) {
      int newSpeed = cmd.substring(5).toInt();
      NodeMCU.write('V');  // 'V' signals speed change
      NodeMCU.write(newSpeed);
    } else {
      char command = cmd[0];
      if (command == '1' || command == '2') mode = command - '0';
      NodeMCU.write(command);
    }

    Serial.println("Sent: " + cmd);
    server.send(200, "text/plain", "Command Sent: " + cmd);
  }
}