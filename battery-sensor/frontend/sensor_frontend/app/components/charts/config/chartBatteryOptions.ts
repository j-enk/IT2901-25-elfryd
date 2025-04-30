import { baseChartOptions } from "./chartBaseConfig";

export const batteryChartOptions = {
  ...baseChartOptions,
  scales: {
    ...baseChartOptions.scales,
    x: {
      ...baseChartOptions.scales.x,
      type: "time",
      time: {
        tooltipFormat: "PPpp",
        unit: "hour",
      },
      title: {
        display: true,
        text: "Timestamp",
        color: "#e5e7eb",
        font: { weight: "bold" },
      },
    },
    y: {
      ...baseChartOptions.scales.y,
      title: {
        display: true,
        text: "Voltage (mV)",
        color: "#e5e7eb",
        font: { weight: "bold" },
      },
    },
  },
};
