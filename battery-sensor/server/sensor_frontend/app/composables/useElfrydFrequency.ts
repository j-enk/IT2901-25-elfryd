export const useElfrydFrequency = () => {
    const error = ref<string | null>(null)

    const updateFrequency = async (rawValue: string) => {
        const frequencyValue = rawValue.replace('Freq: ', '') || '10'
        const formattedFrequency = `freq ${frequencyValue}`

        try {
            const response = await fetch('http://localhost:5196/api/Elfryd/config/frequency', {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify({ command: formattedFrequency }),
            })

            if (!response.ok) throw new Error(`API error: ${response.status}`)

            const result = await response.json()
            console.log('Frequency updated successfully:', result)
        } catch (err) {
            error.value = err instanceof Error ? err.message : 'Unknown error'
            console.error('Error updating frequency:', error.value)
        }
    }

    return { updateFrequency, error }
}
