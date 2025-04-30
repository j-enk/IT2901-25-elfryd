import { baseChartOptions } from "./chartBaseConfig";

export const gyroChartOptions = {
  ...baseChartOptions,
  scales: {
    ...baseChartOptions.scales,
    x: {
      ...baseChartOptions.scales.x,
      type: "time",
      time: {
        tooltipFormat: "PPpp",
        unit: "minute",
      },
      title: {
        display: true,
        text: "Timestamp",
        color: "#e5e7eb",
        font: { weight: "bold" },
      },
    },
    y1: {
      type: "linear",
      position: "left",
      title: {
        display: true,
        text: "Orientation (° / °/s)",
        color: "#e5e7eb",
        font: { weight: "bold" },
      },
      ticks: {
        color: "#e5e7eb",
      },
    },
    y2: {
      type: "linear",
      position: "right",
      grid: {
        drawOnChartArea: false,
      },
      title: {
        display: true,
        text: "Heave (m)",
        color: "#e5e7eb",
        font: { weight: "bold" },
      },
      ticks: {
        color: "#e5e7eb",
      },
    },
  },
};
