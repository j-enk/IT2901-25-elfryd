import { baseChartOptions } from "./chartBaseConfig";

export const tempChartOptions = {
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
        text: "Temperature (°C)",
        color: "#e5e7eb",
        font: { weight: "bold" },
      },
    },
  },
};
