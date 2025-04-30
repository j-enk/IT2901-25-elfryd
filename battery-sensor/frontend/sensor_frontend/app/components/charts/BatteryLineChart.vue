<template>
    <div class="chart-container">
      <Line :data="chartData" :options="batteryChartOptions" />
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
    TimeScale,
    Filler,
  } from 'chart.js'
  import 'chartjs-adapter-date-fns'
  
  import type { BatteryData } from '~/types/elfryd'
  import { batteryChartColors, batteryChartOptions } from '~/components/charts/config/batteryChartConfig'
  
  ChartJS.register(
    Title,
    Tooltip,
    Legend,
    LineElement,
    PointElement,
    LinearScale,
    TimeScale,
    Filler,
  )
  
  const props = defineProps<{ data: BatteryData[] }>()
  
  const chartData = computed(() => {
    const sorted = [...props.data].sort((a, b) => a.device_timestamp - b.device_timestamp)

    const grouped = sorted.reduce<Record<number, BatteryData[]>>((acc, entry) => {
      const id = entry.battery_id
      if (id === undefined) return acc
      if (!acc[id]) acc[id] = []
      acc[id].push(entry)
      return acc
    }, {})
  
    return {
      datasets: Object.entries(grouped).map(([id, entries], idx) => ({
        label: `Battery ${id}`,
        data: entries.map(e => ({
          x: e.device_timestamp * 1000,
          y: e.voltage,
        })),
        borderColor: batteryChartColors[idx % batteryChartColors.length],
        backgroundColor: batteryChartColors[idx % batteryChartColors.length],
        borderWidth: 2,
        fill: false,
        tension: 0.4,
        pointRadius: 3,
        pointHoverRadius: 5,
      })),
    }
  })
  </script>
  
  <style scoped>
  .chart-container {
    position: relative;
    height: 450px;
  }
  </style>
  