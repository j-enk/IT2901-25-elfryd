<script setup lang="ts">
import { useVModel } from '@vueuse/core'
import type { ParsedCommandResult } from '~/types/elfryd'
import { defineProps, defineEmits } from 'vue'
import ConfigDataDisplay from '~/components/displays/ConfigDataDisplay.vue'

const props = defineProps<{
    configSensor: string[]
    modelValueConfigSensorVal: string
    configFreq: string[]
    modelValueConfigFreqVal: string
    updateConfig: () => void
    sendLoading: boolean
    lastCommandResult: ParsedCommandResult | null
    sendError: string | null
}>()

const emit = defineEmits([
    'update:modelValueConfigSensorVal',
    'update:modelValueConfigFreqVal'
])

const configSensorVal = useVModel(props, 'modelValueConfigSensorVal', emit)
const configFreqVal = useVModel(props, 'modelValueConfigFreqVal', emit)

const handleUpdateConfig = () => {
    emit('update:modelValueConfigSensorVal', configSensorVal.value)
    emit('update:modelValueConfigFreqVal', configFreqVal.value)
    props.updateConfig()
}
</script>

<template>
    <section class="mb-6 flex flex-col w-full gap-4">
        <label class="form-control w-full">
            <span class="label font-semibold">
                Sensor
            </span>
            <select v-model="configSensorVal" class="select select-success select-bordered w-full">
                <option v-for="option in configSensor" :key="option" :value="option">{{ option }}</option>
            </select>
        </label>

        <label class="form-control w-full">
            <span class="label font-semibold">Update Frequency</span>
            <div class="w-full">
                <select v-model="configFreqVal" class="select select-success select-bordered w-full pr-10">
                    <option :value="''"></option>
                    <option v-for="option in configFreq" :key="option" :value="option">{{ option }}</option>
                </select>
            </div>


            <span class="label validator-hint">Optional</span>
        </label>



        <button @click="handleUpdateConfig" :disabled="sendLoading" class="btn btn-outline btn-success w-full">
            <span v-if="sendLoading" class="loading loading-spinner loading-md mr-2"></span>
            <span v-else>Update Frequency</span>
        </button>

        <div v-if="lastCommandResult" class="bg-base-300 border-1 border-neutral rounded-lg p-4 mt-4">
            <pre
                class="text-sm font-mono max-w-full overflow-auto max-h-[300px]">{{ JSON.stringify(lastCommandResult, null, 2) }}</pre>
        </div>

        <div v-if="sendError" class="bg-error/20 border border-error text-sm p-4 rounded mt-2" role="alert">
            Error: {{ sendError }}
        </div>
    </section>
</template>
