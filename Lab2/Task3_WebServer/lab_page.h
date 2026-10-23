#pragma once
#include <Arduino.h>
const char LAB_PAGE[] PROGMEM = R"LABHTML(
<!doctype html>
<html lang="en"><head><meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1"><title>IoT Dashboard | ESP32-S3</title>
<style>
:root{--ink:#152f35;--muted:#677d82;--line:#dce5e5;--teal:#087f77;--paper:#f3f6f5}*{box-sizing:border-box}body{margin:0;background:var(--paper);color:var(--ink);font:14px/1.5 system-ui,-apple-system,Segoe UI,sans-serif}button,input{font:inherit}button{cursor:pointer;border:0}button:disabled{opacity:.5;cursor:wait}button:focus-visible,input:focus-visible{outline:3px solid #e8a63b;outline-offset:4px}.topbar{height:76px;background:#fff;border-bottom:1px solid var(--line);display:flex;align-items:center;justify-content:space-between;padding:0 max(28px,calc((100vw - 1280px)/2));gap:18px}.brand{display:flex;align-items:center;gap:12px;font-size:17px;font-weight:750;letter-spacing:-.4px}.mark{width:35px;height:35px;display:grid;place-items:center;border-radius:10px;background:var(--ink);color:#9ae2cf;font-size:22px}.device{font-size:12px;font-weight:600;border:1px solid var(--line);border-radius:30px;padding:7px 13px;color:var(--muted)}main{max-width:1336px;margin:auto;padding:36px 28px 22px}.eyebrow{font-size:11px;text-transform:uppercase;letter-spacing:2px;font-weight:750;color:var(--teal);margin:0 0 7px}h1{font-size:34px;line-height:1.2;letter-spacing:-1.3px;margin:0 0 9px}h2{margin:0;font-size:20px;letter-spacing:-.5px}.intro{color:var(--muted);margin:0 0 29px}.layout{display:grid;grid-template-columns:310px 1fr;gap:26px;align-items:start}.sensor{background:#153f43;color:white;border-radius:20px;padding:26px;overflow:hidden}.sensor .eyebrow{color:#9bcac4}.sensor h2{font-size:23px}.tag{display:inline-block;border:1px solid #4e7779;color:#c1dfdb;font-size:11px;padding:4px 9px;border-radius:20px;margin:15px 0}.sensor-graphic{height:100px;display:flex;align-items:center;justify-content:center;color:#7fd7c7;margin:3px 0 8px}.sensor-graphic svg{width:150px;height:100px}.readout{display:flex;align-items:baseline;gap:9px;margin:2px 0 0}.readout output{font-size:62px;letter-spacing:-3px;font-weight:650;font-variant-numeric:tabular-nums;overflow-wrap:anywhere;line-height:1.2}.unit{font-size:20px;color:#a6c8c5}.reading-state{color:#c4dedb;min-height:22px;margin:9px 0 28px}.request{background:#b6f0d8;color:#153f43;font-weight:750;border-radius:10px;width:100%;padding:13px}.sensor .status{font-size:12px;color:#b7d2cf;min-height:40px;margin:14px 0 0}.sensor-note{border-top:1px solid #41676b;margin-top:19px;padding-top:16px;color:#a9c8c5;font-size:12px}.panel-head{display:flex;justify-content:space-between;align-items:center;gap:15px;margin:1px 0 8px}.load{background:white;color:var(--ink);border:1px solid #cddada;border-radius:9px;padding:9px 13px;font-size:12px;font-weight:650;white-space:nowrap}.led-intro{color:var(--muted);font-size:13px;margin:0 0 20px}.grid{display:grid;grid-template-columns:repeat(4,minmax(0,1fr));gap:14px}.led-card{background:#fff;border:1px solid var(--line);border-radius:14px;padding:17px;box-shadow:0 3px 5px #153f4303}.card-top{display:flex;justify-content:space-between;align-items:center;gap:6px}.led-name{font-weight:750;letter-spacing:.8px;font-size:12px}.card-state{font-size:9px;color:var(--muted);white-space:nowrap}.colour-line{display:flex;align-items:center;gap:10px;margin:24px 0}.colour-line input{width:43px;height:43px;border:1px solid #d4dede;border-radius:12px;background:#fff;padding:3px;cursor:pointer;flex-shrink:0}input[type=color]::-webkit-color-swatch-wrapper{padding:0}input[type=color]::-webkit-color-swatch{border:0;border-radius:8px}input[type=color]::-moz-color-swatch{border:0;border-radius:8px}.colour-line label{display:block;font-size:11px;color:var(--muted)}.hex{font:11px ui-monospace,Consolas,monospace}.range-label{display:flex;justify-content:space-between;font-size:10px;gap:4px}.muted{color:var(--muted)}input[type=range]{width:100%;accent-color:var(--teal);margin:11px 0 21px;cursor:pointer}.apply{display:flex;justify-content:space-between;gap:5px;align-items:center;background:#edf5f3;color:#16645e;width:100%;padding:9px;border-radius:7px;font-size:11px;font-weight:650}.apply:hover,.load:hover{background:#dfefe9}.status{overflow-wrap:anywhere}.led-status{font-size:12px;color:var(--muted);padding:13px 0;margin:0;min-height:45px}.footer{margin-top:27px;border-top:1px solid var(--line);padding-top:16px;display:flex;justify-content:space-between;gap:15px;color:var(--muted);font-size:11px}.footer strong{color:#375c60;font-weight:600}@media(min-width:1400px){main{padding-top:46px}}@media(max-width:1150px){.grid{grid-template-columns:repeat(2,minmax(0,1fr))}.layout{grid-template-columns:290px 1fr}}@media(max-width:690px){.topbar{height:66px;padding:0 20px}.brand{font-size:15px}.device{font-size:10px}main{padding:25px 18px}.layout{grid-template-columns:1fr;gap:24px}h1{font-size:29px}.sensor-graphic{display:none}.readout{margin-top:12px}.reading-state{margin-bottom:18px}.sensor-note{margin-top:13px}.sensor .status{min-height:20px}.grid{gap:10px}.led-card{padding:14px}.panel-head{flex-wrap:wrap}.footer{flex-direction:column;gap:4px}}@media(prefers-reduced-motion:no-preference){button{transition:background .15s ease,opacity .15s ease}}
</style></head><body>
<header class="topbar"><div class="brand"><span class="mark" aria-hidden="true">▦</span>IoT Dashboard</div><span class="device">ESP32-S3 · LAB 02</span></header>
<main><p class="eyebrow">Sense &amp; control</p><h1>Your devices, at a glance.</h1><p class="intro">Request a distance measurement and configure each LED independently.</p>
<div class="layout"><section class="sensor" aria-labelledby="sensorTitle"><p class="eyebrow">01 / Sensor</p><h2 id="sensorTitle">Ultrasonic distance</h2><span class="tag">On-demand measurement</span>
<div class="sensor-graphic" aria-hidden="true"><svg viewBox="0 0 160 100" fill="none"><rect x="20" y="23" width="120" height="52" rx="12" stroke="currentColor" stroke-width="1.4"/><circle cx="49" cy="49" r="17" stroke="currentColor" stroke-width="2"/><circle cx="111" cy="49" r="17" stroke="currentColor" stroke-width="2"/><circle cx="49" cy="49" r="10" stroke="currentColor" opacity=".4"/><circle cx="111" cy="49" r="10" stroke="currentColor" opacity=".4"/><path d="M67 83v8m9-8v8m9-8v8m9-8v8" stroke="currentColor"/><path d="M64 12Q80 1 96 12M70 18Q80 10 90 18" stroke="currentColor" stroke-linecap="round" opacity=".65"/></svg></div>
<div class="readout"><output id="distance" aria-label="Distance in centimetres">—</output><span class="unit">cm</span></div><p class="reading-state" id="readingState" role="status">No measurement yet</p><button class="request" id="measure">Request distance <span aria-hidden="true">↗</span></button><p id="distanceStatus" class="status" role="status">Ready when you are.</p><div class="sensor-note">Readings update only when requested. The most recent result stays on screen.</div></section>
<section aria-labelledby="ledTitle"><div class="panel-head"><div><p class="eyebrow">02 / Actuators</p><h2 id="ledTitle">RGB LED control</h2></div><button class="load" id="load">↻ Load device settings</button></div><p class="led-intro">Choose a colour and brightness, then apply it to that LED.</p><div id="ledRows" class="grid"></div><p id="ledStatus" class="status led-status" role="status">Device settings have not been loaded. Controls show defaults.</p></section></div>
<footer class="footer"><span><strong>1 ultrasonic sensor</strong> &nbsp; / &nbsp; <strong>8 individually addressable LEDs</strong></span><span>Local device dashboard</span></footer></main>
<script>
// Student work: implement the matching ESP32 HTTP handlers. No external JS libraries.
const byId = id => document.getElementById(id);
async function api(path, options = {}) {
  const controller = new AbortController();
  const timer = setTimeout(() => controller.abort(), 20000);
  try {
    const response = await fetch(path, {...options, cache:'no-store', signal:controller.signal});
    const data = await response.json();
    if (!response.ok) throw new Error(data.error || `HTTP ${response.status}`);
    return data;
  } finally { clearTimeout(timer); }
}
function message(error) { return error.name === 'AbortError' ? 'Request timed out. Check the connection and try again.' : error.message; }
function validLed(v) {return v && ['id','r','g','b','brightness'].every(k=>Number.isInteger(v[k])) && v.id>=0 && v.id<8 && ['r','g','b','brightness'].every(k=>v[k]>=0 && v[k]<=255);}
function setRow(v) {
  if (!validLed(v)) throw new Error('Invalid LED state returned by server.');
  byId(`colour${v.id}`).value = '#' + [v.r,v.g,v.b].map(n=>n.toString(16).padStart(2,'0')).join('');
  byId(`level${v.id}`).value = v.brightness;
  byId(`value${v.id}`).textContent = v.brightness;
  byId(`hex${v.id}`).textContent = byId(`colour${v.id}`).value.toUpperCase();
  byId(`ledState${v.id}`).textContent = 'Synced';
}
for(let id=0;id<8;id++) {
  const row=document.createElement('article'); row.className='led-card';
  row.innerHTML=`<div class="card-top"><span class="led-name">LED ${String(id+1).padStart(2,'0')}</span><span id="ledState${id}" class="card-state">Not loaded</span></div>
  <div class="colour-line"><input aria-label="LED ${id+1} colour" type="color" id="colour${id}" value="#000000"><div><label for="colour${id}">Colour</label><span id="hex${id}" class="hex">#000000</span></div></div>
  <div class="range-label"><label for="level${id}">Brightness</label><span><output id="value${id}">128</output><span class="muted"> / 255</span></span></div>
  <input aria-label="LED ${id+1} brightness" type="range" min="0" max="255" value="128" id="level${id}">
  <button class="apply" id="apply${id}">Apply to LED ${id+1}<span aria-hidden="true">↗</span></button>`;
  byId('ledRows').appendChild(row);
  byId(`level${id}`).addEventListener('input',e=>{byId(`value${id}`).textContent=e.target.value;byId(`ledState${id}`).textContent='Unsaved';});
  byId(`colour${id}`).addEventListener('input',e=>{byId(`hex${id}`).textContent=e.target.value.toUpperCase();byId(`ledState${id}`).textContent='Unsaved';});
  byId(`apply${id}`).addEventListener('click',async()=>{
    const button=byId(`apply${id}`); button.disabled=true;
    byId(`colour${id}`).disabled=true;byId(`level${id}`).disabled=true;byId('load').disabled=true;
    const colour=byId(`colour${id}`).value;
    const state={r:parseInt(colour.slice(1,3),16),g:parseInt(colour.slice(3,5),16),b:parseInt(colour.slice(5,7),16),brightness:Number(byId(`level${id}`).value)};
    try {
      const data=await api(`/api/leds/${id}`,{method:'PUT',headers:{'Content-Type':'application/json'},body:JSON.stringify(state)});
      if(data.id!==id) throw new Error('Server returned the wrong LED id.');
      setRow(data); byId('ledStatus').textContent=`LED ${id+1} updated.`;
    } catch(e) {byId('ledStatus').textContent=message(e);}
    finally {button.disabled=false;byId(`colour${id}`).disabled=false;byId(`level${id}`).disabled=false;byId('load').disabled=Array.from({length:8},(_,i)=>byId(`apply${i}`).disabled).some(Boolean);}
  });
}
byId('load').addEventListener('click',async()=>{
  byId('load').disabled=true;
  for(let i=0;i<8;i++)for(const key of ['apply','colour','level'])byId(key+i).disabled=true;
  try {
    const data=await api('/api/leds');
    if(!Array.isArray(data.leds)||data.leds.length!==8||!data.leds.every(validLed)||new Set(data.leds.map(v=>v.id)).size!==8) throw new Error('Expected eight unique LED states.');
    data.leds.forEach(setRow);byId('ledStatus').textContent='Current settings loaded.';
  } catch(e){byId('ledStatus').textContent=message(e);}
  finally{byId('load').disabled=false;for(let i=0;i<8;i++)for(const key of ['apply','colour','level'])byId(key+i).disabled=false;}
});
byId('measure').addEventListener('click',async()=>{
  byId('measure').disabled=true;byId('distanceStatus').textContent='Requesting a new measurement...';
  try{
    const data=await api('/api/distance');
    if(data.valid===false){byId('distance').textContent='—';byId('readingState').textContent='No valid echo';}
    else if(data.valid===true && typeof data.distance_cm==='number' && Number.isFinite(data.distance_cm) && data.distance_cm>=0){byId('distance').textContent=data.distance_cm.toFixed(1);byId('readingState').textContent='Measurement received';}
    else throw new Error('Invalid distance response.');
    byId('distanceStatus').textContent='Last request completed at '+new Date().toLocaleTimeString();
  }catch(e){byId('distance').textContent='—';byId('readingState').textContent='Reading unavailable';byId('distanceStatus').textContent=message(e);}
  finally{byId('measure').disabled=false;}
});

// No automatic polling, startup measurements, or automatic LED writes.
</script></body></html>
)LABHTML";
