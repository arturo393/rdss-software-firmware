import React, { useEffect, useState } from "react"
import { Container, Card, Table } from "react-bootstrap"
import { setMonitorDataEvent } from "../redux/actions/main"
import axios from "axios"
import { connect } from "react-redux"
import green from "../images/green.svg"
import red from "../images/red.svg"
import gray from "../images/gray.svg"

let a_connected
let a_voltage
let a_current
let a_agcup
let a_agcdown
let a_ptx
let a_smartTune
let a_reverse
let a_attenuation

const Alerts = (props) => {
  const [alerts, setAlerts] = useState([])
  const [deviceData, setDeviceData] = useState({})
  const [devices, setDevices] = useState([])
  const [fields, setFields] = useState([])
  const [fields_groups, setFieldsGroups] = useState([])
  const [devices_groups, setDevicesGroups] = useState([])
  const [data, setData] = useState([])
  const [displayDeviceFieldGroup, setDisplayDeviceFieldGroup] = useState([])
  const [range_value, setRangeValue] = useState({});
  const api_url = process.env.NEXT_PUBLIC_APIPROTO + "://" + process.env.NEXT_PUBLIC_APIHOST + ":" + process.env.NEXT_PUBLIC_APIPORT

  const url = `${process.env.NEXT_PUBLIC_APIPROTO}://${process.env.NEXT_PUBLIC_APIHOST}:${process.env.NEXT_PUBLIC_APIPORT}`;

  const { monitorData } = props

  // useEffect(()=>{
  //   const loadDevices = async () => {
  //     const dbDevices = await axios.get(url + "/api/devices/devices").then((res) => {
  //       return res.data
  //     })
  //     setDevices(dbDevices)
  //   }
  //   loadDevices()

  // },[])

  // Leyendo los fields provisionados al cargar el componente
  useEffect(() => {
    const loadDevices = async () => {
      const res = await axios.get(url + "/api/devices/devices")
      setDevices(res.data)
    }
    const getFieldsData = async () => {
      const res = await axios.get(`${url}/api/fields`);
      setFields(res.data)
    }
    const getFieldsGroups = async () => {
      const res = await axios.get(`${url}/api/fields_group`);
      setFieldsGroups(res.data)
    }
    const getDevicesGroups = async () => {
      const res = await axios.get(`${url}/api/devices_group`);
      setDevicesGroups(res.data)
    }
    //nos interesan los field.readable o field.writable
    getFieldsData()
    getFieldsGroups()
    getDevicesGroups()
    loadDevices()
  }, [])


  useEffect(() => {
    setData(monitorData?.map(monitor => JSON.parse(monitor)))
  }, [monitorData])


  const handleChange = (e) => {
    e.preventDefault()
  }

  const saveDevice = async (e) => {
    e.preventDefault();

    // get device data
    const device = await axios.get(api_url + "/api/device/" + e.target.id.value);

    // console.log("device", device.data[0]);

    const fieldId = e.target.field.id;
    const fieldValue = e.target.field.value;

    // Check if fields_values exists, if not initialize it
    if (!device.data[0]?.fields_values) {
      device.data[0].fields_values = {};
    }

    const newDeviceData = {
      id: parseInt(e.target.id.value, 10),
      ...device.data[0],
      fields_values: {
        ...device.data[0].fields_values,
        [fieldId]: {
          ...(device.data[0]?.fields_values[fieldId] || {}), // Provide default value if undefined
          default_value: fieldValue,
        },
      },
    };

    const res = await axios.post(api_url + "/api/device/save", newDeviceData);
    let status = document.getElementById(e.target.id.value + "status");
    let message;
    if (res.data) {
      message = "Device updated successfully";
    } else {
      message = "Error saving device data";
    }
    // console.log("RES", res);
    status.style.display = "block";
    status.innerHTML = message;
  };

  const handleRangeOnChange = async (newValue, field_id, device_id) => {
    const value = { ["value"]: newValue,
  ["field_id"]:field_id,
["device_id"]:device_id}
    console.log("field_id", field_id);
    console.log("device_id", device_id);
    console.log("value",value);
    setRangeValue(value);
  }

  const handleRangeOnMouseUp = async (newValue, field_id, device_id) => {
    // Update fieldValue in state
    // setFieldValue(fieldId, newValue);
    console.log("newvalue", newValue);
    console.log("fieldId", field_id);


    // get device data
    const device = await axios.get(api_url + "/api/device/" + device_id);

    console.log("device", device.data[0]);
    const fieldId = field_id;
    const fieldValue = newValue;

    // Check if fields_values exists, if not initialize it
    if (!device.data[0]?.fields_values) {
      device.data[0].fields_values = {};
    }

    const newDeviceData = {
      id: parseInt(device_id, 10),
      ...device.data[0],
      fields_values: {
        ...device.data[0].fields_values,
        [fieldId]: {
          ...(device.data[0]?.fields_values[fieldId] || {}), // Provide default value if undefined
          default_value: fieldValue,
        },
      },
    };

    const res = await axios.post(api_url + "/api/device/save", newDeviceData);
    let status = document.getElementById(device_id + "status");
    let message;
    if (res.data) {
      message = "Device updated successfully";
    } else {
      message = "Error saving device data";
    }
    // console.log("RES", res);
    //status.style.display = "block";
  };

  const handleToggleChange = async (newValue, field_id, device_id) => {
    // Update fieldValue in state
    // setFieldValue(fieldId, newValue);
    console.log("newvalue", newValue);
    console.log("fieldId", field_id);
    // get device data
    const device = await axios.get(api_url + "/api/device/" + device_id);

    console.log("device", device.data[0]);
    const fieldId = field_id;
    const fieldValue = newValue;

    // Check if fields_values exists, if not initialize it
    if (!device.data[0]?.fields_values) {
      device.data[0].fields_values = {};
    }

    const newDeviceData = {
      id: parseInt(device_id, 10),
      ...device.data[0],
      fields_values: {
        ...device.data[0].fields_values,
        [fieldId]: {
          ...(device.data[0]?.fields_values[fieldId] || {}), // Provide default value if undefined
          default_value: fieldValue,
        },
      },
    };

    const res = await axios.post(api_url + "/api/device/save", newDeviceData);
    let status = document.getElementById(device_id + "status");
    let message;
    if (res.data) {
      message = "Device updated successfully";
    } else {
      message = "Error saving device data";
    }
    // console.log("RES", res);
    //status.style.display = "block";

  };
  const isNumber = (value) => !isNaN(parseFloat(value)); // Helper function to check for number

  return (
    <>
      <h5 className="text-center w-100 sigmaRed text-light">Devices Status</h5>
      <div className="container table-responsive text-center col-10">
        {/* DEVICE GROUP LOOP */}
        {devices_groups && devices_groups.map(device_group => (
          <div className="input-group mb-5 rounded shadow-sm">
            <span className="input-group-text text-light bg-dark w-100 rounded">{device_group?.name}</span>
            {/* DEVICE ID/NAME LOOP */}
            {data && data.filter(d => (devices.some(deviceItem => deviceItem.id === d.id && deviceItem.group_id === device_group._id))).map(device => (
              <>
                <div className="d-flex w-100 bg-light border-0">
                  {devices.find(d => d.id === device.id)?.image ? (
                    <img src={devices.find(d => d.id === device.id)?.image} width={100} height={100} alt={device?.id} />
                  ) : (
                    <img src="/images/no_image.png" width={100} height={100} alt="no image" />
                  )}
                  <span width={100} height={100}></span>

                  <span className="input-group-text text-dark bg-light w-25 text-wrap">
                    <img alt="" className="m-2" src={device?.connected ? green.src : red.src} width={20} height={20} />
                    ({device?.id}) {device?.name} {!device?.connected && ""}
                  </span>
                  {/* QUERIES */}
                  <div className="input-group-text text-dark bg-light w-100 mx-0 d-flex justify-content-start flex-column">
                    {device?.field_values &&
                      Object.entries(device?.field_values).map(([fieldId, fieldValue]) => {
                        const field = fields.find((f) => f._id === fieldId);
                        const fieldGroup = fields_groups.find((fg) => fg._id === field?.group_id);
                        const thisDevice = devices.find(d => d.id === device.id);
                        const fieldDef = thisDevice?.fields_values && field?._id && thisDevice?.fields_values[field._id] || null;


                        if (fieldDef?.visible) {
                          if (field?.set) {
                            return (
                              <form onSubmit={saveDevice} key={fieldId} className="d-flex m-0 p-0  mx-0 w-100">
                                {/* <div key={fieldId} className="d-flex m-0 p-0  mx-1 w-100"> */}
                                <span className="input-group-text m-0 p-0"><img alt="" src={device.connected ? (!fieldValue.alert ? green.src : red.src) : gray.src} width={20} height={20} /></span>
                                {fieldValue?.name ?
                                  (<span className="input-group-text text-dark bg-light w-75">{fieldValue?.name}</span>)
                                  : (<span className="input-group-text text-dark bg-light w-75">{field?.name}</span>)}
  
                                {isNumber(fieldValue?.value) ? (
                                  <div className="input-group-text w-25 m-0 p-0 ">
                                    <span className="input-group-text text-dark bg-light w-15 m-0 p-1">{range_value["field_id"] == fieldId ? (range_value["device_id"] == device.id? (range_value["value"]):(parseFloat(fieldValue?.value))):(parseFloat(fieldValue?.value)) || parseFloat(fieldValue?.value)}</span>
                                    <div className="slider" style={{ display: "flex", width: "90%", justifyContent: "space-between" }}>
                                    <span className="range-min" style={{marginLeft: "5px"}}>{field?.conv_min || fieldValue?.conv_min || 0}</span>
                                      <input
                                        type="range"
                                        name="field"
                                        style={{ display: "flex", width: "100%", justifyContent: "space-between" }}
                                        id={field?._id}
                                        key={fieldId}
                                        min={field?.conv_min || fieldValue?.conv_min || 0}  // Set default min to 0 if not provided
                                        max={field?.conv_max || fieldValue?.conv_max || 4095} // Set default max to 100 if not provided
                                        value={range_value["field_id"] == fieldId ? (range_value["device_id"] == device.id? (range_value["value"]):(parseFloat(fieldValue?.value))):(parseFloat(fieldValue?.value)) || parseFloat(fieldValue?.value)}
                                        step="0.01"
                                        onChange={(event) => {
                                          const newValue = parseFloat(event.target.value);
                                          handleRangeOnChange(newValue, fieldId, device.id);
                                        }}
                                        onMouseUp={(event) => {
                                          const newValue = parseFloat(event.target.value);
                                          handleRangeOnMouseUp(newValue, fieldId, device.id);
                                        }}
                                      />
                                    </div>
                                    <span className="range-max">{field?.conv_max || fieldValue?.conv_max || 4095}</span>
                                  </div>
                                ) : (
                                  <div className="input-group-text w-25 m-0 p-0 ">
                                    <button
                                      type="button"
                                      className={`btn  ${fieldValue?.value === field?.conv_min ? "btn-off" : "btn-active"}`}
                                      // Other attributes remain the same (data-bs-toggle, autocomplete, onClick)
                                      onClick={() =>
                                        handleToggleChange(
                                          fieldValue?.value === field?.conv_min ? field?.conv_max : field?.conv_min,
                                          fieldId,
                                          device.id
                                        )
                                      }
                                    >
                                      {fieldValue?.value}
                                    </button>
                                  </div>
                                )
                                }
                              </form>
                            )
                          }
                          else if (field?.query) {
                            return (
                              // <div key={fieldId} >
                              //   <span className="text-dark"><img alt="" src={device.connected?(!fieldValue.alert?green.src:red.src):gray.src} width={20} height={20} /> {fieldGroup?.name}/{field?.name} <span className="badge bg-secondary">{fieldValue.value}</span> </span>
                              // </div>
                              <div key={fieldId} className="d-flex m-0 p-0  mx-1 w-100">
                                <span className="input-group-text m-0 p-0"><img alt="" src={device.connected ? (!fieldValue.alert ? green.src : red.src) : gray.src} width={20} height={20} /></span>
  
                                {/* {fieldValue?.name?(<span className="input-group-text text-dark bg-light w-75">{fieldValue?.name}</span>):(<span className="input-group-text text-dark bg-light w-75">{fieldGroup?.name}/{field?.name}</span>)} */}
                                {fieldValue?.name ? (<span className="input-group-text text-dark bg-light w-75">{fieldValue?.name}</span>) : (<span className="input-group-text text-dark bg-light w-75">{field?.name}</span>)}
  
                                <span className="input-group-text w-25">{fieldValue.value}</span>
                              </div>
                            );
                          }
                          else {
                            return null
                          }
                        }
                        



                        


                      })}
                  </div>

                </div>
              </>
            ))}
          </div>
        ))}

        {/* END DEVICE GROUP LOOP */}





      </div>
    </>
  )
}

const mapStateToProps = (state) => {
  return {
    monitorData: state.main.monitorData,
    // devices: state.main.devices,
  }
}

export default connect(mapStateToProps)(Alerts)
