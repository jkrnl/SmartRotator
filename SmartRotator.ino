#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebSrv.h>
#include <AsyncElegantOTA.h>
#include <esp32-hal.h>
#include <math.h>

#define PIN_PUL 21
#define PIN_DIR 22
#define PIN_ENA 23
#define PIN_PEND 18
#define PIN_ALM 19

#define PIN_LED 2

#define GEAR_RATIO 80        // n:1
#define FULL_TURN_STEPS 200  // 200 steps to 360 deg
#define ONE_STEP_DEG 1.8     // 360 deg per FULL_TURN_STEPS

#define CALIBRATION_DEG_DEV 0  // degs to compensate

const char* ssid = "<yourssid>";
const char* password = "<yourpass>";
const char* paramName = "target";
int targetHeadingInDeg = 0;
int actualHeadingInDeg = 0;
int lastActualHeadingInDeg = 0;
int actualHeadingInSteps = 0;
bool isRotating = false;
int stepsYet = 0;
bool directionCw = true;
bool toHalt = false;

AsyncWebServer server(80);
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">

<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Heading Angle Line</title>
    <style>
        .canvas-container {
            display: inline-block;
            border: 2px solid green;
            border-radius: 50%;
            overflow: hidden;
            position: relative;
        }

        canvas {
            display: block;
            background-color: lightgreen;
        }

        .direction-label {
            position: absolute;
            font-weight: bold;
            font-size: 16px;
            transform-origin: center;
        }

        .north {
            top: 10px;
            left: 50%;
            transform: translateX(-50%);
        }

        .northwest {
            top: 10px;
            left: 10px;
        }

        .northeast {
            top: 10px;
            right: 10px;
        }

        .south {
            bottom: 10px;
            left: 50%;
            transform: translateX(-50%);
        }

        .southwest {
            bottom: 10px;
            left: 10px;
        }

        .southeast {
            bottom: 10px;
            right: 10px;
        }

        .west {
            top: 50%;
            left: 10px;
            transform: translateY(-50%);
        }

        .east {
            top: 50%;
            right: 10px;
            transform: translateY(-50%);
        }
    </style>
</head>
<body>
    <h1>SmartRotator v1.0</h1>
    <label for="angleInput">Target heading</label>
    <input type="number" id="angleInput" min="0" max="360" step="1">
    <span>deg</span>
    <button id="updateButton" default style="background-color: green; color: black; border: none; cursor: pointer; border-radius: 4px;font-size: 16px; padding: 10px 20px; " >Go!</button>
    <button id="haltButton" style="background-color: red; color: yellow; font-size: 16px; padding: 20px 30px; border: none; cursor: pointer; border-radius: 10px; margin-left: 20px;">Halt</button>

    <p>Actual heading <span id="movingAngleLabel">0</span> deg</p>

    <div class="canvas-container">
        <canvas id="angleCanvas" width="400" height="400"></canvas>
        <div class="direction-label north">N</div>
        <div class="direction-label northwest">NW</div>
        <div class="direction-label northeast">NE</div>
        <div class="direction-label south">S</div>
        <div class="direction-label southwest">SW</div>
        <div class="direction-label southeast">SE</div>
        <div class="direction-label west">W</div>
        <div class="direction-label east">E</div>
    </div>
