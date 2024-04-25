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
  const { monitorData } = props
  useEffect(() => {
    let currentAlerts = []
    monitorData?.map((monitor) => {
      const data = JSON.parse(monitor)

      // console.log(data)
      if (data.connected) {
        a_connected = green.src
        a_voltage = data.alerts.voltage ? red.src : green.src
        a_current = data.alerts.current ? red.src : green.src
        a_agcup = data.alerts.gupl ? red.src : green.src
        a_agcdown = data.alerts.gdwl ? red.src : green.src
        a_ptx = data.alerts.power ? red.src : green.src
      } else {
        a_connected = red.src
        a_voltage = gray.src
        a_current = gray.src
        a_agcup = gray.src
        a_agcdown = gray.src
        a_ptx = gray.src
      }
  
      const device = {
        id: data.id,
        name: data.name,
        type: data.type,
        connected: a_connected,
        voltage: a_voltage,
        current: a_current,
        gupl: a_agcup,
        guwl: a_agcdown,
        power: a_ptx,
        smartTune: data.rtData.smartTune!=undefined?(data.rtData.smartTune == true?green.src:gray.src):gray.src,
        reverse: data.rtData.reverse!=undefined?(data.rtData.reverse== true?green.src:gray.src):gray.src,
        attenuation: data.rtData.attenuation
      }
      currentAlerts.push(device)
    })
    setAlerts(currentAlerts)
  }, [monitorData])

  const handleChange = (e) => {
    e.preventDefault()
  }
  const saveDevice = (e) => {
    e.preventDefault()
    let new_attenuation = e.target.attenuation.value || -1
    if (new_attenuation < 0 || new_attenuation > 30) {
      let resultado = document.getElementById(e.target.id.value+"status")
      resultado.style.display = "block"
      resultado.innerHTML = "Value should be between 0 and 30"
      return false
    }

    axios.get(process.env.NEXT_PUBLIC_APIPROTO + "://" + process.env.NEXT_PUBLIC_APIHOST + ":" + process.env.NEXT_PUBLIC_APIPORT + "/api/device/" + e.target.id.value)
                      .then(
                        (result) => {
                          return result.data[0]
                        },
                        (error) => {
                          console.log(error)
                        }
                      )
                      .then((device) => {
                        device.attenuation = new_attenuation  * 1 // ensure integer
                        let status = document.getElementById(e.target.id.value+"status")
                        let message
                        axios.post(process.env.NEXT_PUBLIC_APIPROTO + "://" + process.env.NEXT_PUBLIC_APIHOST + ":" + process.env.NEXT_PUBLIC_APIPORT + "/api/device/save", device).then(
                          (result) => {
                            message = "Device updated successfully"
                          },
                          (error) => {
                            message = "Error saving device data"
                            console.log(error)
                          }
                        ).then(() => {
                          status.style.display = "block"
                          status.innerHTML = message
                        })
                      })

  }
<<<<<<< HEAD

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
                                    <span className="input-group-text text-dark bg-light w-50 m-0 p-1">{range_value["field_id"] == fieldId ? (range_value["device_id"] == device.id? (range_value["value"]):(parseFloat(fieldValue?.value))):(parseFloat(fieldValue?.value)) || parseFloat(fieldValue?.value)}</span>
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
                                      className={`w-50 btn  ${fieldValue?.value === field?.conv_min ? "btn-off" : "btn-active"}`}
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
              
            ))}
          </div>
        ))}

        {/* END DEVICE GROUP LOOP */}





=======
  return (
    <>
      <h5 className="text-center">Devices Status</h5>
      <div className="container table-responsive text-center ">
        {/* CONTENIDO */}
        <div className="row">
          <table className="table table-striped table-bordered table-light">
            <thead>
              <tr>
                <th>
                  <h6>Device</h6>
                </th>
                <th>
                  <h6>Connection</h6>
                </th>
                <th>
                  <h6>Voltage</h6>
                </th>
                <th>
                  <h6>Current</h6>
                </th>
                <th>
                  <h6>AGC Uplink</h6>
                </th>
                <th>
                  <h6>AGC Downlink</h6>
                </th>
                <th>
                  <h6>Downlink Power</h6>
                </th>
                <th>
                  <h6>Smart Tune</h6>
                </th>
                <th>
                  <h6>Reverse</h6>
                </th>
                <th>
                  <h6>Attenuation [dB]</h6>
                </th>
                
              </tr>
            </thead>
            <tbody>
              {alerts?.map((data) => {
                return (
                  <tr>
                    <td>{data.name + "(" + data.type + "-" + data.id + ")"}</td>
                    <td>
                      <img alt="" src={data.connected} width={20} height={20} />
                    </td>
                    <td>
                      <img alt="" src={data.voltage} width={20} height={20} />
                    </td>
                    <td>
                      <img alt="" src={data.current} width={20} height={20} />
                    </td>
                    <td>
                      <img alt="" src={data.gupl} width={20} height={20} />
                    </td>
                    <td>
                      <img alt="" src={data.guwl} width={20} height={20} />
                    </td>
                    <td>
                      <img alt="" src={data.power} width={20} height={20} />
                    </td>
                    <td>
                    <img alt="" src={data.smartTune} width={20} height={20} />
                    </td>
                    <td>
                    <img alt="" src={data.reverse} width={20} height={20} />
                    </td>
                    <td width={200}>
                      <form onSubmit={saveDevice}>
                        <div className="input-group col">
                          <span className="input-group-text">{data?.attenuation}</span>
                          <input type="number" className="form-control" id={"attenuation"}  onChange={handleChange} />
                          <button className="btn btn-primary" type="submit">Save</button>
                          <input type="hidden" name="id" id="id" value={data.id}/>
                        </div>
                        <div className="row">
                          <div className="col-md-12 text-center">
                            <div className="alert alert-info hidden" role="alert" id={data.id+"status"}>OK
                            </div>
                          </div>
                        </div>
                      </form>
                    </td>
                  </tr>
                )
              })}
            </tbody>
          </table>
        </div>
        {/* FIN CONTENIDO */}
>>>>>>> development
      </div>
    </>
  )
}

const mapStateToProps = (state) => {
  return {
    monitorData: state.main.monitorData,
  }
}

export default connect(mapStateToProps)(Alerts)
