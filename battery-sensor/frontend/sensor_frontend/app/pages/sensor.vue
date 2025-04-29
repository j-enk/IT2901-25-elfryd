<script setup lang="ts">
import { ref, onMounted, onBeforeUnmount, computed } from 'vue';
import Chart from 'chart.js/auto';

// State for voltage data
const voltageData = ref<number[]>([]);
const timestamps = ref<string[]>([]);
const chartInstance = ref<Chart | null>(null);
const isLoading = ref(true);
const error = ref<string | null>(null);

// Computed properties for cleaner template
// Computed properties with proper type safety
// const currentVoltage = computed(() => {
//   // Check if array has elements before accessing the last one
//   if (!voltageData.value || voltageData.value.length === 0) return 'N/A';

//   const lastValue = voltageData.value[voltageData.value.length - 1];
//   // Make sure the value is a number before using toFixed
//   return typeof lastValue === 'number' ? lastValue.toFixed(2) : 'N/A';
// });

const currentVoltage = computed(() => {
  if (!voltageData.value.length) return null;
  const last = voltageData.value[voltageData.value.length - 1];
  return typeof last === 'number' ? last : null;
});

const formattedVoltage = computed(() =>
  currentVoltage.value !== null ? currentVoltage.value.toFixed(2) : 'N/A'
);

const formattedAverageVoltage = computed(() => {
  if (!voltageData.value.length) return 'N/A';
  const sum = voltageData.value.reduce((a, b) => a + b, 0);
  return (sum / voltageData.value.length).toFixed(2);
});

const batteryStatus = computed(() => {
  if (currentVoltage.value === null) return 'N/A';
  if (currentVoltage.value > 18.0) return 'Good';
  if (currentVoltage.value > 14.0) return 'Low';
  return 'Critical';
});

const batteryStatusClass = computed(() => {
  if (currentVoltage.value === null) return '';
  if (currentVoltage.value > 18.0) return 'good';
  if (currentVoltage.value > 14.0) return 'warning';
  return 'critical';
});

// const averageVoltage = computed(() => {
//   // Check if array has elements before calculating average
//   if (!voltageData.value || voltageData.value.length === 0) return 'N/A';

//   const sum = voltageData.value.reduce((a, b) => {
//     return a + (typeof b === 'number' ? b : 0);
//   }, 0);

//   return (sum / voltageData.value.length).toFixed(2);
// });

// Function to initialize chart
const initChart = () => {
  const ctx = document.getElementById('voltageChart') as HTMLCanvasElement;
  if (!ctx) return;

  chartInstance.value = new Chart(ctx, {
    type: 'line',
    data: {
      labels: [...timestamps.value],
      datasets: [{
        label: 'Battery Voltage (V)',
        data: [...voltageData.value],
        borderColor: '#36A2EB',
        backgroundColor: 'rgba(54, 162, 235, 0.2)',
        borderWidth: 2,
        stepped: false,
        tension: 0.3,
        fill: true
      }]
    },
    options: {
      responsive: true,
      maintainAspectRatio: false,
      plugins: {
        title: {
          display: true,
          text: 'Battery Voltage over Time',
          font: {
            size: 18
          }
        },
        tooltip: {
          mode: 'index',
          intersect: false
        }
      },
      scales: {
        y: {
          beginAtZero: false,
          min: 12.0,
          max: 20.0,
          title: {
            display: true,
            text: 'Voltage (V)'
          }
        },
        x: {
          title: {
            display: true,
            text: 'Time'
          }
        }
      }
    }
  });
};

// Function to update chart with new data
const updateChart = () => {
  if (chartInstance.value) {
    chartInstance.value.data.labels = [...timestamps.value];
    chartInstance.value.data.datasets[0].data = [...voltageData.value];
    chartInstance.value.update();
  }
};


let lastFetchTime = 0;
const fetchInterval = 5000;
// Function to fetch data from the .NET backend
const fetchVoltageData = async () => {
  const now = Date.now();
  // Prevent too frequent API calls
  if (now - lastFetchTime < fetchInterval) {
    console.log('Skipping fetch - too soon since last fetch');
    return;
  }

  lastFetchTime = now;

  try {
    if (!voltageData.value.length) {
      isLoading.value = true;
    }
    error.value = null;

    console.log('Fetching data from API...');
    const response = await fetch('http://localhost:5196/api/BatterySensor/recent');

    if (!response.ok) {
      throw new Error(`API error: ${response.status}`);
    }

    const data = await response.json();
    console.log(`Received ${data.length} readings from API`);

    // Update data arrays
    voltageData.value = data.map((item: any) => item.voltage);
    timestamps.value = data.map((item: any) => new Date(item.timestamp).toLocaleTimeString());

    updateChart();
  } catch (err) {
    console.error('Error fetching voltage data:', err);
    error.value = err instanceof Error ? err.message : 'Unknown error';
  } finally {
    isLoading.value = false;
  }
};

