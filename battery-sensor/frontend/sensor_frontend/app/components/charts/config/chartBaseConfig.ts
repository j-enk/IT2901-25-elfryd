export const baseChartColors = [
  "#3ABFF8",
  "#36D399",
  "#FBBD23",
  "#F87272",
  "#C084FC",
  "#818CF8",
  "#F472B6",
  "#60A5FA",
  "#34D399",
  "#F87171",
];

export const baseChartOptions = {
  responsive: true,
  maintainAspectRatio: false,
  parsing: false,
  interaction: {
    mode: "nearest",
    intersect: false,
  },
  plugins: {
    legend: {
      labels: { color: "#e5e7eb" },
      position: "bottom",
    },
    tooltip: {
      backgroundColor: "rgba(30, 41, 59, 0.9)",
      titleColor: "#f9fafb",
      bodyColor: "#d1d5db",
      borderColor: "#3b82f6",
      borderWidth: 1,
    },
  },
  scales: {
    x: {
      ticks: { color: "#d1d5db" },
      grid: { color: "rgba(255, 255, 255, 0.1)" },
    },
    y: {
      ticks: { color: "#d1d5db" },
      grid: { color: "rgba(255, 255, 255, 0.1)" },
    },
  },
};
