import React, { useEffect, useState } from "react"
import { connect } from "react-redux"

import { Container, Card, Button } from "react-bootstrap"
import axios from "axios"

import { setDevices } from "../../redux/actions/main"

const DevicesEdit = (props) => {
  const { devices, setDevices } = props

  const [device, setDevice] = useState({})
  const [deviceData, setDeviceData] = useState({})
  const [newDevices, setNewDevices] = useState([])
  const [status, setStatus] = useState()

  const [fields, setFields] = useState([])
  const [fields_group, setFields_group] = useState([])

  const [selectedFieldGroup, setSelectedFieldGroup] = useState(null);

  const [selectedGroup, setSelectedGroup] = useState()
  const  [groups, setGroups] =  useState([])

  const url = `${process.env.NEXT_PUBLIC_APIPROTO}://${process.env.NEXT_PUBLIC_APIHOST}:${process.env.NEXT_PUBLIC_APIPORT}`

  useEffect(() => {
    const getFieldsData  = async() => {
      const fields = await axios.get(`${url}/api/fields`)
      const fields_group = await axios.get(`${url}/api/fields_group`)
      const groups = await axios.get(`${url}/api/devices_group`)
      setFields(fields.data)
      setFields_group(fields_group.data)
      setGroups(groups.data)
    }
    getFieldsData()
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
    setSelectedFieldGroup(null);
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
        })
      // setDeviceData(devices.find((device) => device.id === Number(deviceId)))
    }
  }

  const saveDevice = (e) => {
    e.preventDefault()

    const structuredData = {
        _id:deviceData._id,
        id:deviceData.id,
        name:deviceData.name,
        group_id:deviceData.group_id,
        status: deviceData.status,
        type: deviceData.type,
        fields_values: deviceData.fields_values
      }
    
    axios.post(process.env.NEXT_PUBLIC_APIPROTO + "://" + process.env.NEXT_PUBLIC_APIHOST + ":" + process.env.NEXT_PUBLIC_APIPORT + "/api/device/save", structuredData).then(
      (result) => {
        document.getElementById("status").style.display = "block"
        result ? setStatus("Device attributes updated successfully") : setStatus("Error when try to save device data")
        const objIndex = newDevices.findIndex((d) => d.id == parseInt(deviceData.id))
        let dev = devices.find((d) => d.id == parseInt(deviceData.id))
        dev = { ...dev, group_id:deviceData?.group_id, ...deviceData }
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
    const { id, value } = e.target;
  
    // If the id is from a field input, update the fields_values in deviceData
    if (fields.some(field => field._id === id)) {
      setDeviceData((prevDeviceData) => {
        const updatedFieldsValues = [...prevDeviceData.fields_values];
        const index = updatedFieldsValues.findIndex(fv => fv.field_id === id);
  
        if (index !== -1) {
          // Update existing field value
          updatedFieldsValues[index].value = value;
        } else {
          // Add new field value
          updatedFieldsValues.push({ field_id: id, value });
        }
  
        return { ...prevDeviceData, fields_values: updatedFieldsValues };
      });
    } else {
      // If the id is from a non-field input, update directly
      setDeviceData({
        ...deviceData,
        [id]: value,
      });
    }
  };
  


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
                    {device.name} ({device.type}-{device.id})
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
                {/* <span className="input-group-text">Type</span> */}
                <input type="hidden" className="form-control" id="type" value={deviceData?.type} onChange={handleChange} />
              </div>
              {/* LIST OF GROUPS */}
            <div className="input-group mb-3">
              <span className="input-group-text text-light bg-dark w-25">Group Name</span>
              <select className="form-control" id="group_id" onChange={handleChange} value={selectedGroup || deviceData?.group_id}>
                <option value={""}>=== Select a Group ===</option>
                {groups.map((group) => (
                  <option key={group._id} value={group._id}>
                    {group.name}
                  </option>
                ))}
              </select>
            </div>
            {/* END LIST OF GROUPS */}

              
                {/* {fields_group && fields_group.map(group => (
                  <div className="input-group mb-5">
                  <span className="input-group-text w-100 bg-dark text-light">{group.name}</span>
                    {fields && fields.filter((field) => field.group_id === group._id).map(field => (
                      <div className="input-group mb-1">

                      <span className="input-group-text w-25 text-wrap">{field.name}</span>
                      <input
                        type="text"
                        className="form-control"
                        id={field.id} value={deviceData[field?.id] || field?.default_value}
                        onChange={handleChange}
                      />
                    </div>
                    ))}              
                  </div>
                ))} */}
                {fields_group && fields_group.map(group => (
  <div className="input-group mb-5" key={group._id}>
    <span className="input-group-text w-100 bg-dark text-light">{group.name}</span>
    {fields && fields.filter((field) => field.group_id === group._id).map(field => (
      
      <div className="input-group mb-1 d-flex bd-highlight" key={field.id}>
        <span className="input-group-text w-25 text-wrap flex-grow-1 bd-highlight">{field.name}</span>
        <div className="bd-highligh w-25">
          <span className="badge bg-primary mx-1">{field?.visible?"visible":null}</span>
          <span className="badge bg-success mx-1">{field?.editable?"editable":null}</span>
          <span className="badge bg-info mx-1">{field?.plottable?"plottable":null}</span>
        </div>
        
        <input
          type="text"
          className="form-control"
          id={field._id}
          value={deviceData.fields_values?.find(fv => fv.field_id === field._id)?.value || field.default_value}
          onChange={handleChange}
        />
      </div>
    ))}
  </div>
))}


              <button className="btn btn-primary w-100" type="button" onClick={saveDevice}>
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
