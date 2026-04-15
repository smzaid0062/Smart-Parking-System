// Parking Dashboard JS - Mirrors C++ parking_sys exactly
// Enums matching C++
const VehicleType = { Car: 0, Bike: 1, Truck: 2 };
const SpotSize = { Small: 0, Medium: 1, Large: 2 };

const vehicleNames = ['Car', 'Bike', 'Truck'];
const spotNames = ['Small', 'Medium', 'Large'];
const emojis = { Car: '🚗', Bike: '🅱️', Truck: '🚚' };
const colors = { Car: '#10b981', Bike: '#3b82f6', Truck: '#f59e0b' };
const rates = { Car: 20.0, Bike: 10.0, Truck: 40.0 };
const requiredSpot = { Car: SpotSize.Medium, Bike: SpotSize.Small, Truck: SpotSize.Large };

// ParkingSpot class
class ParkingSpot {
  constructor(id, size) {
    this.id = id;
    this.size = size;
    this.isOccupied = false;
    this.occupantPlate = '';
  }
}

// Vehicle class
class Vehicle {
  constructor(plate, type, spotId) {
    this.plate = plate.toUpperCase();
    this.type = type;
    this.entryTime = Date.now() / 1000; // seconds
    this.spotId = spotId;
  }
}

// Ticket class
class Ticket {
  constructor(ticketId, plate, type, spotId, entryTime, exitTime, fee) {
    this.ticketId = ticketId;
    this.plate = plate;
    this.vehicleType = type;
    this.spotId = spotId;
    this.entryTime = entryTime;
    this.exitTime = exitTime;
    this.fee = fee;
  }
}

// ParkingLot class - EXACT C++ replica
class ParkingLot {
  constructor(name) {
    this.name = name;
    this.spots = [];
    this.parkedVehicles = new Map();
    this.ticketHistory = [];
    this.nextTicketId = 1;
    this.totalServed = 0;
    this.totalRevenue = 0.0;

    // Match C++ init: 5 small, 5 medium, 3 large
    let id = 1;
    for (let i = 0; i < 5; i++, id++) this.spots.push(new ParkingSpot(id, SpotSize.Small));
    for (let i = 0; i < 5; i++, id++) this.spots.push(new ParkingSpot(id, SpotSize.Medium));
    for (let i = 0; i < 3; i++, id++) this.spots.push(new ParkingSpot(id, SpotSize.Large));
  }

  findFreeSpot(required) {
    for (let spot of this.spots) {
      if (spot.size === required && !spot.isOccupied) {
        return spot;
      }
    }
    return null;
  }

  parkVehicle(plate, type) {
    if (this.parkedVehicles.has(plate)) {
      return { success: false, message: `Vehicle ${plate} already parked!` };
    }

    const required = requiredSpot[type];
    const spot = this.findFreeSpot(required);

    if (!spot) {
      return { success: false, message: `No ${spotNames[required]} spot available for ${vehicleNames[type]}.` };
    }

    spot.isOccupied = true;
    spot.occupantPlate = plate;
    const vehicle = new Vehicle(plate, type, spot.id);
    this.parkedVehicles.set(plate, vehicle);
    this.totalServed++;

    return { success: true, spot, vehicle };
  }

  exitVehicle(plate) {
    if (!this.parkedVehicles.has(plate)) {
      return { success: false, message: `Vehicle ${plate} not found.` };
    }

    const vehicle = this.parkedVehicles.get(plate);
    const exitTime = Date.now() / 1000;
    const hours = Math.max(1, (exitTime - vehicle.entryTime) / 3600);
    const fee = hours * rates[vehicle.type];

    const ticket = new Ticket(this.nextTicketId++, plate, vehicle.type, vehicle.spotId, vehicle.entryTime, exitTime, fee);
    this.ticketHistory.push(ticket);
    this.totalRevenue += fee;

    // Free spot
    const spot = this.spots.find(s => s.id === vehicle.spotId);
    spot.isOccupied = false;
    spot.occupantPlate = '';
    this.parkedVehicles.delete(plate);

    return { success: true, ticket, hours, fee };
  }

