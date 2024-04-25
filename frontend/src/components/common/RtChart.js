// Usage:
// *  import RtChart from "../components/common/RtChart"
// const labels = ["Red", "Blue", "Yellow", "Green", "Purple", "Orange"]
// const dataset = [
//   {
//     label: "# of Votes",
//     data: [12, 19, 3, 5, 2, 3],
//   },
// ]
// *  <RtChart title="hola" labels={labels} dataSet={dataset} />
//
import { Line, Chart } from "react-chartjs-2"
import zoomPlugin from "chartjs-plugin-zoom"

Chart.register(zoomPlugin) // REGISTER PLUGIN

const RtChart = (props) => {
  const data = {
    title: props.title,
    labels: props.labels,
    datasets: props.datasets,
  }

  const options = {
    maintainAspectRatio: false,
    responsive: true,
    elements: {
      point: {
        radius: 0,
      },
      line: {
        borderWidth: 1.5,
      },
    },
    scales: {
      x: {
        ticks: {
          color: "rgba( 0, 0, 1)",
        },
        grid: {
          color: "rgba(0, 0, 0, 1)",
        },
      },
      y: {
        min: 1,
        max: 100,
        type: "logarithmic",
        ticks: {
          color: "rgba(0, 0, 0, 1)",
        },
        grid: {
          color: "rgba(0, 0, 0, 1)",
        },
      },
    },
    plugins: {
      zoom: {
        zoom: {
          wheel: {
            enabled: true, // SET SCROOL ZOOM TO TRUE
          },
          mode: "xy",
          speed: 100,
        },
        pan: {
          enabled: true,
          mode: "xy",
          speed: 100,
        },
      },
    },
  }

  return (
    <div>
      <Line type="line" data={data} options={options} width={900} height={450} />
    </div>
  )
}

export default RtChart
