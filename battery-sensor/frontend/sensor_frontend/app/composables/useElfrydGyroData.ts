import { ref } from "vue";

export interface MotionRow {
  t: Date;
  ax: number;
  ay: number;
  az: number;    // g
  gx: number;
  gy: number;
  gz: number;    // °/s
  roll: number;  // °
  pitch: number; // °
  yawRate: number; // °/s
  heave: number; // g above rest
}

const gyroData = ref<MotionRow[] | null>(null);
const isLoading = ref(false);
const error = ref<string | null>(null);

/**
 * Fetches processed gyro/motion data
 * @param limit number of points to fetch (0 = no limit)
 * @param hours lookback window in hours
 * @param time_offset offset in hours
 */
const fetchGyroData = async (
  limit: number = 0,
  hours: number = 168,
  time_offset: number = 0
) => {
  isLoading.value = true;
  error.value = null;

  try {
    const url = new URL("http://localhost:5196/api/Elfryd/gyro");
    url.searchParams.append("limit", limit.toString());
    url.searchParams.append("hours", hours.toString());
    url.searchParams.append("time_offset", time_offset.toString());

    const res = await fetch(url.toString());
    if (!res.ok) throw new Error(`API error: ${res.status} ${res.statusText}`);

    const data = (await res.json()) as Array<Omit<MotionRow, 't'> & { t: string }>;
    // Convert timestamp strings into Date objects
    gyroData.value = data.map(r => ({
      ...r,
      t: new Date(r.t)
    }));
  } catch (e) {
    error.value = e instanceof Error ? e.message : String(e);
  } finally {
    isLoading.value = false;
  }
};

export const useElfrydGyroData = () => ({
  gyroData,
  isLoading,
  error,
  fetchGyroData,
});
