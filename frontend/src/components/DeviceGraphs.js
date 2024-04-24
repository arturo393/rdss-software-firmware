import { useEffect, useState } from "react"
import axios from "axios"
import { Chart, Line, Bar } from "react-chartjs-2"

import { connect } from "react-redux"
import { Button } from "react-bootstrap"
import { convertISODateToTimeFormat, initConfig, setGraficoData, addGraficoData, initOptions, setBorderColor } from "../lib/Utils"
//import RtChart from "../components/common/RtChart"
import zoomPlugin from "chartjs-plugin-zoom"
Chart.register(zoomPlugin) // REGISTER PLUGIN

const DeviceGraphs = (props) => {
  const { deviceId, monitorData } = props
  const [voltaje, setVoltaje] = useState()
  const [power, setPower] = useState()
  const [current, setCurrent] = useState()

  const [device, setDevice] = useState({})
  const [graficoVoltaje, setGraficoVoltaje] = useState(initConfig("rgba(75,192,192,1)", "rgba(75,192,255,0.8)", "Voltaje"))
  const [graficoCurrent, setGraficoCurrent] = useState(initConfig("rgba(75,192,192,1)", "rgba(75,192,255,0.8)", "Current"))
  const [graficoPower, setGraficoPower] = useState(initConfig("rgba(75,192,192,1)", "rgba(75,192,255,0.8)", "Power"))

  const valorLecturasNull = -9999

  useEffect(() => {}, [])

  /* Actualizacion por BD*/
  useEffect(() => {
    if (deviceId > 0) {
      setVoltaje(valorLecturasNull)
      setPower(valorLecturasNull)
      setCurrent(valorLecturasNull)

      const today = new Date()
      const yesterday = new Date(today)
      yesterday.setDate(yesterday.getDate() - 1)

      let device = {}
      const deviceReq = { id: parseInt(deviceId), fechaHaciaAdelante: yesterday.toISOString() }
      axios.post(process.env.NEXT_PUBLIC_APIPROTO + "://" + process.env.NEXT_PUBLIC_APIHOST + ":" + process.env.NEXT_PUBLIC_APIPORT + "/api/devices/deviceId", deviceReq).then((res) => {
        device = res.data.find((data) => data.id == deviceId)

        setDevice(device)
        let labelGraphVoltaje = []
        let labelGraphCurrent = []
        let labelGraphPower = []

        let dataGraphVoltaje = []
        let dataGraphCurrent = []
        let dataGraphPower = []

        device.rtData?.map((obj) => {
          labelGraphVoltaje.push(convertISODateToTimeFormat(obj.sampleTime))
          labelGraphCurrent.push(convertISODateToTimeFormat(obj.sampleTime))
          labelGraphPower.push(convertISODateToTimeFormat(obj.sampleTime))

          dataGraphCurrent.push(obj.current)
          dataGraphPower.push(obj.power)
          dataGraphVoltaje.push(obj.voltage)
        })

        setGraficoVoltaje(setGraficoData({ ...graficoVoltaje }, labelGraphVoltaje, dataGraphVoltaje))
        setGraficoCurrent(setGraficoData({ ...graficoCurrent }, labelGraphCurrent, dataGraphCurrent))
        setGraficoPower(setGraficoData({ ...graficoPower }, labelGraphPower, dataGraphPower))
      })
    }
  }, [deviceId])

  /* Actualizacion por Sockect */
  useEffect(() => {
    if (deviceId > 0) {
      let dataDevice = {}
      monitorData?.map((monitor) => {
        const data = JSON.parse(monitor)
        if (data.id == deviceId) {
          dataDevice = data
        }
      })

      if (dataDevice.rtData != undefined) {
        setVoltaje(dataDevice.rtData.voltage)
        setPower(dataDevice.rtData.power)
        setCurrent(dataDevice.rtData.current)

        if (dataDevice.rtData.gdwl > 0) {
          setBorderColor({ ...graficoPower }, "rgba(255, 99, 132,0.8)")
        } else {
          setBorderColor({ ...graficoPower }, "rgba(75,192,255,0.8)")
        }

        addGraficoData({ ...graficoVoltaje }, dataDevice.rtData.sampleTime, dataDevice.rtData.voltage)

        setGraficoCurrent(addGraficoData({ ...graficoCurrent }, dataDevice.rtData.sampleTime, dataDevice.rtData.current))
        setGraficoPower(addGraficoData({ ...graficoPower }, dataDevice.rtData.sampleTime, dataDevice.rtData.power))
      } else {
        setVoltaje(valorLecturasNull)
        setPower(valorLecturasNull)
        setCurrent(valorLecturasNull)
      }
    }
  }, [monitorData])

  /* Filtro de informacion en grafico */
  const Filtro = (dias) => {
    if (deviceId > 0) {
      let device = {}
      var date = new Date()
      date.setDate(date.getDate() - dias)

      const deviceReq = { id: parseInt(deviceId), fechaHaciaAdelante: date.toISOString() }
      axios.post(process.env.NEXT_PUBLIC_APIPROTO + "://" + process.env.NEXT_PUBLIC_APIHOST + ":" + process.env.NEXT_PUBLIC_APIPORT + "/api/devices/deviceId", deviceReq).then((res) => {
        device = res.data.find((data) => data.id == deviceId)

        let labelGraphVoltaje = []
        let labelGraphCurrent = []
        let labelGraphPower = []

        let dataGraphVoltaje = []
        let dataGraphCurrent = []
        let dataGraphPower = []

        device.rtData?.map((obj) => {
          labelGraphVoltaje.push(convertISODateToTimeFormat(obj.sampleTime))
          labelGraphCurrent.push(convertISODateToTimeFormat(obj.sampleTime))
          labelGraphPower.push(convertISODateToTimeFormat(obj.sampleTime))

          dataGraphVoltaje.push(obj.voltage)
          dataGraphCurrent.push(obj.current)
          dataGraphPower.push(obj.power)
        })

        setGraficoVoltaje(setGraficoData({ ...graficoVoltaje }, labelGraphVoltaje, dataGraphVoltaje))
        setGraficoCurrent(setGraficoData({ ...graficoCurrent }, dataGraphCurrent, dataGraphCurrent))
        setGraficoPower(setGraficoData({ ...graficoPower }, dataGraphPower, dataGraphPower))
      })
    }
  }

  return (
    <>
      {deviceId > 0 && (
        <div>
          <button className="col-md-4 text-center btn btn-primary" type="button" id="dia" onClick={() => Filtro(1)}>
            Day
          </button>
          <button className="col-md-4 text-center  btn btn-secondary " variant="primary" type="button" id="mes" onClick={() => Filtro(30)}>
            Month
          </button>
          <button className="col-md-4 text-center btn btn-dark" variant="primary" type="button" id="ano" onClick={() => Filtro(365)}>
            Year
          </button>

          {/* <RtChart title="hola" labels={graficoVoltaje.labels} datasets={graficoVoltaje.datasets} options={initOptions('Voltaje')} /> */}

          <h5>Voltage: {voltaje != valorLecturasNull ? voltaje : "Waiting signal"} [V] </h5>
          <Line data={graficoVoltaje} options={initOptions("Voltage")} />
          <h5>Power: {power != valorLecturasNull ? power : "Waiting signal"} [dBm] </h5>
          <Line data={graficoPower} options={initOptions("Power")} />
          <h5>Current : {current != valorLecturasNull ? current : "Waiting signal"} [A] </h5>
          <Line data={graficoCurrent} options={initOptions("Current")} />
        </div>
      )}
    </>
  )
}

const mapStateToProps = (state) => {
  return {
    monitorData: state.main.monitorData,
  }
}

export default connect(mapStateToProps)(DeviceGraphs)
