<script setup lang="ts">
import { ref } from 'vue'
import type { ConfigEntry, ParsedCommandResult } from '~/types/elfryd'
import ConfigForm from '~/components/forms/ConfigForm.vue'
import ConfigDataDisplay from '~/components/displays/ConfigDataDisplay.vue'

const props = defineProps<{
    configSensor: string[]
    configFreq: string[]
    configEntries: ConfigEntry[]
    fetchConfig: () => Promise<void>
    sendConfigCommand: (command: string) => Promise<void>
    sendLoading: boolean
    lastCommandResult: ParsedCommandResult | null
    sendError: string | null
    configFetchLoading: boolean
}>()

// Local form state
const configSensorVal = ref('')
const configFreqVal = ref('')

const handleUpdateConfig = () => {
    if (!configSensorVal.value) {
        console.warn('No sensor selected for config update')
        return
    }
    const command = `${configSensorVal.value.toLowerCase()} ${configFreqVal.value}`
    props.sendConfigCommand(command)
}
</script>

<template>
    <section class="w-full max-w-5xl mx-auto rounded-lg mt-4 p-4 bg-base-300 border-1 border-success">
        <div class="flex flex-col gap-6">
            <ConfigForm :configSensor="configSensor" :modelValueConfigSensorVal="configSensorVal"
                :configFreq="configFreq" :modelValueConfigFreqVal="configFreqVal" :updateConfig="handleUpdateConfig"
                :sendLoading="sendLoading" :lastCommandResult="lastCommandResult" :sendError="sendError" />

            <ConfigDataDisplay :configEntries="configEntries" :fetchConfig="fetchConfig"
                :configFetchLoading="configFetchLoading" />
        </div>
    </section>
</template>
