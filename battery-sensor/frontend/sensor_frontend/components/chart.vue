<script setup lang="ts">
import { ref, onMounted, onBeforeUnmount, watch } from 'vue';
import Chart from 'chart.js/auto';

// Props definitions
const props = defineProps<{
  voltageData: number[];
  timestamps: string[];
}>();

// Chart instance reference
const chartInstance = ref<Chart | null>(null);
const chartContainer = ref<HTMLCanvasElement | null>(null);

// Function to initialize chart
const initChart = () => {
  const ctx = document.getElementById('voltageChart') as HTMLCanvasElement;
  if (!ctx) return;
  
  chartInstance.value = new Chart(ctx, {
    type: 'line',
    data: {
      labels: [...props.timestamps],
      datasets: [{
        label: 'Battery Voltage (V)',
        data: [...props.voltageData],
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

// Update chart when props change
watch(
  () => [props.voltageData, props.timestamps],
  () => {
    if (chartInstance.value) {
      chartInstance.value.data.labels = [...props.timestamps];
      chartInstance.value.data.datasets[0].data = [...props.voltageData];
      chartInstance.value.update();
    }
  },
  { deep: true }
);

onMounted(() => {
  // Initialize chart after DOM is loaded
  setTimeout(initChart, 100);
});

onBeforeUnmount(() => {
  // Clean up chart instance
  if (chartInstance.value) {
    chartInstance.value.destroy();
  }
});
</script>

<template>
  <div class="chart-container">
    <canvas id="voltageChart"></canvas>
  </div>
</template>

<style scoped>
.chart-container {
  position: relative;
  height: 400px;
  background-color: #f9f9f9;
  padding: 15px;
  border-radius: 8px;
  box-shadow: 0 2px 10px rgba(0, 0, 0, 0.1);
}
</style>