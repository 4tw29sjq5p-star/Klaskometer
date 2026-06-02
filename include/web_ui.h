#pragma once
// Mobile web GUI served directly by the ESP32-C3.
// Reachable at http://klask.local — no internet required, all assets inline.

static const char WEB_UI[] PROGMEM = R"rawhtml(
<!DOCTYPE html>
<html lang="de">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width,initial-scale=1,maximum-scale=1">
<meta name="apple-mobile-web-app-capable" content="yes">
<meta name="theme-color" content="#0f0f14">
<title>KLASK Tester</title>
<style>
:root{
  --bg:#0f0f14;--surface:#1a1a24;--surface2:#22222e;
  --border:rgba(255,255,255,0.1);
  --purple:#7f77dd;--purple-d:#534ab7;--purple-l:#1e1c3a;
  --teal:#1d9e75;--teal-l:#0c2620;
  --coral:#d85a30;--coral-l:#2e1008;
  --amber:#ef9f27;--amber-l:#2a1c02;
  --gray:#888780;--dim:#37374d;
  --white:#e8e6e0;--text2:#9a9994;
  --r:12px;
}
*{box-sizing:border-box;margin:0;padding:0;-webkit-tap-highlight-color:transparent}
body{font-family:system-ui,-apple-system,sans-serif;background:var(--bg);color:var(--white);
  min-height:100dvh;padding:16px 12px 32px;}
h1{font-size:18px;font-weight:500;margin-bottom:2px}
.sub{font-size:12px;color:var(--text2);margin-bottom:16px}

.status-bar{display:flex;align-items:center;gap:8px;background:var(--surface);
  border:0.5px solid var(--border);border-radius:var(--r);
  padding:10px 14px;margin-bottom:14px;flex-wrap:wrap}
.dot{width:9px;height:9px;border-radius:50%;background:var(--dim);flex-shrink:0;transition:background .3s}
.dot.ok{background:var(--teal)}.dot.err{background:var(--coral)}
#status-text{font-size:13px;color:var(--text2);flex:1}

.grid{display:grid;grid-template-columns:repeat(2,1fr);gap:10px;margin-bottom:14px}
.card{background:var(--surface);border:0.5px solid var(--border);
  border-radius:var(--r);padding:12px 14px}
.card.hi{background:var(--purple-l);border-color:var(--purple)}
.card-lbl{font-size:11px;color:var(--text2);margin-bottom:4px;letter-spacing:.03em}
.card-val{font-size:24px;font-weight:500;letter-spacing:-.5px;line-height:1}
.card-val span{font-size:13px;color:var(--text2);margin-left:2px}
.card.hi .card-val{color:var(--purple)}
.card-wide{grid-column:1/-1}

.gauge-wrap{background:var(--surface);border:0.5px solid var(--border);
  border-radius:var(--r);padding:14px;margin-bottom:14px}
.gauge-title{font-size:12px;color:var(--text2);margin-bottom:10px}
.gauge-track{height:20px;background:var(--dim);border-radius:10px;overflow:hidden}
.gauge-fill{height:100%;width:0%;border-radius:10px;
  background:linear-gradient(90deg,var(--teal),var(--purple) 60%,var(--coral));
  transition:width .12s ease}
.gauge-lbl{display:flex;justify-content:space-between;font-size:10px;
  color:var(--text2);margin-top:4px}

.cal-box{background:var(--amber-l);border:0.5px solid var(--amber);
  border-radius:var(--r);padding:12px 14px;margin-bottom:14px;
  display:flex;align-items:center;gap:12px;flex-wrap:wrap}
.cal-icon{font-size:20px;flex-shrink:0}
.cal-text{flex:1}
.cal-text strong{display:block;font-size:13px;font-weight:500;margin-bottom:2px}
.cal-text small{font-size:12px;color:var(--text2)}
.cal-actions{display:flex;gap:6px}

button{font-size:13px;font-weight:500;padding:8px 14px;border-radius:8px;
  border:0.5px solid var(--border);background:var(--surface2);color:var(--white);
  cursor:pointer;transition:opacity .15s,transform .1s;-webkit-appearance:none}
