import C3Chart from "react-c3js"
import { connect } from "react-redux"
import { useState, useEffect, useRef } from "react"
import { convertISODateToTimeFormat } from "../lib/Utils"

const Chart = (props) => {
  const { deviceId, monitorData } = props
  const [voltaje, setVoltaje] = useState()
  const [power, setPower] = useState()
  const [current, setCurrent] = useState()

  const [device, setDevice] = useState({})
  const chartRef = useRef()

  //Configuración inicial de la gráfica
  //para más información revisar:https://c3js.org/
  const [data, setData] = useState({
    columns: [
      ['x'],
      ['data1']
    ]
  })

  let axis2 = {
    axis: {
      x: {

      }
    }
  }

  //Precarga 50 datos
  useEffect(() => {
    // let arr = data.columns[0]
    // for (var i = 0; i <= 50; i++) {
    //   const number = Math.floor(Math.random() * 100 + 1)
    //   arr.push(number)
    // }
    // setData({ ...data, arr })
  }, [])

  //Acá va la lógica de conversión de datos que vienen desde monitorData
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

        let arrX = data.columns[0]
        let arrY = data.columns[1]
        arrX.push(new Date(dataDevice.rtData.sampleTime))
        arrY.push(dataDevice.rtData.voltage)
        setData({ ...data, arrX })
        setData({ ...data, arrY })

      } else {
        setVoltaje(valorLecturasNull)
        setPower(valorLecturasNull)
        setCurrent(valorLecturasNull)
      }  
    }

  }, [monitorData])


  
  //Acá se vuelve a dibujar el gráfico
  useEffect(() => {
    chartRef.current.chart.load(data)
  }, [data])

  //Propiedades del gráfico
  // más información en:https://c3js.org/
  const zoom = {
    enabled: true,
    rescale: true,
    onzoomend: function (domain) {
      //Con esto se almacena el nuevo estado del zoom dentro de la referencia en memoria de la gráfica
      chartRef.current.chart.zoom(domain)
    },
  }
  const resetChart = () => {
    chartRef.current.chart.zoom([0, 0])
  }

  return (
    <>
      <C3Chart data={{
        x: 'x',
        //xFormat: '%Y-%m-%d %hh:%mm', // 'xFormat' can be used as custom format of 'x'
        columns: [
            ['x'],
            ['data1'],
        ]
    }} zoom={zoom} ref={chartRef} axis={ {
      x: {
          //xFormat: '%Y-%m-%d %H:%M:%S',
          type: 'timeseries',
          tick: {
              format: '%Y-%m-%d %H:%M:%S'
          }
      }
  }}/>

      <button className="btn btn-primary btn-sm" onClick={resetChart}>
        Reset Zoom
      </button>
    </>
  )
}

const mapStateToProps = (state) => {
  return {
    monitorData: state.main.monitorData,
  }
}

export default connect(mapStateToProps)(Chart)
