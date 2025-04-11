<script setup lang="ts">
import { ref, onMounted, computed } from 'vue'
import type { RadioGroupItem, RadioGroupValue } from '@nuxt/ui'

import { useElfrydBatteryData } from '~/composables/useElfrydBatteryData'
import { useElfrydConfig } from '~/composables/useElfrydConfig'
import { useElfrydFrequency } from '~/composables/useElfrydFrequency'
import BatteryLineChart from '../components/charts/BatteryLineChart.vue'

// UI states
const configFreq = ref<RadioGroupItem[]>(['10', '60', '120'])
const configFreqVal = ref<RadioGroupValue>('10')
const sendFromSensor = ref<RadioGroupItem[]>(['Battery', 'Gyro', 'Temp'])
const sendFromSensorVal = ref<RadioGroupValue>()

const battery_id = ref<string>('')
const limit = ref<number>(50)

const { batteryData, isLoading, error, fetchBatteryData } = useElfrydBatteryData()

const { configData, configLoading, fetchConfig } = useElfrydConfig()

const { updateFrequency, error: freqError } = useElfrydFrequency()

const processedData = computed(() =>
  batteryData.value.map((entry) => ({
    ...entry,
    date: new Date(entry.device_timestamp * 1000),
  }))
);

// Hook up frequency button to composable
const handleUpdateFrequency = () => {
  if (configFreqVal.value) {
    updateFrequency(configFreqVal.value.toString())
  }
}

const retrieveFromSensor = () => {
  console.log('Retrieve data from sensor:', sendFromSensorVal.value)
  // Add sensor-specific logic here
}

onMounted(() => {
  fetchBatteryData(battery_id.value, limit.value);
})
</script>

<template>
  <main class="w-full h-full bg-base-100 text-base-content">
    <div class="grid grid-cols-3 gap-6 p-4 min-h-screen w-full max-w-screen-xl mx-auto">
      <!-- Data Display Section -->
      <section class="col-span-2 rounded-lg shadow-md p-6" aria-labelledby="data-section-title">
        <h1 id="data-section-title" class="text-2xl font-bold mb-4">Elfryd Data</h1>

        <form
          class="bg-base-300 border-1 border-success rounded-lg p-4 mb-6 flex flex-col gap-4 justify-start items-start"
          aria-label="Filter data by battery_id and limit" v-on:submit="$event.preventDefault()">
          <div class="flex gap-4 w-full">

            <div class="flex-1">
              <label for="battery_id" class="font-semibold block mb-1">battery_id (optional):</label>
              <input id="battery_id" v-model="battery_id" type="text"
                placeholder="Enter battery_id name or leave blank for all battery_ids"
                class="w-full rounded-md input input-md input-success" aria-describedby="battery_idHelp" />
              <small id="battery_idHelp" class="text-gray-600 text-sm">Leave empty to fetch all betteries</small>
            </div>

            <div class="flex-1">
              <label for="limit" class="font-semibold block mb-1">Limit:</label>
              <input id="limit" v-model="limit" type="number" min="1" max="100"
                class="w-full rounded-md input input-md input-success validator" aria-describedby="limitHelp"
                title="Must be between 1 and 100" />
              <small id="limitHelp" class="validator-hint">Must be between 1 and 100</small>
            </div>
          </div>

          <button type="submit" @click="fetchBatteryData(battery_id, limit)" :disabled="isLoading"
            class="btn btn-outline btn-success rounded-md" aria-label="Fetch messages from API">
            {{ isLoading ? 'Loading...' : 'Fetch Data' }}
          </button>
        </form>

        <div v-if="error || freqError" class="bg-red-100 border border-red-300 text-red-800 p-4 rounded mb-4"
          role="alert" aria-live="assertive">
          <p>Error: {{ error || freqError }}</p>
        </div>

        <div v-if="isLoading" class="text-center text-gray-600 p-6" aria-live="polite">
          <p>Loading data from ElfrydAPI...</p>
        </div>

        <div v-else-if="batteryData" class="mt-4" aria-label="Fetched message data">
          <h2 class="text-xl font-bold mb-2">Raw JSON Response</h2>
          <pre
            class="text-base-content text-sm border-1 border-neutral font-mono bg-base-300 p-4 rounded-lg overflow-auto max-h-[500px] font-mono text-sm"
            aria-label="Raw JSON data">{{ JSON.stringify(batteryData, null, 2) }}</pre>

          <BatteryLineChart :data="processedData" />
        </div>
      </section>

      <!-- Config Section -->
      <aside class="col-span-1 bg-base-200 border-1 border-success rounded-lg shadow-md p-4 h-fit"
        aria-labelledby="config-section-title">
        <h2 id="config-section-title" class="text-xl font-bold mb-4">Configuration</h2>

        <!-- Update Frequency -->
        <section class="mb-6" aria-labelledby="update-frequency-title">
          <URadioGroup legend="Update Frequency" v-model="configFreqVal" :items="configFreq" class="mb-3"
            aria-label="Frequency options" />
          <button @click="handleUpdateFrequency" class="w-full btn btn-outline btn-success"
            aria-label="Send selected frequency to API">
            Click me to update frequency
          </button>
        </section>

        <!-- Sensor Selection -->
        <section class="mb-4" aria-labelledby="sensor-section-title">
          <URadioGroup legend="Sensor" v-model="sendFromSensorVal" :items="sendFromSensor" class="mb-3"
            aria-label="Sensor options" />
          <button @click="retrieveFromSensor" class="btn btn-outline btn-success w-full"
            aria-label="Request sensor data">
            Click me to get sensor data
          </button>
        </section>

        <!-- Fetch Config -->
        <section class="mb-6" aria-labelledby="config-data-title">
          <h3 id="config-data-title" class="text-lg font-medium mb-2">Configuration Data</h3>
          <button @click="fetchConfig" :disabled="configLoading" class="btn btn-success w-full mb-4"
            aria-label="Fetch device configuration from API">
            {{ configLoading ? 'Loading...' : 'Fetch Config Data' }}
          </button>

          <!-- Config Display -->
          <div v-if="configData" class="mt-4" aria-live="polite">
            <div class="bg-base-300 border-1 border-neutral rounded-lg p-4">
              <pre class="text-base-content text-sm font-mono overflow-auto max-h-[300px]"
                aria-label="Configuration data">{{ JSON.stringify(configData, null, 2) }}</pre>
            </div>
          </div>

          <div v-else-if="configLoading" class="text-center bg-base-300 border-1 border-info text-info p-3"
            aria-live="polite">
            <p>Loading configuration data...</p>
          </div>

          <div v-else class="text-sm text-info text-center p-3 italic" aria-live="polite">
            <p>Click the button to load configuration data</p>
          </div>
        </section>
      </aside>
    </div>

  </main>
</template>
