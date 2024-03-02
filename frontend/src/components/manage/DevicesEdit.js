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
  const [selectedDevice, setSelectedDevice] = useState()

  const [fields, setFields] = useState([])
  const [fields_group, setFields_group] = useState([])

  const [selectedFieldGroup, setSelectedFieldGroup] = useState(null)

  const [selectedGroup, setSelectedGroup] = useState()
  const [groups, setGroups] = useState([])

  const url = `${process.env.NEXT_PUBLIC_APIPROTO}://${process.env.NEXT_PUBLIC_APIHOST}:${process.env.NEXT_PUBLIC_APIPORT}`

  useEffect(() => {
    const getFieldsData = async () => {
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
    setSelectedFieldGroup(null)
    const deviceId = e.target.value

    if (deviceId) {
      setSelectedDevice(deviceId)
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
      _id: deviceData._id,
      id: deviceData.id,
      name: deviceData.name,
      group_id: deviceData.group_id,
      status: deviceData.status,
      type: deviceData.type,
      fields_values: deviceData?.fields_values,
    }

    console.log("structuredData", structuredData)

    axios.post(process.env.NEXT_PUBLIC_APIPROTO + "://" + process.env.NEXT_PUBLIC_APIHOST + ":" + process.env.NEXT_PUBLIC_APIPORT + "/api/device/save", structuredData).then(
      (result) => {
        document.getElementById("status").style.display = "block"
        result ? setStatus("Device attributes updated successfully") : setStatus("Error when try to save device data")
        const objIndex = newDevices.findIndex((d) => d.id == parseInt(deviceData.id))
        let dev = devices.find((d) => d.id == parseInt(deviceData.id))
        dev = { ...dev, group_id: deviceData?.group_id, ...deviceData }
        let newDevicesList = devices
        newDevicesList[objIndex] = dev
        setNewDevices(newDevicesList)
      },
      (error) => {
        console.log(error)
      }
    )
  }

  // const handleChange = (e) => {
  //   e.preventDefault()
  //   const { id, value } = e.target
  //   const [field_id, param] = id.split("|")

  //   setDeviceData((prevDeviceData) => {
  //     const updatedFieldsValues = prevDeviceData?.fields_values ? [...prevDeviceData.fields_values] : []

  //     // Check if the field already exists in fields_values
  //     const fieldIndex = updatedFieldsValues.findIndex((fv) => fv.field_id === field_id)

  //     if (fieldIndex !== -1) {
  //       // If the field exists, update the corresponding parameter
  //       updatedFieldsValues[fieldIndex][param] = value
  //     } else {
  //       // If the field doesn't exist, create a new entry
  //       updatedFieldsValues.push({
  //         field_id,
  //         default_value: "",
  //         conv_min: "",
  //         conv_max: "",
  //         alert_min: "",
  //         alert_max: "",
  //         [param]: value,
  //       })
  //     }

  //     return {
  //       ...prevDeviceData,
  //       fields_values: updatedFieldsValues,
  //     }
  //   })
  // }

  const handleChange = (e) => {
    e.preventDefault();
    const { id, value } = e.target;
    const [field_id, param] = id.split("|");
    
    if (param) {
      setDeviceData((prevDeviceData) => {
        const updatedFieldsValues = prevDeviceData?.fields_values ? { ...prevDeviceData.fields_values } : {};
    
        // Update the corresponding parameter for the specified field_id
        updatedFieldsValues[field_id] = {
          ...updatedFieldsValues[field_id],
          [param]: value,
        };
    
        return {
          ...prevDeviceData,
          fields_values: updatedFieldsValues,
        };
      });
    } else {
      setDeviceData({
        ...deviceData,
        [id]: value
      })
    }
    
  };

  console.log("DeviceData", deviceData)

  return (
    <>
      <h5 className="text-center">Device Editor</h5>
      <div class="container-fuid">
        <div class="row text-center">
          <div class="col-2"></div>
          <div class="col-8">
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

              {fields_group &&
                selectedDevice &&
                fields_group.map((group) => (
                  <div className="input-group mb-5" key={group._id}>
                    <span className="input-group-text w-100 bg-dark text-light">{group.name}</span>
                    {fields &&
                      fields
                        .filter((field) => field.group_id === group._id)
                        .map((field) => (
                          <div className="input-group mb-1 d-flex bd-highlight" key={field.id}>
                            <div className="d-flex bd-highlight w-100">
                            
                              <span className="input-group-text w-25 text-wrap flex-grow-1 bd-highlight">
                                <strong>{field.name}</strong>
                                
                                <div className="bd-highligh">
                                  <span className="badge bg-primary mx-1">{field?.query ? "query" : null}</span>
                                  <span className="badge bg-success mx-1">{field?.set ? "set" : null}</span>
                                  <span className="badge bg-info mx-1">{field?.plottable ? "plottable" : null}</span>
                                </div>
                              </span>

                              <div className="custom-control custom-switch">
                                      <input type="checkbox" className="custom-control-input" id={`${field._id}|visible`} key={`${field._id}|visible`} name="visible" value={deviceData?.fields_values?.[field._id]?.visible || field?.visible || false} onChange={handleChange} disabled={!selectedDevice}/>
                                      <label className="custom-control-label small" for="visible">visible</label>
                                    </div>

                              {/* FIELD NAME */}
                              <div className="form-floating">
                                <input
                                  type="text"
                                  className="form-control"
                                  name="field_name"
                                  id={`${field._id}|field_name`}
                                  key={`${field._id}|field_name`}
                                  value={deviceData?.fields_values?.[field._id]?.field_name || field?.field_name || ""}
                                  onChange={handleChange}
                                  placeholder="Field Name"
                                  disabled={!selectedDevice}
                                />
                                <label htmlFor="field_name" className="text-sm">
                                  Field name
                                </label>
                              </div>

                              {/* DEFAULT VALUE */}
                              <div class="form-floating">
                                <input
                                  type="text"
                                  className="form-control"
                                  name="default_value"
                                  id={`${field._id}|default_value`}
                                  key={`${field._id}|default_value`}
                                  // value={deviceData?.fields_values?.find((fv) => fv.field_id === field._id)?.default_value || field?.default_value || ""}
                                  value={deviceData?.fields_values?.[field._id]?.default_value || field?.default_value || ""}
                                  onChange={handleChange}
                                  placeholder="Default value"
                                  disabled={!selectedDevice}
                                />
                                <label htmlFor="default_value" className="text-sm">
                                  Default value
                                </label>
                              </div>

                              {/* CONV MIN */}
                              <div class="form-floating">
                                <input
                                  type="text"
                                  className="form-control"
                                  name="conv_min"
                                  id={`${field._id}|conv_min`}
                                  key={`${field._id}|conv_min`}
                                  // value={deviceData?.fields_values?.find((fv) => fv.field_id === field._id)?.conv_min || field?.conv_min || ""}
                                  value={deviceData?.fields_values?.[field._id]?.conv_min || field?.conv_min || ""}
                                  onChange={handleChange}
                                  placeholder="Conv. Min."
                                  disabled={!selectedDevice}
                                />
                                <label htmlFor="conv_min" className="text-sm">
                                  Conv. Min.
                                </label>
                              </div>

                              {/* CONV MAX */}
                              <div class="form-floating">
                                <input
                                  type="text"
                                  className="form-control"
                                  name="conv_max"
                                  id={`${field._id}|conv_max`}
                                  key={`${field._id}|conv_max`}
                                  // value={deviceData?.fields_values?.find((fv) => fv.field_id === field._id)?.conv_max || field?.conv_max || ""}
                                  value={deviceData?.fields_values?.[field._id]?.conv_max || field?.conv_max || ""}
                                  onChange={handleChange}
                                  placeholder="Conv. Max."
                                  disabled={!selectedDevice}
                                />
                                <label htmlFor="conv_max" className="text-sm">
                                  Conv. Max.
                                </label>
                              </div>

                              {/* Alert Min */}
                              <div class="form-floating">
                                <input
                                  type="text"
                                  className="form-control"
                                  name="alert_min"
                                  id={`${field._id}|alert_min`}
                                  key={`${field._id}|alert_min`}
                                  // value={deviceData?.fields_values?.find((fv) => fv.field_id === field._id)?.alert_min || field?.alert_min || ""}
                                  value={deviceData?.fields_values?.[field._id]?.alert_min || field?.alert_min || ""}
                                  onChange={handleChange}
                                  placeholder="Alert Min."
                                  disabled={!selectedDevice}
                                />
                                <label htmlFor="alert_min" className="text-sm">
                                  Alert Min.
                                </label>
                              </div>

                              {/* Alert Max */}
                              <div class="form-floating">
                                <input
                                  type="text"
                                  className="form-control"
                                  name="alert_max"
                                  id={`${field._id}|alert_max`}
                                  key={`${field._id}|alert_max`}
                                  // value={deviceData?.fields_values?.find((fv) => fv.field_id === field._id)?.alert_max || field?.alert_max || ""}
                                  value={deviceData?.fields_values?.[field._id]?.alert_max || field?.alert_max || ""}
                                  onChange={handleChange}
                                  placeholder="Alert Max."
                                  disabled={!selectedDevice}
                                />
                                <label htmlFor="alert_max" className="text-sm">
                                  Alert Max.
                                </label>
                              </div>

                            </div>
                          </div>
                        ))}
                  </div>
                ))}

              <button className="btn btn-primary w-100" type="button" onClick={saveDevice} disabled={!selectedDevice}>
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
