#define ARDUINOJSON_USE_LONG_LONG 1
#include <ArduinoJson.h>
#include "FS.h"
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WebSocketsServer.h>
#include <ESP8266mDNS.h>
#include <IRrecv.h>
#include <IRremoteESP8266.h>
#include <IRac.h>
#include <IRtext.h>
#include <IRutils.h>
#include <SchedulerESP8266.h>

// Button to start in ap mode
// press the button on boot for ~ 10 sec
#define APSWITCH 16

// IR reciver pin
const uint16_t kRecvPin = 5;
// IR transmitter pin
const uint16_t kIrLed = 4;

// WIFI CONFIG
const char* SSID = "";
const char* PASSWORD = "";
String hostname = "";

// STATIC IP CONFIG
bool useStaticIP = false;
IPAddress ipaddress = IPAddress(192,168,1,21);
IPAddress gateway = IPAddress(192,168,4,1);
IPAddress subnet = IPAddress(255,255,255,0);
IPAddress dns = IPAddress(8,8,8,8);


Scheduler scheduler;
ESP8266WebServer server(80); 
WebSocketsServer webSocket = WebSocketsServer(8080); 
IRrecv irrecv(kRecvPin, 1024, 50, true);
IRsend irsend(kIrLed);
DynamicJsonDocument configJson(19000);
decode_results results; 
bool apMode;

