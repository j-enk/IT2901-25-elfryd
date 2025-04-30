<script setup lang="ts">
import { Tab } from "~/types/enums/Tab";
import type { DefineComponent } from "vue";

const props = defineProps<{
  activeTab: Tab;
  batterySection: DefineComponent;
  gyroSection: DefineComponent;
  tempSection: DefineComponent;
  configSection: DefineComponent;
}>();

const emit = defineEmits<{
  (e: "update:activeTab", value: Tab): void;
}>();

const changeTab = (tab: Tab) => {
  emit("update:activeTab", tab);
};
</script>

<template>
  <div class="tabs tabs-bordered w-full">
    <!-- Battery -->
    <input
      type="radio"
      name="elfryd_tabs"
      class="tab"
      aria-label="Battery"
      :checked="props.activeTab === Tab.Battery"
      @change="changeTab(Tab.Battery)"
    />
    <div v-if="props.activeTab === Tab.Battery" class="tab-content bg-base-100">
      <component :is="batterySection" />
    </div>

    <!-- Gyro -->
    <input
      type="radio"
      name="elfryd_tabs"
      class="tab"
      aria-label="Gyro"
      :checked="props.activeTab === Tab.Gyro"
      @change="changeTab(Tab.Gyro)"
    />
    <div v-if="props.activeTab === Tab.Gyro" class="tab-content bg-base-100">
      <component :is="gyroSection" />
    </div>

    <!-- Temp -->
    <input
      type="radio"
      name="elfryd_tabs"
      class="tab"
      aria-label="Temp"
      :checked="props.activeTab === Tab.Temp"
      @change="changeTab(Tab.Temp)"
    />
    <div v-if="props.activeTab === Tab.Temp" class="tab-content bg-base-100">
      <component :is="tempSection" />
    </div>

    <!-- Config -->
    <input
      type="radio"
      name="elfryd_tabs"
      class="tab"
      aria-label="Config"
      :checked="props.activeTab === Tab.Config"
      @change="changeTab(Tab.Config)"
    />
    <div v-if="props.activeTab === Tab.Config" class="tab-content bg-base-100">
      <component :is="configSection" />
    </div>
  </div>
</template>
