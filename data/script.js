var canvas = document.getElementById('oscope');
var ctx = canvas.getContext('2d');

// Commenta la riga per la produzione (LittleFS/ESP32):
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
  var type = data[0]; // Leggiamo la nuova etichetta (OSC, MENU o GEN)

  var overlay = document.getElementById('status-overlay');
  var overlayTitle = document.getElementById('overlay-title');
  var overlayDesc = document.getElementById('overlay-desc');

  // --- GESTIONE STATI ---
  if (type === "MENU") {
    overlay.classList.remove('hidden');
    overlayTitle.innerText = "Menù Principale";
    var voce = data[1] === "0" ? "1. Oscilloscopio" : "2. Gen. d'Onda";
    overlayDesc.innerText = "Selezione attuale: " + voce;
    return; // Ci fermiamo qui, non disegniamo il grafico
  } 
  else if (type === "GEN") {
    overlay.classList.remove('hidden');
    overlayTitle.innerText = "Generatore d'Onda Attivo";
    var onde = ["Sinusoide", "Quadra", "Triangolare"];
    var tipoStr = onde[parseInt(data[2])];
    overlayDesc.innerText = "Forma: " + tipoStr + " | Freq: " + data[1] + " Hz";
    return; // Ci fermiamo qui
  } 
  else if (type === "OSC") {
    // Siamo nell'oscilloscopio: nascondiamo l'overlay e togliamo l'etichetta "OSC" dall'array
    overlay.classList.add('hidden');
    data.shift(); // Elimina il primo elemento ("OSC"), così data[0] torna a essere il Timebase!
  } 
  else {
    // Fallback: se arriva un pacchetto senza etichetta (vecchio firmware), lo disegniamo comunque
    overlay.classList.add('hidden');
  }

  // ==========================================
  // DA QUI IN POI IL TUO CODICE ORIGINALE RIMANE IDENTICO!
  // ==========================================
  
  // 1. Aggiorniamo il Timebase...
  var tbInput = document.getElementById('tb');
  if (document.activeElement !== tbInput) {
    tbInput.value = data[0];
  }
  
  // 2. Aggiorniamo VMax e Frequenza
  document.getElementById('vmax').innerHTML = parseFloat(data[1]).toFixed(1);
  var freqValue = parseInt(data[2]);
  document.getElementById('freq').innerHTML = (freqValue > 0) ? freqValue : "--";

  // 3. Estraiamo lo stato di Hold e i punti dell'onda
  var inHold = (data[3] === "1"); 
  var points = data[4].split(',');
  
  // --- DISEGNO DEL GRAFICO SU CANVAS ---
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

// Avviamo la connessione al caricamento della pagina
window.addEventListener('load', initWebSocket);


// ==========================================
// --- GESTIONE DEI PULSANTI E INPUT WEB ---
// ==========================================

// Funzione base per inviare un comando stringa
function sendCommand(cmd) {
  if (websocket && websocket.readyState === 1) { // 1 = WebSocket OPEN
    websocket.send(cmd);
  }
}

// 1️⃣ Inserimento MANUALE da tastiera nella casella
document.getElementById('tb').addEventListener('change', function() {
  let val = parseInt(this.value);
  if (!isNaN(val)) {
    sendCommand("CMD:TB_SET:" + val); // Inviamo il comando con il valore esatto
  }
  this.blur(); // Rimuove il focus dalla casella dopo aver premuto Invio o cliccato fuori
});


// 2️⃣ Pressione CONTINUA dei bottoni + e - (Hold-to-Repeat)
let pressInterval;

function startPress(cmd) {
  sendCommand(cmd); // Fa un primo scatto immediato al tocco
  // Imposta un timer: ripete il comando ogni 100 millisecondi (10 volte al secondo)
  pressInterval = setInterval(() => {
    sendCommand(cmd);
  }, 100); 
}

function stopPress() {
  clearInterval(pressInterval); // Ferma il timer quando alzi il dito/mouse
}

let btnUp = document.getElementById('btnTbUp');
let btnDown = document.getElementById('btnTbDown');

// Usiamo gli eventi "pointer" perché supportano sia il mouse su PC che il touch su Mobile!
btnUp.addEventListener('pointerdown', (e) => { e.preventDefault(); startPress("CMD:TB_UP"); });
btnUp.addEventListener('pointerup', stopPress);
btnUp.addEventListener('pointerleave', stopPress);
btnUp.addEventListener('pointercancel', stopPress);

btnDown.addEventListener('pointerdown', (e) => { e.preventDefault(); startPress("CMD:TB_DOWN"); });
btnDown.addEventListener('pointerup', stopPress);
btnDown.addEventListener('pointerleave', stopPress);
btnDown.addEventListener('pointercancel', stopPress);


// 3️⃣ Pulsante Pausa / Play (singolo click classico)
document.getElementById('btnHold').addEventListener('click', function() {
  sendCommand("CMD:HOLD");
});

