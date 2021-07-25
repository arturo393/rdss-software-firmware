import { useEffect } from "react"
import axios from "axios"

const DeviceGraphs = (props) => {
  const { device } = props
  // Va a buscar los datos a la API
  // Refresca los gráficos desde monitorData del redux

  useEffect(() => {
    //AXIOS
  }, [device])

  return (
    <>
      Estos son los gráficos del device ID: {device}
      {device > 0 && (
        <div>
          <h5>gráfico1</h5>
          <h5>gráfico2</h5>
          <h5>gráfico3</h5>
        </div>
      )}
    </>
  )
}

export default DeviceGraphs
