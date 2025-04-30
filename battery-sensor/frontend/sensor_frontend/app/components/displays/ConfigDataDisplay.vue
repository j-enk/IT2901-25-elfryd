<script setup lang="ts">
import { ref, watch } from "vue";
import { useElfrydConfig } from "~/composables/useElfrydConfig";

const { configEntries, fetchLoading, fetchError, fetchConfig } =
  useElfrydConfig();

const hasFetched = ref(false);

watch(fetchLoading, (loading) => {
  if (!loading) {
    hasFetched.value = true;
  }
});
</script>

<template>
  <section class="w-full">
    <h3 class="text-lg font-medium">
      Configuration Data <small class="label">(Last 30 minutes)</small>
    </h3>

    <!-- Error Display -->
    <div
      v-if="fetchError"
      class="bg-error/20 border border-error text-sm p-4 rounded mt-2"
      role="alert"
    >
      Error: {{ fetchError }}
    </div>

    <!-- Data Display -->
    <div
      v-else-if="configEntries.length"
      class="bg-base-100 p-4 rounded-md overflow-auto max-h-[500px]"
    >
      <pre>{{ JSON.stringify(configEntries, null, 2) }}</pre>
    </div>

    <!-- No Data Message (only after fetch) -->
    <div
      v-else-if="!fetchLoading && hasFetched"
      class="bg-base-200 text-sm text-base-content/80 p-4 rounded-md mt-2"
    >
      No configuration data available for the last 30 minutes.
    </div>

    <!-- Fetch Button -->
    <button
      class="btn btn-success w-full mt-4"
      :disabled="fetchLoading"
      @click="fetchConfig"
    >
      <span
        v-if="fetchLoading"
        class="loading loading-spinner loading-md mr-2"
      />
      <span v-else>Fetch Configuration Data</span>
    </button>
  </section>
</template>
