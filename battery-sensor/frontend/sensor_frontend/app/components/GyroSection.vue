<script setup lang="ts">
import type { BatteryData } from '~/types/elfryd'

defineProps<{
    batteryData: BatteryData[]
    batteryLoading: boolean
    batteryError: string | null
    battery_id: string
    limit: number
    handleFormSubmit: (e: Event) => void
    fetchBattery: () => void
}>()
</script>

<template>
    <section class="w-full max-w-5xl mx-auto rounded-lg p-6 mt-4" aria-labelledby="data-section-title">
        <h1 id="data-section-title" class="text-2xl font-bold mb-4">Gyro</h1>
    </section>
</template>
