<script setup lang="ts">
import { ref } from "vue";
import { useElfrydConfig } from "~/composables/useElfrydConfig";

const { commandResult, sendLoading, sendError, sendConfigCommand } =
  useElfrydConfig();

const configSensor = [
  { value: "Battery", label: "Battery" },
  { value: "Gyro", label: "Gyro" },
  { value: "Temp", label: "Temp" },
];

const configFreq = [
  { value: "0", label: "No publishing" },
  { value: "60", label: "60s" },
  { value: "120", label: "120s" },
  { value: "180", label: "180s" },
];

const configSensorVal = ref("Battery");
const configFreqVal = ref("");

async function handleUpdateConfig() {
  if (!configSensorVal.value) {
    sendError.value = new Error("Please select a sensor");
    return;
  }

  try {
    const command = `${configSensorVal.value} ${configFreqVal.value}`;
    await sendConfigCommand(command);
  } catch (error) {
    console.error("Error sending config command", error);
  }
}
</script>

<template>
  <section class="mb-6 flex flex-col w-full gap-4">
    <label class="form-control w-full">
      <span class="label font-semibold">Sensor</span>
      <select
        v-model="configSensorVal"
        class="select select-success select-bordered w-full"
      >
        <option
          v-for="option in configSensor"
          :key="option.value"
          :value="option.value"
        >
          {{ option.label }}
        </option>
      </select>
    </label>

    <label class="form-control w-full">
      <span class="label font-semibold">Update Frequency</span>
      <div class="w-full">
        <select
          v-model="configFreqVal"
          class="select select-success select-bordered w-full pr-10"
        >
          <option :value="''">Set Frequency</option>
          <option
            v-for="option in configFreq"
            :key="option.value"
            :value="option.value"
          >
            {{ option.label }}
          </option>
        </select>
      </div>
      <span class="label validator-hint">Optional</span>
    </label>

    <button
      :disabled="sendLoading"
      class="btn btn-outline btn-success w-full"
      @click="handleUpdateConfig"
    >
      <span
        v-if="sendLoading"
        class="loading loading-spinner loading-md mr-2"
      />
      <span v-else>Update Frequency</span>
    </button>

    <div
      v-if="commandResult"
      class="bg-base-100 border-1 border-neutral rounded-lg p-4 mt-4"
    >
      <pre class="text-sm font-mono max-w-full overflow-auto max-h-[300px]"
        >{{ JSON.stringify(commandResult, null, 2) }}
            </pre
      >
    </div>

    <div
      v-if="sendError"
      class="bg-error/20 border border-error text-sm p-4 rounded mt-2"
      role="alert"
    >
      Error: {{ sendError }}
    </div>
  </section>
</template>
