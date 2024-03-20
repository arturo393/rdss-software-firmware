import React, { useEffect, useState } from "react"
import { Container, Card, Button } from "react-bootstrap"
import axios from "axios"
// import dynamic from "next/dynamic"
import { Stage, Layer, Image, Group, Text, Circle } from "react-konva"

import { setDevices, setConfig } from "../redux/actions/main"
import { connect } from "react-redux"

import useImage from "use-image"
import { alert } from "react-bootstrap-confirmation"

const Diagram = (props) => {
  const { config, devices, setDevices, setConfig } = props

  const [file, setFile] = useState()
  const [base64, setBase64] = useState()
  const [saving, setSaving] = useState(false)
  const [newConfig, setNewConfig] = useState({})
  const [status, setStatus] = useState()

  const [newDevices, setNewDevices] = useState([])
  const [squares, setSquares] = useState([])
  const [dev, setDev] = useState({})
  const [scale, setScale] = useState(1)
  const [x, setX] = useState(100)
  const [y, setY] = useState(100)

  const [stageX, setStageX] = useState(0)
  const [stageY, setStageY] = useState(0)

  const [width, setWidth] = useState(500)
  const [height, setHeight] = useState(500)

  const [image] = useImage(config.image)

  // const Stage = dynamic(() => import("react-konva").then((module) => module.Stage), { ssr: false })
  // const Layer = dynamic(() => import("react-konva").then((module) => module.Layer), { ssr: false })
  // const Image = dynamic(() => import("react-konva").then((module) => module.Image), { ssr: false })
  // const Circle = dynamic(() => import("react-konva").then((module) => module.Circle), { ssr: false })
  // const Text = dynamic(() => import("react-konva").then((module) => module.Text), { ssr: false })
  // const Group = dynamic(() => import("react-konva").then((module) => module.Group), { ssr: false })

  const url = `${process.env.NEXT_PUBLIC_APIPROTO}://${process.env.NEXT_PUBLIC_APIHOST}:${process.env.NEXT_PUBLIC_APIPORT}`;

  const loadDevices = async () => {
    const dbDevices = await axios.get(url + "/api/devices/devices").then((res) => {
      return res.data
    })
    setDevices(dbDevices)
  }

  function removeDuplicates(data, key) {
    return [...new Map(data.map((item) => [key(item), item])).values()]
  }

  useEffect(() => {
    loadDevices()
    document.getElementById("status").style.display = "none"
    const aspectRatio = 16 / 9
    const width = window.innerWidth * 1
    const height = width / aspectRatio
    setWidth(width)
    setHeight(height)

    if (devices.length > 0) {
      let squaresArr = []
      devices.map((device) => {
        let square = {}
        if (device.status.provisioned) {
          const fill = device.status.connected ? "green" : "red"
          // const label = device.name ? device.name : device.type + "-" + device.id
          const label = device.name ? device.name + " (" + device.type + "-" + device.id + ")" : device.type + "-" + device.id

          square = {
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
          squaresArr.push(square)
        }
      })
      setSquares(removeDuplicates(squaresArr, (square) => square.id))
      // setStageX(config.x)
      // setStageY(config.y)
    }
  }, [])

  useEffect(() => {
    if (devices.length > 0) setNewDevices(devices)
  }, [devices])

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

  const onDragEndSquare = (e) => {
    const devId = e.target.attrs.id
    setDev({ id: devId })
    let newSquares = squares

    let index = squares.findIndex((el) => el.id == devId)
    newSquares[index].x = e.target.x()
    newSquares[index].y = e.target.y()
    setSquares(removeDuplicates(newSquares, (square) => square.id))

    const device = {
      id: parseInt(devId),
      x: e.target.x(),
      y: e.target.y(),
    }

    updateDeviceList("changePos", device)

    axios.post(process.env.NEXT_PUBLIC_APIPROTO + "://" + process.env.NEXT_PUBLIC_APIHOST + ":" + process.env.NEXT_PUBLIC_APIPORT + "/api/devices/editDevice", device).then(
      (result) => {},
      (error) => {
        console.log(error)
      }
    )
  }

  const onChangeDev = (e) => {
    setDev({ id: e.target.value })
  }

  const onClickAdd = (e) => {
    const device = devices.find((d) => d.id == parseInt(dev.id))
    const label = device?.name ? device?.name : device?.type + "-" + device?.id

    if (dev.id == 0) {
      alert("You must select an option.")
    } else if (squares.some((s) => s.id == parseInt(dev.id))) {
      alert("(" + label + ") is already on the map.")
    } else {
      let newSquares = squares

      const square = {
        x: 100,
        y: 100,
        fill: "red",
        name: label,
        id: dev.id,
      }
      newSquares.push(square)
      setSquares(removeDuplicates(newSquares, (square) => square.id))

      const device = { id: parseInt(dev.id) }
      axios.post(process.env.NEXT_PUBLIC_APIPROTO + "://" + process.env.NEXT_PUBLIC_APIHOST + ":" + process.env.NEXT_PUBLIC_APIPORT + "/api/devices/editDeviceProvisioned", device).then(
        (result) => {},
        (error) => {
          console.log(error)
        }
      )
      updateDeviceList("add", device)
    }
  }

  const onClickDel = (e) => {
    if (dev.id == 0) {
      alert("You must select an option.")
    } else if (!squares.some((s) => s.id == parseInt(dev.id))) {
      alert("Device - " + dev.id + " is not on the map.")
    } else {
      let newSquares = squares.filter((square) => square.id != dev.id)
      // let square = squares.find((square) => square.id == dev.id)
      // newSquares.pop(square)
      setSquares(newSquares)

      const device = { id: parseInt(dev.id) }
      axios.post(process.env.NEXT_PUBLIC_APIPROTO + "://" + process.env.NEXT_PUBLIC_APIHOST + ":" + process.env.NEXT_PUBLIC_APIPORT + "/api/devices/delDevice", device).then(
        (result) => {},
        (error) => {
          console.log(error)
        }
      )
      updateDeviceList("del", device)
    }
  }

  const updateDeviceList = (action, data = {}) => {
    if (data.id > 0) {
      const objIndex = newDevices.findIndex((d) => d.id == parseInt(data.id))
      let dev = devices.find((d) => d.id == parseInt(data.id))

      if (action == "add") {
        dev.status.provisioned = true
      }
      if (action == "del") {
        dev.status.provisioned = false
      }
      if (action == "changePos") {
        dev.status.provisioned = true
        dev.status.x = data.x
        dev.status.y = data.y
      }

      let newDevicesList = devices
      newDevicesList[objIndex] = dev
      setNewDevices(newDevicesList)
    } else {
      console.log("THERE IS NO DATA ID")
    }
  }

  useEffect(() => {
    console.log("SETTING NEW DEVICES STATUS")
    setDevices(newDevices)
  }, [newDevices])

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

    // saveConfig({ x: stageX, y: stageY, image: config.image })
  }

  const selectDevice = (deviceId) => {
    document.getElementById("device").value = deviceId
    setDev({ id: deviceId })
  }

  const handleImageChange = (e) => {
    document.getElementById("status").style.display = "none"
    const file = e.target.files[0]
    if (file) {
      let reader = new FileReader()
      reader.readAsDataURL(file)
      reader.onloadend = () => {
        setFile(file)
        setBase64(reader.result)
      }
    }
  }

  const saveConfig = (e) => {
    e.preventDefault()
    setConfig(newConfig)
    setSaving(true)
  }

  useEffect(() => {
    if (base64) setNewConfig({ ...config, image: base64 })
  }, [base64])

  useEffect(() => {
    if (config) {
      setNewConfig(config)
      if (saving) {
        axios.post(process.env.NEXT_PUBLIC_APIPROTO + "://" + process.env.NEXT_PUBLIC_APIHOST + ":" + process.env.NEXT_PUBLIC_APIPORT + "/api/manage/editConfig", config).then(
          (result) => {
            document.getElementById("status").style.display = "block"
            result ? setStatus("Map image updated successfully") : setStatus("Error when try to save config")
          },
          (error) => {
            console.log(error)
          }
        )
        setSaving(false)
      }
    }
  }, [config])

  return (
    <>
      <div class="container-fuid">
        <div class="row text-center">
        <h5 className="text-center w-100 sigmaRed text-light">Diagram Edit</h5>
          <div class="col-2"></div>
          <div class="col-7">
            <label for="image" className="form-label">
            
            
            </label>
            <div class="input-group mb-3">
              <input className="form-control" type="file" id="image" name="image" onChange={handleImageChange} />
              <button className="btn btn-primary" type="button" onClick={saveConfig}>
                Update
              </button>
              *(use PNG or JPG)
            </div>
            <div class="input-group mb-3">
              <span className="input-group-text" id="device-label">
                Device
              </span>
              <select className="form-control" id="device" onChange={onChangeDev}>
                <option value={0}>=== Select a Device ===</option>
                {devices.map((device) => {
                  return (
                    <option value={device.id}>
                      {device.name} ({device.type}-{device.id})
                    </option>
                  )
                })}
              </select>
              <button className="btn btn-primary" type="button" onClick={onClickAdd}>
                Add
              </button>
              <button className="btn btn-danger" type="button" onClick={onClickDel}>
                Del
              </button>
            </div>
          </div>
          <div class="col-2"></div>
        </div>
      </div>
      <div className="row">
        <div className="col-md-12 text-center">
          <div className="alert alert-success" role="alert" id="status">
            {status}
          </div>
        </div>
      </div>

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
        <Stage width={width} height={height} onWheel={handleWheel} scaleX={scale} scaleY={scale} onDragStart={handleDragStart} onDragEnd={handleDragEnd} x={stageX} y={stageY} draggable>
          <Layer>
            <Image image={image} />
            {squares &&
              squares.map((square) => {
                return (
                  <Group>
                    <Text text={square.name} x={square.x + 20} y={square.y + 5} fill="#000000" stroke="#ffffff" fillAfterStrokeEnabled="true" />
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
                      x={square.x}
                      y={square.y}
                      radius={10}
                      fill={square.fill}
                      onDragEnd={onDragEndSquare}
                      draggable
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
    </>
  )
}

const mapStateToProps = (state) => {
  return {
    config: state.main.config,
    devices: state.main.devices,
  }
}

const mapDispatchToProps = {
  setDevices,
  setConfig,
  // saveConfig,
}

export default connect(mapStateToProps, mapDispatchToProps)(Diagram)
