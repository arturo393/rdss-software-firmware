import axios from "axios"
import React, { useState, useEffect } from "react"
import { connect } from "react-redux"
import { setDevices } from "../../../redux/actions/main"

const GroupsEdit = (props) => {
  const { devices, setDevices } = props
  const [status, setStatus] = useState()
  const [displayStatus, setDisplayStatus] = useState("none")
  const [formData, setFormData] = useState({})
  const [selectedGroup, setSelectedGroup] = useState()
  const [selectedDevice, setSelectedDevice] = useState()
  const [groups, setGroups] = useState([])

  const url = `${process.env.NEXT_PUBLIC_APIPROTO}://${process.env.NEXT_PUBLIC_APIHOST}:${process.env.NEXT_PUBLIC_APIPORT}`

  const fetchData = async () => {
    try {
      const groupsResponse = await axios.get(`${url}/api/devices_group`)
      setGroups(groupsResponse.data)
    } catch (error) {
      console.error("Error fetching data:", error)
    }
  }

  useEffect(() => {
    fetchData()
    setDisplayStatus("none")
  }, [])

  const handleGroupSelected = (e) => {
    e.preventDefault()
    setSelectedGroup(e.target.value)
    setDisplayStatus("none")
    fetchData()
  }

  const addGroup = async (e) => {
    e.preventDefault()

    if (!formData.new_group_name) {
      setStatus("Group name cannot be empty")
    } else {
      try {
        const res = await axios.post(`${url}/api/devices_group`, { name: formData.new_group_name })
        setStatus(res.data.message)
        fetchData()
        setSelectedGroup(res.data.group.insertedId)
      } catch (error) {
        console.error("Error adding group:", error)
        setStatus("ERROR")
      }

      setFormData((prevFormData) => ({ ...prevFormData, new_group_name: "" }))
    }
    setDisplayStatus("block")
  }

  const removeGroup = async (e) => {
    e.preventDefault()

    const groupHasDevices = devices.some((device) => device.group_id === selectedGroup)
    if (!groupHasDevices && selectedGroup) {
      try {
        const group = groups.find((group) => group._id === selectedGroup)
        const id = group?._id

        const res = await axios.delete(`${url}/api/devices_group?id=${id}`)
        fetchData()
        setStatus(res.data)
        setSelectedGroup()
      } catch (error) {
        console.error("Error removing group:", error)
        setStatus("ERROR")
      }
    } else {
      setStatus("You cannot delete this group, is not empty")
    }
    setDisplayStatus("block")
  }

  const handleDeviceSelected = (e) => {
    setSelectedDevice(e.target.value)
  }
  
  const addDeviceToGroup = async () => {

    if (!selectedGroup) {
      setStatus("You need to select a group first")
      setDisplayStatus("block")
      return false
    }

    try {
      const device = devices.find((device) => device._id == selectedDevice)
      const updatedDevice = { ...device, group_id: selectedGroup }

      const res = await axios.post(`${url}/api/device/save`, updatedDevice)

      const updatedDevices = devices.map((d) => (d._id === selectedDevice ? updatedDevice : d))
      setDevices(updatedDevices)
      setStatus("Device updated!")
    } catch (error) {
      console.error("Error removing group:", error)
      setStatus("ERROR", error)
    }
    setDisplayStatus("block")
  }

  const removeDeviceFromGroup = async (deviceId) => {
    try {
      const device = devices.find((device) => device._id === deviceId)
      const updatedDevice = { ...device, group_id: "" }
      const res = await axios.post(`${url}/api/device/save`, updatedDevice)
      const updatedDevices = devices.map((d) => (d._id === deviceId ? updatedDevice : d))
      setDevices(updatedDevices)
      setStatus("Device updated!")
    } catch (error) {
      console.error("Error removing device from group:", error)
      setStatus("ERROR", error)
    }
    setDisplayStatus("block")

  }

  const handleChange = (e) => {
    // setFormData({ [e.target.id]: e.target.value })
    setFormData((prevFormData) => ({ ...prevFormData, [e.target.id]: e.target.value }))
    setDisplayStatus("none")
  }

  return (
    <>
    <h5 className="text-center w-100 bg-danger text-light">Devices groups</h5>
    <div className="container-fluid" style={{ minHeight: "100vh" }}>
      <div className="text-center mt-2 mb-2">
        
      </div>
      <div className="row">
        <div className="col-2"></div>

        <div className="col-7">
          {/* ADD GROUP */}
          <div className="input-group mb-5">
            <input type="text" required className="form-control" id="new_group_name" onChange={(e) => handleChange(e)} placeholder="New Group Name" value={formData.new_group_name || ""} />
            <button className="btn btn-success" type="button" onClick={addGroup}>
              Add New Devices Group
            </button>
          </div>
          {/* END ADD GROUP */}

          {/* LIST OF GROUPS */}
          <div className="input-group mb-3">
            <span className="input-group-text text-light bg-dark w-25">Group Name</span>
            <select className="form-control" id="selectedGroup" onChange={handleGroupSelected}>
              <option value={""}>=== Select a Group ===</option>
              {groups.map((group) => (
                <option key={group._id} value={group._id} selected={group._id === selectedGroup}>
                  {group.name}
                </option>
              ))}
            </select>

            <button className="btn btn-primary" type="button" onClick={removeGroup} disabled={!selectedGroup}>
              Delete this group
            </button>
          </div>
          {/* END LIST OF GROUPS */}

          {/* LIST OF DEVICES */}
          
          <div className="input-group mb-3">
    <span className="input-group-text text-light bg-dark w-25">Devices</span>
    <select className="form-control" id="selectedDevice" onChange={handleDeviceSelected}>
      <option value={""}>=== Select a Device ===</option>
      {devices.map((device) => {
        const isValidGroup = groups.some(group => group._id === device.group_id);
        return (
          <option key={device._id} value={device._id} selected={device._id === selectedDevice}>
            {device.name} ({device.type}-{device.id})
            {isValidGroup && ` - In Group: ${groups.find(group => group._id === device.group_id)?.name}`}
          </option>
        );
      })}
    </select>
    <button className="btn btn-success" type="button" onClick={addDeviceToGroup}>
      Add device to group
    </button>
  </div>

          {/* END LIST OF DEVICES */}

          {selectedGroup &&
            devices &&
            devices
              .filter((device) => device.group_id === selectedGroup)
              .map((device) => (
                <React.Fragment key={device._id}>
                  <div className="input-group mb-2">
                    <span className="input-group-text w-75 text-wrap">
                      {device.name} ({device.type}-{device.id})
                    </span>
                    <button className="btn btn-warning w-25" id={`del-${device._id}`} onClick={() => removeDeviceFromGroup(device._id)}>
                      Remove from group
                    </button>
                  </div>
                </React.Fragment>
              ))}
        </div>

        <div className="col-2"></div>
      </div>

      <div className="row fixed-bottom mb-2">
        <div className="col-md-12 text-center">
          <div className="alert alert-warning alert-dismissible fade show" role="alert" style={{ display: displayStatus }}>
            {status}
            <button type="button" className="btn-close" aria-label="Close" onClick={() => setDisplayStatus("none")}></button>
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

export default connect(mapStateToProps, mapDispatchToProps)(GroupsEdit)
