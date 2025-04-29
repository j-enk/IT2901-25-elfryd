import { ref } from "vue";
import axios from "axios";
import type {
  ConfigEntry,
  FetchConfigParams,
  ParsedCommandResult,
} from "~/types/elfryd";

export function useElfrydConfig() {
  const configEntries = ref<ConfigEntry[]>([]);
  const loading = ref(false);
  const error = ref<Error | null>(null);

  const sendLoading = ref(false);
  const sendError = ref<Error | null>(null);
  const lastCommandResult = ref<ParsedCommandResult | null>(null);

  const fetchConfig = async (params: FetchConfigParams = {}) => {
    loading.value = true;
    error.value = null;
    try {
      const response = await axios.get<ConfigEntry[]>(
        "http://localhost:5196/api/Elfryd/config",
        {
          params: {
            limit: params.limit ?? 0,
            hours: params.hours ?? 168,
            time_offset: params.time_offset ?? 0,
          },
        }
      );
      configEntries.value = response.data.reverse();
    } catch (err) {
      error.value = err as Error;
    } finally {
      loading.value = false;
    }
  };

  const sendConfigCommand = async (command: string) => {
    sendLoading.value = true;
    sendError.value = null;
    command = command.trim().toLowerCase();
    console.log("Sending command:", command);

    try {
      const response = await axios.post(
        "http://localhost:5196/api/Elfryd/config/update",
        { command }
      );

      const parsed: ParsedCommandResult = JSON.parse(response.data.result);
      lastCommandResult.value = parsed;
    } catch (err) {
      sendError.value = err as Error;
      lastCommandResult.value = null;
    } finally {
      sendLoading.value = false;
      await fetchConfig();
    }
  };

  return {
    configEntries,
    loading,
    error,
    fetchConfig,
    sendConfigCommand,
    lastCommandResult,
    sendLoading,
    sendError,
  };
}
