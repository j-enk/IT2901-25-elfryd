<script setup lang="ts">
import { ref, onMounted } from 'vue';
import type { RadioGroupItem, RadioGroupValue } from '@nuxt/ui'

const configFreq = ref<RadioGroupItem[]>(['Freq: 10', 'Freq: 60', 'Freq: 120'])
const configFreqVal = ref<RadioGroupValue>("Freq: 10")
const sendFromSensor = ref<RadioGroupItem[]>(["Battery", "Gyro", "Temp"])
const sendFromSensorVal = ref<RadioGroupValue>()

const data = ref<any>(null);
const isLoading = ref(true);
const error = ref<string | null>(null);
const battery_id = ref<number>(0);
const limit = ref<number>(10);
const configData = ref<any>(null);
const configLoading = ref(false);

const updateFrequency = () => {
  const frequencyValue = configFreqVal.value?.toString().replace('Freq: ', '') || '10';
  
  const formattedFrequency = `freq ${frequencyValue}`;
  
  console.log('Updating frequency with:', formattedFrequency);
  
  const url = new URL('http://localhost:5196/api/Elfryd/config/frequency');
  
  fetch(url.toString(), {
    method: 'POST',
    headers: {
      'Content-Type': 'application/json',
    },
    body: JSON.stringify({ 
      command: formattedFrequency,
    }),
  })
    .then((response) => {
      if (!response.ok) {
        throw new Error(`API error: ${response.status}`);
      }
      return response.json();
    })
    .then((dataResponse) => {
      console.log('Frequency updated successfully:', dataResponse);
    })
    .catch((err) => {
      console.error('Error updating frequency:', err);
      error.value = err instanceof Error ? err.message : 'Unknown error';
    });
};

const retrieveFromSensor = () => {
  console.log('Retrieve data from sensor:', sendFromSensorVal.value);
  // Add logic to retrieve data from the selected sensor
};

const fetchConfig = async () => {
  configLoading.value = true;
  error.value = null;
  
  try {
    console.log('Fetching ElfrydAPI config data...');
    
    const url = new URL('http://localhost:5196/api/Elfryd/config');
    
    const response = await fetch(url.toString());
    
    if (!response.ok) {
      throw new Error(`API error: ${response.status}`);
    }
    
    const dataResponse = await response.json();
    console.log('Received config data:', dataResponse);
    configData.value = dataResponse;
  } catch (err) {
    console.error('Error fetching config data:', err);
    error.value = err instanceof Error ? err.message : 'Unknown error';
  } finally {
    configLoading.value = false;
  }
};


const fetchBatteryData = async () => {
  isLoading.value = true;
  error.value = null;
  
  try {
    console.log(`Fetching ElfrydAPI battery data with battery_id: ${battery_id.value}, limit: ${limit.value}`);
    
    const url = new URL('http://localhost:5196/api/Elfryd/battery');
    if (battery_id.value) {
      url.searchParams.append('battery_id', battery_id.value.toString());
    }
    if (limit.value) {
      url.searchParams.append('limit', limit.value.toString());
    }
    console.log('URL:', url.toString());
    
    const response = await fetch(url.toString());
    
    if (!response.ok) {
      throw new Error(`API error: ${response.status}`);
    }
    
    const dataResponse = await response.json();
    console.log('Received data:', dataResponse);
    data.value = dataResponse;
  } catch (err) {
    console.error('Error fetching data:', err);
    error.value = err instanceof Error ? err.message : 'Unknown error';
  } finally {
    isLoading.value = false;
  }
};

onMounted(() => {
  fetchBatteryData();
  // fetchConfig();
});
</script>

