export const useElfrydConfig = () => {
    const configData = ref<any>(null)
    const configLoading = ref(false)
    const error = ref<string | null>(null)

    const fetchConfig = async () => {
        configLoading.value = true
        error.value = null

        try {
            const response = await fetch('http://localhost:5196/api/Elfryd/config')
            if (!response.ok) throw new Error(`API error: ${response.status}`)
            configData.value = await response.json()
        } catch (err) {
            error.value = err instanceof Error ? err.message : 'Unknown error'
        } finally {
            configLoading.value = false
        }
    }

    return { configData, configLoading, error, fetchConfig }
}
