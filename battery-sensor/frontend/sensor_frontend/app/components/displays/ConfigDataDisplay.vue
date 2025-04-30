<script setup lang="ts">
import { useElfrydConfig } from '~/composables/useElfrydConfig'

const { configEntries, fetchLoading, fetchError, fetchConfig } = useElfrydConfig()

function handleFetchClick(e: MouseEvent): void {
  fetchConfig()
}
</script>

<template>
  <section class="w-full">
    <h3 class="text-lg font-medium">
      Configuration Data <small class="label">(Last 30 minutes)</small>
    </h3>

    <div
      v-if="configEntries.length"
      class="bg-base-100 p-4 rounded-md overflow-auto max-h-[500px]"
    >
      <pre>{{ JSON.stringify(configEntries, null, 2) }}</pre>
    </div>

    <button
      class="btn btn-success w-full mt-4"
      :disabled="fetchLoading"
      @click="handleFetchClick"
    >
      {{ fetchLoading ? 'Loading...' : 'Fetch Config Data' }}
    </button>
  </section>
</template>
