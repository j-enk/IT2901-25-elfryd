<script setup lang="ts">
import { ref } from 'vue'
import BatteryLineChart from '~/components/charts/BatteryLineChart.vue'
import type { BatteryData } from '~/types/elfryd'

import { useElfrydBattery } from '~/composables/useElfrydBattery'

const { batteryData, isLoading, error, fetchBattery } = useElfrydBattery();

const batteryId = ref(0)
const limit = ref(1000)
const hours = ref(24)
const timeOffset = ref(0)

const handleFetchBattery = async (e: Event) => {
    e.preventDefault()

    try {
        await fetchBattery({
            batteryId: batteryId.value,
            limit: limit.value,
            hours: hours.value,
            timeOffset: timeOffset.value
        })
    } catch (err) {
        console.error('Error fetching battery data:', err)
        // Optionally, set an error state to display in the UI
    }
}


onMounted(() => {
    fetchBattery({
        batteryId: batteryId.value,
        limit: limit.value,
        hours: hours.value,
        timeOffset: timeOffset.value
    })
})
</script>

<template>
    <section class="w-full max-w-7xl rounded-lg mx-auto mt-4" aria-labelledby="data-section-title">
        <div v-if="error" class="bg-error/20 border border-error p-4 rounded mb-4">
            Error: {{ error }}
        </div>

        <div class="flex flex-wrap md:flex-nowrap gap-6">
            <!-- Left side: Form + Chart -->
            <div class="w-full md:w-2/3 flex flex-col gap-6">

                <!-- Form -->
                <form class="bg-base-300 border-1 border-success rounded-lg p-4 flex flex-col gap-4"
                    @submit="handleFetchBattery">
                    <div class="flex flex-wrap gap-4 w-full">
                        <!-- batteryId -->
                        <div class="flex-1 min-w-[200px]">
                            <label for="batteryId" class="label">Battery ID</label>
                            <input id="batteryId" :value="batteryId"
                                @input="batteryId = Number(($event.target as HTMLInputElement).value)" type="number"
                                class="w-full rounded-md input input-md input-success" min="0" max="8"
                                title="Must be between 0 and 8" />
                            <small id="batteryIdHelp" class="validator-hint label">Set to 0 to fetch all
                                entries</small>
                        </div>

                        <!-- limit -->
                        <div class="flex-1 min-w-[200px]">
                            <label for="limit" class="label">Limit</label>
                            <input id="limit" :value="limit"
                                @input="limit = Number(($event.target as HTMLInputElement).value)" type="number" min="0"
                                max="1000000" class="w-full rounded-md input input-md input-success"
                                aria-describedby="limitHelp" />
                            <small id="limitHelp" class="validator-hint label">Set to 0 to fetch all entries</small>
                        </div>

                        <!-- hours -->
                        <div class="flex-1 min-w-[200px]">
                            <label for="hours" class="label">Hours</label>
                            <input id="hours" :value="hours"
                                @input="hours = Number(($event.target as HTMLInputElement).value)" type="number" min="1"
                                max="1000000" class="w-full rounded-md input input-md input-success" />
                        </div>

                        <!-- timeOffset -->
                        <div class="flex-1 min-w-[200px]">
                            <label for="timeOffset" class="label">Time Offset</label>
                            <input id="timeOffset" :value="timeOffset"
                                @input="timeOffset = Number(($event.target as HTMLInputElement).value)" type="number"
                                min="0" max="1000000" class="w-full rounded-md input input-md input-success" />
                        </div>
                    </div>

                    <button type="submit" :disabled="isLoading" class="btn btn-outline btn-success rounded-md mt-6">
                        {{ isLoading ? 'Loading...' : 'Fetch Data' }}
                    </button>
                </form>

                <!-- Chart -->
                <div v-if="batteryData" class="p-4 bg-base-300 border-1 border-success rounded-lg">
                    <h2 class="text-xl text-base-content font-bold mb-4">Voltage Chart</h2>
                    <BatteryLineChart :data="batteryData" />
                </div>

            </div>

            <!-- Right side: Raw JSON -->
            <aside
                class="w-full md:w-1/3 bg-base-300 p-4 border-1 border-success rounded-lg overflow-auto h-fit max-h-[750px]">
                <h2 class="text-lg font-semibold mb-2">Raw Battery Data</h2>

                <div v-if="batteryData">
                    <pre class="text-xs font-mono bg-base-100 p-4 rounded-lg overflow-auto max-h-[600px]">
{{ JSON.stringify(batteryData, null, 2) }}
                    </pre>
                </div>

                <div v-else class="text-sm italic text-base-content">
                    No data to display yet.
                </div>
            </aside>

        </div>
    </section>
</template>
