import { createFileRoute } from '@tanstack/react-router'
import mqtt, { MqttClient } from 'mqtt'
import { useEffect, useRef, useState } from 'react'
import { Button } from '@/components/ui/button'
import { Card, CardContent, CardHeader, CardTitle } from '@/components/ui/card'

export const Route = createFileRoute('/')({ component: App })

interface SensorData {
  moisture: number
  distance: number
  batteryVoltage: number
  batteryPercent: number
  pumpActive: boolean
}

function App() {
  const clientRef = useRef<MqttClient | null>(null)
  const [connected, setConnected] = useState(false)
  const [sensorData, setSensorData] = useState<SensorData | null>(null)
  const [pumpActive, setPumpActive] = useState(false)

  useEffect(() => {
    // Connect to MQTT broker via WebSocket
    const client = mqtt.connect('ws://192.168.1.23:9001')
    clientRef.current = client

    client.on('connect', () => {
      console.log('✅ Connected to MQTT broker')
      setConnected(true)
      // Subscribe to sensor data topic
      client.subscribe('sirambro/sensors')
      client.subscribe('sirambro/pump/status')
    })

    client.on('message', (topic, message) => {
      const msg = message.toString()
      
      if (topic === 'sirambro/sensors') {
        try {
          const data: SensorData = JSON.parse(msg)
          setSensorData(data)
          setPumpActive(data.pumpActive)
        } catch (e) {
          console.error('Failed to parse sensor data:', e)
        }
      }
      
      if (topic === 'sirambro/pump/status') {
        setPumpActive(msg === 'ON')
      }
    })

    client.on('error', (err) => {
      console.error('MQTT Error:', err)
      setConnected(false)
    })

    client.on('close', () => {
      setConnected(false)
    })

    return () => {
      client.end()
    }
  }, [])

  const handlePumpToggle = () => {
    if (clientRef.current && connected && !pumpActive) {
      clientRef.current.publish('sirambro/pump/control', 'ON')
    }
  }

  return (
    <div className="min-h-screen bg-linear-to-br from-green-50 to-blue-50 p-6">
      <div className="mx-auto max-w-2xl">
        {/* Header */}
        <div className="mb-8 text-center">
          <h1 className="text-4xl font-bold text-green-800">🌱 SiramBro</h1>
          <p className="mt-2 text-gray-600">Smart Plant Watering System</p>
          <div className="mt-2 flex items-center justify-center gap-2">
            <div
              className={`h-3 w-3 rounded-full ${connected ? 'bg-green-500' : 'bg-red-500'}`}
            />
            <span className="text-sm text-gray-500">
              {connected ? 'Connected' : 'Disconnected'}
            </span>
          </div>
        </div>

        {/* Sensor Cards Grid */}
        <div className="mb-6 grid grid-cols-2 gap-4">
          {/* Moisture Card */}
          <Card className="border-green-200 bg-white/80 backdrop-blur">
            <CardHeader className="pb-2">
              <CardTitle className="flex items-center gap-2 text-lg text-green-700">
                💧 Soil Moisture
              </CardTitle>
            </CardHeader>
            <CardContent>
              <p className="text-3xl font-bold text-green-800">
                {sensorData?.moisture ?? '--'}%
              </p>
            </CardContent>
          </Card>

          {/* Water Level Card */}
          <Card className="border-blue-200 bg-white/80 backdrop-blur">
            <CardHeader className="pb-2">
              <CardTitle className="flex items-center gap-2 text-lg text-blue-700">
                🪣 Water Level
              </CardTitle>
            </CardHeader>
            <CardContent>
              <p className="text-3xl font-bold text-blue-800">
                {sensorData?.distance ?? '--'} cm
              </p>
            </CardContent>
          </Card>

          {/* Battery Voltage Card */}
          <Card className="border-yellow-200 bg-white/80 backdrop-blur">
            <CardHeader className="pb-2">
              <CardTitle className="flex items-center gap-2 text-lg text-yellow-700">
                ⚡ Voltage
              </CardTitle>
            </CardHeader>
            <CardContent>
              <p className="text-3xl font-bold text-yellow-800">
                {sensorData?.batteryVoltage?.toFixed(2) ?? '--'} V
              </p>
            </CardContent>
          </Card>

          {/* Battery Percentage Card */}
          <Card className="border-orange-200 bg-white/80 backdrop-blur">
            <CardHeader className="pb-2">
              <CardTitle className="flex items-center gap-2 text-lg text-orange-700">
                🔋 Battery
              </CardTitle>
            </CardHeader>
            <CardContent>
              <p className="text-3xl font-bold text-orange-800">
                {sensorData?.batteryPercent ?? '--'}%
              </p>
            </CardContent>
          </Card>
        </div>

        {/* Pump Control */}
        <Card className="border-cyan-200 bg-white/80 backdrop-blur">
          <CardHeader>
            <CardTitle className="flex items-center gap-2 text-xl text-cyan-700">
              🚿 Water Pump Control
            </CardTitle>
          </CardHeader>
          <CardContent className="flex flex-col items-center gap-4">
            <Button
              onClick={handlePumpToggle}
              disabled={!connected || pumpActive}
              size="lg"
              className={`h-16 w-full text-lg font-semibold ${
                pumpActive
                  ? 'bg-cyan-500 hover:bg-cyan-500'
                  : 'bg-cyan-600 hover:bg-cyan-700'
              }`}
            >
              {pumpActive ? '💦 Watering... (3s)' : '💧 Water Now'}
            </Button>
            <p className="text-center text-sm text-gray-500">
              {pumpActive
                ? 'Pump is running for 3 seconds'
                : 'Press to water plants for 3 seconds'}
            </p>
          </CardContent>
        </Card>
      </div>
    </div>
  )
}
