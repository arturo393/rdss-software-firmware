import React, { useEffect, useState } from "react"
import { connect } from "react-redux"

import { Container, Card, Button } from "react-bootstrap"
import axios from "axios"
import { setDevices } from "../../redux/actions/main"
// import Formulario from "./DeviceFormulario.js";

const DevicesEdit = (props) => {
  const { devices, setDevices } = props
  const [device, setDevice] = useState({})

  const [deviceData, setDeviceData] = useState({})
  const [deviceDataType, setDeviceDataType] = useState({})
  
  const [shopCart, setShopCart] = useState({item1:"Juice", item2: "Icrecream"});

  // let copyOfObject = { ...shopCart }
  // delete copyOfObject['propertyToRemove']
  
  // setShopCart( shopCart => ({
  //       ...copyOfObject
  //     }));



  const [newDevices, setNewDevices] = useState([])
  const [status, setStatus] = useState()

  useEffect(() => {
    document.getElementById("status").style.display = "none"
  }, [])

  useEffect(() => {
    if (devices.length > 0) setNewDevices(devices)
  }, [devices])

  useEffect(() => {
    setDevices(newDevices)
  }, [newDevices])

  const handleDeviceSelected = (e) => {
    e.preventDefault()
    document.getElementById("status").style.display = "none"
    document.getElementById("name").value = ""
    document.getElementById("type").value = ""

    const deviceId = e.target.value
    if (deviceId) {
      const uri = process.env.NEXT_PUBLIC_APIPROTO + "://" + process.env.NEXT_PUBLIC_APIHOST + ":" + process.env.NEXT_PUBLIC_APIPORT + "/api/device/" + deviceId
      axios
        .get(uri)
        .then(
          (result) => {
            return result.data[0]
          },
          (error) => {
            console.log(error)
          }
        )
        .then((res) => {
          setDeviceData(res)
          setDeviceDataType(res.data)
        })
      // setDeviceData(devices.find((device) => device.id === Number(deviceId)))

    }
  }

  const saveDevice = (e) => {
    e.preventDefault()
     

    axios.post(process.env.NEXT_PUBLIC_APIPROTO + "://" + process.env.NEXT_PUBLIC_APIHOST + ":" + process.env.NEXT_PUBLIC_APIPORT + "/api/device/save", deviceData).then(
      (result) => {
        document.getElementById("status").style.display = "block"
        result ? setStatus("Device attributes updated successfully") : setStatus("Error when try to save device data")
        const objIndex = newDevices.findIndex((d) => d.id == parseInt(deviceData.id))
        let dev = devices.find((d) => d.id == parseInt(deviceData.id))
        dev = { ...dev, ...deviceData }
        let newDevicesList = devices
        newDevicesList[objIndex] = dev
        setNewDevices(newDevicesList)
      },
      (error) => {
        console.log(error)
      }
    )
  }
  const handleChange = (e) => {
    setDeviceData({
      ...deviceData,
      [e.target.id]: e.target.value,
    })
  }


  // const handleChange = (e) => {
  //   setDeviceData({
  //     ...deviceData,
  //     data: {
  //       ...deviceData,
  //       din : { ...deviceData.din,
  //         tipo:  e.target.value,
  //         name: "memo"
  //     }
  //   }
  //   })
  // }


  const handleChangetype = (e) => {
    


//  deviceDataType.din.tipo=e.target.value
setDeviceDataType((deviceDataType) => ({
  ...deviceDataType,
  din: {
    ...deviceDataType.din,
    tipo:  e.target.value,
  }
}));

 


  }
  return (
    <>
      <h5 className="text-center">Device Editor</h5>
      <div class="container-fuid">
        <div class="row text-center">
          <div class="col-2"></div>
          <div class="col-7">
            <select className="form-control" id="device" onChange={handleDeviceSelected}>
              <option value={0}>=== Select a Device ===</option>
              {devices.map((device) => {
                return ( 
                  <option value={device.id}>
                    {/* {device.name} ({device.type}-{device.id}) */}
                    {device.name} ({"device"}-{device.id})
                  </option>
                )
              })}
            </select>
            <p></p>
            <div class="input-group mb-3">
              <div className="input-group mb-3">
                <span className="input-group-text">Name</span>
                <input type="text" className="form-control" id="name" value={deviceData?.name} onChange={handleChange} />
              </div>
              <div className="input-group mb-3">
                <span className="input-group-text">Type</span>
                <input type="text" className="form-control" id="type" value={deviceData?.type} onChange={handleChange} />
              </div>

              {/* MEMO FORMULARIO */}
              {/* <  Formulario/>  */}

              <div>
                <div className="">Select Items</div>
                <label>Din </label>
                <div className="input-group mb-3">
                  <span className="input-group-text">tipo</span>
                  {/* <input type="text" className="" id="type1" value={deviceData?.data?.din?.tipo} onChange={handleChange} /> */}
                  <input type="text" className=""  name="tipo1" value={deviceData?.data?.din?.tipo} onChange={handleChange} />
               
                </div>
                <div className="input-group mb-3">
                  <span className="input-group-text">Name</span>
                  <input type="text" className="" id="name1" value={deviceData?.data?.din?.name} />
                </div>
                <div className="input-group mb-4">
                  <span className="input-group-text">stateon</span>
                  <input type="text" className="" id="stateon1" name="stateon" value={deviceData?.data?.din?.stateon} onChange={() => console.log("memo")} />
                </div>
                <div className="input-group mb-3">
                  <span className="input-group-text">stateon</span>
                  <input type="text" className="" id="stateoff1" value={deviceData?.data?.din?.stateoff} onChange={() => console.log("memo")} />
                </div>
                <label>Ain </label>
                <div className="input-group mb-3">
                  <span className="">tipo</span>
                  <input type="text" className="" id="tipo2" value={deviceData?.data?.ain?.tipo} onChange={() => console.log("memo")} />
                </div>
                <div className="input-group mb-3">
                  <span className="input-group-text">Name</span>
                  <input type="text" className="" id="name2" value={deviceData?.data?.ain?.name} onChange={() => console.log("memo")} />
                </div>
                <div className="input-group mb-3">
                  <span className="input-group-text">unit</span>
                  <input type="text" className="" id="unit2" value={deviceData?.data?.ain?.unit} onChange={() => console.log("memo")} />
                </div>
                <div className="input-group mb-3">
                  <span className="input-group-text">maximumanalog</span>
                  <input type="text" className="" id="maximumanalog2" value={deviceData?.data?.ain?.maximumanalog} onChange={() => console.log("memo")} />
                </div>
                <div className="input-group mb-3">
                  <span className="input-group-text">maximumconverted</span>
                  <input type="text" className="" id="maximumconverted2" value={deviceData?.data?.ain?.maximumconverted} onChange={() => console.log("memo")} />
                </div>
                <div className="input-group mb-3">
                  <span className="input-group-text">minimumconverted</span>
                  <input type="text" className="" id="minimumconverted2" value={deviceData?.data?.ain?.minimumconverted} onChange={() => console.log("memo")} />
                </div>
                <div className="input-group mb-3">
                  <span className="input-group-text">maximumalert</span>
                  <input type="text" className="" id="maximumalert2" value={deviceData?.data?.ain?.maximumalert} onChange={() => console.log("memo")} />
                </div>
                <div className="input-group mb-3">
                  <span className="input-group-text">minimumalert</span>
                  <input type="text" className="" id="minimumalert2" value={deviceData?.data?.ain?.minimumalert} onChange={() => console.log("memo")} />
                </div>

                <br></br>
                <label>Aout </label>

                <div className="input-group mb-3">
                  <span className="input-group-text">tipo</span>
                  <input type="text" className="" id="tipo3" value={deviceData?.data?.aout?.tipo} onChange={() => console.log("memo")} />
                </div>
                <div className="input-group mb-3">
                  <span className="input-group-text">Name</span>
                  <input type="text" className="" id="name3" value={deviceData?.data?.aout?.name} onChange={() => console.log("memo")} />
                </div>
                <div className="input-group mb-3">
                  <span className="input-group-text">minimumanalog</span>
                  <input type="text" className="" id="minimumanalog3" value={deviceData?.data?.aout?.minimumanalog} onChange={() => console.log("memo")} />
                </div>
                <div className="input-group mb-3">
                  <span className="input-group-text">maximumanalog</span>
                  <input type="text" className="" id="maximumanalog3" value={deviceData?.data?.aout?.maximumanalog} onChange={() => console.log("memo")} />
                </div>

                <label>modbus </label>

                <div className="input-group mb-3">
                  <span className="input-group-text">tipo</span>
                  <input type="text" className="" id="tipo4" value={deviceData?.data?.modbus?.tipo} onChange={() => console.log("memo")} />
                </div>
                <div className="input-group mb-3">
                  <span className="input-group-text">Name</span>
                  <input type="text" className="" id="name4" value={deviceData?.data?.modbus?.name} onChange={() => console.log("memo")} />
                </div>
                <div className="input-group mb-3">
                  <span className="input-group-text">query</span>
                  <input type="text" className="" id="query4" value={deviceData?.data?.modbus?.query} onChange={() => console.log("memo")} />
                </div>
                <div className="input-group mb-3">
                  <span className="input-group-text">answer</span>
                  <input type="text" className="" id="answer4" value={deviceData?.data?.modbus?.answer} onChange={() => console.log("memo")} />
                </div>
                <div className="input-group mb-3">
                  <span className="input-group-text">startindex</span>
                  <input type="text" className="" id="startindex4" value={deviceData?.data?.modbus?.startindex} onChange={() => console.log("memo")} />
                </div>
                <div className="input-group mb-3">
                  <span className="input-group-text">datalength</span>
                  <input type="text" className="" id="datalength4" value={deviceData?.data?.modbus?.datalength} onChange={() => console.log("memo")} />
                </div>
                <div className="input-group mb-3">
                  <span className="input-group-text">dinimumalert</span>
                  <input type="text" className="" id="dinimumalert4" value={deviceData?.data?.modbus?.maximumanalog} onChange={() => console.log("memo")} />
                </div>
                <div className="input-group mb-3">
                  <span className="input-group-text">dinimumalert</span>
                  <input type="text" className="" id="maximumalert4" value={deviceData?.data?.modbus?.maximumanalog} onChange={() => console.log("memo")} />
                </div>
              </div>

              <br></br>

              <div>
                <button className="btn btn-primary" type="button" onClick={saveDevice}>
                  Save
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
      </div>


    </>
  )
}

const mapStateToProps = (state) => {
  return {
    devices: state.main.devices,
  }
}

const mapDispatchToProps = {
  setDevices,
}

export default connect(mapStateToProps, mapDispatchToProps)(DevicesEdit)
