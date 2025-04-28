<script setup lang="ts">
import { ref, onMounted, onBeforeUnmount, computed, watch } from 'vue';
import BatteryChart from '../components/chart.vue';
import BatteryStatus from '../components/BatteryStatus.vue';

// State for voltage data
const voltageData = ref<number[]>([]);
const timestamps = ref<string[]>([]);
const isLoading = ref(true);
const error = ref<string | null>(null);
const eventSource = ref<EventSource | null>(null);
const hasNewData = ref(false);
const connectionStatus = ref<'connecting' | 'connected' | 'disconnected' | 'failed'>('disconnected');

// Computed values
const currentVoltage = computed(() => {
  if (!voltageData.value.length) return null;
  const last = voltageData.value[voltageData.value.length - 1];
  return typeof last === 'number' ? last : null;
});

const formattedAverageVoltage = computed(() => {
  if (!voltageData.value.length) return 'N/A';
  const sum = voltageData.value.reduce((a, b) => a + b, 0);
  return (sum / voltageData.value.length).toFixed(2);
});

// Function to connect to the SSE stream
const connectToEventStream = () => {
  // Close any existing connection
  if (eventSource.value) {
    eventSource.value.close();
    eventSource.value = null;
  }
  
  isLoading.value = true;
  error.value = null;
  connectionStatus.value = 'connecting';
  
  try {
    console.log('Attempting to connect to SSE endpoint...');
    // Create new EventSource connection to the SSE endpoint
    eventSource.value = new EventSource('http://localhost:5196/api/BatterySensor/stream');
    
    // Set up event listeners
    eventSource.value.onopen = () => {
      console.log('SSE connection established');
      connectionStatus.value = 'connected';
      isLoading.value = false;
    };
    
    eventSource.value.onmessage = (event) => {
  console.log('Received SSE message:', event.data);
  
  // Process the received data
  try {
    const data = JSON.parse(event.data);
    console.log('Parsed SSE data:', data);
    
    if (Array.isArray(data)) {
      console.log(`Successfully parsed array with ${data.length} readings`);
      
      if (data.length > 0) {
        // Format timestamps properly - ensure they're in ISO format
        voltageData.value = data.map((item: any) => Number(item.voltage));
        
        // Properly format the timestamp - assuming it's in ISO format from backend
        timestamps.value = data.map((item: any) => {
          // Check if timestamp is already a valid date string
          const timestamp = new Date(item.timestamp);
          if (isNaN(timestamp.getTime())) {
            console.warn('Invalid timestamp:', item.timestamp);
            return timestamp; // Fallback to string representation
          }
          return timestamp.toLocaleTimeString();
        });
        
        console.log('Updated voltage data:', voltageData.value);
        console.log('Updated timestamps:', timestamps.value);
        
        // Trigger animation for new data
        hasNewData.value = true;
        setTimeout(() => { hasNewData.value = false; }, 1000);
      }
    }
  } catch (parseErr) {
    console.error('Error parsing SSE data:', parseErr);
    error.value = `Error parsing data: ${parseErr.message}`;
  }
};
    
    eventSource.value.onerror = (e) => {
      console.error('SSE connection error:', e);
      connectionStatus.value = 'failed';
      error.value = 'Connection to battery sensor lost. Reconnecting...';
      
      // Close the connection on error
      if (eventSource.value) {
        eventSource.value.close();
        eventSource.value = null;
      }
      
      // Try to reconnect after delay
      console.log('Will attempt to reconnect in 5 seconds...');
      setTimeout(connectToEventStream, 5000);
    };
  } catch (err) {
    console.error('Error setting up SSE connection:', err);
    connectionStatus.value = 'failed';
    error.value = err instanceof Error ? err.message : 'Unknown error';
    isLoading.value = false;
  }
};

// Fallback function for browsers that don't support SSE
const fetchVoltageData = async () => {
  try {
    console.log('Fetching data via polling fallback...');
    isLoading.value = true;
    error.value = null;
    
    const response = await fetch('http://localhost:5196/api/BatterySensor/recent');
    
    if (!response.ok) {
      throw new Error(`API error: ${response.status}`);
    }
    
    const data = await response.json();
    console.log(`Received ${data.length} readings from polling API`);
    
    // Update data arrays
    if (Array.isArray(data) && data.length > 0) {
      voltageData.value = data.map((item: any) => Number(item.voltage));
      timestamps.value = data.map((item: any) => new Date(item.timestamp).toLocaleTimeString());
      
      hasNewData.value = true;
      setTimeout(() => { hasNewData.value = false; }, 1000);
    } else {
      console.warn('Received empty or invalid data from polling API');
    }
  } catch (err) {
    console.error('Error fetching voltage data:', err);
    error.value = err instanceof Error ? err.message : 'Unknown error';
  } finally {
    isLoading.value = false;
  }
};

// Detect if browser supports EventSource
const isSSESupported = () => {
  return typeof EventSource !== 'undefined';
};

let fallbackInterval: number | null = null;

