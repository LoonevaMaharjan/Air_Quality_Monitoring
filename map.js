const map = L.map('map').setView([27.7172, 85.3240], 13); // Kathmandu coords
L.tileLayer('https://{s}.tile.openstreetmap.org/{z}/{x}/{y}.png', {
  attribution: '&copy; OpenStreetMap contributors'
}).addTo(map);

// Example marker
L.marker([27.7172, 85.3240])
  .addTo(map)
  .bindPopup('Collection Point A')
  .openPopup();
