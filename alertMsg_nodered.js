let bpm = msg.payload.bpm;
let threshold = 100;

context.lastAlert = context.lastAlert || "normal";

let alert = {};

if (bpm > threshold && context.lastAlert !== "high") {
    context.lastAlert = "high";

    alert.type = "HIGH";
    alert.message = `HIGH HEART RATE ALERT: ${bpm} BPM`;
    alert.detail = "Please rest and relax.";

} else if (bpm <= threshold && context.lastAlert !== "normal") {
    context.lastAlert = "normal";

    alert.type = "NORMAL";
    alert.message = `Heart Rate Back to Normal: ${bpm} BPM`;
    alert.detail = "Status stable.";
}
else {
    return null; // No new alert condition → No output
}

msg.alert = alert;
msg.bpm = bpm;

return msg;
