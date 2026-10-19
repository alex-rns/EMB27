#pragma once

// The whole page lives in flash as one string. It polls /api/state once a second; the mode buttons POST /api/mode.
static const char PAGE[] = R"HTML(<!doctype html>
<html lang="en">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<meta name="color-scheme" content="light dark">
<title>Night light</title>
<style>
:root{--bg:#f2f2f6;--card:#fff;--sel:#fff;--fg:#17171c;--mute:#666672;--track:#e7e7ed;--acc:#2a78d6;--on:#eda100;--off:#c8c8d0;--vio:#6250d6;--bad:#c62f2f}
@media (prefers-color-scheme:dark){:root{--bg:#0d0d10;--card:#1a1a1f;--sel:#3b3b45;--fg:#ececf1;--mute:#9d9daa;--track:#2a2a31;--acc:#4f94ea;--on:#f5b316;--off:#4b4b55;--vio:#9085e9;--bad:#ff7b72}}
*{box-sizing:border-box}
body{margin:0 auto;max-width:560px;padding:16px;background:var(--bg);color:var(--fg);font:15px/1.4 system-ui,sans-serif}
header,.hero,.cap,li{display:flex;align-items:center;gap:8px}
header{justify-content:space-between;margin:0 2px 12px}
h1{font-size:1.3rem;margin:0}
#offline{display:none;background:#c62f2f;color:#fff;font-weight:700;padding:2px 12px;border-radius:99px}
.off #offline{display:block}
.off main{opacity:.5}
main,.hero,.glass,.glow,#ring{transition:.3s}
.card{background:var(--card);border-radius:18px;padding:16px;margin-bottom:12px}
.cap,#reason,.det,#modeDesc,li span{color:var(--mute)}
.cap{font-size:.82rem;font-weight:600;margin-bottom:8px}
.hero{gap:18px}
.on .hero{background:color-mix(in srgb,var(--on) 12%,var(--card))}
#bulb{width:104px;flex:none}
.s{stop-color:var(--on)}
.glow,#ring{opacity:0}
.trk,#ring{fill:none;stroke:var(--track);stroke-width:6}
#ring{stroke:var(--on);stroke-linecap:round}
.glass{fill:var(--off)}
.on .glow{opacity:1}
.on .glass{fill:var(--on)}
#bulb path+path{fill:none;stroke:var(--mute);stroke-width:3.5;stroke-linecap:round}
#lamp{font-size:2.7rem;font-weight:800;line-height:1.1}
.tiles,.seg{display:grid;grid-template-columns:1fr 1fr;gap:12px}
.tiles{margin-bottom:12px}
.tiles .card{margin:0;min-width:0}
.cap svg{width:18px;fill:none;stroke:currentColor;stroke-width:2;stroke-linecap:round}
#lt svg{color:var(--acc)}
#mt svg{color:var(--vio)}
.moon,.dk .sun{display:none}
.dk .moon{display:inline}
.val{font-size:1.5rem;line-height:2.1rem;font-weight:700;white-space:nowrap}
.sm{font-size:1.1rem}
.det{font-size:.8rem;min-height:2.8em}
.seg{grid-template-columns:1fr 1fr 1fr;gap:4px;padding:4px;border-radius:13px;background:var(--track)}
[aria-busy=true]{opacity:.6}
button{font:inherit;font-weight:600;border:0;border-radius:9px;padding:9px 0;background:none;color:var(--mute)}
[aria-pressed=true]{background:var(--sel);color:var(--fg);box-shadow:0 1px 3px #0003}
#modeDesc,#modeErr{margin-top:8px;font-size:.88rem;min-height:1.4em}
#modeErr{color:var(--bad);font-weight:600}
#modeErr:empty{display:none}
canvas{display:block;width:100%;height:180px}
ul{list-style:none;margin:0;padding:0}
li{gap:10px;padding:8px 0;border-top:1px solid var(--track)}
li:first-child{border:0}
li i{width:8px;height:8px;border-radius:50%;flex:none}
li span{margin-left:auto;white-space:nowrap}
ul:empty:after{content:"no events yet";color:var(--mute)}
</style>
</head>
<body>
<header><h1>Night light</h1><span id="offline">offline</span></header>
<main id="main">
<section class="card hero" id="hero">
<svg id="bulb" viewBox="0 0 100 100" aria-hidden="true">
<radialGradient id="gl"><stop class="s" stop-opacity=".6"/><stop class="s" offset="1" stop-opacity="0"/></radialGradient>
<circle class="glow" cx="50" cy="46" r="42" fill="url(#gl)"/>
<circle class="trk" cx="50" cy="50" r="45"/>
<circle id="ring" cx="50" cy="50" r="45" transform="rotate(-90 50 50)"/>
<path class="glass" d="M42 62c0-5-9-8-9-19a17 17 0 0 1 34 0c0 11-9 14-9 19z"/>
<path d="M42 67h16M44 72h12"/>
</svg>
<div><div class="cap">Lamp</div><div id="lamp">-</div><div id="reason">waiting for data</div></div>
</section>
<div class="tiles">
<section class="card" id="lt">
<div class="cap"><svg viewBox="0 0 24 24"><g class="sun"><circle cx="12" cy="12" r="4"/><path d="M12 2v3m0 14v3M2 12h3m14 0h3"/></g><path class="moon" d="M21 12.8A9 9 0 1 1 11.2 3a7 7 0 0 0 9.8 9.8z"/></svg>Light sensor</div>
<div class="val" id="light">-</div><div class="det" id="raw"></div>
</section>
<section class="card" id="mt">
<div class="cap"><svg viewBox="0 0 24 24"><circle cx="12" cy="12" r="2"/><path d="M8.5 8.5a5 5 0 0 0 0 7m7-7a5 5 0 0 1 0 7m-10-10a9 9 0 0 0 0 13m13-13a9 9 0 0 1 0 13"/></svg>Motion</div>
<div class="val" id="motion">-</div>
</section>
</div>
<section class="card">
<div class="cap">Mode</div>
<div class="seg" id="seg" role="group" aria-label="Mode">
<button id="m-auto" onclick="setMode('auto')">Auto</button>
<button id="m-on" onclick="setMode('on')">On</button>
<button id="m-off" onclick="setMode('off')">Off</button>
</div>
<div id="modeDesc"></div>
<div id="modeErr" role="alert"></div>
</section>
<section class="card"><div class="cap" id="ct"></div><canvas id="chart"></canvas></section>
<section class="card"><div class="cap">Events, newest first</div><ul id="events"></ul></section>
</main>
<script>
const TIMEOUT_S = 15, WINDOW_S = 60, RING = 2 * Math.PI * 45;
let hist = [], th = {dark: 1600, light: 2200}, busy = false, posting = false, errTimer;
const $ = id => document.getElementById(id);
const put = (id, t) => { $(id).textContent = t; };
const el = (tag, t = "") => { const e = document.createElement(tag); e.textContent = t; return e; };
const ring = $("ring").style;
const seconds = ms => Math.round(ms / 1000) + " s";

const MODE_DESC = {
  auto: "auto: light and motion decide",
  on: "on: always on, sensors ignored",
  off: "off: always off, sensors ignored"
};

function reasonText(s) {
  if (s.mode === "on") return "mode on: always on";
  if (s.mode === "off") return "mode off: always off";
  if (!s.lamp) return s.dark ? "dark, waiting for motion" : "it is light, stays off";
  if (s.motionAgoMs === 0) return "motion keeps it on";
  return "no motion, off in " + s.leftS + " s";
}

function lightDetail(s) {
  return "raw " + s.raw + " \u00b7 " + (s.raw < s.darkBelow ? "below " + s.darkBelow :
    s.raw > s.lightAbove ? "above " + s.lightAbove : "between: keeps state");
}

function motionText(ms) {
  return ms === null ? "none yet" : ms === 0 ? "now" : seconds(ms) + " ago";
}

function eventColor(t) {
  return "var(--" + (/^lamp on/.test(t) ? "on" : /^Wi-Fi/.test(t) ? "acc" : /^mode/.test(t) ? "vio" : "mute") + ")";
}

function drawChart() {
  const c = $("chart"), r = devicePixelRatio || 1, w = c.clientWidth, h = c.clientHeight, g = c.getContext("2d");
  const css = getComputedStyle(document.body), col = n => css.getPropertyValue("--" + n);
  const n = hist.length, pw = w - 38, y = v => 8 + (h - 28) * (1 - v / 4095), x = i => pw * (1 - (n - 1 - i) / WINDOW_S);
  c.width = w * r; c.height = h * r;
  g.scale(r, r);
  g.fillStyle = col("track");
  g.fillRect(0, y(th.light), pw, y(th.dark) - y(th.light));
  g.font = "11px system-ui";
  g.fillStyle = g.strokeStyle = col("mute");
  g.setLineDash([3, 3]);
  for (const [v, word, dir] of [[th.light, "light", -1], [th.dark, "dark", 1]]) {
    const ly = y(v);
    g.beginPath(); g.moveTo(0, ly); g.lineTo(pw, ly); g.stroke();
    g.fillText(v, pw + 8, ly + dir * 8 + 4);
    g.fillText(word, pw + 8, ly + dir * 21 + 4);
  }
  g.fillText("-" + WINDOW_S + " s", 0, h - 4);
  g.textAlign = "right";
  g.fillText("now", pw, h - 4);
  if (!n) return;
  const line = new Path2D();
  hist.forEach((v, i) => line.lineTo(x(i), y(v)));
  const area = new Path2D(line);
  area.lineTo(pw, y(0)); area.lineTo(x(0), y(0));
  g.setLineDash([]);
  g.fillStyle = g.strokeStyle = col("acc");
  g.globalAlpha = .15; g.fill(area); g.globalAlpha = 1;
  g.lineWidth = 2; g.lineJoin = "round"; g.stroke(line);
  g.beginPath(); g.arc(pw, y(hist[n - 1]), 4.5, 0, 7); g.fill();
}

// addPoint: only a poll adds a chart point, a mode reply does not
function render(s, addPoint) {
  const p = s.lamp && s.mode === "auto" ? Math.min(1, s.leftS / TIMEOUT_S) : 0, mt = motionText(s.motionAgoMs);
  $("main").className = (s.lamp ? "on " : "") + (s.dark ? "dk" : "");
  put("lamp", s.lamp ? "ON" : "OFF");
  put("reason", reasonText(s));
  ring.strokeDashoffset = RING * (1 - p);
  ring.opacity = p ? 1 : 0;
  put("light", s.dark ? "DARK" : "LIGHT");
  put("raw", lightDetail(s));
  put("motion", mt);
  $("motion").className = "val" + (mt.length > 10 ? " sm" : "");
  for (const m of ["auto", "on", "off"]) $("m-" + m).setAttribute("aria-pressed", s.mode === m);
  put("modeDesc", MODE_DESC[s.mode] || "");
  th = {dark: s.darkBelow, light: s.lightAbove};
  if (addPoint && hist.push(s.raw) > WINDOW_S) hist.shift();
  drawChart();
  $("events").replaceChildren(...s.events.slice().reverse().map(e => {
    const li = el("li"), dot = el("i");
    dot.style.background = eventColor(e.text);
    li.append(dot, e.text, el("span", seconds(s.uptimeMs - e.atMs) + " ago"));
    return li;
  }));
}

// a hung board is cut off after 3 s, so polling goes on
function call(url, opt) {
  const ctl = new AbortController(), timer = setTimeout(() => ctl.abort(), 3000);
  opt.signal = ctl.signal;
  return fetch(url, opt)
    .then(r => r.ok ? r.json() : r.text().then(t => { throw new Error(t || "HTTP " + r.status); }))
    .finally(() => clearTimeout(timer));
}

// an error stays readable: until the next press or for 5 s, polls leave it alone
function showErr(t) {
  put("modeErr", t);
  clearTimeout(errTimer);
  errTimer = setTimeout(() => put("modeErr", ""), 5000);
}

function setMode(m) {
  if (posting) return;
  posting = true;
  $("seg").setAttribute("aria-busy", true);
  showErr("");
  call("/api/mode?m=" + m, {method: "POST"})
    .then(s => render(s, false))
    .catch(e => showErr(e.name === "Error" ? e.message : "no answer from the board"))
    .finally(() => { posting = false; $("seg").setAttribute("aria-busy", false); });
}

function poll() {
  if (busy) return;
  busy = true;
  call("/api/state", {cache: "no-store"})
    .then(s => { render(s, true); document.body.classList.remove("off"); })
    .catch(() => document.body.classList.add("off"))
    .finally(() => { busy = false; });
}

ring.strokeDasharray = ring.strokeDashoffset = RING;
put("ct", "Light, last " + WINDOW_S + " s");
drawChart();
setInterval(poll, 1000);
poll();
addEventListener("resize", drawChart);
</script>
</body>
</html>
)HTML";