let dataInterval: number | null = null;

onMounted(() => {
  // Fetch initial data from the backend
  fetchVoltageData();

  // Initialize chart after DOM is loaded
  setTimeout(initChart, 100);

  // Set up interval for polling the backend API
  dataInterval = window.setInterval(fetchVoltageData, 5000);
});

onBeforeUnmount(() => {
  // Clean up interval when component is unmounted
  if (dataInterval !== null) {
    clearInterval(dataInterval);
  }

  // Clean up chart instance
  if (chartInstance.value) {
    chartInstance.value.destroy();
  }
});
</script>

<template>
  <div class="container">
    <h1>Battery Voltage Monitor</h1>

    <div class="chart-container">
      <div v-if="isLoading && !voltageData.length" class="loading-overlay">
        <p>Loading battery data...</p>
      </div>
      <div v-if="error" class="error-message">
        <p>{{ error }}</p>
      </div>
      <canvas id="voltageChart"></canvas>
    </div>

    <div class="data-summary">
      <div class="data-card" v-if="voltageData.length > 0">
        <h3>Current Voltage</h3>
        <p class="value">{{ formattedVoltage }} V</p>
        <div class="battery-indicator" :style="{
          width: currentVoltage.value !== null
            ? `${Math.max(0, Math.min(100, ((currentVoltage.value - 3.2) / (4.2 - 3.2) * 100)))}%`
            : '0%'
        }"></div>
      </div>

      <div class="data-card" v-if="voltageData.length > 0">
        <h3>Average Voltage</h3>
        <p class="value">{{ formattedAverageVoltage }} V</p>
      </div>

      <div class="data-card" v-if="voltageData.length > 0">
        <h3>Battery Status</h3>
        <p class="status" :class="{ batteryStatusClass }">{{ batteryStatus }}</p>
      </div>
    </div>
  </div>
</template>

<style scoped>
.container {
  max-width: 1200px;
  margin: 0 auto;
  padding: 20px;
  font-family: Arial, sans-serif;
}

h1 {
  text-align: center;
  color: #333;
  margin-bottom: 20px;
}

.chart-container {
  position: relative;
  height: 400px;
  margin-bottom: 30px;
  background-color: #f9f9f9;
  padding: 15px;
  border-radius: 8px;
  box-shadow: 0 2px 10px rgba(0, 0, 0, 0.1);
}

.loading-overlay {
  position: absolute;
  top: 0;
  left: 0;
  right: 0;
  bottom: 0;
  display: flex;
  justify-content: center;
  align-items: center;
  background-color: rgba(255, 255, 255, 0.8);
  z-index: 5;
  border-radius: 8px;
}

.error-message {
  padding: 10px;
  margin-bottom: 10px;
  color: white;
  background-color: #d32f2f;
  border-radius: 4px;
  text-align: center;
}

.data-summary {
  display: flex;
  justify-content: space-around;
  flex-wrap: wrap;
  gap: 20px;
}

.data-card {
  flex: 1;
  min-width: 200px;
  background-color: #f0f7ff;
  padding: 15px;
  border-radius: 8px;
  box-shadow: 0 2px 5px rgba(0, 0, 0, 0.1);
  text-align: center;
  position: relative;
  overflow: hidden;
}

.data-card h3 {
  margin-top: 0;
  color: #555;
  font-size: 16px;
}

.value {
  font-size: 24px;
  font-weight: bold;
  color: #36A2EB;
  margin: 10px 0 0;
}

.status {
  font-size: 24px;
  font-weight: bold;
  margin: 10px 0 0;
}

.good {
  color: #4caf50;
}

.warning {
  color: #ff9800;
}

.critical {
  color: #f44336;
}

.battery-indicator {
  height: 6px;
  background-color: #4caf50;
  position: absolute;
  bottom: 0;
  left: 0;
  transition: width 0.5s ease-in-out;
}
</style>
