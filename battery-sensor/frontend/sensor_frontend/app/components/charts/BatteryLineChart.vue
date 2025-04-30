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
import type { BatteryData } from "~/types/elfryd";

import { batteryChartOptions } from "~/components/charts/config/chartBatteryOptions";
import { baseChartColors } from "./config/chartBaseConfig";

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

const props = defineProps<{ data: BatteryData[] }>();

const chartData = computed(() => {
  const sortedData = [...props.data].sort(
    (a, b) => a.device_timestamp - b.device_timestamp
  );
  const grouped = sortedData.reduce((acc, entry) => {
    if (!acc[entry.battery_id]) {
      acc[entry.battery_id] = [];
    }
    acc[entry.battery_id]!.push(entry);
    return acc;
  }, {} as Record<number, BatteryData[]>);

  return {
    datasets: Object.entries(grouped).map(([batteryId, entries], index) => ({
      label: `Battery ${batteryId}`,
      data: entries.map((e) => ({
        x: e.device_timestamp * 1000,
        y: e.voltage,
      })),
      borderColor: baseChartColors[index % baseChartColors.length],
      backgroundColor: baseChartColors[index % baseChartColors.length],
      borderWidth: 2,
      fill: false,
      tension: 0.4,
      pointRadius: 3,
      pointHoverRadius: 5,
    })),
  };
});
</script>

<template>
  <div class="chart-container">
    <Line :data="chartData" :options="batteryChartOptions" />
  </div>
</template>

<style scoped>
.chart-container {
  position: relative;
  height: 450px;
}
</style>
