<script setup lang="ts">
import { ref, watch, onMounted } from "vue";
import { useRoute, useRouter } from "vue-router";
import { Tab } from "~/types/enums/Tab";

import TabSwitcher from "~/components/TabSwitcher.vue";
import BatterySection from "~/components/BatterySection.vue";
import GyroSection from "~/components/GyroSection.vue";
import TempSection from "~/components/TempSection.vue";
import ConfigSection from "~/components/ConfigSection.vue";

const route = useRoute();
const router = useRouter();

const tabMap = {
  battery: Tab.Battery,
  gyro: Tab.Gyro,
  temp: Tab.Temp,
  config: Tab.Config,
} as const;

const defaultTab = Tab.Battery;

const activeTab = ref<Tab>("" as Tab);

onMounted(() => {
  const tabFromUrl = route.query.tab as string | undefined;

  if (tabFromUrl && tabMap[tabFromUrl.toLowerCase() as keyof typeof tabMap]) {
    activeTab.value = tabMap[tabFromUrl.toLowerCase() as keyof typeof tabMap];
  } else {
    activeTab.value = defaultTab;
    router.replace({ query: { ...route.query, tab: "battery" } });
  }
});

watch(activeTab, (newTab) => {
  if (!newTab) return;

  const tabName = Object.keys(tabMap).find(
    (key) => tabMap[key as keyof typeof tabMap] === newTab
  );

  if (tabName && route.query.tab !== tabName) {
    router.replace({ query: { ...route.query, tab: tabName } });
  }
});
</script>

<template>
  <main class="w-full h-full bg-base-100 text-base-content">
    <div class="min-h-screen max-w-screen-xl mx-auto p-6">
      <h1 class="text-3xl font-bold mb-6">Elfryd Data</h1>

      <TabSwitcher
        v-model:active-tab="activeTab"
        :battery-section="BatterySection"
        :gyro-section="GyroSection"
        :temp-section="TempSection"
        :config-section="ConfigSection"
      />
    </div>
  </main>
</template>
