#pragma once

// The whole page lives in flash as one string. It polls /api/state once a second and only reads.
static const char PAGE[] = R"HTML(<!doctype html>
<html lang="en">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>Night light</title>
<style>
:root{--bg:#fff;--fg:#1c1c1e;--mute:#6b6b70;--card:#f2f2f4;--on:#e8a400;--bar:#d6d6da;--line:#c0392b;--line2:#2a7a3b;--plot:#2c5fb3}
@media (prefers-color-scheme:dark){:root{--bg:#121214;--fg:#ececf0;--mute:#9a9aa2;--card:#1e1e22;--on:#f2b705;--bar:#34343a;--line:#ff6b5b;--line2:#5ccf74;--plot:#6fa0ff}}
*{box-sizing:border-box}
body{margin:0;padding:16px;background:var(--bg);color:var(--fg);font:16px/1.4 system-ui,sans-serif;max-width:560px;margin-inline:auto}
h1{font-size:1.1rem;margin:0 0 12px;display:flex;justify-content:space-between;gap:8px}
#offline{display:none;color:#fff;background:#c0392b;border-radius:6px;padding:0 8px;font-size:.9rem}
.card{background:var(--card);border-radius:10px;padding:12px 14px;margin-bottom:12px}
.big{font-size:2rem;font-weight:700}
.mute{color:var(--mute);font-size:.9rem}
.row{display:flex;justify-content:space-between;gap:12px;flex-wrap:wrap}
#bar{height:8px;border-radius:4px;background:var(--bar);margin-top:8px;overflow:hidden}
#fill{height:100%;width:0;background:var(--on)}
canvas{width:100%;height:160px;display:block}
ul{list-style:none;margin:0;padding:0}
li{display:flex;justify-content:space-between;gap:12px;padding:3px 0}
li span:last-child{color:var(--mute);white-space:nowrap}
</style>
</head>
<body>
<h1><span>Night light</span><span id="offline">offline</span></h1>
<div class="card">
<div class="big" id="lamp">-</div>
<div id="reason" class="mute">waiting for data</div>
<div id="bar"><div id="fill"></div></div>
</div>
<div class="card row">
<div><div class="mute">Light</div><div class="big" id="light">-</div><div class="mute" id="raw"></div></div>
<div><div class="mute">Motion</div><div class="big" id="motion">-</div></div>
</div>
<div class="card">
<div class="mute">Light, last 5 min</div>
<canvas id="chart"></canvas>
</div>
<div class="card">
<div class="mute">Events, newest first</div>
<ul id="events"></ul>
</div>
<script>
var TIMEOUT_S = 15, MAX_POINTS = 300;
var hist = [], th = {dark: 1600, light: 2200};
var $ = function (id) { return document.getElementById(id); };

function seconds(ms) { return Math.round(ms / 1000) + " s"; }

function reasonText(s) {
  if (!s.lamp) return s.dark ? "dark, waiting for motion" : "it is light, stays off";
  if (s.motionAgoMs === 0) return "motion keeps it on";
  return "no motion, off in " + s.leftS + " s";
}

function lightDetail(s) {
  if (s.raw < s.darkBelow) return "raw " + s.raw + " \u00b7 below " + s.darkBelow;
  if (s.raw > s.lightAbove) return "raw " + s.raw + " \u00b7 above " + s.lightAbove;
  return "raw " + s.raw + " \u00b7 between: keeps state";
}

function motionText(ms) {
  if (ms === null) return "none yet";
  if (ms === 0) return "now";
  return seconds(ms) + " ago";
}

function drawChart() {
  var c = $("chart"), r = window.devicePixelRatio || 1;
  var w = c.clientWidth, h = c.clientHeight;
  c.width = w * r; c.height = h * r;
  var g = c.getContext("2d");
  g.scale(r, r);
  g.clearRect(0, 0, w, h);
  var css = getComputedStyle(document.body);
  var y = function (v) { return h - 4 - (h - 8) * v / 4095; };
  g.font = "11px system-ui,sans-serif";
  g.lineWidth = 1;
  g.setLineDash([4, 3]);
  g.strokeStyle = css.getPropertyValue("--line"); g.fillStyle = g.strokeStyle;
  g.beginPath(); g.moveTo(0, y(th.dark)); g.lineTo(w, y(th.dark)); g.stroke();
  g.fillText("dark below " + th.dark, 4, y(th.dark) - 3);
  g.strokeStyle = css.getPropertyValue("--line2"); g.fillStyle = g.strokeStyle;
  g.beginPath(); g.moveTo(0, y(th.light)); g.lineTo(w, y(th.light)); g.stroke();
  g.fillText("light above " + th.light, 4, y(th.light) - 3);
  g.setLineDash([]);
  if (hist.length < 2) return;
  g.strokeStyle = css.getPropertyValue("--plot"); g.lineWidth = 2;
  g.beginPath();
  for (var i = 0; i < hist.length; i++) {
    var x = w - (hist.length - 1 - i) * w / (MAX_POINTS - 1);
    if (i === 0) g.moveTo(x, y(hist[i])); else g.lineTo(x, y(hist[i]));
  }
  g.stroke();
}

function render(s) {
  $("lamp").textContent = s.lamp ? "Lamp ON" : "Lamp OFF";
  $("reason").textContent = reasonText(s);
  $("fill").style.width = s.lamp ? Math.min(100, s.leftS * 100 / TIMEOUT_S) + "%" : "0";
  $("light").textContent = s.dark ? "DARK" : "LIGHT";
  $("raw").textContent = lightDetail(s);
  $("motion").textContent = motionText(s.motionAgoMs);
  th.dark = s.darkBelow; th.light = s.lightAbove;
  hist.push(s.raw);
  if (hist.length > MAX_POINTS) hist.shift();
  drawChart();
  var ul = $("events");
  ul.textContent = "";
  for (var i = s.events.length - 1; i >= 0; i--) {
    var li = document.createElement("li"), a = document.createElement("span"), b = document.createElement("span");
    a.textContent = s.events[i].text;
    b.textContent = seconds(s.uptimeMs - s.events[i].atMs) + " ago";
    li.appendChild(a); li.appendChild(b); ul.appendChild(li);
  }
}

var busy = false;
function poll() {
  if (busy) return;
  busy = true;
  var ctl = new AbortController();
  var timer = setTimeout(function () { ctl.abort(); }, 3000);  // a hung board must not stop the polling
  fetch("/api/state", {cache: "no-store", signal: ctl.signal})
    .then(function (r) { if (!r.ok) throw new Error(r.status); return r.json(); })
    .then(function (s) { render(s); $("offline").style.display = "none"; })
    .catch(function () { $("offline").style.display = "inline"; })
    .then(function () { clearTimeout(timer); busy = false; });
}
setInterval(poll, 1000);
poll();
window.addEventListener("resize", drawChart);
</script>
</body>
</html>
)HTML";
