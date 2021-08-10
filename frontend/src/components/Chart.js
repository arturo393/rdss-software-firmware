import C3Chart from "react-c3js"
import { connect } from "react-redux"
import { useState, useEffect, useRef } from "react"

const Chart = (props) => {
  const { monitorData } = props
  const chartRef = useRef()

  //Configuración inicial de la gráfica
  //para más información revisar:https://c3js.org/
  const [data, setData] = useState({
    columns: [["sample"]],
  })

  //Acá va la lógica de conversión de datos que vienen desde monitorData
  useEffect(() => {
    const number = Math.floor(Math.random() * 100 + 1)
    let arr = data.columns[0]
    arr.push(number)
    setData({ ...data, arr })
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
      console.log(domain)
    },
  }
  const resetChart = () => {
    chartRef.current.chart.zoom([0, 0])
    // chartRef.current.chart.flush()
  }

  return (
    <>
      <C3Chart data={data} zoom={zoom} ref={chartRef} />

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