</body>
<footer>
    <p>by HA2KJ</p>
    <script>

        const canvas = document.getElementById("angleCanvas");
        const ctx = canvas.getContext("2d");
        const angleInput = document.getElementById("angleInput");
        const updateButton = document.getElementById("updateButton");
        const movingAngleLabel = document.getElementById("movingAngleLabel");
        const numLines = 24; // 360 degrees divided by 15 degrees

        const haltButton = document.getElementById("haltButton");

        haltButton.addEventListener("click", function () {
            
            clearInterval(updateTimer); 

            const xhr = new XMLHttpRequest();
            xhr.open('POST', '/halt', true);

            xhr.onreadystatechange = function () {
                if (xhr.readyState === 4 && xhr.status === 200) {
                    console.log('Halt button clicked');
                }
            };

            xhr.send();
        });

        document.addEventListener("keypress", function (event) {
            if (event.key === "Enter") {
                event.preventDefault();
                updateButton.click();
            }
        });

        canvas.addEventListener("click", function (event) {
            const centerX = canvas.width / 2;
            const centerY = canvas.height / 2;
            const clickX = event.clientX - canvas.getBoundingClientRect().left;
            const clickY = event.clientY - canvas.getBoundingClientRect().top;

            // Calculate the angle in degrees with 0 degrees at the top middle position
            let angleInDegrees = Math.atan2(clickX - centerX, centerY - clickY) * (180 / Math.PI);

            // Ensure the angle is positive (between 0 and 360 degrees)
            if (angleInDegrees < 0) {
                angleInDegrees += 360;
            }

            angleInput.value = Math.round(angleInDegrees);
            updateAngles();
        });


        updateButton.addEventListener("click", updateAngles);

        angleInput.addEventListener("input", function () {
            const inputValue = parseFloat(angleInput.value);
            if (isNaN(inputValue) || inputValue < 0 || inputValue > 360) {
                angleInput.setCustomValidity("Angle must be between 0 and 360 degrees.");
                updateButton.disabled = true;
            } else {
                angleInput.setCustomValidity("");
                updateButton.disabled = false
            }
            angleInput.reportValidity();
        });

        let staticAngleDegrees = 45;
        let movingAngleDegrees = 0;
        let updateTimer;

        function updateAngles() {
            staticAngleDegrees = Math.round(parseFloat(angleInput.value));
            if (isNaN(staticAngleDegrees) || staticAngleDegrees < 0 || staticAngleDegrees > 360) {
                updateButton.disabled = true;
                staticAngleDegrees = 0;
            } else {
                updateButton.disabled = false;
                callRotator(staticAngleDegrees);
                drawAngles();

                updateTimer = setInterval(updateMovingAngleFromXHR, 500);
            }
        }

        function drawAngles() {
            ctx.clearRect(0, 0, canvas.width, canvas.height);

            const centerX = canvas.width / 2;
            const centerY = canvas.height / 2;
            const lineLength = Math.min(canvas.width, canvas.height) / 2;

            const staticAngleRadians = (staticAngleDegrees - 90) * (Math.PI / 180);
            const staticEndX = centerX + lineLength * Math.cos(staticAngleRadians);
            const staticEndY = centerY + lineLength * Math.sin(staticAngleRadians);

            ctx.beginPath();
            ctx.moveTo(centerX, centerY);
            ctx.lineTo(staticEndX, staticEndY);
            ctx.strokeStyle = "green";
            ctx.lineWidth = 3;
            ctx.stroke();

            const movingAngleRadians = (movingAngleDegrees - 90) * (Math.PI / 180);
            const movingEndX = centerX + lineLength * Math.cos(movingAngleRadians);
            const movingEndY = centerY + lineLength * Math.sin(movingAngleRadians);

            ctx.setLineDash([5, 5]);
            ctx.beginPath();
            ctx.moveTo(centerX, centerY);
            ctx.lineTo(movingEndX, movingEndY);
            ctx.strokeStyle = "green";
            ctx.lineWidth = 3;
            ctx.stroke();
            ctx.setLineDash([]);
            let radius = canvas.width / 2
            for (let i = 0; i < numLines; i++) {
                const angle = (i * 15) * (Math.PI / 180);
                const x1 = centerX + (radius - 5) * Math.cos(angle);
                const y1 = centerY + (radius - 5) * Math.sin(angle);
                const x2 = centerX + (radius + 5) * Math.cos(angle);
                const y2 = centerY + (radius + 5) * Math.sin(angle);

                ctx.beginPath();
                ctx.moveTo(x1, y1);
                ctx.lineTo(x2, y2);

                if (i * 15 % 90 == 0) {
                    ctx.lineWidth = 4;
                    ctx.strokeStyle = 'black';

                } else {
                    ctx.lineWidth = 2;
                    ctx.strokeStyle = 'red';
                }

                ctx.stroke();
                ctx.closePath();
            }

            requestAnimationFrame(drawAngles);
        }

        function updateMovingAngleFromXHR() {
            const xhr = new XMLHttpRequest();
            xhr.open('GET', '/actual', true);

            xhr.onreadystatechange = function () {
                if (xhr.readyState === 4 && xhr.status === 200) {
                    const response = xhr.responseText;
                    movingAngleDegrees = JSON.parse(xhr.responseText)['a'];
                    movingAngleLabel.textContent = movingAngleDegrees;
                    if (Math.abs(movingAngleDegrees - staticAngleDegrees) < 1) {
                        clearInterval(updateTimer); 
                    }
                    drawAngles();
                }
            };

            xhr.send();
        }

        function callRotator(target) {
            const xhr = new XMLHttpRequest();
            xhr.open('POST', `/turn?target=${target}`, true);

            xhr.onreadystatechange = function () {
                if (xhr.readyState === 4 && xhr.status === 200) {
                    console.log('callrotator called')
                    drawAngles();
                }
            };

            xhr.send();
        }

        drawAngles();

    </script>
</footer>

</html>
)rawliteral";


volatile bool isAlarmed = false;
volatile bool isPending = false;

volatile bool isAlarmHandled = false;
volatile bool isPendingHandled = false;

void IRAM_ATTR onAlarm() {
  isAlarmed = digitalRead(PIN_ALM);
  if (isAlarmed) isAlarmHandled = false;
}

