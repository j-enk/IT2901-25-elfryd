#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/sensor.h>
#include <stdio.h>

static const char *now_str(void)
{
	static char buf[16]; /* ...HH:MM:SS.MMM */
	uint32_t now = k_uptime_get_32();
	unsigned int ms = now % MSEC_PER_SEC;
	unsigned int s;
	unsigned int min;
	unsigned int h;

	now /= MSEC_PER_SEC;
	s = now % 60U;
	now /= 60U;
	min = now % 60U;
	now /= 60U;
	h = now;

	snprintf(buf, sizeof(buf), "%u:%02u:%02u.%03u",
		 h, min, s, ms);
	return buf;
}

static int process_mpu6050(const struct device *dev)
{
	struct sensor_value accel[3];
	struct sensor_value gyro[3];
	int err = sensor_sample_fetch(dev);
	if(err) {
		printf("MPU6050: sample fetch failed: %d\n", err);
		return err;
	}

	err = sensor_channel_get(dev, SENSOR_CHAN_ACCEL_XYZ, accel);
	if(err) {
		printf("MPU6050: accel failed: %d\n", err);
		return err;
	}

	err = sensor_channel_get(dev, SENSOR_CHAN_GYRO_XYZ, gyro);
	if(err) {
		printf("MPU6050: gyro failed: %d\n", err);
		return err;
	}

	printf("[%s]\n"
			"  accel %f %f %f m/s/s\n"
			"  gyro  %f %f %f rad/s\n",
			now_str(),
			sensor_value_to_double(&accel[0]),
			sensor_value_to_double(&accel[1]),
			sensor_value_to_double(&accel[2]),
			sensor_value_to_double(&gyro[0]),
			sensor_value_to_double(&gyro[1]),
			sensor_value_to_double(&gyro[2]));

	return 0;
}

int main(void)
{
	const struct device *const mpu6050 = DEVICE_DT_GET_ONE(invensense_mpu6050);

	if (!device_is_ready(mpu6050)) {
		printf("Device %s is not ready\n", mpu6050->name);
		return 0;
	}

	while (!IS_ENABLED(CONFIG_MPU6050_TRIGGER)) {
		int rc = process_mpu6050(mpu6050);

		if (rc != 0) {
			break;
		}
		k_sleep(K_SECONDS(2));
	}

	/* triggered runs with its own thread after exit */
	return 0;
}
