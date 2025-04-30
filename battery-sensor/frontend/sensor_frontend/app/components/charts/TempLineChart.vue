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
} from "chart.js";
import "chartjs-adapter-date-fns";

import type { TempData } from "~/types/elfryd";
import { baseChartColors } from "./config/chartBaseConfig";
import { tempChartOptions } from "./config/chartTempOptions";

ChartJS.register(
  Title,
  Tooltip,
  Legend,
  LineElement,
  PointElement,
  LinearScale,
  TimeScale,
  Filler
);

const props = defineProps<{ data: TempData[] }>();

const chartData = computed(() => {
  const sortedData = [...props.data].sort(
    (a, b) => a.device_timestamp - b.device_timestamp
  );

  return {
    datasets: [
      {
        label: "Temperature",
        data: sortedData.map((entry) => ({
          x: entry.device_timestamp * 1000,
          y: entry.temperature,
        })),
        borderColor: baseChartColors[0],
        backgroundColor: baseChartColors[0],
        borderWidth: 2,
        fill: false,
        tension: 0.4,
        pointRadius: 3,
        pointHoverRadius: 5,
      },
    ],
  };
});
</script>

<template>
  <div class="chart-container">
    <Line :data="chartData" :options="tempChartOptions" />
  </div>
</template>

<style scoped>
.chart-container {
  position: relative;
  height: 450px;
}
</style>
