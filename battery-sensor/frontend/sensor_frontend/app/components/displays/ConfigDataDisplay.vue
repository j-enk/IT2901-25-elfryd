<script setup lang="ts">
import { defineProps } from 'vue'
import type { ConfigEntry } from '~/types/elfryd'

defineProps<{
    configEntries: ConfigEntry[]
    fetchConfig: () => void
    configFetchLoading: boolean
}>()
</script>

<template>
    <section class="w-full">
        <h3 class="text-lg font-medium mb-2">Configuration Data</h3>

        <div v-if="configEntries.length" class="bg-base-100 p-4 rounded-md overflow-auto max-h-[500px]">
            <pre>{{ JSON.stringify(configEntries, null, 2) }}</pre>
        </div>

        <button @click="fetchConfig" :disabled="configFetchLoading" class="btn btn-success w-full mt-4">
            {{ configFetchLoading ? 'Loading...' : 'Fetch Config Data' }}
        </button>
    </section>
</template>
