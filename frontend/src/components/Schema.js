import { useEffect, useState } from "react"
import { Container, Card } from "react-bootstrap"
import dynamic from "next/dynamic"
import { connect } from "react-redux"
import { setActiveComponent, setActiveDeviceId } from "../redux/actions/main"

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

  const { monitorData, config, devices, setActiveComponent, setActiveDeviceId } =
    props
  const [image, setImage] = useState(null)
  const [scale, setScale] = useState(1)
  const [stageX, setStageX] = useState(0)
  const [stageY, setStageY] = useState(0)
  const [squares, setSquares] = useState([])
  const [width, setWidth] = useState(500)
  const [height, setHeight] = useState(500)

  useEffect(() => {
    if (config.image) {
      const newImage = new window.Image()
      newImage.src = config.image
      setImage(config.image)
    }

    if (window.innerWidth > 600) {
      setWidth((window.innerWidth / 100) * 55)
    } else {
      setWidth((window.innerWidth / 100) * 80)
    }
    setHeight(window.innerHeight)
  }, [])

  useEffect(() => {
    if (window.innerWidth > 600) {
      setWidth((window.innerWidth / 100) * 55)
    } else {
      setWidth((window.innerWidth / 100) * 80)
    }
    setHeight(window.innerHeight)
  }, [window])

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
      if (data.connected && Object.entries(data.alerts).length != 0) {
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

  const selectDevice = (id) => {
    setActiveComponent("rtdata")
    setActiveDeviceId(id)
  }

  const handleWheel = (e) => {
    e.evt.preventDefault()

    const scaleBy = 1.02
    const stage = e.target.getStage()
    const oldScale = stage.scaleX()
    const mousePointTo = {
      key: stageX * stageY,
      x: stage.getPointerPosition().x / oldScale - stage.x() / oldScale,
      y: stage.getPointerPosition().y / oldScale - stage.y() / oldScale,
    }

    const newScale = e.evt.deltaY < 0 ? oldScale * scaleBy : oldScale / scaleBy
    setScale(newScale)
    setStageX((stage.getPointerPosition().x / newScale - mousePointTo.x) * newScale)
    setStageY((stage.getPointerPosition().y / newScale - mousePointTo.y) * newScale)
  }

  const handleDragStart = (e) => {
    e.evt.preventDefault()
  }
  const handleDragEnd = (e) => {
    e.evt?.preventDefault()
    const scaleBy = 1.02
    const stage = e.target.getStage()
    const oldScale = stage.scaleX()
    const mousePointTo = {
      key: stageX * stageY,
      x: stage.getPointerPosition().x / oldScale - stage.x() / oldScale,
      y: stage.getPointerPosition().y / oldScale - stage.y() / oldScale,
    }
    const newScale = e.evt?.deltaY < 0 ? oldScale * scaleBy : oldScale / scaleBy
    setScale(newScale)
    setStageX((stage.getPointerPosition().x / newScale - mousePointTo.x) * newScale)
    setStageY((stage.getPointerPosition().y / newScale - mousePointTo.y) * newScale)
  }
  const handleMultiTouch = (e) => {
    e.evt.preventDefault()
    console.log("MultiTouch")
    var touch1 = e.evt.touches[0]
    var touch2 = e.evt.touches[1]
    const stage = e.target.getStage()

    if (touch1 && touch2) {
      if (stage.isDragging()) {
        stage.stopDrag()
      }

      imageMove = false

      var p1 = {
        x: touch1.clientX,
        y: touch1.clientY,
      }
      var p2 = {
        x: touch2.clientX,
        y: touch2.clientY,
      }

      if (!lastCenter) {
        lastCenter = getCenter(p1, p2)
        return
      }
      var newCenter = getCenter(p1, p2)

      var dist = getDistance(p1, p2)

      if (!lastDist) {
        lastDist = dist
      }

      // local coordinates of center point
      var pointTo = {
        x: (newCenter.x - stage.x()) / stage.scaleX(),
        y: (newCenter.y - stage.y()) / stage.scaleX(),
      }

      var scale = stage.scaleX() * (dist / lastDist)

      stage.scaleX(scale)
      stage.scaleY(scale)

      // calculate new position of the stage
      var dx = newCenter.x - lastCenter.x
      var dy = newCenter.y - lastCenter.y

      var newPos = {
        x: newCenter.x - pointTo.x * scale + dx,
        y: newCenter.y - pointTo.y * scale + dy,
      }

      stage.position(newPos)
      stage.batchDraw()

      lastDist = dist
      lastCenter = newCenter
    }
  }

  const multiTouchEnd = () => {
    let lastCenter = null
    let lastDist = 0
  }
  // return <>Schema </>

  return (
    <div className="container-fluid">
      <div className="row">
        <div className="col">
          <Stage
            width={width}
            height={height}
            onWheel={handleWheel}
            scaleX={scale}
            scaleY={scale}
            onDragStart={handleDragStart}
            onDragEnd={handleDragEnd}
            x={stageX}
            y={stageY}
            draggable
            id="myStage"
            onTouchMove={handleMultiTouch}
            onTouchEnd={() => {
              multiTouchEnd()
            }}
          >
            <Layer>
              <Image image={image} layout="fill" />
              {squares.map((square) => {
                return (
                  <Group>
                    <Text
                      text={square.name}
                      x={square.x + 20}
                      y={square.y + 5}
                      fill="#000000"
                      stroke="#ffffff"
                      fillAfterStrokeEnabled="true"
                    />
                    <Circle
                      radius={10}
                      x={square.x}
                      y={square.y}
                      fill={square.fill}
                      id={square.id.toString()}
                      onClick={() => selectDevice(square.id.toString())}
                      onTap={() => selectDevice(square.id.toString())}
                    />
                  </Group>
                )
              })}
            </Layer>
          </Stage>
        </div>
      </div>
    </div>
  )
}
const mapStateToProps = (state) => {
  return {
    monitorData: state.main.monitorData,
    config: state.main.config,
    devices: state.main.devices,
  }
}

const mapDispatchToProps = {
  setActiveComponent,
  setActiveDeviceId,
}

export default connect(mapStateToProps, mapDispatchToProps)(Schema)
