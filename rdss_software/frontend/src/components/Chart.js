import { connect } from "react-redux"
import { useState, useEffect, useRef } from "react"

import plotly from "plotly.js/dist/plotly"
import createPlotComponent from "react-plotly.js/factory"

import { DateTime } from "luxon"

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

    if (deviceId && Object.entries(rtData).length) {
      const data = { x: rtData.x, y: rtData.rtd[filter], marker: rtData.marker, text: rtData.text }
      
      setRevision(rtData.x.length + deviceId + Math.floor(Math.random() * 100 + 1))
      setPlotData(data)
    }
  }, [])

  useEffect(() => {
    if (Object.entries(rtData).length) {
      // console.log("rtData changed",rtData)
      const data = { x: rtData.x, y: rtData.rtd[filter], marker: rtData.marker, text: rtData.text }
      setRevision(rtData.x.length + deviceId + Math.floor(Math.random() * 100 + 1))
      setPlotData(data)
    }
  }, [rtData])

  useEffect(() => {
    setAutoArrange(true)
  }, [deviceId])

  useEffect(() => {
    // console.log("plotData changed")
    renderPlot()
  }, [revision])

  const getPointText = (status = {}) => {

    let alerted = false

    let t = "<b>Device: " + deviceId + "</b><br><br>\n"

    if (!status.connected) {
      t += " * Disconnected<br>\n"
    }
    t += (status?.alert)?"Value Out of Limits":"<br>\n"

    alerted = status?.alert || !status.connected

    // console.log("Connected", status.connected)
    // console.log("Alert", status?.alert)
    // console.log("Alerted", alerted)
    // console.log("T", t)

    return { text: t, alerted: alerted }
  }

  // //Acá va la lógica de conversión de datos que vienen desde monitorData
  useEffect(() => {
    console.log("Getting data from monitor...",deviceId)
    

    if (deviceId) {
      //Gets device data from monitorData
      let currentDeviceData = {}
      

      monitorData && monitorData?.map((monitor) => {
        const data = JSON.parse(monitor)

        if (data.id == deviceId) {
          currentDeviceData = data
        }
        if (Object.entries(plotData).length !== 0) {
          let data = plotData
          // console.log("plotData", plotData)
          // console.log("currentDeviceData",currentDeviceData)
          const currData = currentDeviceData?.field_values?.[filter]
          // console.log("currData",currData)
          // console.log("currData",currData)
          const alertStatus = getPointText({connected: currData?.connected, alert: currData?.field_values?.[filter]?.alert})
          // console.log("ALERT STATUS", alertStatus)
  
          // data?.y?.push((typeof currentDeviceData?.field_values?.[filter]?.value) === 'string'? parseFloat(currentDeviceData?.field_values?.[filter]?.value):currentDeviceData?.field_values?.[filter]?.value || data.y[data.y.length - 1])
          const currentIndex = data?.y?.length 

          let tmpTS = ""
          let TS = {}
          if (currentDeviceData.sampleTime !== undefined) tmpTS = JSON.stringify(currentDeviceData.sampleTime).replace("$date", "date").replace("T", " ").replace("Z", "")
          if (tmpTS) TS = JSON.parse(tmpTS)
 

          data.x.push(TS)
          data.y.push(parseFloat(currentDeviceData?.field_values?.[filter]?.value))
          data.text.push(alertStatus.text)
          data.marker.color.push(alertStatus.alerted ? "red" : color)
          
         
          setRevision(currentIndex + deviceId + Math.floor(Math.random() * 100 + 1))
          setPlotData(data)
        } else {
          console.log("sin plotData")
        }
      })

      


      
    }
  }, [monitorData])


  const renderPlot = () => {
    if (Object.entries(plotData).length !== 0 && autoArrange) {
      let d = plotData

      let colorArray = []
      d.marker.color.map((c) => {
        colorArray.push(c == "red" ? c : color)
      })

      const plotOptions = { responsive: true, displaylogo: false, scrollZoom: true }
      setPlot(
        {
          data: [
            {
              type: "scatter",
              mode: "lines+markers",
              // name: label,

              ...d,
              marker: { color: colorArray },
            },
          ],

          layout: {
            yaxis: {
              title: label,
              // ticktext: ["long label", "Very long label", "3", "label"],
              // tickvals: [1, 2, 3, 4],
              tickmode: "array",
              automargin: true,
              titlefont: { size: 20 },
            },
            // title: { text: label, x: 0, font: { size: 12, color: "white" } },
            margin: { l: 10, r: 10, t: 50, b: 50 },
            // font: { size: 10 },
            // paper_bgcolor: "#000000",
            // plot_bgcolor: "#c7c7c7",
            hovermode: "x",
            // title: label,
            height: 300,
            width: 1500,
            autosize: true,
            datarevision: revision + 1,
          },
          revision: revision,
          config: plotOptions,
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
      }}
      revision={revision}
      data={plot.data}
      ref={plotRef}
      layout={plot.layout}
      debug={true}
      config={plot.config}
      className="w-75"
    />
  )
}

const mapStateToProps = (state) => {
  return {
    monitorData: state.main.monitorData,
  }
}

export default connect(mapStateToProps)(Chart)
