import React, { useEffect, useState } from "react"
import { connect } from "react-redux"
import { setConfig } from "../../../redux/actions/main"
import axios from "axios"

const NetworkForm = (props) => {
  const { config, setConfig } = props

  const [newConfig, setNewConfig] = useState({})
  const [status, setStatus] = useState()
  const [file, setFile] = useState()
  const [base64, setBase64] = useState()
  const [saving, setSaving] = useState(false)

  useEffect(() => {
    document.getElementById("status").style.display = "none"
    if (config) setNewConfig(config)
  }, [])

  useEffect(() => {
    if (config) {
      setNewConfig(config)
      if (saving) {
        axios
          .post(
            "http://" +
              process.env.NEXT_PUBLIC_APIHOST +
              ":" +
              process.env.NEXT_PUBLIC_APIPORT +
              "/api/manage/editConfig",
            config
          )
          .then(
            (result) => {
              document.getElementById("status").style.display = "block"
              result
                ? setStatus("Networks Parameters updated successfully")
                : setStatus("Error when try to save config")
            },
            (error) => {
              console.log(error)
            }
          )
        setSaving(false)
      }
    }
  }, [config])

  const handleChange = (e) => {
    document.getElementById("status").style.display = "none"
    setNewConfig({
      ...newConfig,
      [e.target.id]: e.target.value,
    })
  }
  useEffect(() => {
    if (base64) setNewConfig({ ...newConfig, image: base64 })
  }, [base64])

  const saveConfig = (e) => {
    e.preventDefault()
    setConfig(newConfig)
    setSaving(true)
  }

  const restoreConfig = () => {
    document.getElementById("status").style.display = "none"
    setNewConfig(config)
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

  return (
    <>
      <form onSubmit={saveConfig}>
        <div className="container-fluid">
          <div className="row">
            <div className="col-md-12">
              <div className="row">
                <div className="col-md-12 mb-3">
                  <label for="image" className="form-label">
                    Map image
                  </label>
                  <input
                    className="form-control"
                    type="file"
                    id="image"
                    name="image"
                    onChange={handleImageChange}
                  />
                </div>
              </div>
              <div className="row">
                <div className="col-md-6">
                  <div className="form-floating input-group mb-3">
                    <input
                      type="number"
                      value={newConfig.minVoltage}
                      className="form-control"
                      id="minVoltage"
                      placeholder={newConfig.minVoltage}
                      onChange={handleChange}
                    />
                    <label for="minVoltage">Min Voltage</label>
                    <span className="input-group-text">[Volts]</span>
                  </div>
                </div>
                <div className="col-md-6">
                  <div className="form-floating input-group mb-3">
                    <input
                      type="number"
                      value={newConfig.maxVoltage}
                      className="form-control"
                      id="maxVoltage"
                      placeholder={newConfig.maxVoltage}
                      onChange={handleChange}
                    />
                    <label for="maxVoltage">Max Voltage</label>
                    <span className="input-group-text">[Volts]</span>
                  </div>
                </div>
              </div>
              <div className="row">
                <div className="col-md-6">
                  <div className="form-floating input-group mb-3">
                    <input
                      type="number"
                      value={newConfig.minCurrent}
                      className="form-control"
                      id="minCurrent"
                      placeholder={newConfig.minCurrent}
                      onChange={handleChange}
                    />
                    <label for="minCurrent">Min Current</label>
                    <span className="input-group-text">[A]</span>
                  </div>
                </div>
                <div className="col-md-6">
                  <div className="form-floating input-group mb-3">
                    <input
                      type="number"
                      value={newConfig.maxCurrent}
                      className="form-control"
                      id="maxCurrent"
                      placeholder={newConfig.maxCurrent}
                      onChange={handleChange}
                    />
                    <label for="maxCurrent">Max Current</label>
                    <span className="input-group-text">[A]</span>
                  </div>
                </div>
              </div>
              <div className="row">
                <div className="col-md-6">
                  <div className="form-floating input-group mb-3">
                    <input
                      type="number"
                      value={newConfig.minUplink}
                      className="form-control"
                      id="minUplink"
                      placeholder={newConfig.minUplink}
                      onChange={handleChange}
                    />
                    <label for="minUplink">Min Uplink</label>
                    <span className="input-group-text">[dB]</span>
                  </div>
                </div>
                <div className="col-md-6">
                  <div className="form-floating input-group mb-3">
                    <input
                      type="number"
                      value={newConfig.maxUplink}
                      className="form-control"
                      id="maxUplink"
                      placeholder={newConfig.maxUplink}
                      onChange={handleChange}
                    />
                    <label for="maxUplink">Max Uplink AGC</label>
                    <span className="input-group-text">[dB]</span>
                  </div>
                </div>
              </div>
              <div className="row">
                <div className="col-md-6">
                  <div className="form-floating input-group mb-3">
                    <input
                      type="number"
                      value={newConfig.minDownlink}
                      className="form-control"
                      id="minDownlink"
                      placeholder={newConfig.minDownlink}
                      onChange={handleChange}
                    />
                    <label for="minDownlink">Min Downlink</label>
                    <span className="input-group-text">[dB]</span>
                  </div>
                </div>
                <div className="col-md-6">
                  <div className="form-floating input-group mb-3">
                    <input
                      type="number"
                      value={newConfig.maxDownlink}
                      className="form-control"
                      id="maxDownlink"
                      placeholder={newConfig.maxDownlink}
                      onChange={handleChange}
                    />
                    <label for="maxDownlink">Max Downlink</label>
                    <span className="input-group-text">[dB]</span>
                  </div>
                </div>
              </div>
              <div className="row">
                <div className="col-md-6">
                  <div className="form-floating input-group mb-3">
                    <input
                      type="number"
                      value={newConfig.minDownlinkOut}
                      className="form-control"
                      id="minDownlinkOut"
                      placeholder={newConfig.minDownlinkOut}
                      onChange={handleChange}
                    />
                    <label for="minDownlinkOut">Min DL Output Power</label>
                    <span className="input-group-text">[dBm]</span>
                  </div>
                </div>
                <div className="col-md-6">
                  <div className="form-floating input-group mb-3">
                    <input
                      type="number"
                      value={newConfig.maxDownlinkOut}
                      className="form-control"
                      id="maxDownlinkOut"
                      placeholder={newConfig.maxDownlinkOut}
                      onChange={handleChange}
                    />
                    <label for="maxDownlinkOut">Max DL Output Power</label>
                    <span className="input-group-text">[dBm]</span>
                  </div>
                </div>
              </div>
              <div className="row">
                <div className="col-md-12 text-center">
                  <div class="alert alert-success" role="alert" id="status">
                    {status}
                  </div>
                </div>
              </div>
              <div className="row">
                <div class="container">
                  <div class="row">
                    <div class="col text-center">
                      <button
                        className="btn btn-secondary"
                        type="button"
                        onClick={restoreConfig}
                      >
                        Cancel
                      </button>
                    </div>
                    <div class="col text-center">
                      <button className="btn btn-primary" type="submit">
                        Update
                      </button>
                    </div>
                  </div>
                </div>
              </div>
            </div>
          </div>
        </div>
      </form>
    </>
  )
}

const mapStateToProps = (state) => {
  return {
    config: state.main.config,
  }
}

const mapDispatchToProps = {
  setConfig,
}

export default connect(mapStateToProps, mapDispatchToProps)(NetworkForm)
