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
  const [fields,setFields] = useState([])
  const [fields_groups,setFieldsGroups] = useState([])
  const [devices_groups,setDevicesGroups] = useState([])
  const [data,setData] = useState([])
  const [displayDeviceFieldGroup, setDisplayDeviceFieldGroup] = useState([])

  const api_url = process.env.NEXT_PUBLIC_APIPROTO + "://" + process.env.NEXT_PUBLIC_APIHOST + ":" + process.env.NEXT_PUBLIC_APIPORT

//   const fakeMonitorData = [
//     {
//         id: 1,
//         name: "prueba",
//         type: "sniffer",
//         connected: true,
//         group_id: "65c1362edc83920013291fc4",
//         rtData: 
//             {
//                 "65c135fddc83920013291fc2": {
//                     alert: true,
//                     value: 10
//                 },
//                 "65c13600dc83920013291fc3": {
//                     alert: false,
//                     value: 99
//                 },
//                 "65c13c52dc83920013291fc5": {
//                   alert: false,
//                   value: 35
//                 }
//             }
//     },
//     {
//         id: 2,
//         name: "prueba 2",
//         type: "sniffer",
//         connected: true,
//         group_id: "65c1362edc83920013291fc4",
//         rtData: 
//             {
//                 "65c135fddc83920013291fc2": {
//                     alert: false,
//                     value: 13
//                 },
//                 "65c13600dc83920013291fc3": {
//                     alert: true,
//                     value: 8
//                 }
//             }
//     }
// ]

  const url = `${process.env.NEXT_PUBLIC_APIPROTO}://${process.env.NEXT_PUBLIC_APIHOST}:${process.env.NEXT_PUBLIC_APIPORT}`;

  const { monitorData } = props

  // Leyendo los fields provisionados al cargar el componente
  useEffect(() => {
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
  },[])

  console.log("data",data)

  useEffect(() => {
    setData(monitorData?.map(monitor => JSON.parse(monitor)))
  },[monitorData])
  
  /**
   * MonitorData
   *  [{
   *    id,
   *    name,
   *    connected,
   *    rtData,
   *    alerts,
   *  }]
   */

  // useEffect(() => {
  //   let currentAlerts = []
  //   monitorData?.map((monitor) => {
  //     const data = JSON.parse(monitor)

  //     // console.log(data)
  //     if (data.connected) {
  //       a_connected = green.src
  //       a_voltage = data.alerts.voltage ? red.src : green.src
  //       a_current = data.alerts.current ? red.src : green.src
  //       a_agcup = data.alerts.gupl ? red.src : green.src
  //       a_agcdown = data.alerts.gdwl ? red.src : green.src
  //       a_ptx = data.alerts.power ? red.src : green.src
  //     } else {
  //       a_connected = red.src
  //       a_voltage = gray.src
  //       a_current = gray.src
  //       a_agcup = gray.src
  //       a_agcdown = gray.src
  //       a_ptx = gray.src
  //     }

  //     const device = {
  //       // valores generales
  //       id: data.id,
  //       name: data.name,
  //       type: data.type,
  //       connected: a_connected,
  //       group_id: data.group_id,
        
  //       // valores de los campos/atributos
  //       voltage: a_voltage,
  //       current: a_current,
  //       gupl: a_agcup,
  //       guwl: a_agcdown,
  //       power: a_ptx,
  //       smartTune: data.rtData.smartTune!=undefined?(data.rtData.smartTune == "ON"?green.src:gray.src):gray.src,
  //       reverse: data.rtData.reverse!=undefined?(data.rtData.reverse== "ON"?green.src:gray.src):gray.src,
  //       attenuation: data.rtData.attenuation
  //     }
  //     currentAlerts.push(device)
  //   })

  //   // alerts  contiene los  datos de cada device
  //   setAlerts(currentAlerts)

  // }, [monitorData])

  const handleChange = (e) => {
    e.preventDefault()
  }


  // const saveDevice = async (e) => {
  //   e.preventDefault()
  
  //   // get device data
  //   const device = await axios.get(api_url+"/api/device/" + e.target.id.value)

  //   console.log("device", device.data[0])

  //   const newDeviceData =  {
  //     id: parseInt(e.target.id.value,10),
  //     ...device.data[0],
  //     fields_values: {
  //       ...device.data[0].fields_values,
  //       [e.target.field.id]: {...device.data[0]?.fields_values[e.target.field.id], default_value: e.target.field.value}
  //     }
  //   }

  //   const res = await axios.post(api_url+"/api/device/save", newDeviceData)
  //   let status = document.getElementById(e.target.id.value+"status")
  //   let message
  //   if (res.data) {
  //     message = "Device updated successfully"
  //   } else {
  //     message = "Error saving device data"
  //   }
  //   console.log("RES", res)
  //   status.style.display = "block"
  //   status.innerHTML = message

  //   // let new_attenuation = e.target.attenuation.value || -1
  //   // if (new_attenuation < 0 || new_attenuation > 30) {
  //   //   let resultado = document.getElementById(e.target.id.value+"status")
  //   //   resultado.style.display = "block"
  //   //   resultado.innerHTML = "Value should be between 0 and 30"
  //   //   return false
  //   // }

  //   // axios.get(process.env.NEXT_PUBLIC_APIPROTO + "://" + process.env.NEXT_PUBLIC_APIHOST + ":" + process.env.NEXT_PUBLIC_APIPORT + "/api/device/" + e.target.id.value)
  //   //                   .then(
  //   //                     (result) => {
  //   //                       return result.data[0]
  //   //                     },
  //   //                     (error) => {
  //   //                       console.log(error)
  //   //                     }
  //   //                   )
  //   //                   .then((device) => {
  //   //                     device.attenuation = new_attenuation  * 1 // ensure integer
  //   //                     let status = document.getElementById(e.target.id.value+"status")
  //   //                     let message
  //   //                     axios.post(process.env.NEXT_PUBLIC_APIPROTO + "://" + process.env.NEXT_PUBLIC_APIHOST + ":" + process.env.NEXT_PUBLIC_APIPORT + "/api/device/save", device).then(
  //   //                       (result) => {
  //   //                         message = "Device updated successfully"
  //   //                       },
  //   //                       (error) => {
  //   //                         message = "Error saving device data"
  //   //                         console.log(error)
  //   //                       }
  //   //                     ).then(() => {
  //   //                       status.style.display = "block"
  //   //                       status.innerHTML = message
  //   //                     })
  //   //                   })

  // }

  const saveDevice = async (e) => {
    e.preventDefault();
  
    // get device data
    const device = await axios.get(api_url + "/api/device/" + e.target.id.value);
  
    console.log("device", device.data[0]);
  
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
    console.log("RES", res);
    status.style.display = "block";
    status.innerHTML = message;
  };



  return (
    <>
      <h5 className="text-center">Devices Status</h5>
      

      <div className="container table-responsive text-center ">
        {/* DEVICE GROUP LOOP */}
        {devices_groups && devices_groups.map(device_group => (
            <div className="input-group mb-5" key={device_group._id}>
              <span className="input-group-text text-light bg-dark w-100">{device_group?.name}</span>
              {/* DEVICE ID/NAME LOOP */}
              {data && data.map(device => (
                <div className="d-flex w-100">
                  <span className="input-group-text text-dark bg-light w-25"><img alt="" className="m-2" src={device?.connected?green.src:red.src} width={20} height={20} /> ({device?.id}) {device?.name} {!device?.connected && "(not connected)"} </span>
                  {/* QUERIES */}
                  <div className="input-group-text text-dark bg-light w-25 mx-0 d-flex justify-content-start flex-column">
                  {device?.field_values &&
                        Object.entries(device?.field_values).map(([fieldId, fieldValue]) => {
                          const field = fields.find((f) => f._id === fieldId);
                          const fieldGroup = fields_groups.find((fg) => fg._id === field?.group_id);
                          
                          if (field?.query) {
                            return (
                              // <div key={fieldId} >
                              //   <span className="text-dark"><img alt="" src={device.connected?(!fieldValue.alert?green.src:red.src):gray.src} width={20} height={20} /> {fieldGroup?.name}/{field?.name} <span className="badge bg-secondary">{fieldValue.value}</span> </span>
                              // </div>
                              <div key={fieldId} className="d-flex m-0 p-0  mx-1 w-100">
                                    <span className="input-group-text m-0 p-0"><img alt="" src={device.connected?(!fieldValue.alert?green.src:red.src):gray.src} width={20} height={20} /></span>
                                    
                                    {fieldValue?.name?(<span className="input-group-text text-dark bg-light w-50">{fieldValue?.name}</span>):(<span className="input-group-text text-dark bg-light w-50">{fieldGroup?.name}/{field?.name}</span>)}
                                    
                                    <span className="input-group-text">{fieldValue.value}</span>
                              </div>
                            );
                          } 
                          
                          
                          return null
                          
                          
                        })}
                  </div>
                  {/* SETTERS */}
                  <div className="w-25">
                  {device?.field_values &&
                        Object.entries(device?.field_values).map(([fieldId, fieldValue]) => {
                          const field = fields.find((f) => f._id === fieldId);
                          const fieldGroup = fields_groups.find((fg) => fg._id === field?.group_id);
                          
                          
                          
                          if (field?.set) {
                            return (
                              <div key={fieldId}>
                                <form onSubmit={saveDevice}>
                                  <div className="input-group col">
                                    {fieldValue?.name?(<span className="input-group-text w-50">{fieldValue?.name}</span>):(<span className="input-group-text w-50">{fieldGroup?.name}/{field?.name}</span>)}
                                    
                                    <input className="form-control" name="field" id={field?._id} onChange={handleChange} />
                                    <button className="btn btn-primary" type="submit">Save</button>
                                    <input type="hidden" name="id" id="id" value={device.id}/>
                                  </div>
                                  <div className="row">
                                    <div className="col-md-12 text-center">
                                      <div className="alert alert-info hidden" role="alert" id={device.id+"status"}>OK
                                      </div>
                                    </div>
                                  </div>
                                </form>
                              </div>
                            )
                          }

                          return null
                          
                          
                        })}
        
                  </div>
                </div>
              ))}
            </div>
          ))}
          
        {/* END DEVICE GROUP LOOP */}
          



        {/* CONTENIDO */}
        {/* <div className="row">
          <table className="table table-striped table-bordered table-light">
            <thead>
              <tr>
                <th>
                  <h6>Device</h6>
                </th>
                <th>
                  <h6>Connection</h6>
                </th>
              </tr>
            </thead>
            <tbody>
              {alerts && alerts?.map((data) => {
                return (
                  <tr>
                    <td>{data.name + "(" + data.type + "-" + data.id + ")"}</td>
                    <td><img alt="" src={data.connected} width={20} height={20} /></td>
                    {fields && fields.map(field => (
                      <td>asd
                        <img alt="" src={data[field?._id]} width={20} height={20} />
                      </td>
                    ))}
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
        </div> */}
        {/* FIN CONTENIDO */}
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
