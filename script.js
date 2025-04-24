const labels = ['10am', '11am', '12pm', '1pm', '2pm'];
const options = {
  type: 'line',
  data: {
    labels: labels,
    datasets: [{
      label: '',
      data: [12, 19, 3, 5, 2],
      fill: true,
      borderColor: 'cyan',
      tension: 0.4
    }]
  },
  options: {
    plugins: { legend: { display: false } },
    scales: { y: { beginAtZero: true } }
  }
};

new Chart(document.getElementById('pm25Chart'), {
  ...options, data: { ...options.data, datasets: [{ ...options.data.datasets[0], label: 'PM2.5', data: [42, 35, 38, 30, 28] }] }
});
new Chart(document.getElementById('co2Chart'), {
  ...options, data: { ...options.data, datasets: [{ ...options.data.datasets[0], label: 'CO2', data: [410, 420, 418, 416, 415] }] }
});
new Chart(document.getElementById('tempChart'), {
  ...options, data: { ...options.data, datasets: [{ ...options.data.datasets[0], label: 'Temp', data: [22, 24, 25, 23, 22] }] }
});
new Chart(document.getElementById('aqiChart'), {
  ...options, data: { ...options.data, datasets: [{ ...options.data.datasets[0], label: 'AQI', data: [78, 70, 65, 60, 55] }] }
});
