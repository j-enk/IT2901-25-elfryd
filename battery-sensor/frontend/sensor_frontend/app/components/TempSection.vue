<script setup lang="ts">
import { ref, onMounted } from "vue";
import TempLineChart from "~/components/charts/TempLineChart.vue"; // Replace or create this component
import { useElfrydTemp } from "~/composables/useElfrydTemp";

const { tempData, isLoading, error, fetchTemp } = useElfrydTemp();

const limit = ref(100);
const hours = ref(24);
const timeOffset = ref(0);

const handleFetchTemp = async (e: Event) => {
  e.preventDefault();

  const parsedLimit = Number(limit.value);
  const parsedHours = Number(hours.value);
  const parsedTimeOffset = Number(timeOffset.value);

  if (isNaN(parsedLimit) || isNaN(parsedHours) || isNaN(parsedTimeOffset)) {
    console.error(
      "Validation error: limit, hours, and timeOffset must be numbers."
    );
    return;
  }

  try {
    await fetchTemp({
      limit: parsedLimit,
      hours: parsedHours,
      timeOffset: parsedTimeOffset,
    });
  } catch (err) {
    console.error("Error fetching temperature data:", err);
  }
};

onMounted(() => {
  fetchTemp({
    limit: limit.value,
    hours: hours.value,
    timeOffset: timeOffset.value,
  });
});
</script>

<template>
  <section
    class="w-full max-w-7xl rounded-lg mx-auto mt-4"
    aria-labelledby="data-section-title"
  >
    <div v-if="error" class="bg-error/20 border border-error p-4 rounded mb-4">
      Error: {{ error }}
    </div>

    <div class="flex flex-wrap md:flex-nowrap gap-6">
      <!-- Left side: Form + Chart -->
      <div class="w-full md:w-2/3 flex flex-col gap-6">
        <!-- Form -->
        <form
          class="bg-base-300 border-1 border-info rounded-lg p-4 flex flex-col gap-4"
          @submit="handleFetchTemp"
        >
          <div class="flex flex-wrap gap-4 w-full">
            <!-- limit -->
            <div class="flex-1 min-w-[200px]">
              <label for="limit" class="label">Limit</label>
              <input
                id="limit"
                :value="limit"
                type="text"
                class="w-full rounded-md input input-md input-info"
                aria-describedby="limitHelp"
                @input="
                  limit = Number(($event.target as HTMLInputElement).value)
                "
              />
              <small id="limitHelp" class="validator-hint label"
                >Set to 0 to fetch all entries</small
              >
            </div>

            <!-- hours -->
            <div class="flex-1 min-w-[200px]">
              <label for="hours" class="label">Hours</label>
              <input
                id="hours"
                :value="hours"
                type="text"
                class="w-full rounded-md input input-md input-info"
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
                type="text"
                class="w-full rounded-md input input-md input-info"
                @input="
                  timeOffset = Number(($event.target as HTMLInputElement).value)
                "
              />
            </div>
          </div>

          <button
            type="submit"
            :disabled="isLoading"
            class="btn btn-outline btn-info rounded-md mt-6"
          >
            {{ isLoading ? "Loading..." : "Fetch Data" }}
          </button>
        </form>

        <!-- Chart -->
        <div
          v-if="tempData.length"
          class="p-4 bg-base-300 border-1 border-info rounded-lg"
        >
          <h2 class="text-xl text-base-content font-bold mb-4">
            Temperature Chart
          </h2>
          <TempLineChart :data="tempData" />
        </div>
      </div>

      <!-- Right side: Raw JSON -->
      <aside
        class="w-full md:w-1/3 bg-base-300 p-4 border-1 border-info rounded-lg overflow-auto h-fit max-h-[750px]"
      >
        <h2 class="text-lg font-semibold mb-2">Raw Temperature Data</h2>

        <div v-if="tempData.length">
          <pre
            class="text-xs font-mono bg-base-100 p-4 rounded-lg overflow-auto max-h-[600px]"
            >{{ JSON.stringify(tempData, null, 2) }}</pre
          >
        </div>

        <div v-else class="text-sm italic text-base-content">
          No data to display yet.
        </div>
      </aside>
    </div>
  </section>
</template>