<template>
  <div class="flex flex-row gap-6 p-4 max-w-[1600px] mx-auto bg-pink-100">
    <!-- Left side - Data Display -->
    <div class="w-2/3 rounded-lg shadow-md p-6">
      <h1 class="text-2xl font-bold mb-4">Elfryd Data</h1>
      
      <div class="bg-green-100 rounded-lg p-4 mb-6 flex gap-4 items-end">
        <div class="flex-1">
          <label for="battery_id" class="font-semibold block mb-1">Battery ID (optional):</label>
          <input 
            id="battery_id" 
            v-model="battery_id" 
            type="text" 
            placeholder="Enter topic name or leave blank for all topics"
            class="w-full p-2 border rounded"
          />
        </div>
        
        <div class="flex-1">
          <label for="limit" class="font-semibold block mb-1">Limit:</label>
          <input 
            id="limit" 
            v-model="limit" 
            type="number" 
            min="1" 
            max="100"
            class="w-full p-2 border rounded"
          />
        </div>
        
        <button 
          @click="fetchBatteryData" 
          :disabled="isLoading"
          class="bg-blue-500 hover:bg-blue-600 disabled:bg-gray-400 text-white px-4 py-2 rounded"
        >
          {{ isLoading ? 'Loading...' : 'Fetch Data' }}
        </button>
      </div>
      
      <div v-if="error" class="bg-red-100 border border-red-300 text-red-800 p-4 rounded mb-4">
        <p>Error: {{ error }}</p>
      </div>
      
      <div v-if="isLoading" class="text-center text-gray-600 p-6">
        <p>Loading data from ElfrydAPI...</p>
      </div>
      
      <div v-else-if="data" class="mt-4">
        <h2 class="text-xl font-bold mb-2">Raw JSON Response</h2>
        <pre class=" text-red-500 bg-gray-100 p-4 rounded-lg overflow-auto max-h-[500px] font-mono text-sm">{{ JSON.stringify(data, null, 2) }}</pre>
      </div>
    </div>

    <!-- Right side - Config Section -->
  <div class="w-1/3 bg-white rounded-lg shadow-md p-6">
    <h3 class="text-xl font-bold mb-4">Config</h3>
    
    <div class="mb-6">
      <h5 class="text-lg font-medium mb-2">Update frequency</h5>
      <URadioGroup legend="Frequency" v-model="configFreqVal" :items="configFreq" class="mb-3" />
      <UButton @click="updateFrequency" class="w-full">Click me to update frequency</UButton>
    </div>
    
    <div class="mb-4">
      <h5 class="text-lg font-medium mb-2">Retrieve data from sensor</h5>
      <URadioGroup legend="Sensor" v-model="sendFromSensorVal" :items="sendFromSensor" class="mb-3" />
      <UButton @click="retrieveFromSensor" class="w-full">Click me to get sensor data</UButton>
    </div>
    
    <div class="mb-6">
      <h5 class="text-lg font-medium mb-2">Configuration Data</h5>
      <button 
        @click="fetchConfig" 
        :disabled="configLoading"
        class="bg-blue-500 hover:bg-blue-600 disabled:bg-gray-400 text-white px-4 py-2 rounded w-full mb-4"
      >
        {{ configLoading ? 'Loading...' : 'Fetch Config Data' }}
      </button>
      
      <!-- Config Data Display Section -->
      <div v-if="configData" class="mt-4">
        <div class="bg-orange-50 border border-gray-200 rounded-lg p-4">
          <pre class="text-red-500 text-sm font-mono overflow-auto max-h-[300px]">{{ JSON.stringify(configData, null, 2) }}</pre>
        </div>
      </div>
      <div v-else-if="configLoading" class="text-center text-gray-600 p-3">
        <p>Loading configuration data...</p>
      </div>
      <div v-else-if="error" class="bg-red-50 text-red-700 p-3 rounded-lg border border-red-200 text-sm">
        <p>{{ error }}</p>
      </div>
      <div v-else class="text-red-500 text-center p-3 italic">
        <p>Click the button to load configuration data</p>
      </div>
    </div>
  </div>
  </div>
</template>

