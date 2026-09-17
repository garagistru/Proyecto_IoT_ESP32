// ==================================================
//  EnrollaDatos (ED) — Monitor  v1.3.2
//  Единственный JS-файл. Всё в одном месте.
// ==================================================

var CONFIG = {
  API: 'http://192.168.4.1/api/state',
  INTERVAL: 2000,
  BUFFER_MAX: 30,
};

var $ = function(id) { return document.getElementById(id); };
var els = {
  version:          $('version'),
  spiderIcon:       $('spiderIcon'),
  statusDot:        $('statusDot'),
  connectionStatus: $('connectionStatus'),
  totalNodes:       $('totalNodes'),
  activeNodes:      $('activeNodes'),
  dormantNodes:     $('dormantNodes'),
  lastReceive:      $('lastReceive'),
  lastTransmit:     $('lastTransmit'),
  bufferSize:       $('bufferSize'),
  bufferMax:        $('bufferMax'),
  bufferBar:        $('bufferBar'),
  devicesList:      $('devicesList'),
  updateTime:       $('updateTime'),
  debug:            $('debug'),
  debugState:       $('debug-state'),
};

function fetchState() {
  return fetch(CONFIG.API).then(function(res) {
    if (!res.ok) throw new Error('HTTP ' + res.status);
    return res.json();
  });
}

function val(v, fallback) {
  if (v === undefined || v === null) return fallback;
  return v;
}

function setConnected(ok) {
  els.spiderIcon.classList.toggle('disconnected', !ok);
  els.statusDot.classList.toggle('disconnected', !ok);
  els.connectionStatus.classList.toggle('disconnected', !ok);
  els.connectionStatus.textContent = ok ? 'Conectado' : 'Desconectado';
}

function renderCounts(s) {
  els.totalNodes.textContent   = val(s.total,   '—');
  els.activeNodes.textContent  = val(s.active,  '—');
  els.dormantNodes.textContent = val(s.dormant, '—');
}

function renderTransfer(s) {
  els.lastReceive.textContent  = val(s.lastReceive,  '—');
  els.lastTransmit.textContent = val(s.lastTransmit, '—');
}

function renderBuffer(s) {
  var size = val(s.bufferSize, 0);
  var max  = val(s.bufferMax,  CONFIG.BUFFER_MAX);

  els.bufferSize.textContent = size;
  els.bufferMax.textContent  = max;

  var pct = Math.min(100, (size / max) * 100);
  els.bufferBar.style.width = pct + '%';

  els.bufferBar.classList.remove('medium', 'high');
  if (size >= max * 0.8)      els.bufferBar.classList.add('high');
  else if (size >= max * 0.4) els.bufferBar.classList.add('medium');
}

function escapeHtml(str) {
  return String(str).replace(/[&<>"']/g, function(c) {
    var map = { '&':'&amp;', '<':'&lt;', '>':'&gt;', '"':'&quot;', "'":'&#39;' };
    return map[c];
  });
}

function renderDevices(devices) {
  if (!devices || !devices.length) {
    els.devicesList.innerHTML = '<div class="device-empty">Sin dispositivos</div>';
    return;
  }
  els.devicesList.innerHTML = devices.map(function(d) {
    var cls    = d.active ? 'active' : 'dormant';
    var status = (d.status && d.status.length) ? d.status : '--';
    var seen   = d.lastSeen ? d.lastSeen : '--';
    return '<div class="device-item ' + cls + '">' +
           '<span class="device-dot"></span>' +
           '<span class="device-name">' + escapeHtml(d.name) + '</span>' +
           '<span class="device-status">' + escapeHtml(status) + '</span>' +
           '<span class="device-seen">' + escapeHtml(seen) + '</span>' +
           '</div>';
  }).join('');
}

function renderFooter() {
  els.updateTime.textContent = new Date().toLocaleTimeString('es-ES');
}

function render(s) {
  renderCounts(s);
  renderTransfer(s);
  renderBuffer(s);
  renderDevices(s.devices);
  renderFooter();

  if (s.version) els.version.textContent = 'v' + s.version;

  if (els.debug && els.debug.style.display !== 'none') {
    els.debugState.textContent = JSON.stringify(s, null, 2);
  }
}

function tick() {
  fetchState().then(function(state) {
    render(state);
    setConnected(true);
  }).catch(function(e) {
    console.warn('fetch failed:', e.message);
    setConnected(false);
  });
}

document.addEventListener('DOMContentLoaded', function() {
  console.log('EnrollaDatos (ED) starting...');

  if (location.search.indexOf('debug=1') !== -1 && els.debug) {
    els.debug.style.display = 'block';
  }

  tick();
  setInterval(tick, CONFIG.INTERVAL);
});