void IRAM_ATTR onPending() {
  isPending = digitalRead(PIN_PEND);
  if (isAlarmed) isPendingHandled = false;
}

void halt() {
  toHalt = true;
}

void setup(void) {
  pinMode(PIN_PUL, OUTPUT);
  pinMode(PIN_DIR, OUTPUT);
  pinMode(PIN_ENA, OUTPUT);

  pinMode(PIN_PEND, INPUT);
  pinMode(PIN_ALM, INPUT);

  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());


  pinMode(PIN_LED, OUTPUT);

  server.on("/halt", HTTP_POST, [](AsyncWebServerRequest* request) {
    Serial.println("/halt");
    halt();
    request->send(200, "text/plain", "OK");
  });

  server.on("/", HTTP_GET, [](AsyncWebServerRequest* request) {
    request->send(200, "text/html", index_html);
  });

  server.on("/actual", HTTP_GET, [](AsyncWebServerRequest* request) {
    String act = "{\"a\":" + String(actualHeadingInDeg) + "}";
    //Serial.println("/actual");
    request->send(200, "application/json", act);
  });

  server.on("/turn", HTTP_POST, [](AsyncWebServerRequest* request) {
    String inputMessage1;
    String inputMessage2;
    Serial.print("/turn:");
    if (request->hasParam(paramName)) {
      inputMessage1 = request->getParam(paramName)->value();
      Serial.println(inputMessage1);
      turnTo(inputMessage1.toInt());
    } else {
    }
    request->send(200, "text/plain", "OK");
  });

  AsyncElegantOTA.begin(&server);
  server.begin();
  Serial.println("HTTP server started");
  pinMode(PIN_ALM, INPUT);
  attachInterrupt(digitalPinToInterrupt(PIN_ALM), onAlarm, CHANGE);
  pinMode(PIN_PEND, INPUT);
  attachInterrupt(digitalPinToInterrupt(PIN_PEND), onPending, CHANGE);
  Serial.println("Setup completed");
}

void loop(void) {
  if (toHalt) {
    Serial.println("Halted...");
    stepsYet = 0;
    lastActualHeadingInDeg = actualHeadingInDeg;
  }
  if (isAlarmed and !isAlarmHandled) {
    Serial.println("***ALARM****");
    isAlarmHandled = true;
  }
  if (isPending and !isPendingHandled) {
    Serial.println("***Pending****");
    isPendingHandled = true;
  }

  if (stepsYet-- > 0) {
    digitalWrite(PIN_PUL, HIGH);
    delayMicroseconds(10);
    digitalWrite(PIN_PUL, LOW);
    //delayMicroseconds(5);

    //if (stepsYet % 10 == 0) { Serial.print("*"); }
    if (directionCw) {

      actualHeadingInDeg = steps2deg(actualHeadingInSteps++);
      lastActualHeadingInDeg = actualHeadingInDeg;
    } else {

      actualHeadingInDeg = steps2deg(actualHeadingInSteps--);
      lastActualHeadingInDeg = actualHeadingInDeg;
    }
    if (actualHeadingInDeg > 360 or actualHeadingInDeg < 0) {
      actualHeadingInDeg = 0;
      actualHeadingInSteps = 0;
    }
  }

  if (actualHeadingInDeg != lastActualHeadingInDeg) {
    isRotating = true;
  } else {
    isRotating = false;
  }
  delayMicroseconds(3500);
}

void turnTo(int heading) {
  Serial.print("Turning to: ");
  if (heading > 360 or heading < 0)
    return;
  Serial.println(heading);
  if (heading > actualHeadingInDeg) {
    directionCw = true;
    step(heading - actualHeadingInDeg);
  } else {
    directionCw = false;
    step(actualHeadingInDeg - heading);
  }
  targetHeadingInDeg = heading;
}

int deg2steps(int degrees) {
  float steps = (degrees - CALIBRATION_DEG_DEV) / ONE_STEP_DEG * GEAR_RATIO;
  return round(steps);
}

int steps2deg(int steps) {
  float stepsNum = steps / GEAR_RATIO * ONE_STEP_DEG;
  return round(stepsNum) - CALIBRATION_DEG_DEV;
}

void step(int degrees) {
  stepsYet = deg2steps(degrees);
  Serial.print("Degrees: ");
  Serial.println(degrees);
  Serial.print("Steps: ");
  Serial.println(stepsYet);
  if (stepsYet != 0) {
    if (directionCw) {
      digitalWrite(PIN_DIR, LOW);
      delayMicroseconds(5);
    } else {
      digitalWrite(PIN_DIR, HIGH);
      delayMicroseconds(5);
    }
  }
  if (toHalt) {
    toHalt = false;
  }
}
