<script setup lang="ts">
import { ref, onMounted } from "vue";
import GyroChart from "~/components/charts/GyroLineChart.vue";

import { useElfrydGyro } from "~/composables/useElfrydGyro";

const { gyroData, isLoading, error, fetchGyro } = useElfrydGyro();

const limit = ref(100);
const hours = ref(24);
const timeOffset = ref(0);

const handleFetchGyro = async (e: Event) => {
  e.preventDefault();

  try {
    await fetchGyro({
      limit: limit.value,
      hours: hours.value,
      timeOffset: timeOffset.value,
    });
  } catch (err) {
    console.error("Error fetching gyro data:", err);
  }
};

onMounted(() => {
  fetchGyro({
    limit: limit.value,
    hours: hours.value,
    timeOffset: timeOffset.value,
  });
});
</script>

<template>
  <section
    class="w-full max-w-7xl rounded-lg mx-auto mt-4"
    aria-labelledby="gyro-data-section"
  >
    <div v-if="error" class="bg-error/20 border border-error p-4 rounded mb-4">
      Error: {{ error }}
    </div>

    <div class="flex flex-wrap md:flex-nowrap gap-6">
      <!-- Left side: Form + Chart -->
      <div class="w-full md:w-2/3 flex flex-col gap-6">
        <!-- Form -->
        <form
          class="bg-base-300 border-1 border-warning rounded-lg p-4 flex flex-col gap-4"
          @submit="handleFetchGyro"
        >
          <div class="flex flex-wrap gap-4 w-full">
            <!-- limit -->
            <div class="flex-1 min-w-[200px]">
              <label for="limit" class="label">Limit</label>
              <input
                id="limit"
                :value="limit"
                type="number"
                class="w-full rounded-md input input-md input-warning"
                min="0"
                max="1000000"
                @input="
                  limit = Number(($event.target as HTMLInputElement).value)
                "
              />
              <small class="validator-hint label"
                >Set to 0 to fetch all entries</small
              >
            </div>

            <!-- hours -->
            <div class="flex-1 min-w-[200px]">
              <label for="hours" class="label">Hours</label>
              <input
                id="hours"
                :value="hours"
                type="number"
                class="w-full rounded-md input input-md input-warning"
                min="1"
                max="1000000"
                @input="
                  hours = Number(($event.target as HTMLInputElement).value)
                "
              />
            </div>

            <!-- timeOffset -->
            <div class="flex-1 min-w-[200px]">
              <label for="timeOffset" class="label">Time Offset</label>
              <input
                id="timeOffset"
                :value="timeOffset"
                type="number"
                class="w-full rounded-md input input-md input-warning"
                min="0"
                max="1000000"
                @input="
                  timeOffset = Number(($event.target as HTMLInputElement).value)
                "
              />
            </div>
          </div>

          <button
            type="submit"
            :disabled="isLoading"
            class="btn btn-outline btn-warning rounded-md mt-6"
          >
            {{ isLoading ? "Loading..." : "Fetch Gyro Data" }}
          </button>
        </form>

        <!-- Chart -->
        <div
          v-if="gyroData.length"
          class="p-4 bg-base-300 border-1 border-warning rounded-lg"
        >
          <h2 class="text-xl text-base-content font-bold mb-4">Gyro Chart</h2>
          <GyroChart :data="gyroData" />
        </div>
      </div>

      <!-- Right side: Raw JSON -->
      <aside
        class="w-full md:w-1/3 bg-base-300 p-4 border-1 border-warning rounded-lg overflow-auto h-fit max-h-[750px]"
      >
        <h2 class="text-lg font-semibold mb-2">Raw Gyro Data</h2>

        <div v-if="gyroData.length">
          <pre
            class="text-xs font-mono bg-base-100 p-4 rounded-lg overflow-auto max-h-[600px]"
            >{{ JSON.stringify(gyroData, null, 2) }}</pre
          >
        </div>

        <div v-else class="text-sm italic text-base-content">
          No data to display yet.
        </div>
      </aside>
    </div>
  </section>
</template>
