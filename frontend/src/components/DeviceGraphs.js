import { useEffect, useState } from "react"
import axios from "axios"
import { Chart, Line, Bar } from "react-chartjs-2"

import { connect } from "react-redux"
import { Button } from "react-bootstrap"
import {
  convertISODateToTimeFormat,
  initConfig,
  setGraficoData,
  addGraficoData,
  initOptions,
} from "../lib/Utils"
//import RtChart from "../components/common/RtChart"
import zoomPlugin from "chartjs-plugin-zoom"
Chart.register(zoomPlugin) // REGISTER PLUGIN

const DeviceGraphs = (props) => {
  const { deviceId, monitorData } = props
  const [voltaje, setVoltaje] = useState()
  const [power, setPower] = useState()
  const [current, setCurrent] = useState()

  const [device, setDevice] = useState({})
  const [graficoVoltaje, setGraficoVoltaje] = useState(
    initConfig("rgba(75,192,192,1)", "rgba(0,0,0,1)", "Voltaje")
  )
  const [graficoCurrent, setGraficoCurrent] = useState(
    initConfig("rgba(75,192,192,1)", "rgba(0,0,0,1)", "Current")
  )
  const [graficoPower, setGraficoPower] = useState(
    initConfig("rgba(75,192,192,1)", "rgba(0,0,0,1)", "Power")
  )

  useEffect(() => {}, [])

  /* Actualizacion por BD*/
  useEffect(() => {
    if (deviceId > 0) {
      setVoltaje(0)
      setPower(0)
      setCurrent(0)

      let device = {}
      axios
        .get(
          "http://" +
            process.env.NEXT_PUBLIC_APIHOST +
            ":" +
            process.env.NEXT_PUBLIC_APIPORT +
            "/api/devices/devices"
        )
        .then((res) => {
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
            dataGraphVoltaje.push(obj.voltaje)
          })

          setGraficoVoltaje(
            setGraficoData(
              { ...graficoVoltaje },
              labelGraphVoltaje,
              dataGraphVoltaje
            )
          )
          setGraficoCurrent(
            setGraficoData(
              { ...graficoCurrent },
              labelGraphCurrent,
              dataGraphCurrent
            )
          )
          setGraficoPower(
            setGraficoData({ ...graficoPower }, labelGraphPower, dataGraphPower)
          )
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

      if (monitorData.rtData != undefined) {
        setVoltaje(dataDevice.rtData.voltaje)
        setPower(dataDevice.rtData.power)
        setCurrent(dataDevice.rtData.current)

        setGraficoVoltaje(
          addGraficoData(
            { ...graficoVoltaje },
            dataDevice.rtData.sampleTime,
            dataDevice.rtData.voltaje
          )
        )
        setGraficoCurrent(
          addGraficoData(
            { ...graficoCurrent },
            dataDevice.rtData.sampleTime,
            dataDevice.rtData.current
          )
        )
        setGraficoPower(
          addGraficoData(
            { ...graficoPower },
            dataDevice.rtData.sampleTime,
            dataDevice.rtData.power
          )
        )
      }
    }
  }, [monitorData])

  /* Filtro de informacion en grafico */
  const Filtro = (dias) => {
    if (deviceId > 0) {
      let device = {}
      var date = new Date()
      date.setDate(date.getDate() - dias)

      const deviceReq = { id: parseInt(deviceId) }
      axios
        .post(
          "http://" +
            process.env.NEXT_PUBLIC_APIHOST +
            ":" +
            process.env.NEXT_PUBLIC_APIPORT +
            "/api/devices/deviceId",
          deviceReq
        )
        .then((res) => {
          device = res.data.find((data) => data.id == deviceId)

          let labelGraphVoltaje = []
          let labelGraphCurrent = []
          let labelGraphPower = []

          let dataGraphVoltaje = []
          let dataGraphCurrent = []
          let dataGraphPower = []

          device.rtData?.map((obj) => {
            if (new Date(obj.sampleTime) > date) {
              labelGraphVoltaje.push(convertISODateToTimeFormat(obj.sampleTime))
              labelGraphCurrent.push(convertISODateToTimeFormat(obj.sampleTime))
              labelGraphPower.push(convertISODateToTimeFormat(obj.sampleTime))

              dataGraphVoltaje.push(obj.voltaje)
              dataGraphCurrent.push(obj.current)
              dataGraphPower.push(obj.power)
            }
          })

          setGraficoVoltaje(
            setGraficoData(
              { ...graficoVoltaje },
              labelGraphVoltaje,
              dataGraphVoltaje
            )
          )
          setGraficoCurrent(
            setGraficoData({ ...graficoCurrent }, dataGraphCurrent, dataGraphCurrent)
          )
          setGraficoPower(
            setGraficoData({ ...graficoPower }, dataGraphPower, dataGraphPower)
          )
        })
    }
  }

  return (
    <>
      {deviceId > 0 && (
        <div>
          <Button
            className="col-md-4 text-center"
            variant="primary"
            type="button"
            id="dia"
            onClick={() => Filtro(1)}
          >
            Dia
          </Button>
          <Button
            className="col-md-4 text-center  btn btn-info"
            variant="primary"
            type="button"
            id="mes"
            onClick={() => Filtro(30)}
          >
            Mes
          </Button>
          <Button
            className="col-md-4 text-center btn btn-secondary"
            variant="primary"
            type="button"
            id="ano"
            onClick={() => Filtro(365)}
          >
            Año
          </Button>

          {/* <RtChart title="hola" labels={graficoVoltaje.labels} datasets={graficoVoltaje.datasets} options={initOptions('Voltaje')} /> */}

          <h5>Voltaje: {voltaje > 0 ? voltaje : "Esperando señal"} [V] </h5>
          <Line data={graficoVoltaje} options={initOptions("Voltaje")} />
          <h5>Power: {power > 0 ? power : "Esperando señal"} [A] </h5>
          <Line data={graficoPower} options={initOptions("Power")} />
          <h5>Current : {current > 0 ? current : "Esperando señal"} [U] </h5>
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