onMounted(() => {
  // Try regular fetch first to confirm data is available
  fetchVoltageData().then(() => {
    if (isSSESupported()) {
      // Connect to SSE stream if supported
      console.log('Using Server-Sent Events for real-time updates');
      connectToEventStream();
    } else {
      // Fall back to polling if SSE not supported
      console.log('Server-Sent Events not supported, falling back to polling');
      fallbackInterval = window.setInterval(fetchVoltageData, 5000);
    }
  });
});

onBeforeUnmount(() => {
  // Clean up EventSource connection
  if (eventSource.value) {
    console.log('Closing SSE connection');
    eventSource.value.close();
    eventSource.value = null;
    connectionStatus.value = 'disconnected';
  }
  
  // Clean up fallback interval if it exists
  if (fallbackInterval !== null) {
    clearInterval(fallbackInterval);
    fallbackInterval = null;
  }
});

// Watch for debugging
watch(voltageData, (newValue) => {
  console.log('voltageData changed:', newValue);
}, { deep: true });
</script>

<template>
  <div class="container">
    <h1>Battery Voltage Monitor</h1>
    
    <div class="connection-status">
      <span 
        :class="{
          'status-connecting': connectionStatus === 'connecting',
          'status-connected': connectionStatus === 'connected',
          'status-disconnected': connectionStatus === 'disconnected',
          'status-failed': connectionStatus === 'failed'
        }"
      >
        {{ 
          connectionStatus === 'connecting' ? 'Connecting to sensor...' :
          connectionStatus === 'connected' ? 'Connected to sensor' :
          connectionStatus === 'disconnected' ? 'Disconnected' :
          'Connection failed' 
        }}
      </span>
    </div>
    
    <div class="chart-wrapper" :class="{ 'new-data-pulse': hasNewData }">
      <div v-if="isLoading && !voltageData.length" class="loading-overlay">
        <p>Loading battery data...</p>
      </div>
      <div v-if="error" class="error-message">
        <p>{{ error }}</p>
      </div>
      
      <div v-if="!voltageData.length && !isLoading" class="no-data-message">
        <p>No battery data available. Please check the sensor connection.</p>
      </div>
      
      <BatteryChart 
        v-if="voltageData.length > 0" 
        :voltage-data="voltageData" 
        :timestamps="timestamps"
        :has-new-data="hasNewData" 
      />
    </div>
    
    <BatteryStatus 
      v-if="voltageData.length > 0"
      :current-voltage="currentVoltage"
      :average-voltage="formattedAverageVoltage"
      :has-new-data="hasNewData"
    />
    
    <div class="debug-panel">
      <h3>Debug Info</h3>
      <p>Connection: {{ connectionStatus }}</p>
      <p>Data Points: {{ voltageData.length }}</p>
      <p>Last Value: {{ currentVoltage }}</p>
      <div v-if="voltageData.length > 0" class="data-preview">
        <p>First 3 values: {{ voltageData.slice(0, 3).join(', ') }}</p>
      </div>
    </div>

    <div class="raw-data-debug" v-if="voltageData.length > 0">
    <details>
      <summary>Debug Raw Data</summary>
      <p>Timestamps ({{timestamps.length}} items): {{ timestamps.join(', ') }}</p>
      <p>Voltage Data ({{voltageData.length}} items): {{ voltageData.join(', ') }}</p>
    </details>
  </div>

  </div>

</template>

<style scoped>
.container {
  max-width: 1200px;
  margin: 0 auto;
  padding: 20px;
  font-family: Arial, sans-serif;
}

h1 {
  text-align: center;
  color: #333;
  margin-bottom: 20px;
}

.chart-wrapper {
  position: relative;
  margin-bottom: 30px;
  transition: all 0.3s ease;
  min-height: 400px;
  background-color: #f9f9f9;
  padding: 15px;
  border-radius: 8px;
  box-shadow: 0 2px 10px rgba(0, 0, 0, 0.1);
}

.new-data-pulse {
  box-shadow: 0 0 15px rgba(54, 162, 235, 0.7);
}

.loading-overlay {
  position: absolute;
  top: 0;
  left: 0;
  right: 0;
  bottom: 0;
  display: flex;
  justify-content: center;
  align-items: center;
  background-color: rgba(255, 255, 255, 0.8);
  z-index: 5;
  border-radius: 8px;
}

.error-message {
  padding: 10px;
  margin-bottom: 10px;
  color: white;
  background-color: #d32f2f;
  border-radius: 4px;
  text-align: center;
}

.no-data-message {
  padding: 20px;
  text-align: center;
  color: #666;
}

.connection-status {
  text-align: center;
  margin-bottom: 15px;
  padding: 8px;
  border-radius: 4px;
  font-weight: bold;
}

.status-connecting {
  color: #ff9800;
}

.status-connected {
  color: #4caf50;
}

.status-disconnected {
  color: #607d8b;
}

.status-failed {
  color: #f44336;
}

.debug-panel {
  margin-top: 20px;
  border: 1px solid #ddd;
  border-radius: 4px;
  padding: 10px;
  background-color: #f5f5f5;
  font-family: monospace;
  font-size: 12px;
}

.debug-panel h3 {
  margin-top: 0;
  margin-bottom: 10px;
  font-size: 14px;
  color: #333;
}

.data-preview {
  background-color: #e0e0e0;
  padding: 5px;
  border-radius: 3px;
  margin-top: 8px;
}
</style>