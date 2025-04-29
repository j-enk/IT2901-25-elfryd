import { ref } from "vue";

export interface RawGyroRow {
  id: number;
  accel_x: number;
  accel_y: number;
  accel_z: number;
  gyro_x: number;
  gyro_y: number;
  gyro_z: number;
  device_timestamp: number;
}

export interface MotionRow {
  /* raw values */
  t: Date;
  ax: number;
  ay: number;
  az: number; // g
  gx: number;
  gy: number;
  gz: number; // ° s-¹
  /* derived values */
  roll: number; // °
  pitch: number; // °
  yawRate: number; // ° s-¹
  heave: number; // g above rest
}

const gyroData = ref<MotionRow[] | null>(null);
const isLoading = ref(false);
const error = ref<string | null>(null);

const µg = 1_000_000; // adjust?
const mdeg = 1_000;

const toMotionRow = (r: RawGyroRow): MotionRow => {
  const ax = r.accel_x / µg;
  const ay = r.accel_y / µg;
  const az = r.accel_z / µg;
  const gx = r.gyro_x / mdeg;
  const gy = r.gyro_y / mdeg;
  const gz = r.gyro_z / mdeg;

  const roll = Math.atan2(ay, az) * 57.2958; // rad->deg
  const pitch = Math.atan2(-ax, Math.hypot(ay, az)) * 57.2958;
  const heave = Math.hypot(ax, ay, az) - 1; // remove 1 g gravity

  return {
    t: new Date(r.device_timestamp * 1_000),
    ax,
    ay,
    az,
    gx,
    gy,
    gz,
    roll,
    pitch,
    yawRate: gz,
    heave,
  };
};

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

    const raw: RawGyroRow[] = await res.json();
    gyroData.value = raw.map(toMotionRow);
  } catch (e) {
    error.value = e instanceof Error ? e.message : "Unknown error";
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