const char indexPage[] PROGMEM= R"=====(
<html>
<head>
  <style>
    *{padding: 0;margin: 0;font-family: "Arial", sans-serif}
    body{background: #262626}
    p,h1,h2,h3{color: white;}
    .btnClass{width: 75px;height: 35px;background: #ccffda;border-style: none;color: white;}
    #remote{width: 460px;min-height: 65%;background: #dedede;margin: 100 auto;padding: 20px;position: relative;}
    .btn{height:60px;border-style:none; float:left; color:white}
    #addNewButton p{color:black}
  </style>
</head>
<body>
  <div id="remote">
      <h2 id="remoteTitle" style="text-align:center; color:black; margin:20px 0px;">EZRemote8266</h2>
      <div id="DivButtons"></div>
      <button onclick="showAddWindow()" style="width: 55px; height: 55px; border-radius: 50%; background:#9be0bc; color: #262626; border-style: none; margin: 10 auto; position:absolute; bottom:0;right:15; font-weight: 900; font-size:19pt;">+</button>
  </div>
    <div style="display: none; width:300px; height:600px; padding:30px;  background-color:#dedede; border: 10px solid #262626; position:absolute; top: 50%;left: 50%;transform: translate(-50%, -50%)" id="addNewButton">
            <p>Name:</p>
            <input name="buttonName" placeholder="Button name" id="buttonName" value="Button" style="width:200px;height:35px;text-transform: uppercase;" maxlength="20"/>
            <p>Hex:</p>
            <input name="hexAddress" placeholder="HEX Address" id="hexAddress" style="width:200px;height:35px; float:left;"/>
            <button class="btnClass" style="background:#34eb89; float:left; margin-left:20px; width:80px; height:35px; color:black" onClick="useLastButton()" >Use last button</button>
            <p>Type:</p>
            <select id="Type" style="width:200px;height:35px;">
              <option value="1">RC5</option><option value="2">RC6</option><option value="3">NEC</option><option value="4">SONY</option><option value="5">PANASONIC</option><option value="6">JVC</option><option value="7">SAMSUNG</option>
              <option value="8">WHYNTER</option><option value="9">AIWA_RC_T501</option><option value="10">LG</option><option value="11">SANYO</option><option value="12">MITSUBISHI</option><option value="13">DISH</option>
              <option value="14">SHARP</option><option value="15">COOLIX</option><option value="16">DAIKIN</option><option value="17">DENON</option><option value="18">KELVINATOR</option><option value="19">SHERWOOD</option>
              <option value="20">MITSUBISHI_AC</option><option value="21">RCMM</option><option value="22">SANYO_LC7461</option><option value="23">RC5X</option><option value="24">GREE</option><option value="25">PRONTO</option>
              <option value="26">NEC_LIKE</option><option value="27">ARGO</option><option value="28">TROTEC</option><option value="29">NIKAI</option><option value="30">RAW</option><option value="31">GLOBALCACHE</option>
              <option value="32">TOSHIBA_AC</option><option value="33">FUJITSU_AC</option><option value="34">MIDEA</option><option value="35">MAGIQUEST</option><option value="36">LASERTAG</option><option value="37">CARRIER_AC</option>
              <option value="38">HAIER_AC</option><option value="39">MITSUBISHI2</option><option value="40">HITACHI_AC</option><option value="41">HITACHI_AC1</option><option value="42">HITACHI_AC2</option><option value="43">GICABLE</option>
              <option value="44">HAIER_AC_YRW02</option><option value="45">WHIRLPOOL_AC</option><option value="46">SAMSUNG_AC</option><option value="47">LUTRON</option><option value="48">ELECTRA_AC</option><option value="49">PANASONIC_AC</option>
              <option value="50">PIONEER</option><option value="51">LG2</option><option value="52">MWM</option><option value="53">DAIKIN2</option><option value="54">VESTEL_AC</option><option value="55">TECO</option>
              <option value="56">SAMSUNG36</option><option value="57">TCL112AC</option><option value="58">LEGOPF</option><option value="59">MITSUBISHI_HEAVY_88</option><option value="60">MITSUBISHI_HEAVY_152</option>
              <option value="61">DAIKIN216</option><option value="62">SHARP_AC</option><option value="63">GOODWEATHER</option><option value="64">INAX</option><option value="65">DAIKIN160</option><option value="66">NEOCLIMA</option>
              <option value="67">DAIKIN176</option><option value="68">DAIKIN128</option><option value="69">AMCOR</option><option value="70">DAIKIN152</option><option value="71">MITSUBISHI136</option><option value="72">MITSUBISHI112</option>
              <option value="73">HITACHI_AC424</option><option value="74">SONY_38K</option><option value="75">EPSON</option><option value="76">SYMPHONY</option><option value="77">HITACHI_AC3</option>
              <option value="78">DAIKIN64</option><option value="79">AIRWELL</option><option value="80">DELONGHI_AC</option><option value="81">DOSHISHA</option><option value="82">MULTIBRACKETS</option>
              <option value="83">CARRIER_AC40</option><option value="84">CARRIER_AC64</option><option value="85">HITACHI_AC344</option><option value="86">CORONA_AC</option>
              <option value="87">MIDEA24</option><option value="88">ZEPEAL</option><option value="89">SANYO_AC</option><option value="90">VOLTAS</option><option value="91">METZ</option>
              <option value="92">TRANSCOLD</option><option value="93">TECHNIBEL_AC</option><option value="94">MIRAGE</option><option value="95">ELITESCREENS</option><option value="96">PANASONIC_AC32</option>
            </select>
            <p>Last button recorded: <b id="lastHex"></b></p>
            <p>Type: <b id="lastType"></b></p>
            <p>Message: <b id="lastMessage" style="font-size:10pt; width:90%; word-break: break-all;"></b></p>
            <p>State: <textarea id="lastState" style="font-size:10pt;resize: none; width:90%; height: 50px;"></textarea></p>
            <b style="font-size:12pt;">Use schedule</b>
            <input style="width:15px; height:15px;" type="checkbox" id="useSchedule" />
            <table>
              <tr><td>Hour</td><td>Minute</td></tr>
              <tr>
                <td><input id="hourTextBox" style="width:50px;height:25;" type="number" min="0" max="23" value=""/></td>
                <td><input id="minuteTextBox" style="width:50px;height:25;" type="number" min="0" max="59" value=""/></td></tr>
            </table>
              <p style="display:inline-block; margin-right:5px; font-size:8pt;">Su</p>
              <p style="display:inline-block; margin-right:5px; font-size:8pt;">Mo</p>
              <p style="display:inline-block; margin-right:5px; font-size:8pt;">Tu</p>
              <p style="display:inline-block; margin-right:5px; font-size:8pt;">We</p>
              <p style="display:inline-block; margin-right:5px; font-size:8pt;">Th</p>
              <p style="display:inline-block; margin-right:5px; font-size:8pt;">Fr</p>
              <p style="display:inline-block; margin-right:5px; font-size:8pt;">Sa</p>
              <br>
              <input id="SuCheckbox" type="checkbox" style="display:inline-block; margin-right:3px; width:15px; height15px;" checked/>
              <input id="MoCheckbox" type="checkbox" style="display:inline-block; margin-right:3px; width:15px; height15px;" checked/>
              <input id="TuCheckbox" type="checkbox" style="display:inline-block; margin-right:3px; width:15px; height15px;" checked/>
              <input id="WeCheckbox" type="checkbox" style="display:inline-block; margin-right:3px; width:15px; height15px;" checked/>
              <input id="ThCheckbox" type="checkbox" style="display:inline-block; margin-right:3px; width:15px; height15px;" checked/>
              <input id="FrCheckbox" type="checkbox" style="display:inline-block; margin-right:3px; width:15px; height15px;" checked/>
              <input id="SaCheckbox" type="checkbox" style="display:inline-block; margin-right:3px; width:15px; height15px;" checked/>
            <div style="position:absolute; bottom:10; left:30%; ">
              <button class="btnClass" style="background:#eb4034" onclick="document.getElementById('addNewButton').style.display = 'none';">Cancel</button>
              <button class="btnClass" style="background:#34eb89" onclick="createButton()" id="finishButton">Finish</button>
            <div>
          </div>
  <script>
    var ws = new WebSocket(`ws://${window.location.hostname}:8080/`);
    var buttonsJson;
    dragElement(document.getElementById('addNewButton'));
    ws.onmessage = function(event) {
      var data = JSON.parse(event.data);
      if("Buttons" in data) {
        createAllButtons(data["Buttons"]);
      }
      if('lastButton' in data){
        document.getElementById('lastMessage').innerText = "";
        document.getElementById('lastState').innerText = "";
        lastBtn = data['lastButton'];
        if('Hex' in lastBtn) document.getElementById('lastHex').innerText = lastBtn['Hex'];
        if('Type' in lastBtn) {
          var opt = document.getElementById('Type').getElementsByTagName('option');
          for (index = 0; index < opt.length; index++) if (opt[index].value == lastBtn['Type']) document.getElementById('lastType').innerText = opt[index].innerText;
        }
        document.getElementById('lastMessage').innerText = lastBtn['Message'];
        document.getElementById('lastState').innerText = lastBtn['State'];
      }
    };
    ws.onclose = function(event) {
      location.reload();
    };
    function createAllButtons(json){
      buttonsJson = json;
      var buttons = document.getElementById('DivButtons');
      buttons.innerHTML = "";
      for (var i = 0; i < buttonsJson.length; i++) {
        var newButton = `
          <div style="margin: 5px; font-size: 0; display:inline-block;">
            <button onclick="modifyButtonWindow(this)" class="btn" style="width:20px; background:#637d87" value="${buttonsJson[i]["buttonName"]}">&#9881;</button>
            <button onclick="ws.send(JSON.stringify({'pressButton':{'buttonName':this.id}}));" class="btn" style="width:100px; background:#626262;  padding:10px;  word-wrap: break-word" id="${buttonsJson[i]["buttonName"]}">${buttonsJson[i]["buttonName"]}</button>
            <button onclick="ws.send(JSON.stringify({'removeButton':{'buttonName':this.value}}));" class="btn" style="width:20px; background:#f44336; float:left" value="${buttonsJson[i]["buttonName"]}">&#10006;</button>
          </div>
        `;
        let frag = document.createRange().createContextualFragment(newButton);
        buttons.appendChild(frag);
      }
    }
    function getButtonJson(){
      if (document.getElementById('buttonName').value.length == 0) return alert("You need a button name");
      else if (document.getElementById('Type').value.length == 0) return alert("You need a type");
      else if (document.getElementById('hexAddress').value.length == 0 && document.getElementById('lastState').value.length == 0) return alert("You need a hex or a state");
      var time = new Date();
      var timestamp = new Date(time.getFullYear(), time.getMonth(), time.getDate(), document.getElementById("hourTextBox").value, document.getElementById("minuteTextBox").value, 0);
      var data = {
        "buttonName":document.getElementById('buttonName').value.toUpperCase(),
        "Hex":parseInt(document.getElementById('hexAddress').value),
        "Type":parseInt(document.getElementById('Type').value),
        "Message":document.getElementById('lastMessage').innerText,
        "State":[],
        "useSchedule":document.getElementById('useSchedule').checked,
        "Timestamp":timestamp.getTime()/1000,
        "Days":[document.getElementById("SuCheckbox").checked,document.getElementById("MoCheckbox").checked,document.getElementById("TuCheckbox").checked,document.getElementById("WeCheckbox").checked,document.getElementById("ThCheckbox").checked,document.getElementById("FrCheckbox").checked,document.getElementById("SaCheckbox").checked]
      }
      if(document.getElementById('lastState').value.length>0) data["State"] = document.getElementById('lastState').value.split(',').map(Number)
      return data;
    }
    function createButton(){
      var name = document.getElementById('buttonName').value.toUpperCase();
      for (var i = 0; i < buttonsJson.length; i++) {
        if(buttonsJson[i]["buttonName"] == name) return alert("Button already exists!");
      }
      ws.send(JSON.stringify({"createNewButton":getButtonJson()}));
      document.getElementById('addNewButton').style.display = "none";
    }
    function modifyButton(oldButton){
      var name = document.getElementById('buttonName').value.toUpperCase();
      if(oldButton != name){
        for (var i = 0; i < buttonsJson.length; i++) {
          if(buttonsJson[i]["buttonName"] == name) return alert("Button already exists!");
        }
      }
      ws.send(JSON.stringify({"modifyButton":{"oldButtonName":oldButton,"newValue":getButtonJson()}}));
      document.getElementById('addNewButton').style.display = "none";
    }
    function modifyButtonWindow(e){
      for (var i = 0; i < buttonsJson.length; i++) {
        if(buttonsJson[i]["buttonName"] == e.value) {
          showAddWindow();
          document.getElementById('buttonName').value = buttonsJson[i]["buttonName"];
          document.getElementById('hexAddress').value = buttonsJson[i]["Hex"];
          document.getElementById('Type').value = buttonsJson[i]["Type"];
          document.getElementById('useSchedule').checked = buttonsJson[i]["useSchedule"];
          var time = new Date(buttonsJson[i]["Timestamp"] * 1000);
          document.getElementById("hourTextBox").value = ("0" + time.getHours()).slice(-2);
          document.getElementById("minuteTextBox").value = ("0" + time.getMinutes()).slice(-2);
          document.getElementById("SuCheckbox").checked = buttonsJson[i]["Days"][0];
          document.getElementById("MoCheckbox").checked = buttonsJson[i]["Days"][1];
          document.getElementById("TuCheckbox").checked = buttonsJson[i]["Days"][2];
          document.getElementById("WeCheckbox").checked = buttonsJson[i]["Days"][3];
          document.getElementById("ThCheckbox").checked = buttonsJson[i]["Days"][4];
          document.getElementById("FrCheckbox").checked = buttonsJson[i]["Days"][5];
          document.getElementById("SaCheckbox").checked = buttonsJson[i]["Days"][6];
          document.getElementById("finishButton").onclick = function(){ modifyButton(buttonsJson[i]["buttonName"])};
          return;
        }
      }

    }
    function showAddWindow(){
      if(document.getElementById('addNewButton').style.display == "none"){
        var time = new Date();
        document.getElementById('buttonName').value = "";
        document.getElementById('hexAddress').value = "";
        document.getElementById('Type').value = -1;
        document.getElementById('useSchedule').checked = false;
        document.getElementById("hourTextBox").value = ("0" + time.getHours()).slice(-2);
        document.getElementById("minuteTextBox").value = ("0" + time.getMinutes()).slice(-2);
        document.getElementById('addNewButton').style.display = "block";
      }
    }
    function useLastButton(){
      document.getElementById('hexAddress').value = document.getElementById('lastHex').innerText;
      var type = document.getElementById('lastType').innerText;
      var opt = document.getElementById('Type').getElementsByTagName('option');
      for (index = 0; index < opt.length; index++) if (opt[index].innerText == type) document.getElementById('Type').value = opt[index].value;
    }
    function dragElement(elmnt) {
      var pos1 = 0, pos2 = 0, pos3 = 0, pos4 = 0;
      elmnt.onmousedown = dragMouseDown;
      function dragMouseDown(e) {
        e = e || window.event;
        if(e.srcElement.id == elmnt.id){
          pos3 = e.clientX;
          pos4 = e.clientY;
          document.onmouseup = closeDragElement;
          document.onmousemove = elementDrag;
        }
      }
      function elementDrag(e) {
        e = e || window.event;
        e.preventDefault();
        pos1 = pos3 - e.clientX;
        pos2 = pos4 - e.clientY;
        pos3 = e.clientX;
        pos4 = e.clientY;
        elmnt.style.top = (elmnt.offsetTop - pos2) + "px";
        elmnt.style.left = (elmnt.offsetLeft - pos1) + "px";
      }
      function closeDragElement() {document.onmouseup = null;document.onmousemove = null;}
    }
  </script>
</body>
</html>
)=====";

bool saveConfig() {
  File configFile = SPIFFS.open("/user_settings.json", "w+");
  serializeJson(configJson, configFile);
  configFile.close(); 
  return true;
}

bool loadConfig() {
  File configFile = SPIFFS.open("/user_settings.json", "r");
  if (!configFile) return false;
  deserializeJson(configJson, configFile);
  for (size_t i = 0; i < configJson["Buttons"].size(); i++)
  {
    if (configJson["Buttons"][i]["useSchedule"].as<bool>()){
      bool days[7];
      for (size_t j = 0; j < 7; j++) days[i] = configJson["Buttons"][i]["Days"][j];         
      ScheduleTask task = ScheduleTask(configJson["Buttons"][i]["buttonName"], days, configJson["Buttons"][i]["Timestamp"],  
      [=](){    
          irsend.send(configJson["Buttons"][i]["Type"].as<decode_type_t>(), configJson["Buttons"][i]["Hex"].as<uint64_t>(),irsend.defaultBits(configJson["Buttons"][i]["Type"].as<decode_type_t>()));   
      });
      scheduler.add(task);
    }
  }
  configFile.close();
  return true;
}

bool handleHardReset(int waitTime){
  int secconds=0;
  pinMode(APSWITCH, INPUT);
  while(digitalRead(APSWITCH) == HIGH){
    secconds+=1;   
    digitalWrite(LED_BUILTIN, HIGH); delay(100); digitalWrite(LED_BUILTIN, LOW);
    if(secconds >= waitTime) return true; 
  }
  return false;
}

void dump(decode_results *results) {
  yield();
  if(!results->repeat && results->decode_type != UNKNOWN){ 
    configJson["lastButton"]["Hex"] = results->value;
    configJson["lastButton"]["Type"] = results->decode_type;
    configJson["lastButton"]["Message"] = "";
    configJson["lastButton"]["State"].clear();
    configJson["lastButton"].createNestedArray("State");
    if(hasACState(results->decode_type)) {
      configJson["lastButton"]["Message"] = IRAcUtils::resultAcToString(results);
      for (int i = 0; i < results->bits / 8; i++) configJson["lastButton"]["State"][i] = results->state[i];
    }
    webSocket.broadcastTXT("{\"lastButton\":"+configJson["lastButton"].as<String>()+"}");
    irrecv.resume();
    saveConfig();
  }
}

void removeButton(String name){
  JsonArray arr = configJson["Buttons"];
  for (size_t i = 0; i < arr.size(); i++)
  {
    if (arr[i]["buttonName"] == name) {
      arr.remove(i);
      if(arr[i]["useSchedule"]) scheduler.Remove(i);
      return;
    }
  }
  saveConfig();
}

void removeButtonHTTP(){
  DynamicJsonDocument recv_json(524);
  deserializeJson(recv_json, server.arg("plain"));
  removeButton(recv_json["buttonName"].as<String>());
  server.send(200, "application/json", "{\"Buttons\":"+configJson["Buttons"].as<String>()+"}");  
}
void createNewButton(JsonObject& obj){
  JsonObject asd = configJson["Buttons"].createNestedObject();
  asd["buttonName"] = obj["buttonName"].as<String>();
  asd["Hex"] = obj["Hex"].as<uint64_t>();
  asd["Type"] = obj["Type"].as<int>();
  asd["Message"] = obj["Message"].as<String>();
  asd["State"] = obj["State"].as<JsonArray>();
  asd["useSchedule"] = obj["useSchedule"].as<bool>();
  asd["Timestamp"] = obj["Timestamp"].as<long>();
  JsonArray days = asd.createNestedArray("Days");
  for (int i = 0; i < 7; i++) days[i] = obj["Days"][i];
  if (asd["useSchedule"].as<bool>()){
    bool days[7];
    for (size_t i = 0; i < 7; i++) days[i] = asd["Days"][i].as<bool>();         
    ScheduleTask task = ScheduleTask(asd["buttonName"].as<String>(), days, asd["Timestamp"].as<long>(),  
    [=](){    
        irsend.send(asd["Type"].as<decode_type_t>(), asd["Hex"].as<uint64_t>(),irsend.defaultBits(asd["Type"].as<decode_type_t>()));   
    });
    scheduler.add(task);
  }
  saveConfig();
}

void createNewButtonHTTP(){
  DynamicJsonDocument recv_json(524);
  deserializeJson(recv_json, server.arg("plain"));
  JsonObject json = recv_json.as<JsonObject>();
  createNewButton(json);
  server.send(200, "application/json", "{\"Buttons\":"+configJson["Buttons"].as<String>()+"}");  
}

void modifyButton(JsonObject& obj){
  removeButton(obj["oldButtonName"].as<String>());
  JsonObject a = obj["newValue"].as<JsonObject>();
  createNewButton(a);
  saveConfig();
}

void modifyButtonHTTP(){
  DynamicJsonDocument recv_json(524);
  deserializeJson(recv_json, server.arg("plain"));
  JsonObject obj = recv_json.as<JsonObject>();
  modifyButton(obj);
  server.send(200, "application/json", "{\"Buttons\":"+configJson["Buttons"].as<String>()+"}");  
}


void pressButton(String name){
  JsonArray arr = configJson["Buttons"];
  for (size_t i = 0; i < arr.size(); i++)
  {
    if (arr[i]["buttonName"] == name){
      Serial.println(arr[i]["buttonName"].as<String>());
      if(hasACState(arr[i]["Type"].as<decode_type_t>())){
        uint8_t state[kStateSizeMax];
        JsonArray st = arr[i]["State"].as<JsonArray>();
        for (size_t i = 0; i < st.size(); i++) state[i] = st[i];    
        irsend.send(arr[i]["Type"].as<decode_type_t>(), state, irsend.defaultBits(arr[i]["Type"].as<decode_type_t>()) / 8);
      }
      else irsend.send(arr[i]["Type"].as<decode_type_t>(), arr[i]["Hex"].as<uint64_t>(),irsend.defaultBits(arr[i]["Type"].as<decode_type_t>()));   
      return;
    }
  }
}

void pressButtonHTTP(){
  DynamicJsonDocument recv_json(524);
  deserializeJson(recv_json, server.arg("plain"));
  pressButton(recv_json["buttonName"].as<String>());
  server.send(200, "text/html", "Success!");
}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t *payload, size_t length)
{
  if(type == WStype_CONNECTED){
      Serial.println("Client "+String(num)+" connected");  
      String cfg = configJson.as<String>();
      webSocket.sendTXT(num,cfg);
  }
  if (type == WStype_TEXT)
  {
    if(webSocket.connectedClients() > 1) {
      for (int i = 0; i < webSocket.connectedClients(); i++)  if(i != num) webSocket.sendTXT(i,payload,length);
    }
    DynamicJsonDocument json(length+2048);   
    deserializeJson(json, payload);
    if(json.containsKey("pressButton")){
      pressButton(json["pressButton"]["buttonName"].as<String>());
    }
    else if(json.containsKey("createNewButton")){
      JsonObject obj = json["createNewButton"];
      createNewButton(obj);
      webSocket.broadcastTXT("{\"Buttons\":"+configJson["Buttons"].as<String>()+"}");
    }
    else if(json.containsKey("modifyButton")){
      JsonObject obj = json["modifyButton"];
      modifyButton(obj);
      webSocket.broadcastTXT("{\"Buttons\":"+configJson["Buttons"].as<String>()+"}");
    }
    else if(json.containsKey("removeButton")){
      removeButton(json["removeButton"]["buttonName"]);
      webSocket.broadcastTXT("{\"Buttons\":"+configJson["Buttons"].as<String>()+"}");
    }
    json.clear();
  }
  if(type == WStype_DISCONNECTED){
    Serial.println("Client "+String(num)+" disconnected");  
  }
}

