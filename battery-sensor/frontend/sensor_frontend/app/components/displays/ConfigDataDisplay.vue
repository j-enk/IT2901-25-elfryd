<script setup lang="ts">
import type { ConfigEntry } from '~/types/elfryd'

import { useElfrydConfig } from '~/composables/useElfrydConfig'

const { configEntries, fetchLoading, fetchError, fetchConfig } = useElfrydConfig()


</script>

<template>
    <section class="w-full">
        <h3 class="text-lg font-medium">Configuration Data <small class="label">(Last 30 minutes)</small></h3>


        <div v-if="configEntries.length" class="bg-base-100 p-4 rounded-md overflow-auto max-h-[500px]">
            <pre>{{ JSON.stringify(configEntries, null, 2) }}</pre>
        </div>

        <button @click="fetchConfig" :disabled="fetchLoading" class="btn btn-success w-full mt-4">
            {{ fetchLoading ? 'Loading...' : 'Fetch Config Data' }}
        </button>
    </section>
</template>
