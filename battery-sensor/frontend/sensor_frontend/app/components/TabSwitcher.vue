<script setup lang="ts">
import { defineProps, defineEmits } from 'vue'
import type { BatteryData, ConfigEntry, ParsedCommandResult } from '~/types/elfryd'
import { Tab } from '~/types/enums/Tab'

// Props from parent (only important global data and functions)
const props = defineProps<{
    BatterySection: any
    GyroSection: any
    TempSection: any
    ConfigSection: any

    // Battery data
    batteryData: BatteryData[] | null
    batteryLoading: boolean
    batteryError: string | null
    fetchBattery: (params: { batteryId: string, limit: number, hours: number, timeOffset: number }) => Promise<void>

    // Gyro data
    gyroData: any
    gyroLoading: boolean
    gyroError: string | null
    fetchGyroData: (limit: number) => Promise<void>

    // Config data
    configEntries: ConfigEntry[]
    fetchConfig: () => Promise<void>
    sendConfigCommand: (command: string) => Promise<void>
    sendLoading: boolean
    lastCommandResult: ParsedCommandResult | null
    sendError: string | null

    activeTab: Tab
}>()

const emit = defineEmits<{
    (e: 'update:activeTab', value: Tab): void
}>()

const changeTab = (tab: Tab) => {
    emit('update:activeTab', tab)
}
</script>

<template>
    <div class="tabs tabs-bordered w-full">

        <!-- Battery -->
        <input type="radio" name="elfryd_tabs" class="tab" aria-label="Battery"
            :checked="props.activeTab === Tab.Battery" @change="changeTab(Tab.Battery)" />
        <div v-if="props.activeTab === Tab.Battery" class="tab-content bg-base-100">
            <component :is="BatterySection" :batteryData="batteryData" :batteryLoading="batteryLoading"
                :batteryError="batteryError" :fetchBattery="fetchBattery" />
        </div>

        <!-- Gyro -->
        <input type="radio" name="elfryd_tabs" class="tab" aria-label="Gyro" :checked="props.activeTab === Tab.Gyro"
            @change="changeTab(Tab.Gyro)" />
        <div v-if="props.activeTab === Tab.Gyro" class="tab-content bg-base-100">
            <component :is="GyroSection" :gyroData="gyroData" :gyroLoading="gyroLoading" :gyroError="gyroError"
                :fetchGyroData="fetchGyroData" />
        </div>

        <!-- Temp -->
        <input type="radio" name="elfryd_tabs" class="tab" aria-label="Temp" :checked="props.activeTab === Tab.Temp"
            @change="changeTab(Tab.Temp)" />
        <div v-if="props.activeTab === Tab.Temp" class="tab-content bg-base-100">
            <component :is="TempSection" />
        </div>

        <!-- Config -->
        <input type="radio" name="elfryd_tabs" class="tab" aria-label="Config" :checked="props.activeTab === Tab.Config"
            @change="changeTab(Tab.Config)" />
        <div v-if="props.activeTab === Tab.Config" class="tab-content bg-base-100">
            <component :is="ConfigSection" :configEntries="configEntries" :fetchConfig="fetchConfig"
                :sendConfigCommand="sendConfigCommand" :sendLoading="sendLoading" :lastCommandResult="lastCommandResult"
                :sendError="sendError" />
        </div>

    </div>
</template>
