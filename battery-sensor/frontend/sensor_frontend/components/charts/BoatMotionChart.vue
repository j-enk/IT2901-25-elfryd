<template>
    <div class="chart-container">
      <canvas ref="c"></canvas>
    </div>
  </template>
  
  <script setup lang="ts">
  import { ref, onMounted, onBeforeUnmount, watch } from 'vue'
  import Chart from 'chart.js/auto'
  import type { MotionRow } from '~/composables/useElfrydGyroData'
  
  const props = defineProps<{ data: MotionRow[] }>()
  const c     = ref<HTMLCanvasElement | null>(null)
  let chart: Chart | null = null
  
  const build = () => {
    if (!c.value) return
    chart?.destroy()
  
    const labels = props.data.map(r => r.t.toLocaleTimeString())
  
    const makeDS = (label: string, key: keyof MotionRow, color: string, axis = 'y') => ({
      label, borderColor: color, borderWidth: 1.4, fill: false, tension: 0.25,
      data: props.data.map(r => r[key] as number),
      yAxisID: axis, pointRadius: 0
    })
  
    chart = new Chart(c.value, {
      type: 'line',
      data: {
        labels,
        datasets: [
          makeDS('Roll (°)',      'roll',    '#e11d48'),
          makeDS('Pitch (°)',     'pitch',   '#0ea5e9'),
          makeDS('Yaw-rate (°/s)','yawRate', '#6366f1'),
          makeDS('Heave (g)',     'heave',   '#84cc16', 'y2')
        ]
      },
      options: {
        responsive: true,
        maintainAspectRatio: false,
        interaction: { mode: 'index', intersect: false },
        scales: {
          y:  { title: { display: true, text: 'Deg  /  ° s-¹' } },
          y2: { position: 'right', grid: { drawOnChartArea: false },
                title: { display: true, text: 'g' } }
        },
        plugins: { legend: { position: 'bottom' } }
      }
    })
  }
  
  onMounted(build)
  watch(() => props.data, build, { deep: true })
  onBeforeUnmount(() => chart?.destroy())
  </script>
  
  <style scoped>
  .chart-container { position: relative; height: 400px; }
  </style>
  