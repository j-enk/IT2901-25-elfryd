<script setup lang="ts">
import { ref, onMounted } from 'vue';

const rawData = ref<any>(null);
const isLoading = ref(true);
const error = ref<string | null>(null);
const topic = ref<string>('');
const limit = ref<number>(10);

const fetchMessages = async () => {
  isLoading.value = true;
  error.value = null;
  
  try {
    console.log(`Fetching ElfrydAPI data with topic: ${topic.value || 'all'}, limit: ${limit.value}`);
    
    const url = new URL('http://localhost:5196/api/Elfryd/messages');
    if (topic.value) {
      url.searchParams.append('topic', topic.value);
    }
    url.searchParams.append('limit', limit.value.toString());
    
    const response = await fetch(url.toString());
    
    if (!response.ok) {
      throw new Error(`API error: ${response.status}`);
    }
    
    const data = await response.json();
    console.log('Received raw data:', data);
    rawData.value = data;
  } catch (err) {
    console.error('Error fetching raw data:', err);
    error.value = err instanceof Error ? err.message : 'Unknown error';
  } finally {
    isLoading.value = false;
  }
};

onMounted(() => {
  fetchMessages();
});
</script>

<template>
  <div class="container">
    <h1>Elfryd Raw Data</h1>
    
    <div class="controls">
      <div class="form-group">
        <label for="topic">Topic (optional):</label>
        <input 
          id="topic" 
          v-model="topic" 
          type="text" 
          placeholder="Enter topic name or leave blank for all topics"
        />
      </div>
      
      <div class="form-group">
        <label for="limit">Limit:</label>
        <input 
          id="limit" 
          v-model="limit" 
          type="number" 
          min="1" 
          max="100"
        />
      </div>
      
      <button @click="fetchMessages" :disabled="isLoading">
        {{ isLoading ? 'Loading...' : 'Fetch Data' }}
      </button>
    </div>
    
    <div v-if="error" class="error-box">
      <p>Error: {{ error }}</p>
    </div>
    
    <div v-if="isLoading" class="loading">
      <p>Loading data from ElfrydAPI...</p>
    </div>
    
    <div v-else-if="rawData" class="data-display">
      <h2>Raw JSON Response</h2>
      <pre class="json-display">{{ JSON.stringify(rawData, null, 2) }}</pre>
    </div>
  </div>
</template>

<style scoped>
.container {
  max-width: 1200px;
  margin: 0 auto;
  padding: 20px;
  font-family: Arial, sans-serif;
}

h1 {
  color: #333;
  margin-bottom: 20px;
}

.controls {
  display: flex;
  gap: 15px;
  align-items: flex-end;
  margin-bottom: 20px;
  padding: 15px;
  background-color: green;
  border-radius: 8px;
}

.form-group {
  display: flex;
  flex-direction: column;
  flex: 1;
}

label {
  font-weight: bold;
  margin-bottom: 5px;
}

input {
  padding: 8px;
  border: 1px solid #ccc;
  border-radius: 4px;
  font-size: 14px;
}

button {
  padding: 8px 16px;
  background-color: #36A2EB;
  color: white;
  border: none;
  border-radius: 4px;
  cursor: pointer;
  height: 35px;
  font-size: 14px;
}

button:hover {
  background-color: #2a8fc7;
}

button:disabled {
  background-color: #ccc;
  cursor: not-allowed;
}

.error-box {
  padding: 15px;
  background-color: #ffeeee;
  border: 1px solid #ff8888;
  border-radius: 4px;
  color: #cc0000;
  margin-bottom: 20px;
}

.data-display {
  margin-top: 20px;
}

.json-display {
  background-color: redgray;
  padding: 15px;
  border-radius: 4px;
  overflow: auto;
  max-height: 600px;
  border: 1px solid #e0e0e0;
  white-space: pre-wrap;
  word-break: break-word;
  font-family: 'Courier New', Courier, monospace;
}

.loading {
  padding: 20px;
  text-align: center;
  color: #666;
}
</style>