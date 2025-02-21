#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <WebServer.h>

int led0 = 15;
int led1 = 4;

// WiFi credentials
const char* ssid = "Sellamuthu";
const char* password = "36180402";
const char* apiKey = "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";

// Create WebServer object on port 80
WebServer server(80);

// HTML content to be served
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>DFRobot Chatbot</title>
  <style>
    body {
      background-color: #121212;
      color: #ffffff;
      font-family: Arial, sans-serif;
      text-align: center;
      padding: 20px;
    }
    #inputField, #answerField, #debugPanel {
      background-color: #333;
      color: #ffffff;
      border: 1px solid #555;
      border-radius: 10px;
      margin: 10px;
      padding: 15px;
      width: 80%;
      max-width: 500px;
      display: block;
      margin-left: auto;
      margin-right: auto;
      font-size: 1.1em;
    }
    #submitButton, #toggleDebugButton {
      background-color: #6200ee;
      color: #ffffff;
      border: none;
      border-radius: 10px;
      margin: 10px;
      padding: 15px 30px;
      font-size: 1.2em;
      cursor: pointer;
      transition: background-color 0.3s ease;
    }
    #submitButton:hover, #toggleDebugButton:hover {
      background-color: #3700b3;
    }
    #dfrobotLogo {
      width: 300px; /* Increased size */
      margin-top: 20px;
    }
    h1, h2 {
      color: #bb86fc;
    }
  </style>
</head>
<body>
  <img id="dfrobotLogo" src="https://th.bing.com/th/id/R.54efac417a0601c7ec9a065db1cf3e09?rik=X3ZTCpI2uP71jw&riu=http%3a%2f%2fthepihut.com%2fcdn%2fshop%2fcollections%2fdfrobot.jpg%3fv%3d1625593139%26width%3d2048&ehk=h1A%2bwwWukHZIUGkulQSSazwDhn%2bLm7koxyyOfo701A0%3d&risl=&pid=ImgRaw&r=0" alt="DFRobot Logo">
  <h1>DFRobot Beetle ESP32 C6 Chatbot</h1>
  <input id="inputField" type="text" placeholder="Enter your text here">
  <button id="submitButton" onclick="sendQuestion()">Submit</button>
  <div id="answerField"></div>
  <button id="toggleDebugButton" onclick="toggleDebugPanel()">Toggle Debug Panel</button>
  <h2>Debug Panel</h2>
  <div id="debugPanel" style="display:none;"></div>

  <script>
    async function sendQuestion() {
      const userQuestion = document.getElementById("inputField").value;
      const response = await fetch("/getQuestion", {
        method: "POST",
        headers: {
          "Content-Type": "application/json"
        },
        body: JSON.stringify({ question: userQuestion })
      });
      const data = await response.json();
      document.getElementById("answerField").innerText = data.choices[0].message.content; // Show the answer
      document.getElementById("debugPanel").innerText = JSON.stringify(data, null, 2); // Show complete API response
    }

    function toggleDebugPanel() {
      const debugPanel = document.getElementById("debugPanel");
      if (debugPanel.style.display === "none") {
        debugPanel.style.display = "block";
      } else {
        debugPanel.style.display = "none";
      }
    }
  </script>
</body>
</html>
)rawliteral";

// Function to process the user question and get the response
String processQuestion(String question) {
  Serial.print("User Question: ");
  Serial.println(question); // Print the user question

  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin("https://openrouter.ai/api/v1/chat/completions");
    http.addHeader("Content-Type", "application/json");
    http.addHeader("Authorization", String("Bearer ") + apiKey);

    StaticJsonDocument<512> jsonDoc;
    jsonDoc["model"] = "deepseek/deepseek-r1-distill-llama-70b";  // deepseek/deepseek-r1-distill-llama-70b //openai/gpt-4o-mini-2024-07-18
    JsonArray messages = jsonDoc.createNestedArray("messages");

    JsonObject systemMessage = messages.createNestedObject();
    systemMessage["role"] = "system";
    systemMessage["content"] = "Answer the user's question concisely and informatively.";

    JsonObject userMessage = messages.createNestedObject();
    userMessage["role"] = "user";
    userMessage["content"] = question;

    String requestBody;
    serializeJson(jsonDoc, requestBody);

    Serial.println("Sending HTTP POST request...");
    int httpResponseCode = http.POST(requestBody);
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);

    String response = http.getString();
    Serial.print("HTTP Response: ");
    Serial.println(response);

    StaticJsonDocument<1024> responseDoc;
    DeserializationError error = deserializeJson(responseDoc, response);

    if (!error) {
      String assistantResponse = responseDoc["choices"][0]["message"]["content"].as<String>();
      Serial.print("Assistant Response: ");
      Serial.println(assistantResponse); // Print the assistant response
      Serial1.println(assistantResponse);
      return response; // Return entire API response
    } else {
      return "Failed to parse JSON response.";
    }
  }
  return "WiFi not connected!";
}

void setup() {
  // Start Serial Monitor
  Serial.begin(115200);
  Serial1.begin(9600, SERIAL_8N1, /*rx =*/17, /*tx =*/16);
  pinMode(led0, OUTPUT);
  pinMode(led1, OUTPUT);

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("\nConnected to WiFi");

  // Print ESP32 Local IP Address
  Serial.print("ESP32 IP Address: ");
  Serial.println(WiFi.localIP());

  // Serve HTML content
  server.on("/", HTTP_GET, []() {
    server.send_P(200, "text/html", index_html);
  });

  // Handle POST request from the web page
  server.on("/getQuestion", HTTP_POST, []() {
    if (server.hasArg("plain")) {
      String body = server.arg("plain");

      // Debugging: Print the received body
      Serial.print("Received body: ");
      Serial.println(body);

      // Parse JSON from the received payload
      StaticJsonDocument<200> doc;
      DeserializationError error = deserializeJson(doc, body);

      if (error) {
        Serial.println("Failed to parse JSON from body");
        server.send(400, "application/json", "{\"error\":\"Invalid JSON\"}");
        return;
      }

      String userQuestion = doc["question"];
      Serial.print("User question: ");
      Serial.println(userQuestion);
      Serial1.println(userQuestion);

      String responseText = processQuestion(userQuestion);
      Serial.print("Response text: ");
      Serial.println(responseText);

      String jsonResponse = responseText; // Use entire API response as JSON response

      // Send response to the client
      server.send(200, "application/json", jsonResponse);
    } else {
      server.send(400, "application/json", "{\"error\":\"No body received\"}");
    }
  });

  // Start server
  server.begin();
  Serial.println("Server started");
}

void loop() {
  server.handleClient();

  // Blink LED
  digitalWrite(led0, HIGH);
  digitalWrite(led1, LOW);
  delay(100);
  digitalWrite(led0, LOW);
  digitalWrite(led1, HIGH);
  delay(100);
}