button:active{transform:scale(.96);opacity:.85}
button:disabled{opacity:.35;cursor:not-allowed;transform:none}
.btn-p{background:var(--purple-d);border-color:transparent;color:#fff}
.btn-t{background:var(--teal);border-color:transparent;color:#fff}
.btn-sm{font-size:12px;padding:6px 10px}
.btn-danger{background:var(--coral-l);color:var(--coral);border-color:var(--coral)}

.section{background:var(--surface);border:0.5px solid var(--border);
  border-radius:var(--r);padding:14px;margin-bottom:14px}
.section-hdr{display:flex;justify-content:space-between;align-items:center;
  margin-bottom:12px}
.section-title{font-size:14px;font-weight:500}
.save-row{display:flex;gap:8px;margin-bottom:14px}
.save-row input{flex:1;font-size:13px;padding:8px 10px;border:0.5px solid var(--border);
  border-radius:8px;background:var(--surface2);color:var(--white);outline:none}
.save-row input:focus{border-color:var(--purple)}

.striker-list{display:flex;flex-direction:column;gap:10px}
.s-row{display:flex;align-items:center;gap:10px}
.s-badge{min-width:22px;height:22px;border-radius:5px;display:flex;align-items:center;
  justify-content:center;font-size:12px;font-weight:600;
  background:var(--dim);color:var(--white);flex-shrink:0}
.s-badge.best{background:var(--purple-d)}
.s-name{font-size:13px;flex:1;white-space:nowrap;overflow:hidden;text-overflow:ellipsis}
.s-bar-bg{flex:2;height:14px;background:var(--dim);border-radius:7px;overflow:hidden}
.s-bar{height:100%;border-radius:7px;background:var(--teal);transition:width .4s}
.s-bar.best{background:var(--purple-d)}
.s-val{font-size:13px;font-weight:500;min-width:52px;text-align:right}
.s-del{font-size:16px;color:var(--text2);background:none;border:none;
  padding:2px 4px;line-height:1;cursor:pointer}
.s-del:active{color:var(--coral)}
.no-data{font-size:13px;color:var(--text2);text-align:center;padding:16px 0}

.chart-wrap{background:var(--surface);border:0.5px solid var(--border);
  border-radius:var(--r);padding:14px;margin-bottom:14px}
.chart-hdr{display:flex;justify-content:space-between;align-items:center;margin-bottom:10px}
.chart-title{font-size:13px;font-weight:500}
canvas{display:block;width:100%!important;height:160px!important}

.toast{position:fixed;bottom:24px;left:50%;transform:translateX(-50%) translateY(80px);
  background:var(--teal);color:#fff;padding:10px 18px;border-radius:10px;
  font-size:13px;font-weight:500;transition:transform .25s;z-index:999;
  white-space:nowrap;pointer-events:none}
.toast.show{transform:translateX(-50%) translateY(0)}
</style>
</head>
<body>
<h1>&#x1F9F2; KLASK Tester</h1>
<p class="sub">TMAG5273 &middot; ESP32-C3 &middot; <span id="url-hint">klask.local</span></p>

<div class="status-bar">
  <div class="dot" id="dot"></div>
  <span id="status-text">Verbinde&hellip;</span>
  <button class="btn-sm" id="btn-refresh" onclick="fetchData()">&#8635;</button>
</div>

<div class="cal-box">
  <div class="cal-icon">&#9881;&#65039;</div>
  <div class="cal-text">
    <strong>Kalibrierung</strong>
    <small>Sensor ohne Magnet &ndash; Nullpunkt setzen</small>
  </div>
  <div class="cal-actions">
    <button class="btn-t btn-sm" id="btn-cal" onclick="doCalibrate()">Kalibrieren</button>
    <button class="btn-sm" id="btn-rcal" onclick="doResetCal()">Reset</button>
  </div>
</div>

<div class="grid">
  <div class="card"><div class="card-lbl">Bx</div>
    <div class="card-val" id="v-bx">&ndash;<span>mT</span></div></div>
  <div class="card"><div class="card-lbl">By</div>
    <div class="card-val" id="v-by">&ndash;<span>mT</span></div></div>
  <div class="card"><div class="card-lbl">Bz</div>
    <div class="card-val" id="v-bz">&ndash;<span>mT</span></div></div>
  <div class="card hi"><div class="card-lbl">|B| Betrag</div>
    <div class="card-val" id="v-mag">&ndash;<span>mT</span></div></div>
  <div class="card" style="grid-column:1/-1;display:flex;justify-content:space-between;align-items:center">
    <div>
      <div class="card-lbl">Temperatur</div>
      <div class="card-val" id="v-temp">&ndash;<span>&deg;C</span></div>
    </div>
    <div style="text-align:right">
      <div class="card-lbl">Peak |B|</div>
      <div class="card-val" style="color:var(--purple)" id="v-peak">&ndash;<span>mT</span></div>
    </div>
  </div>
</div>

<div class="gauge-wrap">
  <div class="gauge-title">Feldst&auml;rke |B| &mdash; 0 bis 60 mT</div>
  <div class="gauge-track"><div class="gauge-fill" id="gauge"></div></div>
  <div class="gauge-lbl"><span>0</span><span>15</span><span>30</span><span>45</span><span>60 mT</span></div>
</div>

<div class="chart-wrap">
  <div class="chart-hdr">
    <span class="chart-title">Verlauf |B|</span>
    <button class="btn-sm" onclick="clearChart()">L&ouml;schen</button>
  </div>
  <canvas id="chart"></canvas>
</div>

<div class="section">
  <div class="section-hdr">
    <span class="section-title">Striker Vergleich</span>
    <button class="btn-sm btn-danger" onclick="clearStrikers()">Liste leeren</button>
  </div>
  <div class="save-row">
    <input type="text" id="s-name" placeholder="Striker-Name (z.B. Rot, Blau &hellip;)" maxlength="22">
    <button class="btn-p" id="btn-save" onclick="doSave()">Speichern</button>
  </div>
  <div class="striker-list" id="striker-list">
    <div class="no-data">Noch keine Messungen gespeichert.</div>
  </div>
</div>

<div class="toast" id="toast"></div>

<script>
const BASE = '';
const POLL_MS = 300;
const CHART_MAX = 150;
const chartData = [];
const canvas = document.getElementById('chart');
const ctx2d   = canvas.getContext('2d');

function clearChart() { chartData.length = 0; renderChart(); }

function pushChart(val) {
  chartData.push(val);
  if (chartData.length > CHART_MAX) chartData.shift();
  renderChart();
}

function renderChart() {
  const W = canvas.offsetWidth * devicePixelRatio;
  const H = canvas.offsetHeight * devicePixelRatio;
  canvas.width = W; canvas.height = H;
  const pad = 28 * devicePixelRatio;
  const w = W - pad * 1.5, h = H - pad * 1.2;
  const max = Math.max(60, ...chartData) * 1.05;

  ctx2d.clearRect(0, 0, W, H);
  ctx2d.strokeStyle = 'rgba(255,255,255,0.07)';
  ctx2d.lineWidth = 1;
  for (let g of [0, 0.25, 0.5, 0.75, 1]) {
    const y = pad + h * (1 - g);
    ctx2d.beginPath(); ctx2d.moveTo(pad, y); ctx2d.lineTo(pad + w, y); ctx2d.stroke();
    ctx2d.fillStyle = 'rgba(255,255,255,0.35)';
    ctx2d.font = `${10 * devicePixelRatio}px system-ui`;
    ctx2d.textAlign = 'right';
    ctx2d.fillText((max * g).toFixed(0), pad - 4, y + 4);
  }

  if (chartData.length < 2) return;
  const step = w / (CHART_MAX - 1);

  ctx2d.beginPath();
  ctx2d.moveTo(pad + (CHART_MAX - chartData.length) * step, pad + h);
  chartData.forEach((v, i) => {
    const x = pad + (CHART_MAX - chartData.length + i) * step;
    const y = pad + h * (1 - v / max);
    ctx2d.lineTo(x, y);
  });
  ctx2d.lineTo(pad + w, pad + h);
  ctx2d.closePath();
  const grad = ctx2d.createLinearGradient(0, pad, 0, pad + h);
  grad.addColorStop(0, 'rgba(127,119,221,0.4)');
  grad.addColorStop(1, 'rgba(127,119,221,0)');
  ctx2d.fillStyle = grad; ctx2d.fill();

  ctx2d.beginPath();
  ctx2d.strokeStyle = '#7f77dd';
  ctx2d.lineWidth = 1.5 * devicePixelRatio;
  ctx2d.lineJoin = 'round';
  chartData.forEach((v, i) => {
    const x = pad + (CHART_MAX - chartData.length + i) * step;
    const y = pad + h * (1 - v / max);
    i === 0 ? ctx2d.moveTo(x, y) : ctx2d.lineTo(x, y);
  });
  ctx2d.stroke();
}

let strikers = [];

function renderStrikers() {
  const el = document.getElementById('striker-list');
  if (!strikers.length) {
    el.innerHTML = '<div class="no-data">Noch keine Messungen gespeichert.</div>';
    return;
  }
  const best = strikers[0].magnitude;
  el.innerHTML = strikers.map((s, i) => `
    <div class="s-row">
      <div class="s-badge ${i===0?'best':''}">${i===0?'\u{1F3C6}':i+1}</div>
      <div class="s-name">${esc(s.name)}</div>
      <div class="s-bar-bg">
        <div class="s-bar ${i===0?'best':''}" style="width:${(s.magnitude/best*100).toFixed(1)}%"></div>
      </div>
      <div class="s-val">${s.magnitude.toFixed(2)} mT</div>
      <button class="s-del" onclick="deleteStriker(${i})" title="Entfernen">&times;</button>
    </div>`).join('');
}

async function fetchStrikers() {
  try {
    const r = await fetch(BASE + '/strikers');
    const j = await r.json();
    strikers = j.strikers || [];
    renderStrikers();
  } catch {}
}

async function deleteStriker(idx) {
  try {
    await fetch(BASE + '/delete', {
      method:'POST', headers:{'Content-Type':'application/json'},
      body: JSON.stringify({index: idx})
    });
    await fetchStrikers();
  } catch { toast('Fehler beim Löschen'); }
}

async function clearStrikers() {
  if (!confirm('Alle Striker löschen?')) return;
  for (let i = strikers.length - 1; i >= 0; i--) {
    await fetch(BASE + '/delete', {
      method:'POST', headers:{'Content-Type':'application/json'},
      body: JSON.stringify({index: i})
    });
  }
  strikers = []; renderStrikers();
}

function setVal(id, v, dec=2) {
  const el = document.getElementById(id);
  if (id === 'v-mag' || id === 'v-peak' || id === 'v-temp') {
    el.childNodes[0].textContent = v.toFixed(dec);
  } else {
    el.childNodes[0].textContent = (v >= 0 ? '+' : '') + v.toFixed(dec);
  }
}

async function fetchData() {
  try {
    const r = await fetch(BASE + '/data');
    if (!r.ok) throw new Error();
    const d = await r.json();
    setVal('v-bx',   d.bx);
    setVal('v-by',   d.by);
    setVal('v-bz',   d.bz);
    setVal('v-mag',  d.magnitude);
    setVal('v-temp', d.temp, 1);
    setVal('v-peak', d.peak);
    document.getElementById('gauge').style.width =
      Math.min(100, d.magnitude / 60 * 100).toFixed(1) + '%';
    pushChart(d.magnitude);
    document.getElementById('dot').className = 'dot ok';
    document.getElementById('status-text').textContent =
      'Verbunden · ' + new Date().toLocaleTimeString();
  } catch {
    document.getElementById('dot').className = 'dot err';
    document.getElementById('status-text').textContent = 'Keine Verbindung';
  }
}

async function doCalibrate() {
  try { await fetch(BASE + '/calibrate', {method:'POST'}); toast('Kalibriert ✓'); }
  catch { toast('Fehler'); }
}

async function doResetCal() {
  try { await fetch(BASE + '/reset_cal', {method:'POST'}); toast('Kalibrierung zurückgesetzt'); }
  catch { toast('Fehler'); }
}

async function doSave() {
  const name = document.getElementById('s-name').value.trim() ||
               'Striker ' + (strikers.length + 1);
  try {
    await fetch(BASE + '/save', {
      method:'POST', headers:{'Content-Type':'application/json'},
      body: JSON.stringify({name})
    });
    document.getElementById('s-name').value = '';
    await fetchStrikers();
    toast(name + ' gespeichert ✓');
  } catch { toast('Fehler beim Speichern'); }
}

let toastTimer;
function toast(msg) {
  const el = document.getElementById('toast');
  el.textContent = msg;
  el.classList.add('show');
  clearTimeout(toastTimer);
  toastTimer = setTimeout(() => el.classList.remove('show'), 2200);
}

function esc(s) {
  return s.replace(/&/g,'&amp;').replace(/</g,'&lt;').replace(/>/g,'&gt;');
}

document.getElementById('url-hint').textContent = location.hostname || 'klask.local';
fetchData();
fetchStrikers();
setInterval(fetchData, POLL_MS);
setInterval(fetchStrikers, 3000);
</script>
</body>
</html>
)rawhtml";
