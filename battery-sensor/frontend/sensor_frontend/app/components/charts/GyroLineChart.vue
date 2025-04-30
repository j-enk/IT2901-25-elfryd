<script setup lang="ts">
import { computed } from "vue";
import { Line } from "vue-chartjs";
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
  CategoryScale,
} from "chart.js";
import "chartjs-adapter-date-fns";
import type { GyroData } from "~/types/elfryd";

import { gyroChartOptions } from "~/components/charts/config/chartGyroOptions";
import { baseChartColors } from "./config/chartBaseConfig";

ChartJS.register(
  Title,
  Tooltip,
  Legend,
  LineElement,
  PointElement,
  LinearScale,
  TimeScale,
  Filler,
  CategoryScale
);

const props = defineProps<{ data: GyroData[] }>();

const chartData = computed(() => {
  const sortedData = [...props.data].sort(
    (a, b) => a.device_timestamp - b.device_timestamp
  );

  return {
    datasets: [
      {
        label: "Roll (°)",
        data: sortedData.map((e) => ({
          x: e.device_timestamp * 1000,
          y: e.roll,
        })),
        borderColor: baseChartColors[0],
        backgroundColor: baseChartColors[0],
        yAxisID: "y1",
        tension: 0.4,
        pointRadius: 2,
        fill: false,
      },
      {
        label: "Pitch (°)",
        data: sortedData.map((e) => ({
          x: e.device_timestamp * 1000,
          y: e.pitch,
        })),
        borderColor: baseChartColors[1],
        backgroundColor: baseChartColors[1],
        yAxisID: "y1",
        tension: 0.4,
        pointRadius: 2,
        fill: false,
      },
      {
        label: "Yaw Rate (°/s)",
        data: sortedData.map((e) => ({
          x: e.device_timestamp * 1000,
          y: e.yawRate,
        })),
        borderColor: baseChartColors[2],
        backgroundColor: baseChartColors[2],
        yAxisID: "y1",
        tension: 0.4,
        pointRadius: 2,
        fill: false,
      },
      {
        label: "Heave (m)",
        data: sortedData.map((e) => ({
          x: e.device_timestamp * 1000,
          y: e.heave,
        })),
        borderColor: baseChartColors[3],
        backgroundColor: baseChartColors[3],
        yAxisID: "y2",
        tension: 0.4,
        pointRadius: 2,
        fill: false,
      },
    ],
  };
});
</script>

<template>
  <div class="chart-container">
    <Line :data="chartData" :options="gyroChartOptions" />
  </div>
</template>

<style scoped>
.chart-container {
  position: relative;
  height: 450px;
}
</style>
