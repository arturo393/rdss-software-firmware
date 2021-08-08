import React, { useEffect, useState } from "react"
import { Container, Card, Button } from "react-bootstrap"
import axios from "axios"
// import dynamic from "next/dynamic"
import { Stage, Layer, Image, Group, Text, Circle } from "react-konva"

import { setDevices } from "../redux/actions/main"
import { connect } from "react-redux"

import useImage from "use-image"
import { alert } from "react-bootstrap-confirmation"

const Diagram = (props) => {
  const { config, devices, setDevices } = props

  const [newDevices, setNewDevices] = useState([])
  const [squares, setSquares] = useState([])
  const [vlad, setVlad] = useState({})
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

  function removeDuplicates(data, key) {
    return [...new Map(data.map((item) => [key(item), item])).values()]
  }

  useEffect(() => {
    setWidth((window.innerWidth / 100) * 80)
    setHeight(window.innerHeight / 2)
    if (devices.length > 0) {
      let squaresArr = []

      devices.map((device) => {
        let square = {}
        if (device.status.provisioned) {
          const fill = device.status.connected ? "green" : "red"
          square = {
            x: device.status.x,
            y: device.status.y,
            fill: fill,
            name: device.type + "-" + device.id,
            id: device.id,
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
    const vladId = e.target.attrs.id
    setVlad({ id: vladId })
    let newSquares = squares

    let index = squares.findIndex((el) => el.id == vladId)
    newSquares[index].x = e.target.x()
    newSquares[index].y = e.target.y()
    setSquares(removeDuplicates(newSquares, (square) => square.id))

    const device = {
      id: parseInt(vladId),
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

  const onChangeVlad = (e) => {
    setVlad({ id: e.target.value })
  }

  const onClickAdd = (e) => {
    if (vlad.id == 0) {
      alert("You must select an option.")
    } else if (squares.some((s) => s.id == parseInt(vlad.id))) {
      alert("Vlad-" + vlad.id + " is already on the map.")
    } else {
      let newSquares = squares
      const square = {
        x: 100,
        y: 100,
        fill: "red",
        name: "Vlad - " + vlad.id,
        id: vlad.id,
      }
      newSquares.push(square)
      setSquares(removeDuplicates(newSquares, (square) => square.id))

      const device = { id: parseInt(vlad.id) }
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
    if (vlad.id == 0) {
      alert("You must select an option.")
    } else if (!squares.some((s) => s.id == parseInt(vlad.id))) {
      alert("Vlad - " + vlad.id + " is not on the map.")
    } else {
      let newSquares = squares
      let square = squares.find((square) => square.id == vlad.id)
      newSquares.pop(square)
      setSquares(removeDuplicates(newSquares, (square) => square.id))

      const device = { id: parseInt(vlad.id) }
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

      let newDevicesList = devices || newDevices
      newDevicesList[objIndex] = dev
      setNewDevices(newDevicesList)
      setDevices(newDevicesList)
    } else {
      console.log("NO HAY DATA ID")
    }
  }

  useEffect(() => {
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

  return (
    <>
      <div className="container-fluid">
        <div className="row">
          <div className="col-2"></div>
          <div className="col-7">
            <div className="card-body text-center">
              <div className="input-group mb-3">
                <span className="input-group-text" id="device-label">
                  Device
                </span>
                <select className="form-control" id="device" onChange={onChangeVlad}>
                  <option value={0}>=== Select a Device ===</option>
                  {devices.map((device) => {
                    return (
                      <option value={device.id}>
                        {device.type}
                        {device.id}
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
          </div>
          <div className="col-2"></div>
        </div>
      </div>
      <Container className="sigmaDarkBg">
        <Stage width={width} height={height} onWheel={handleWheel} scaleX={scale} scaleY={scale} onDragStart={handleDragStart} onDragEnd={handleDragEnd} x={stageX} y={stageY} draggable>
          <Layer>
            <Image image={image} />
            {squares &&
              squares.map((square) => {
                return (
                  <Group>
                    <Text text={square.name} x={square.x + 20} y={square.y + 5} fill="#000000" stroke="#ffffff" fillAfterStrokeEnabled="true" />
                    <Circle x={square.x} y={square.y} radius={10} fill={square.fill} onDragEnd={onDragEndSquare} draggable id={square.id.toString()} />
                  </Group>
                )
              })}
          </Layer>
        </Stage>
      </Container>
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
  // saveConfig,
}

export default connect(mapStateToProps, mapDispatchToProps)(Diagram)
