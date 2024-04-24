import { useEffect, useState, useRef } from "react"
import { Stage, Layer, Image, Group, Text, Circle } from "react-konva"
// import dynamic from "next/dynamic"
import axios from "axios"

import { connect } from "react-redux"
import { setActiveComponent, setActiveDeviceId } from "../redux/actions/main"

const Schema = (props) => {
  const stageRef = useRef()
  // const Stage = dynamic(() => import("react-konva").then((module) => module.Stage), { ssr: false })
  // const Layer = dynamic(() => import("react-konva").then((module) => module.Layer), { ssr: false })
  // const Image = dynamic(() => import("react-konva").then((module) => module.Image), { ssr: false })
  // const Rect = dynamic(() => import("react-konva").then((module) => module.Rect), { ssr: false })
  // const Circle = dynamic(() => import("react-konva").then((module) => module.Circle), { ssr: false })
  // const Text = dynamic(() => import("react-konva").then((module) => module.Text), { ssr: false })
  // const Group = dynamic(() => import("react-konva").then((module) => module.Group), { ssr: false })

  const { monitorData, config, setActiveComponent, setActiveDeviceId } = props
  const [provisioned, setProvisioned] = useState([])
  const [devices, setDevices] = useState([])

  const [image, setImage] = useState(null)
  const [scale, setScale] = useState(1)
  const [stageX, setStageX] = useState(0)
  const [stageY, setStageY] = useState(0)
  const [squares, setSquares] = useState([])
  const [width, setWidth] = useState(500)
  const [height, setHeight] = useState(500)

  const url = `${process.env.NEXT_PUBLIC_APIPROTO}://${process.env.NEXT_PUBLIC_APIHOST}:${process.env.NEXT_PUBLIC_APIPORT}`;


  useEffect(()=>{
    const loadDevices = async () => {
      const dbDevices = await axios.get(url + "/api/devices/devices").then((res) => {
        return res.data
      })
      setDevices(dbDevices)
    }
    loadDevices()
    console.log("Schema inicio...")
    
  },[])

  useEffect(() => {
    if (config.image) {
      const newImage = new window.Image()
      newImage.src = config.image
      setImage(config.image)
    }
    const aspectRatio = 16 / 9
    const width = window.innerWidth * 0.87
    const height = width / aspectRatio
    setWidth(width)
    setHeight(height)

    // if (window.innerWidth > 600) {
    //   setWidth((window.innerWidth / 100) * 55)
    // } else {
    //   setWidth((window.innerWidth / 100) * 80)
    // }
    // setHeight(window.innerHeight)

    let prov = devices.filter((device) => device.status.provisioned === true)
    setProvisioned(prov)
  }, [])

  useEffect(() => {
    setWidth(window.innerWidth)
    // if (window.innerWidth > 600) {
    //   setWidth((window.innerWidth / 100) * 99)
    // } else {
    //   setWidth((window.innerWidth / 100) * 99)
    // }
    setHeight(window.innerHeight)
  }, [window])

  useEffect(() => {
    if (config.image) {
      const newImage = new window.Image()
      newImage.src = config.image
      setImage(newImage)
    }
  }, [config])

  useEffect(() => {
    console.log("===RECIBIENDO DATOS DESDE MONITOR===")

    let newSquares = []
    monitorData && monitorData?.map((monitor) => {
      const mdevice = JSON.parse(monitor)
      let fill = mdevice.connected ? "green" : "red"

      if (mdevice.connected && Object.values(mdevice.field_values || {}).some(fieldValue => fieldValue.alert)) {
        fill = "yellow"
      }
      
      let device = devices.find((square) => square.id == mdevice.id)
      if(device){
      const label = device?.name ? device?.name + " (" + device?.type + "-" + device?.id + ")" : device?.type + "-" + device?.id
      device = {
        ...device,
        x: device?.status.x,
        y: device?.status.y,
        fill: fill,
        name: label,
        id: device?.id,
        key: device?.status.x * device?.status.y,
      }
      if (device?.image) {
        const newImage = new window.Image()
        newImage.src = device?.image
        device.image = newImage
      }
      newSquares.push(device)
    }
    })
    setSquares(newSquares)
  }, [monitorData, devices])

  useEffect(() => {
    let prov = devices.filter((device) => device.status.provisioned === true)
    setProvisioned(prov)
    let newSquares = []
    prov.map((device) => {
      if (device.status.provisioned) {
        const fill = device.status.connected ? "green" : "red"

        // const label = device.name ? device.name : device.type + "-" + device.id
        const label = device.name ? device.name + " (" + device.type + "-" + device.id + ")" : device.type + "-" + device.id

        const square = {
          x: device.status.x,
          y: device.status.y,
          fill: fill,
          name: label,
          id: device.id,
        }

        if (device.image) {
          const newImage = new window.Image()
          newImage.src = device.image
          square.image = newImage
        }

        newSquares.push(square)
      }
    })
    setSquares(newSquares)
    console.log("NEW DEVICES STATUS")
  }, [devices])

  function removeDuplicates(data, key) {
    return [...new Map(data.map((item) => [key(item), item])).values()]
  }

  const reCenterMap = (e) => {
    e.preventDefault()
    setScale(1)
    setStageX(0)
    setStageY(0)
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
    const scaleBy = 1.00
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
    const lastCenter = null
    const lastDist = 0
  }
  // return <>Schema </>
  
  return (
    <div
      style={{
        width: "100%",
        border: "1px solid grey",
        backgroundColor: "#212529",
      }}
      ref={(node) => {
        const container = node
      }}
    >
      <button className="btn btn-sm btn-secondary position-absolute bottom-0 start-50 translate-middle-x" style={{ zIndex: 1000 }} onClick={reCenterMap}>
        Reset Map Position/Size
      </button>

      <Stage
        ref={stageRef}
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
          {squares.map((square) => (
            <>
            <Group key={square.id}>
              <Text text={square.name} x={square.x + 20} y={square.y + 0} fill="#000000" stroke="#ffffff" fillAfterStrokeEnabled="true" />
              {square.image && (
                <>
                <Image
                  image={square.image}
                  layout="fill"
                  x={square.x + -10}
                  y={square.y + 15}
                  width={100}
                  height={100}
                  onError={(e) => console.error("Error loading image:", e)}
                />
                </>
                
              )}
              <Circle
                radius={10}
                x={square.x}
                y={square.y}
                fill={square.fill}
                id={square.id?.toString()}
                onClick={() => selectDevice(square.id.toString())}
                onTap={() => selectDevice(square.id.toString())}
              />
            </Group>
            </>
          ))}
        </Layer>
      </Stage>
    </div>
  )
}
const mapStateToProps = (state) => {
  return {
    monitorData: state.main.monitorData,
    config: state.main.config,
    // devices: state.main.devices,
  }
}

const mapDispatchToProps = {
  setActiveComponent,
  setActiveDeviceId,
}

export default connect(mapStateToProps, mapDispatchToProps)(Schema)
