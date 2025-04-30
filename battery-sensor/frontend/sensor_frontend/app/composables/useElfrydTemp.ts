import { ref } from "vue";
import axios from "axios";
import type { TempData, FetchTempOptions } from "~/types/elfryd";

export const useElfrydTemp = () => {
  const tempData = ref<TempData[]>([]);
  const isLoading = ref(false);
  const error = ref<string | null>(null);

  const fetchTemp = async ({
    limit = 100,
    hours = 24,
    timeOffset = 0,
  }: FetchTempOptions = {}) => {
    isLoading.value = true;
    error.value = null;

    try {
      if (hours < 1 || hours > 1_000_000) {
        throw new Error("hours must be between 1 and 1,000,000.");
      }
      if (limit < 0 || limit > 1_000_000) {
        throw new Error("limit must be between 0 and 1,000,000.");
      }
      if (timeOffset < 0 || timeOffset > 1_000_000) {
        throw new Error("timeOffset must be between 0 and 1,000,000.");
      }

      const params = {
        limit,
        hours,
        time_offset: timeOffset,
      };

      const response = await axios.get<TempData[]>(
        "http://localhost:5196/api/Elfryd/temp",
        {
          params,
          headers: {
            Accept: "*/*",
          },
        }
      );

      tempData.value = response.data;
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
    tempData,
    isLoading,
    error,
    fetchTemp,
  };
};
