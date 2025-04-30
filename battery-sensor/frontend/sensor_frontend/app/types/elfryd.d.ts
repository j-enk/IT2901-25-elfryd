export interface ConfigEntry {
  id: number;
  command: string;
  topic: string;
  timestamp: string;
}

export interface FetchConfigParams {
  limit?: number;
  hours?: number;
  time_offset?: number;
}

interface FetchTempOptions {
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
