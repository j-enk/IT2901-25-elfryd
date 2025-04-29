<script setup lang="ts">
import { ref, onMounted } from 'vue'

import { useElfrydBattery } from '~/composables/useElfrydBattery'
import { useElfrydGyroData } from '~/composables/useElfrydGyroData'
import { useElfrydConfig } from '~/composables/useElfrydConfig'

import { Tab } from '~/types/enums/Tab'

import TabSwitcher from '~/components/TabSwitcher.vue'
import BatterySection from '~/components/BatterySection.vue'
import GyroSection from '~/components/GyroSection.vue'
import TempSection from '~/components/TempSection.vue'
import ConfigSection from '~/components/ConfigSection.vue'

const activeTab = ref<Tab>(Tab.Battery)

// Composables
const { batteryData, isLoading: batteryLoading, error: batteryError, fetchBattery } = useElfrydBattery()
const { gyroData, isLoading: gyroLoading, error: gyroError, fetchGyroData } = useElfrydGyroData()
const { configEntries, loading: configFetchLoading, fetchConfig, sendConfigCommand, lastCommandResult, sendLoading, sendError } = useElfrydConfig()

onMounted(() => {
    fetchBattery({ batteryId: '', limit: 20, hours: 168, timeOffset: 0 })
})
</script>

<template>
    <TabSwitcher :BatterySection="BatterySection" :GyroSection="GyroSection" :TempSection="TempSection"
        :ConfigSection="ConfigSection" :batteryData="batteryData" :batteryLoading="batteryLoading"
        :batteryError="batteryError" :fetchBattery="fetchBattery" :gyroData="gyroData" :gyroLoading="gyroLoading"
        :gyroError="gyroError" :fetchGyroData="fetchGyroData" :configEntries="configEntries" :fetchConfig="fetchConfig"
        :sendConfigCommand="sendConfigCommand" :sendLoading="sendLoading" :lastCommandResult="lastCommandResult"
        :sendError="sendError" v-model:activeTab="activeTab" />
</template>
