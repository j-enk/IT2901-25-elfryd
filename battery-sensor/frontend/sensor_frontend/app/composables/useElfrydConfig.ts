import { ref } from "vue";
import axios from "axios";
import type {
  ConfigEntry,
  FetchConfigOptions,
  ParsedCommandResult,
} from "~/types/elfryd";

export function useElfrydConfig() {
  const configEntries = ref<ConfigEntry[]>([]);
  const fetchLoading = ref(false);
  const fetchError = ref<Error | null>(null);

  const sendLoading = ref(false);
  const sendError = ref<Error | null>(null);
  const commandResult = ref<ParsedCommandResult | null>(null);

  const fetchConfig = async (params: FetchConfigOptions = {}) => {
    fetchLoading.value = true;
    fetchError.value = null;
    try {
      const response = await axios.get<ConfigEntry[]>(
        "http://localhost:5196/api/Elfryd/config",
        {
          params: {
            limit: params.limit ?? 0,
            hours: params.hours ?? 0.5,
            time_offset: params.time_offset ?? 0,
          },
        }
      );
      configEntries.value = response.data.reverse();
    } catch (err) {
      fetchError.value = err as Error;
    } finally {
      fetchLoading.value = false;
    }
  };

  const sendConfigCommand = async (command: string) => {
    sendLoading.value = true;
    sendError.value = null;
    command = command.trim().toLowerCase();

    try {
      const response = await axios.post(
        "http://localhost:5196/api/Elfryd/config/update",
        { command }
      );

      const parsed: ParsedCommandResult = JSON.parse(response.data.result);
      commandResult.value = parsed;
    } catch (err) {
      sendError.value = err as Error;
      commandResult.value = null;
    } finally {
      sendLoading.value = false;
    }
  };

  return {
    configEntries,
    fetchLoading,
    fetchError,
    fetchConfig,
    commandResult,
    sendLoading,
    sendError,
    sendConfigCommand,
  };
}
