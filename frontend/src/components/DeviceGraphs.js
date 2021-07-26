import { useEffect, useState } from "react"
import axios from "axios"
//import hammer from 'hammerjs'
import {Chart, Line } from 'react-chartjs-2'
import * as Zoom from "chartjs-plugin-zoom";
import { connect } from "react-redux"
import { Button } from 'react-bootstrap'
import { convertISODateToTimeFormat, initConfig, setGraficoData, addGraficoData} from '../lib/Utils'

const DeviceGraphs = (props) => {

  const { deviceId, monitorData } = props
  const [device, setDevice] = useState({})
  const [graficoVoltaje, setGraficoVoltaje] = useState(initConfig('rgba(75,192,192,1)','rgba(0,0,0,1)', 'Voltaje'))
  const [graficoCurrent, setGraficoCurrent] = useState(initConfig('rgba(75,192,192,1)','rgba(0,0,0,1)', 'Current'))
  const [graficoPower, setGraficoPower] = useState(initConfig('rgba(75,192,192,1)','rgba(0,0,0,1)', 'Power'))


  useEffect(() => {
    //Chart.register(Zoom);
  }, []);

 /* Actualizacion por BD*/
  useEffect(() => {

    if (deviceId > 0) {
      let device = {}
      axios.get("http://localhost:3000/api/devices/devices").then((res) => {
        device = res.data.find(data => data.id == deviceId)

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

        setGraficoVoltaje(setGraficoData({ ...graficoVoltaje },labelGraphVoltaje,dataGraphVoltaje))
        setGraficoCurrent(setGraficoData({ ...graficoCurrent },labelGraphCurrent,dataGraphCurrent))
        setGraficoPower(setGraficoData({ ...graficoPower },labelGraphPower,dataGraphPower))

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

      /*INICIO DUMMY */
      const rtData = {
        voltaje: 12,
        current: 50,
        gupl: 23,
        gdwl: 70,
        power: 100,
        sampleTime: "2021-07-27 14:29:22"
      }
      dataDevice.rtData = rtData
      /*FIN  DUMMY */

      setGraficoVoltaje(addGraficoData({ ...graficoVoltaje },dataDevice.rtData.sampleTime,dataDevice.rtData.voltaje))
      setGraficoCurrent(addGraficoData({ ...graficoCurrent },dataDevice.rtData.sampleTime,dataDevice.rtData.current))
      setGraficoPower(addGraficoData({ ...graficoPower },dataDevice.rtData.sampleTime,dataDevice.rtData.power))

    }
  }, [monitorData])

  /* Filtro de informacion en grafico */
  const Filtro = (dias) => {

    if (deviceId > 0) {
      let device = {}
      var date = new Date()
      date.setDate(date.getDate() - dias)

      const deviceReq = { id: parseInt(deviceId) }
      axios.post("http://localhost:3000/api/devices/deviceId", deviceReq).then((res) => {

        device = res.data.find(data => data.id == deviceId)

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

        setGraficoVoltaje(setGraficoData({ ...graficoVoltaje },labelGraphVoltaje,dataGraphVoltaje))
        setGraficoCurrent(setGraficoData({ ...graficoCurrent },dataGraphCurrent,dataGraphCurrent))
        setGraficoPower(setGraficoData({ ...graficoPower },dataGraphPower,dataGraphPower))
      })
    }
  }

  return (
    <>
      Estos son los gráficos del device ID: {deviceId}
      {deviceId > 0 && (
        <div>
          <Button variant="primary" type="button" id='dia' onClick={() => Filtro(1)}>Dia</Button>
          <Button variant="primary" type="button" id='mes' onClick={() => Filtro(30)}>Mes</Button>
          <Button variant="primary" type="button" id='ano' onClick={() => Filtro(365)}>Año</Button>
          <h5>Gráfico de Voltaje</h5>
          <Line
            data={graficoVoltaje}
            options={{
              title: {
                display: true,
                text: 'Voltaje',
                fontSize: 20
              },
              legend: {
                display: true,
                position: 'right'
              },
              plugins: {
                zoom: {
                  limits: {
                    y: { min: 0, max: 100 },
                    y2: { min: -5, max: 5 }
                  },
                }
              }

            }}
          />
          <h5>Grafico de Power</h5>
          <Line
            data={graficoPower}
            options={{
              title: {
                display: true,
                text: 'Power',
                fontSize: 20
              },
              legend: {
                display: true,
                position: 'right'
              },
              plugins: {
                zoom: {
                  limits: {
                    y: { min: 0, max: 100 },
                    y2: { min: -5, max: 5 }
                  },
                }
              }

            }}
          />
          <h5>Grafico de Current</h5>
          <Line
            data={graficoCurrent}
            options={{
              title: {
                display: true,
                text: 'Current',
                fontSize: 20
              },
              legend: {
                display: true,
                position: 'right'
              },
              plugins: {
                zoom: {
                  limits: {
                    y: { min: 0, max: 100 },
                    y2: { min: -5, max: 5 }
                  },
                }
              }

            }}
          />          
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

