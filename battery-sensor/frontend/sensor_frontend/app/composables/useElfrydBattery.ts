import { ref } from "vue";
import axios from "axios";
import type { BatteryData } from "~/types/elfryd";

interface FetchBatteryOptions {
  batteryId: number;
  limit?: number;
  hours?: number;
  timeOffset?: number;
}

export const useElfrydBattery = () => {
  const batteryData = ref<BatteryData[]>([]);
  const isLoading = ref(false);
  const error = ref<string | null>(null);

  const fetchBattery = async ({
    batteryId = 0,
    limit = 20,
    hours = 168,
    timeOffset = 0,
  }: FetchBatteryOptions) => {
    isLoading.value = true;
    error.value = null;

    try {
      const params = {
        battery_id: batteryId,
        limit,
        hours,
        time_offset: timeOffset,
      };

      const response = await axios.get<BatteryData[]>(
        "http://localhost:5196/api/Elfryd/battery",
        {
          params,
          headers: {
            Accept: "*/*",
          },
        }
      );

      batteryData.value = response.data;
    } catch (err: unknown) {
      if (axios.isAxiosError(err)) {
        error.value =
          err.response?.data?.message || `HTTP error: ${err.message}`;
      } else if (err instanceof Error) {
        error.value = err.message;
      } else {
        error.value = "Unknown error occurred";
      }
    } finally {
      isLoading.value = false;
    }
  };

  return {
    batteryData,
    isLoading,
    error,
    fetchBattery,
  };
};
