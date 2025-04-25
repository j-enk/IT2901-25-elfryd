#ifndef ADC_DRIVER_H_
#define ADC_DRIVER_H_

#include <stdint.h>
#include <inttypes.h>

#ifdef __cplusplus
extern "C"
{
#endif

    /**
     * @brief Initialize the ADC driver.
     *
     * This function sets up the ADC channel using the devicetree configuration.
     *
     * @return 0 if successful, or a negative error code otherwise.
     */
    int adc_driver_init(void);

    /**
     * @brief Read voltage from the ADC.
     *
     * Reads a single sample from the ADC, converts it to millivolts, and
     * compensates for a voltage divider (assuming R1 = R2).
     *
     * @param voltage_mv Pointer to store the measured voltage in millivolts.
     *
     * @return 0 if successful, or a negative error code otherwise.
     */
    int adc_driver_read(int32_t *voltage_mv);

#ifdef __cplusplus
}
#endif

#endif /* ADC_DRIVER_H_ */