  searchVehicle(plate) {
    if (this.parkedVehicles.has(plate)) {
      const v = this.parkedVehicles.get(plate);
      const now = Date.now() / 1000;
      const hours = Math.max(1, (now - v.entryTime) / 3600);
      const estFee = hours * rates[v.type];
      return { found: true, parked: true, vehicle: v, estFee };
    }
    // Check history
    const history = this.ticketHistory.find(t => t.plate === plate);
    return { found: !!history, parked: false, history };
  }

  getStats() {
    const total = this.spots.length;
    const free = this.spots.filter(s => !s.isOccupied).length;
    const occupancy = 1 - (free / total);

    const small = this.spots.filter(s => s.size === SpotSize.Small);
    const med = this.spots.filter(s => s.size === SpotSize.Medium);
    const large = this.spots.filter(s => s.size === SpotSize.Large);

    return {
      totalSpots: total,
      occupancy,
      revenue: this.totalRevenue,
      parkedCount: this.parkedVehicles.size,
      small: { total: small.length, free: small.filter(s => !s.isOccupied).length },
      med: { total: med.length, free: med.filter(s => !s.isOccupied).length },
      large: { total: large.length, free: large.filter(s => !s.isOccupied).length }
    };
  }
}

// Init
const lot = new ParkingLot('UIC Parking');
let demoMode = false;

// DOM Elements
const elements = {
  grid: document.getElementById('parking-grid'),
  tableBody: document.getElementById('parked-table').querySelector('tbody'),
  occupancyBar: document.getElementById('occupancy-bar'),
  occPercent: document.getElementById('occ-percent'),
  totalSpots: document.getElementById('total-spots'),
  revenue: document.getElementById('revenue'),
  parkedCount: document.getElementById('parked-count'),
  activityLog: document.getElementById('activity-log'),
  plateInput: document.getElementById('plate-input'),
  typeSelect: document.getElementById('type-select'),
  parkBtn: document.getElementById('park-btn'),
  exitBtn: document.getElementById('exit-btn'),
  searchInput: document.getElementById('search-input'),
  demoBtn: document.getElementById('demo-btn'),
  spotsCanvas: document.getElementById('spots-canvas'),
  revenueChart: document.getElementById('revenue-chart')
};

// Utils
function formatTime(timestamp) {
  return new Date(timestamp * 1000).toLocaleString();
}

function createConfetti() {
  for (let i = 0; i < 50; i++) {
    const confetti = document.createElement('div');
    confetti.className = 'confetti';
    confetti.style.left = Math.random() * 100 + '%';
    confetti.style.background = `hsl(${Math.random() * 60 + 180}, 70%, 60%)`;
    confetti.style.animationDelay = Math.random() * 0.5 + 's';
    confetti.style.animationDuration = (Math.random() * 1 + 2) + 's';
    document.body.appendChild(confetti);
    setTimeout(() => confetti.remove(), 4000);
  }
}

function addActivity(message, type = 'info') {
  const item = document.createElement('div');
  item.className = `activity-item ${type}`;
  item.innerHTML = `<strong>${new Date().toLocaleTimeString()}</strong>: ${message}`;
  elements.activityLog.insertBefore(item, elements.activityLog.firstChild);
  if (elements.activityLog.children.length > 10) {
    elements.activityLog.removeChild(elements.activityLog.lastChild);
  }
}

function getTypeFromName(name) {
  return Object.keys(VehicleType).find(key => vehicleNames[VehicleType[key]] === name);
}

