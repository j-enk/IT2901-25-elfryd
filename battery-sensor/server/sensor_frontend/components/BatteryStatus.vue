<script setup lang="ts">
const props = defineProps<{
  currentVoltage: number | null;
  averageVoltage: string;
}>();

// Computed properties
const formattedVoltage = () => 
  props.currentVoltage !== null ? props.currentVoltage.toFixed(2) : 'N/A';

const batteryStatus = () => {
  if (props.currentVoltage === null) return 'N/A';
  if (props.currentVoltage > 18.0) return 'Good';
  if (props.currentVoltage > 14.0) return 'Low';
  return 'Critical';
};

const batteryStatusClass = () => {
  if (props.currentVoltage === null) return '';
  if (props.currentVoltage > 18.0) return 'good';
  if (props.currentVoltage > 14.0) return 'warning';
  return 'critical';
};

const batteryPercentage = () => {
  if (props.currentVoltage === null) return 0;
  // Scale between 12V (0%) and 20V (100%)
  const percentage = ((props.currentVoltage - 12.0) / (20.0 - 12.0)) * 100;
  return Math.max(0, Math.min(100, percentage)); // Clamp between 0-100%
};
</script>

<template>
  <div class="data-summary">
    <div class="data-card">
      <h3>Current Voltage</h3>
      <p class="value">{{ formattedVoltage() }} V</p>
      <div class="battery-indicator" :style="{ width: `${batteryPercentage()}%` }"></div>
    </div>
    
    <div class="data-card">
      <h3>Average Voltage</h3>
      <p class="value">{{ averageVoltage }} V</p>
    </div>
    
    <div class="data-card">
      <h3>Battery Status</h3>
      <p class="status" :class="batteryStatusClass()">
        {{ batteryStatus() }}
      </p>
    </div>
  </div>
</template>

<style scoped>
.data-summary {
  display: flex;
  justify-content: space-around;
  flex-wrap: wrap;
  gap: 20px;
}

.data-card {
  flex: 1;
  min-width: 200px;
  background-color: #f0f7ff;
  padding: 15px;
  border-radius: 8px;
  box-shadow: 0 2px 5px rgba(0, 0, 0, 0.1);
  text-align: center;
  position: relative;
  overflow: hidden;
}

.data-card h3 {
  margin-top: 0;
  color: #555;
  font-size: 16px;
}

.value {
  font-size: 24px;
  font-weight: bold;
  color: #36A2EB;
  margin: 10px 0 0;
}

.status {
  font-size: 24px;
  font-weight: bold;
  margin: 10px 0 0;
}

.good {
  color: #4caf50;
}

.warning {
  color: #ff9800;
}

.critical {
  color: #f44336;
}

.battery-indicator {
  height: 6px;
  background-color: #4caf50;
  position: absolute;
  bottom: 0;
  left: 0;
  transition: width 0.5s ease-in-out;
}
</style>