// Convert incoming data to ThingSpeak URL format
let data = msg.payload; // {bpm:110, temp:25.8}
let writeKey = "J842P3F8F2EMWXJ4"; // ← Replace this

let url = "https://api.thingspeak.com/update?api_key=" + writeKey;

if (data.bpm !== undefined) url += "&field1=" + encodeURIComponent(data.bpm);
if (data.temp !== undefined) url += "&field2=" + encodeURIComponent(data.temp);

msg.method = "GET";
msg.url = url;
return msg;

