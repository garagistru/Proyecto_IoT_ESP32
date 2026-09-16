alert('1. JS LOADED');

document.addEventListener('DOMContentLoaded', () => {
  alert('2. DOM READY');

  fetch('http://192.168.4.1/api/state')
    .then(r => r.json())
    .then(d => {
      alert('3. DATA: total=' + d.total);
      document.getElementById('totalNodes').textContent = d.total;
      document.getElementById('activeNodes').textContent = d.active;
      document.getElementById('dormantNodes').textContent = d.dormant;
      document.getElementById('lastReceive').textContent = d.lastReceive;
      document.getElementById('lastTransmit').textContent = d.lastTransmit;
      document.getElementById('updateTime').textContent = new Date().toLocaleTimeString();
    })
    .catch(e => alert('4. ERR: ' + e.message));
});