// Render Grid (visual 5x6 grid, spots placed by size groups)
function renderGrid() {
  elements.grid.innerHTML = '';
  const visualGrid = Array(5 * 6).fill(null); // 30 cells for visual

  // Place spots: rows 0-1 small (cols 0-5, but only 5), 2-3 med, 4 large (3)
  let spotIdx = 0;
  for (let row = 0; row < 5; row++) {
    const perRow = row < 2 ? 5 : (row < 4 ? 5 : 3); // visual cols
    for (let col = 0; col < perRow && spotIdx < lot.spots.length; col++) {
      const spot = lot.spots[spotIdx++];
      const cellIndex = row * 6 + col;
      const spotEl = document.createElement('div');
      spotEl.className = `spot ${spot.isOccupied ? 'occupied ' + vehicleNames[lot.parkedVehicles.get(spot.occupantPlate)?.type] : 'free'}`;
      spotEl.dataset.id = spot.id;
      spotEl.dataset.plate = spot.occupantPlate;
      spotEl.dataset.size = spotNames[spot.size];
      spotEl.title = `${spotNames[spot.size]} Spot ${spot.id}${spot.isOccupied ? `\\n${spot.occupantPlate}` : ' (Free)'}`;
      spotEl.addEventListener('click', () => {
        if (spot.isOccupied) {
          showSpotInfo(spot);
        }
      });
      visualGrid[cellIndex] = spotEl;
    }
  }

  visualGrid.forEach(cell => {
    if (cell) elements.grid.appendChild(cell);
    else {
      const empty = document.createElement('div');
      empty.style.gridColumn = 'span 1';
      empty.innerHTML = '&nbsp;';
      elements.grid.appendChild(empty);
    }
  });
}

function showSpotInfo(spot) {
  const type = lot.parkedVehicles.get(spot.occupantPlate)?.type;
  alert(`Spot ${spot.id} (${spotNames[spot.size]})\\n${spot.isOccupied ? `Vehicle: ${spot.occupantPlate} (${vehicleNames[type]})` : 'Free'}`);
}

// Render Table
function renderTable(search = '') {
  const parked = Array.from(lot.parkedVehicles.entries())
    .filter(([plate]) => plate.toLowerCase().includes(search.toLowerCase()))
    .sort((a, b) => b[1].entryTime - a[1].entryTime);

  elements.tableBody.innerHTML = '';
  parked.forEach(([plate, vehicle]) => {
    const row = document.createElement('tr');
    const now = Date.now() / 1000;
    const hours = Math.max(1, (now - vehicle.entryTime) / 3600);
    const estFee = (hours * rates[vehicle.type]).toFixed(2);
    row.innerHTML = `
      <td><strong>${plate}</strong></td>
      <td>${emojis[vehicle.type]} ${vehicleNames[vehicle.type]}</td>
      <td>${vehicle.spotId}</td>
      <td>${formatTime(vehicle.entryTime)}</td>
      <td>${hours.toFixed(1)}h</td>
      <td>Rs. ${estFee}</td>
    `;
    elements.tableBody.appendChild(row);
  });
  elements.parkedCount.textContent = `(${parked.length})`;
}

// Render Stats
function renderStats() {
  const stats = lot.getStats();
  const percent = (stats.occupancy * 100).toFixed(1);

  elements.occupancyBar.style.width = `${percent}%`;
  elements.occPercent.textContent = `${percent}%`;
  elements.totalSpots.textContent = stats.totalSpots;
  elements.revenue.textContent = `Rs.${stats.revenue.toFixed(2)}`;
  elements.parkedCount.textContent = `(${stats.parkedCount})`;


  // Spots pie chart (Canvas)

  const ctx = elements.spotsCanvas.getContext('2d');
  const centerX = 60, centerY = 60, radius = 50;
  ctx.clearRect(0, 0, 120, 120);
  const smallFree = stats.small.free / stats.small.total;
  const medFree = stats.med.free / stats.med.total;
  const largeFree = stats.large.free / stats.large.total;
  let startAngle = 0;

  // Small blue
  ctx.fillStyle = colors.Bike;
  ctx.beginPath();
  ctx.moveTo(centerX, centerY);
  ctx.arc(centerX, centerY, radius, startAngle, startAngle + Math.PI * 2 * smallFree);
  ctx.closePath();
  ctx.fill();
  startAngle += Math.PI * 2 * smallFree;

  // Med green
  ctx.fillStyle = colors.Car;
  ctx.beginPath();
  ctx.moveTo(centerX, centerY);
  ctx.arc(centerX, centerY, radius, startAngle, startAngle + Math.PI * 2 * medFree);
  ctx.closePath();
  ctx.fill();
  startAngle += Math.PI * 2 * medFree;

  // Large yellow
  ctx.fillStyle = colors.Truck;
  ctx.beginPath();
  ctx.moveTo(centerX, centerY);
  ctx.arc(centerX, centerY, radius, startAngle, startAngle + Math.PI * 2 * largeFree);
  ctx.closePath();
  ctx.fill();

  // Legend
  document.getElementById('spots-summary').innerHTML = `
    S: ${stats.small.free}/${stats.small.total} | 
    M: ${stats.med.free}/${stats.med.total} | 
    L: ${stats.large.free}/${stats.large.total}
  `;

  // Simple revenue bar (placeholder - accumulate from tickets)
  const revenueCtx = elements.revenueChart.getContext('2d');
  revenueCtx.clearRect(0, 0, 200, 100);
  const maxRevenue = Math.max(1000, lot.totalRevenue * 1.2);
  const barHeight = (lot.totalRevenue / maxRevenue) * 80;
  revenueCtx.fillStyle = '#10b981';
  revenueCtx.fillRect(20, 100 - barHeight, 30, barHeight);
  revenueCtx.fillStyle = '#ffffff';
  revenueCtx.font = 'bold 16px Inter';
  revenueCtx.textAlign = 'center';
  revenueCtx.fillText(`Rs.${lot.totalRevenue.toFixed(0)}`, 35, 25);

}

