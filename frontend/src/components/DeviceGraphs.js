import { useEffect, useState } from "react"
import axios from "axios"
//import hammer from 'hammerjs'
import { Line } from 'react-chartjs-2';
//import zoomPlugin from 'chartjs-plugin-zoom';
import { connect } from "react-redux"
import { Button } from 'react-bootstrap';
import { convertISODateToTimeFormat  } from '../lib/Utils';

const DeviceGraphs = (props) => {

  const { deviceId, monitorData } = props
  const [device, setDevice] = useState({})
  const [grafico, setGrafico] = useState({})
  
  // Va a buscar los datos a la API
  // Refresca los gráficos desde monitorData del redux
  useEffect(() => {

    if (deviceId > 0) {
      let device = {}
      axios.get("http://localhost:3000/api/devices/devices").then((res) => {
        device = res.data.find(data => data.id == deviceId)

        setDevice(device)
        let labelGraph = []
        let dataGraph = []

        device.rtData?.map((obj) => {
          labelGraph.push(convertISODateToTimeFormat(obj.sampleTime))
        })

        device.rtData?.map((obj) => {
          dataGraph.push(obj.voltaje)
        })

        setGrafico({
          labels: labelGraph,
          datasets: [
            {
              label: 'Voltaje',
              fill: false,
              lineTension: 0.5,
              backgroundColor: 'rgba(75,192,192,1)',
              borderColor: 'rgba(0,0,0,1)',
              borderWidth: 2,
              data: dataGraph
            }
          ]
        })
      })
    }
  }, [deviceId])

  useEffect(() => {

    if (deviceId > 0) {
      let dataDevice = {}
      monitorData?.map((monitor) => {
        const data = JSON.parse(monitor)
        if (data.id == deviceId) {
          dataDevice = data
        }
      })

      const rtData = {
        voltaje: 12,
        current: 50,
        gupl: 23,
        gdwl: 70,
        power: 100,
        sampleTime: "2021-07-27 14:29:22"
      }

      dataDevice.rtData = rtData
      let stateGrafico = { ...grafico }
      stateGrafico.labels.push(dataDevice.rtData.sampleTime)
      stateGrafico.datasets[0].data.push(dataDevice.rtData.voltaje)
      setGrafico(stateGrafico)

    }
  }, [monitorData])

  const Filtro = (dias) => {

    if (deviceId > 0) {
      let device = {}
      var date = new Date()
      date.setDate(date.getDate() - dias)

      const deviceReq = { id: parseInt(deviceId) }
      axios.post("http://localhost:3000/api/devices/deviceId", deviceReq).then((res) => {

        device = res.data.find(data => data.id == deviceId)

        let labelGraph = []
        let dataGraph = []

        device.rtData?.map((obj) => {
          if (new Date(obj.sampleTime) > date) {
            labelGraph.push(convertISODateToTimeFormat(obj.sampleTime))
          }
        })

        device.rtData?.map((obj) => {
          if (new Date(obj.sampleTime) > date) {
            dataGraph.push(obj.voltaje)
          }
        })

        setGrafico({
          labels: labelGraph,
          datasets: [
            {
              label: 'Voltaje',
              fill: false,
              lineTension: 0.5,
              backgroundColor: 'rgba(75,192,192,1)',
              borderColor: 'rgba(0,0,0,1)',
              borderWidth: 2,
              data: dataGraph
            }
          ]
        })
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
            data={grafico}
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
          <h5>Grafico de Voltaje</h5>
          <h5>Grafico de Voltaje</h5>
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