void initServer(){
  server.on("/", []() {server.send(200, "text/html", indexPage);});
  server.on("/getLastButton", []() {server.send(200, "application/json", "{\"lastButton\":"+configJson["lastButton"].as<String>()+"}");});
  server.on("/getAllButtons", []() {server.send(200, "application/json", "{\"Buttons\":"+configJson["Buttons"].as<String>()+"}");});
  server.on("/createNewButton", createNewButtonHTTP);
  server.on("/modifyButton", modifyButtonHTTP);
  server.on("/removeButton", removeButtonHTTP);
  server.on("/pressButton", pressButtonHTTP);
  server.onNotFound([]() { server.send(404, "text/plain", "Error 404!");}); 
  server.begin();
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
  scheduler.begin();
}

void setup() {
  Serial.begin(115200); 
  pinMode(LED_BUILTIN,OUTPUT);
  configJson.createNestedObject("lastButton");
  configJson.createNestedArray("Buttons");
  SPIFFS.begin();
  loadConfig();
  WiFi.softAPdisconnect(true);
  WiFi.disconnect();
  if(apMode = handleHardReset(60)){
    unsigned char mac[6];
    char dataString[50] = {0};    
    WiFi.macAddress(mac);
    sprintf(dataString, "REMOTE(%02X:%02X:%02X)",mac[3],mac[4],mac[5]);
    WiFi.mode(WIFI_AP);
    WiFi.softAPConfig(IPAddress(192,168,1,1), IPAddress(192,168,1,1), IPAddress(255, 255, 255, 0));
    WiFi.softAP(dataString);
    Serial.println(WiFi.softAPIP());  
    digitalWrite(LED_BUILTIN, LOW);
  }
  else{
    WiFi.mode(WIFI_STA); 
    WiFi.hostname(hostname); 
    if (useStaticIP) WiFi.config(ipaddress, gateway, subnet, dns);
    WiFi.begin(SSID, PASSWORD);  
    while(WiFi.status() != WL_CONNECTED){
      digitalWrite(LED_BUILTIN, HIGH); delay(250); digitalWrite(LED_BUILTIN, LOW);
      Serial.print("."); 
    }
    digitalWrite(LED_BUILTIN, HIGH);
    Serial.println("");
    if(MDNS.begin(hostname)) Serial.println("http://"+String(wifi_station_get_hostname())+".local : "+WiFi.localIP().toString());
  }
  initServer();
  irrecv.setUnknownThreshold(12);
  irrecv.setTolerance(kTolerance);
  irrecv.enableIRIn();
  irsend.begin();
}

void loop() {
  server.handleClient(); 
  webSocket.loop();
  MDNS.update();
  if(!apMode) scheduler.update();
  if (irrecv.decode(&results)) {
    dump(&results);    
    yield();
  } 
}