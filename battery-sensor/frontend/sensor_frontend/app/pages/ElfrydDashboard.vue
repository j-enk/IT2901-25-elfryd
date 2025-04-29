<script setup lang="ts">
import { ref, watch, onMounted } from 'vue'
import { useRoute, useRouter } from 'vue-router'

import { Tab } from '~/types/enums/Tab'

import TabSwitcher from '~/components/TabSwitcher.vue'
import BatterySection from '~/components/BatterySection.vue'
import GyroSection from '~/components/GyroSection.vue'
import TempSection from '~/components/TempSection.vue'
import ConfigSection from '~/components/ConfigSection.vue'

const route = useRoute()
const router = useRouter()

const tabMap = {
    battery: Tab.Battery,
    gyro: Tab.Gyro,
    temp: Tab.Temp,
    config: Tab.Config
} as const

const defaultTab = Tab.Battery

const activeTab = ref<Tab>('')

onMounted(() => {
    const tabFromUrl = route.query.tab as string | undefined

    if (tabFromUrl && tabMap[tabFromUrl.toLowerCase() as keyof typeof tabMap]) {
        activeTab.value = tabMap[tabFromUrl.toLowerCase() as keyof typeof tabMap]
    } else {
        // No valid tab -> set default
        activeTab.value = defaultTab
        router.replace({ query: { ...route.query, tab: 'battery' } })
    }
})

watch(activeTab, (newTab) => {
    if (!newTab) return // Don't update URL if still null

    const tabName = Object.keys(tabMap).find(key => tabMap[key as keyof typeof tabMap] === newTab)

    if (tabName) {
        if (route.query.tab !== tabName) {
            router.replace({ query: { ...route.query, tab: tabName } })
        }
    }
})
</script>

<template>
    <TabSwitcher :BatterySection="BatterySection" :GyroSection="GyroSection" :TempSection="TempSection"
        :ConfigSection="ConfigSection" v-model:activeTab="activeTab" />
</template>
