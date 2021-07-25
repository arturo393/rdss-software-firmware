import { useEffect, useState } from "react"
import { Container, Card } from "react-bootstrap"
import axios from "axios"
import dynamic from "next/dynamic"
import useImage from "use-image"
import { connect } from "react-redux"
import Test from "../images/test.jpg"

const Schema = (props) => {

  const Stage = dynamic(() => import("react-konva").then((module) => module.Stage), {
    ssr: false,
  })
  const Layer = dynamic(() => import("react-konva").then((module) => module.Layer), {
    ssr: false,
  })
  const Image = dynamic(() => import("react-konva").then((module) => module.Image), {
    ssr: false,
  })
  const Rect = dynamic(() => import("react-konva").then((module) => module.Rect), {
    ssr: false,
  })
  const Text = dynamic(() => import("react-konva").then((module) => module.Text), {
    ssr: false,
  })
  const Group = dynamic(() => import("react-konva").then((module) => module.Group), {
    ssr: false,
  })


  const [devices, setDevices] = useState([])

  const { monitorData } = props
  
  const [stateConfig] = useState({
    config: [{ image: Test.src }],
  })

  const [state, setState] = useState({
    scale: 1,
    x: 0,
    y: 0,
    devices: props.devices,
  })

  const [stateSquare, setSquare] = useState({
    squares: [],
  })

  const [image] = useImage(stateConfig.config[0].image)

  useEffect(() => {

    monitorData?.map((monitor) => {
    const data = JSON.parse(monitor)

    let fill = data.connected ? "green" : "red"
    if(data.connected || data.alerts != undefined){
      fill = 'yellow'
    }

    let squares = [...stateSquare.squares]
    let square = squares.find(square => square.id == data.id)
    squares.pop(square)
    square.fill = fill
    squares.push(square)
    setSquare({ squares })

    })
  }, [monitorData])


  useEffect(() => {
    devices?.map((device) => {
      if (device.status.provisioned) {
        const fill = device.status.connected ? "green" : "red"
        setSquare((prevSquare) => ({
          squares: [
            ...prevSquare.squares,
            {
              x: device.status.x,
              y: device.status.y,
              fill: fill,
              name: "Vlad - " + device.id,
              id: device.id,
            },
          ],
        }))
      }
    })
  }, [devices])

  useEffect(() => {
    axios.get("http://localhost:3000/api/devices/devices").then((res) => {
      const devices = res.data
      setDevices(devices)
    })

  }, [])

  const handleWheel = (e) => {
    e.evt.preventDefault()

    const scaleBy = 1.02
    const stage = e.target.getStage()
    const oldScale = stage.scaleX()
    const mousePointTo = {
      x: stage.getPointerPosition().x / oldScale - stage.x() / oldScale,
      y: stage.getPointerPosition().y / oldScale - stage.y() / oldScale,
    }

    const newScale = e.evt.deltaY < 0 ? oldScale * scaleBy : oldScale / scaleBy

    setState({
      ...state,
      scale: newScale,
      x: (stage.getPointerPosition().x / newScale - mousePointTo.x) * newScale,
      y: (stage.getPointerPosition().y / newScale - mousePointTo.y) * newScale,
    })
  }

  return (
    <Container>
      <Card>
        <Card.Header>Leaky Feeder network status</Card.Header>
        <Card.Body>
          <blockquote className="blockquote mb-0">
            <Stage
              width={500}
              height={800}
              onWheel={handleWheel}
              scaleX={state.scale}
              scaleY={state.scale}
              x={state.x}
              y={state.y}
              draggable
            >
              <Layer>
                <Image image={image} layout="fill" />
                {stateSquare.squares?.map((square) => {
                  return (
                    <Group>
                      <Text text={square.name} x={square.x + 20} y={square.y + 5} />
                      <Rect
                        x={square.x}
                        y={square.y}
                        width={20}
                        height={20}
                        fill={square.fill}
                        id={square.id}
                      />
                    </Group>
                  )
                })}
              </Layer>
            </Stage>
          </blockquote>
        </Card.Body>
      </Card>
    </Container>
  )
}
const mapStateToProps = (state) => {
  return {
    monitorData: state.main.monitorData,
  }
}

export default connect(mapStateToProps)(Schema)


