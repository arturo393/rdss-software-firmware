import { connect } from "react-redux"
import { useState, useEffect, useRef } from "react"

import plotly from "plotly.js/dist/plotly"
import createPlotComponent from "react-plotly.js/factory"

import { alpha } from "@material-ui/core/styles"

const Plot = createPlotComponent(plotly)

function Chart(props) {
  const defaultNullValue = 0
  const { deviceId, rtData, label, filter, color, monitorData } = props

  const plotRef = useRef()

  const [plot, setPlot] = useState({ data: [], layout: {}, frames: [], config: {} })
  const [plotData, setPlotData] = useState({})
  const [revision, setRevision] = useState(0)
  const [autoArrange, setAutoArrange] = useState(true)

  useEffect(() => {
    console.log("setting up Plot Data")
    if (deviceId && Object.entries(rtData).length !== 0) {
      const data = { x: rtData.x, y: rtData.rtd[filter], marker: rtData.marker, text: rtData.text }
      setRevision(rtData.x.length + deviceId + Math.floor(Math.random() * 100 + 1))
      setPlotData(data)
    }
  }, [])

  useEffect(() => {
    if (Object.entries(rtData).length !== 0) {
      console.log("rtData changed")
      const data = { x: rtData.x, y: rtData.rtd[filter], marker: rtData.marker, text: rtData.text }
      setRevision(rtData.x.length + deviceId + Math.floor(Math.random() * 100 + 1))
      setPlotData(data)
    }
  }, [rtData])

  useEffect(() => {
    console.log("plotData changed")
    renderPlot()
  }, [revision])

  const getPointText = (alerts = {}) => {
    const connectionAlert = alerts?.connection ? true : false
    const voltageAlert = alerts?.voltage ? true : false
    const currentAlert = alerts?.current ? true : false
    const powerAlert = alerts?.power ? true : false

    let alerted = false
    let t = "Device: " + deviceId + "\n"
    if (connectionAlert) {
      t += "Connection Alert\n"
      alerted = true
    }
    if (voltageAlert) {
      t += "Voltage Alert\n"
      alerted = true
    }
    if (currentAlert) {
      t += "Current Alert\n"
      alerted = true
    }
    if (powerAlert) {
      t += "Power Alert\n"
      alerted = true
    }

    return { text: t, status: alerted }
  }

  // //Acá va la lógica de conversión de datos que vienen desde monitorData
  useEffect(() => {
    console.log("Getting data from monitor...")

    if (deviceId) {
      //Gets device data from monitorData
      let currentDeviceData = { rtData: {} }
      monitorData?.map((monitor) => {
        const data = JSON.parse(monitor)
        if (data.id == deviceId) {
          currentDeviceData = data
        }
      })

      if (Object.entries(plotData).length !== 0) {
        let data = plotData
        console.log(data)

        const alertStatus = getPointText(currentDeviceData.rtData.alerts)

        data.y.push(currentDeviceData.rtData[filter] || data.y[data.y.length - 1])
        const currentIndex = data.y.length - 1
        data.x[currentIndex] = currentDeviceData.rtData.sampleTime

        if (!data.text[currentIndex]) {
          data.text[currentIndex] = alertStatus.text
          data.marker.color[currentIndex] = alertStatus.status ? "red" : color
        }
        setRevision(currentIndex + deviceId + Math.floor(Math.random() * 100 + 1))
        setPlotData(data)
      }
    }
  }, [monitorData])

  const renderPlot = () => {
    if (Object.entries(plotData).length !== 0 && autoArrange) {
      let d = plotData

      console.log("RENDERING: " + deviceId + " chart: " + filter + " Revision:" + revision + " color:" + color)
      let colorArray = []
      d.marker.color.map((c) => {
        colorArray.push(c == "red" ? c : color)
      })

      console.log(colorArray)
      const plotOptions = { responsive: true, displaylogo: false }
      setPlot(
        {
          data: [
            {
              type: "scatter",
              mode: "lines+markers",

              ...d,
              marker: { color: colorArray },
            },
          ],

          layout: {
            title: label,
            font: { size: 12 },
            height: 320,
            width: "550",
            datarevision: revision + 1,
          },
          revision: revision,
        },
        plotOptions
      )
    }
  }

  return (
    <Plot
      onInitialized={(figure) => setPlot(figure)}
      onUpdate={(figure) => {
        setPlot(figure)
        setAutoArrange(figure.layout.xaxis.autorange)
        console.log(figure)
      }}
      revision={revision}
      data={plot.data}
      ref={plotRef}
      layout={plot.layout}
      debug={true}
    />
  )
}

const mapStateToProps = (state) => {
  return {
    monitorData: state.main.monitorData,
  }
}

export default connect(mapStateToProps)(Chart)
