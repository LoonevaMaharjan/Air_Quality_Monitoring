const map = L.map('map').setView([27.7172, 85.3240], 13); // Kathmandu coords
L.tileLayer('https://{s}.tile.openstreetmap.org/{z}/{x}/{y}.png', {
  attribution: '&copy; OpenStreetMap contributors'
}).addTo(map);

L.marker([27.704758252922993, 85.2976758535288])
  .addTo(map)
  .bindPopup('Collection Point A')
  .openPopup();

  L.marker([27.674930802873437, 85.32507128977])
  .addTo(map)
  .bindPopup('Collection Point B')
  .openPopup();

  L.marker([27.673378374738203, 85.31187862411826])
  .addTo(map)
  .bindPopup('Collection Point C')
  .openPopup();  
