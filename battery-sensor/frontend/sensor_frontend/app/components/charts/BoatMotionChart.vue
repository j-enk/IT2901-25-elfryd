<template>
  <div class="chart-container">
    <Line :data="chartData" :options="chartOptions" />
  </div>
</template>

<script setup lang="ts">
import { computed } from 'vue'
import { Line } from 'vue-chartjs'
import {
  Chart as ChartJS,
  Title,
  Tooltip,
  Legend,
  LineElement,
  PointElement,
  LinearScale,
  CategoryScale
} from 'chart.js'
import { debounce } from 'lodash-es'
import type { MotionRow } from '~/composables/useElfrydGyroData'

const props = defineProps<{ data: MotionRow[] }>()

ChartJS.register(
  Title,
  Tooltip,
  Legend,
  LineElement,
  PointElement,
  LinearScale,
  CategoryScale
)

// Helper to create a dataset
const makeDataset = (label: string, key: keyof MotionRow, color: string, axis = 'y') => ({
  label,
  borderColor: color,
  borderWidth: 1.4,
  fill: false,
  tension: 0.25,
  data: props.data.map(r => r[key] as number),
  yAxisID: axis,
  pointRadius: 0
})

// Computed chart data
const chartData = computed(() => {
  const labels = props.data.map(r => r.t.toLocaleTimeString())

  return {
    labels,
    datasets: [
      makeDataset('Roll (°)', 'roll', '#e11d48'),
      makeDataset('Pitch (°)', 'pitch', '#0ea5e9'),
      makeDataset('Yaw-rate (°/s)', 'yawRate', '#6366f1'),
      makeDataset('Heave (g)', 'heave', '#84cc16', 'y2')
    ]
  }
})

// Chart options
const chartOptions = computed(() => ({
  responsive: true,
  maintainAspectRatio: false,
  interaction: {
    mode: 'index',
    intersect: false
  },
  scales: {
    y: {
      title: {
        display: true,
        text: '°/s'
      }
    },
    y2: {
      position: 'right',
      grid: {
        drawOnChartArea: false
      },
      title: {
        display: true,
        text: 'g'
      }
    }
  },
  plugins: {
    legend: {
      position: 'bottom'
    }
  }
}))
</script>

<style scoped>
.chart-container {
  position: relative;
  height: 450px;
}
</style>
