var canvas = document.getElementById('oscope');
var ctx = canvas.getContext('2d');

// Commenta la riga per la produzione:
var gateway = `ws://${window.location.hostname}:81/`;

// Usa questa riga per lo sviluppo locale (inserisci il TUO ip!):
// var gateway = `ws://192.168.1.254:81/`;

var websocket;

function initWebSocket() {
  websocket = new WebSocket(gateway);
  websocket.onmessage = onMessage;
}

function onMessage(event) {
  var data = event.data.split(';');
  
  document.getElementById('tb').innerHTML = data[0];
  document.getElementById('vmax').innerHTML = parseFloat(data[1]).toFixed(1);
  
  var freqValue = parseInt(data[2]);
  document.getElementById('freq').innerHTML = (freqValue > 0) ? freqValue : "--";

  var inHold = (data[3] === "1"); 
  var points = data[4].split(',');
  
  ctx.clearRect(0, 0, canvas.width, canvas.height);
  
  var leftMargin = 40;
  var graphWidth = canvas.width - leftMargin;

  // Asse Y e Testi
  ctx.fillStyle = '#ccc';
  ctx.font = '16px Segoe UI';
  ctx.fillText('5V', 10, 20);
  ctx.fillText('0V', 10, canvas.height - 10);

  ctx.strokeStyle = '#ccc';
  ctx.lineWidth = 2;
  ctx.beginPath();
  ctx.moveTo(leftMargin, 0);
  ctx.lineTo(leftMargin, canvas.height);
  ctx.stroke();

  // Linea tratteggiata centrale
  ctx.strokeStyle = '#333';
  ctx.lineWidth = 1;
  ctx.setLineDash([10, 10]); 
  ctx.beginPath();
  ctx.moveTo(leftMargin, canvas.height / 2);
  ctx.lineTo(canvas.width, canvas.height / 2);
  ctx.stroke();
  ctx.setLineDash([]); 

  // Disegno Onda
  ctx.beginPath();
  ctx.strokeStyle = '#00ff00';
  ctx.lineWidth = 3;
  var xStep = graphWidth / (points.length - 1);
  
  for(var i = 0; i < points.length; i++) {
    var v = parseFloat(points[i]);
    var y = canvas.height - (v / 5.0 * canvas.height); 
    var x = leftMargin + (i * xStep);
    if(i === 0) ctx.moveTo(x, y);
    else ctx.lineTo(x, y);
  }
  ctx.stroke();

  // Overlay Pausa
  if (inHold) {
    var px = leftMargin + (graphWidth / 2) - 10;
    var py = (canvas.height / 2) - 20;
    ctx.fillStyle = '#fff';
    ctx.fillRect(px, py, 6, 40);
    ctx.fillRect(px + 14, py, 6, 40);
  }
}
window.addEventListener('load', initWebSocket);