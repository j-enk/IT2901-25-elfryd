<script setup lang="ts">
import { ref, onMounted } from "vue";
import BoatMotionChart from "../components/charts/BoatMotionChart.vue";
import { useElfrydGyroData } from "../composables/useElfrydGyroData";

const { gyroData, isLoading, error, fetchGyroData } = useElfrydGyroData();

const limit = ref(100);
const hours = ref(24);
const timeOffset = ref(0);

const handleFetchGyro = async (e: Event) => {
  e.preventDefault();
  try {
    await fetchGyroData(
      limit.value,
      hours.value,
      timeOffset.value
    );
  } catch (err) {
    console.error("Error fetching gyro data:", err);
  }
};

onMounted(() => {
  fetchGyroData(
    limit.value,
    hours.value,
    timeOffset.value
  );
});
</script>

<template>
  <section
    class="w-full max-w-7xl rounded-lg mx-auto mt-4"
    aria-labelledby="gyro-section-title"
  >
    <h2 id="gyro-section-title" class="text-2xl font-bold mb-4">Gyro Motion</h2>

    <div v-if="error" class="bg-error/20 border border-error p-4 rounded mb-4">
      Error: {{ error }}
    </div>

    <div class="flex flex-wrap md:flex-nowrap gap-6">
      <div class="w-full md:w-2/3 flex flex-col gap-6">
        <form
          class="bg-base-300 border-1 border-success rounded-lg p-4 flex flex-col gap-4"
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
                class="w-full rounded-md input input-md input-success"
                aria-describedby="limitHelp"
                min="0"
                @input="limit = Number(($event.target as HTMLInputElement).value)"
              />
              <small id="limitHelp" class="validator-hint label">
                Set to 0 to fetch all entries
              </small>
            </div>

            <!-- hours -->
            <div class="flex-1 min-w-[200px]">
              <label for="hours" class="label">Hours</label>
              <input
                id="hours"
                :value="hours"
                type="number"
                class="w-full rounded-md input input-md input-success"
                min="1"
                @input="hours = Number(($event.target as HTMLInputElement).value)"
              />
            </div>

            <!-- timeOffset -->
            <div class="flex-1 min-w-[200px]">
              <label for="timeOffset" class="label">Time Offset</label>
              <input
                id="timeOffset"
                :value="timeOffset"
                type="number"
                class="w-full rounded-md input input-md input-success"
                min="0"
                @input="timeOffset = Number(($event.target as HTMLInputElement).value)"
              />
            </div>
          </div>

          <button
            type="submit"
            :disabled="isLoading"
            class="btn btn-outline btn-success rounded-md mt-6"
          >
            {{ isLoading ? "Loading..." : "Fetch Data" }}
          </button>
        </form>

        <!-- Chart -->
        <div
          v-if="gyroData && gyroData.length"
          class="p-4 bg-base-300 border-1 border-success rounded-lg"
        >
          <h2 class="text-xl text-base-content font-bold mb-4">
            Motion Chart
          </h2>
          <BoatMotionChart :data="gyroData" />
        </div>
      </div>

      <aside
        class="w-full md:w-1/3 bg-base-300 p-4 border-1 border-success rounded-lg overflow-auto h-fit max-h-[750px]"
      >
        <h2 class="text-lg font-semibold mb-2">Raw Gyro Data</h2>

        <div v-if="gyroData && gyroData.length">
          <pre
            class="text-xs font-mono bg-base-100 p-4 rounded-lg overflow-auto max-h-[600px]"
          >{{ JSON.stringify(gyroData, null, 2) }}</pre>
        </div>

        <div v-else class="text-sm italic text-base-content">
          No data to display yet.
        </div>
      </aside>
    </div>
  </section>
</template>
