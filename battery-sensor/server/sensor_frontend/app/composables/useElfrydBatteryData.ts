export const useElfrydBatteryData = () => {
    const batteryData = ref<any>(null)
    const isLoading = ref(false)
    const error = ref<string | null>(null)

    const fetchBatteryData = async (batteryId: number, limit: number = 10) => {
        isLoading.value = true
        error.value = null

        try {
            const url = new URL('http://localhost:5196/api/Elfryd/battery')
            url.searchParams.append('battery_id', batteryId.toString())
            url.searchParams.append('limit', limit.toString())

            const response = await fetch(url.toString())
            if (!response.ok) throw new Error(`API error: ${response.status}`)

            batteryData.value = await response.json()
        } catch (err) {
            error.value = err instanceof Error ? err.message : 'Unknown error'
        } finally {
            isLoading.value = false
        }
    }

    return { batteryData, isLoading, error, fetchBatteryData }
}
