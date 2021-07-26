import { useEffect, useState } from "react"
import { Container, Card } from "react-bootstrap"
import dynamic from "next/dynamic"
import { connect } from "react-redux"

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
  const Circle = dynamic(
    () => import("react-konva").then((module) => module.Circle),
    {
      ssr: false,
    }
  )
  const Text = dynamic(() => import("react-konva").then((module) => module.Text), {
    ssr: false,
  })
  const Group = dynamic(() => import("react-konva").then((module) => module.Group), {
    ssr: false,
  })

  const { monitorData, config, devices } = props
  const [image, setImage] = useState(null)
  const [scale, setScale] = useState(1)
  const [x, setX] = useState(100)
  const [y, setY] = useState(100)
  const [squares, setSquares] = useState([])
  const [width, setWidth] = useState(500)
  const [height, setHeight] = useState(500)

  useEffect(() => {
    if (config.image) {
      const newImage = new window.Image()
      newImage.src = config.image
      setImage(config.image)
    }

    setWidth((window.innerWidth / 100) * 50)
    setHeight(window.innerHeight / 1.65)
  }, [])

  // useEffect(() => {
  //   setWidth((window.innerWidth / 100) * 50)
  //   setHeight(window.innerHeight)
  // }, [window])

  useEffect(() => {
    if (config.image) {
      const newImage = new window.Image()
      newImage.src = config.image
      setImage(newImage)
    }
  }, [config.image])

  useEffect(() => {
    console.log("===RECIBIENDO DATOS DESDE MONITOR===")
    monitorData.map((monitor) => {
      const data = JSON.parse(monitor)
      let fill = data.connected ? "green" : "red"
      if (data.connected || data.alerts != undefined) {
        fill = "yellow"
      }

      let newSquares = squares
      let square = devices.find((square) => square.id == data.id)
      square = {
        ...square,
        x: square.status.x,
        y: square.status.y,
        fill: fill,
        name: square.type + "-" + square.id,
        id: square.id,
      }
      if (square.status.provisioned) newSquares.push(square)
      setSquares(removeDuplicates(newSquares, (square) => square.id))
    })
  }, [monitorData])

  useEffect(() => {
    console.log("===RECIBIENDO DATOS DESDE DEVICES===")

    if (devices) {
      devices.map((device) => {
        if (device.status.provisioned) {
          const fill = device.status.connected ? "green" : "red"
          let newSquares = squares
          const square = {
            x: device.status.x,
            y: device.status.y,
            fill: fill,
            name: device.type + "-" + device.id,
            id: device.id,
          }
          newSquares.push(square)
          setSquares(removeDuplicates(newSquares, (square) => square.id))
        }
      })
    }
  }, [devices])

  function removeDuplicates(data, key) {
    return [...new Map(data.map((item) => [key(item), item])).values()]
  }

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
    setScale(newScale)
    setX((stage.getPointerPosition().x / newScale - mousePointTo.x) * newScale)
    setY((stage.getPointerPosition().y / newScale - mousePointTo.y) * newScale)
  }
  // return <>Schema </>

  return (
    <Container>
      <Card>
        <Card.Header>Leaky Feeder network status</Card.Header>
        <Card.Body>
          <Stage
            width={width}
            height={height}
            onWheel={handleWheel}
            scaleX={scale}
            scaleY={scale}
            x={x}
            y={y}
            draggable
            id="myStage"
          >
            <Layer>
              <Image image={image} layout="fill" />
              {squares.map((square) => {
                return (
                  <Group>
                    <Text text={square.name} x={square.x + 20} y={square.y + 5} />
                    <Circle
                      radius={10}
                      x={square.x}
                      y={square.y}
                      fill={square.fill}
                      id={square.id.toString()}
                    />
                  </Group>
                )
              })}
            </Layer>
          </Stage>
        </Card.Body>
      </Card>
    </Container>
  )
}
const mapStateToProps = (state) => {
  return {
    monitorData: state.main.monitorData,
    config: state.main.config,
    devices: state.main.devices,
  }
}

export default connect(mapStateToProps)(Schema)
