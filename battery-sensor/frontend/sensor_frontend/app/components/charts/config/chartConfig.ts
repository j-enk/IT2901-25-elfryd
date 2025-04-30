export const chartColors = [
  "#3ABFF8", // info (blue)
  "#36D399", // success (green)
  "#FBBD23", // warning (yellow)
  "#F87272", // error (red)
  "#C084FC", // accent (purple)
  "#818CF8", // primary (blue-violet)
  "#F472B6", // secondary (pink)
  "#60A5FA", // light blue
  "#34D399", // teal
  "#F87171", // soft red
];

export const chartOptions = {
  responsive: true,
  maintainAspectRatio: false,
  parsing: false,
  interaction: {
    mode: "nearest",
    intersect: false,
  },
  scales: {
    x: {
      type: "time",
      time: {
        tooltipFormat: "PPpp",
        unit: "hour",
      },
      ticks: { color: "#d1d5db" },
      grid: { color: "rgba(255, 255, 255, 0.1)" },
      title: {
        display: true,
        text: "Timestamp",
        color: "#e5e7eb",
        font: { weight: "bold" },
      },
    },
    y: {
      ticks: { color: "#d1d5db" },
      grid: { color: "rgba(255, 255, 255, 0.1)" },
      title: {
        display: true,
        text: "Voltage (mV)",
        color: "#e5e7eb",
        font: { weight: "bold" },
      },
    },
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
};