// Event Listeners
document.addEventListener('DOMContentLoaded', () => {
  // Load from storage
  const savedVehicles = JSON.parse(localStorage.getItem('parkingVehicles') || '[]');
  savedVehicles.forEach(v => {
    const type = getTypeFromName(v.type);
    if (type !== undefined) {
      const res = lot.parkVehicle(v.plate, type);
      if (res.success) v.entryTime = res.vehicle.entryTime; // Update time
    }
  });

  renderAll();
  setupListeners();
});

function setupListeners() {
  elements.parkBtn.addEventListener('click', parkHandler);
  elements.exitBtn.addEventListener('click', exitHandler);
  elements.demoBtn.addEventListener('click', demoHandler);
  elements.searchInput.addEventListener('input', (e) => renderTable(e.target.value));
  elements.plateInput.addEventListener('keypress', (e) => {
    if (e.key === 'Enter') parkHandler();
  });
}

function parkHandler() {
  const plate = elements.plateInput.value.trim();
  const typeName = elements.typeSelect.value;
  const type = getTypeFromName(typeName);

  if (!plate) {
    alert('Enter plate number!');
    return;
  }

  const res = lot.parkVehicle(plate, type);
  if (res.success) {
    addActivity(`✅ Parked ${plate} (${typeName}) at Spot ${res.spot.id}`);
    createConfetti();
    elements.plateInput.value = '';
  } else {
    addActivity(`❌ ${res.message}`, 'error');
  }
  renderAll();
  saveState();
}

function exitHandler() {
  const plate = elements.plateInput.value.trim() || elements.searchInput.value.trim();
  if (!plate) {
    alert('Enter plate number!');
    return;
  }

  const res = lot.exitVehicle(plate);
  if (res.success) {
    addActivity(`💰 Exited ${plate} | Fee: Rs.${res.fee.toFixed(2)} | Ticket #${res.ticket.ticketId}`);
    createConfetti();
  } else {
    addActivity(`❌ ${res.message}`, 'error');
  }
  renderAll();
  saveState();
}

function demoHandler() {
  demoMode = !demoMode;
  if (demoMode) {
    // Park demo vehicles matching C++
    lot.parkVehicle('MH01AB1234', VehicleType.Car);
    lot.parkVehicle('DL02CD5678', VehicleType.Bike);
    lot.parkVehicle('KA03EF9012', VehicleType.Truck);
    addActivity('🎉 Demo mode activated - 3 vehicles parked!');
    createConfetti();
    demoMode = false;
  }
  renderAll();
  saveState();
}

function renderAll() {
  renderGrid();
  renderTable();
  renderStats();
}

function saveState() {
  const vehicles = Array.from(lot.parkedVehicles.values()).map(v => ({
    plate: v.plate,
    type: vehicleNames[v.type],
    entryTime: v.entryTime,
    spotId: v.spotId
  }));
  localStorage.setItem('parkingVehicles', JSON.stringify(vehicles));
  localStorage.setItem('totalRevenue', lot.totalRevenue.toString());
  localStorage.setItem('ticketHistory', JSON.stringify(lot.ticketHistory));
  localStorage.setItem('totalServed', lot.totalServed.toString());
}
