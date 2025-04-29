<script setup lang="ts">
import { ref, onMounted } from 'vue'

import { Tab } from '~/types/enums/Tab'

import TabSwitcher from '~/components/TabSwitcher.vue'
import BatterySection from '~/components/BatterySection.vue'
import GyroSection from '~/components/GyroSection.vue'
import TempSection from '~/components/TempSection.vue'
import ConfigSection from '~/components/ConfigSection.vue'

const activeTab = ref<Tab>(Tab.Battery)
</script>

<template>
    <TabSwitcher :BatterySection="BatterySection" :GyroSection="GyroSection" :TempSection="TempSection"
        :ConfigSection="ConfigSection" v-model:activeTab="activeTab" />
</template>
