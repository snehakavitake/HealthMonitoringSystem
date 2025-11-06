let bpm = msg.payload.bpm;
if (bpm < 60) msg.payload = "🟡 Low";
else if (bpm > 120) msg.payload = "🔴 High";
else msg.payload = "🟢 Normal";
return msg;
