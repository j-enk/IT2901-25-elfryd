export interface ConfigEntry {
  id: number;
  command: string;
  topic: string;
  timestamp: string;
}

export interface FetchBatteryOptions {
  batteryId: number;
  limit?: number;
  hours?: number;
  timeOffset?: number;
}

export interface FetchConfigOptions {
  limit?: number;
  hours?: number;
  time_offset?: number;
}

export interface FetchTempOptions {
  limit?: number;
  hours?: number;
  timeOffset?: number;
}

export interface FetchGyroOptions {
  limit?: number;
  hours?: number;
  timeOffset?: number;
}

export interface SendConfigResponse {
  success: boolean;
  result: string; // JSON string, need to parse
}

export interface ParsedCommandResult {
  success: boolean;
  message: string;
  topic: string;
  commands: string[];
}

export interface BatteryData {
  id: number;
  battery_id: number;
  voltage: number;
  device_timestamp: number;
}

export interface TempData {
  id: number;
  temperature: number;
  device_timestamp: number;
}

export interface GyroData {
  id: number;
  ax: number;
  ay: number;
  az: number;
  gx: number;
  gy: number;
  gz: number;
  roll: number;
  pitch: number;
  yawRate: number;
  heave: number;
  device_timestamp: number;
}
