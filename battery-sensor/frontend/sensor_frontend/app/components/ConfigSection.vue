<script setup lang="ts">
import ConfigForm from "~/components/forms/ConfigForm.vue";
import ConfigDataDisplay from "~/components/displays/ConfigDataDisplay.vue";
</script>

<template>
  <section
    class="w-full max-w-5xl mx-auto rounded-lg mt-4 p-4 bg-base-300 border-1 border-error"
  >
    <div class="flex flex-col gap-6">
      <ConfigForm />
      <ConfigDataDisplay />
    </div>
  </section>
</template>
