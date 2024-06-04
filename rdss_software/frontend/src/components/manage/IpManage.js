import React, { useEffect, useState } from "react";
import { connect } from "react-redux";
import { Container, Card, Button, Alert, Spinner } from "react-bootstrap";
import axios from "axios";
import { setDevices } from "../../redux/actions/main";

const IpManage = (props) => {
  const { devices, setDevices } = props;

  const [deviceData, setDeviceData] = useState({});
  const [newDevices, setNewDevices] = useState([]);
  const [status, setStatus] = useState("");
  const [error, setError] = useState("");
  const [loading, setLoading] = useState(false);
  const [rebuilding, setRebuilding] = useState(false);
  const [restarting, setRestarting] = useState(false);

  useEffect(() => {
    document.getElementById("status").style.display = "none";
  }, []);

  useEffect(() => {
    if (devices.length > 0) setNewDevices(devices);
  }, [devices]);

  useEffect(() => {
    setDevices(newDevices);
  }, [newDevices]);

  const handleChange = (e) => {
    setDeviceData({
      ...deviceData,
      [e.target.id]: e.target.value,
    });
  };

  const validateIP = (ip) => {
    const ipPattern = /^(\d{1,3}\.){3}\d{1,3}$/;
    return ipPattern.test(ip);
  };

  const validateGateway = (gateway) => {
    const gatewayPattern = /^\d{1,3}\.\d{1,3}\.\d{1,3}\.\d{1,3}$/;
    return gatewayPattern.test(gateway);
  };

  const validateNetmask = (netmask) => {
    const netmaskPattern = /^(?:255\.){3}(?:0|128|192|224|240|248|252|254)$/;
    return netmaskPattern.test(netmask);
  };

  const saveDevice = async (e) => {
    e.preventDefault();
    setError("");
    setStatus("");
    setLoading(true);
    setRebuilding(true);

    const { ip, gateway, netmask } = deviceData;

    if (!validateIP(ip)) {
      setError("Please enter a valid IP address format (e.g., 192.168.60.103)");
      setLoading(false);
      setRebuilding(false);
      return;
    }

    if (!validateGateway(gateway)) {
      setError("Please enter a valid gateway format (e.g., 192.168.60.1)");
      setLoading(false);
      setRebuilding(false);
      return;
    }

    if (!validateNetmask(netmask)) {
      setError("Please enter a valid netmask format (e.g., 255.255.255.0)");
      setLoading(false);
      setRebuilding(false);
      return;
    }

    try {
      const response = await axios.post(`${process.env.NEXT_PUBLIC_APIPROTO}://${process.env.NEXT_PUBLIC_APIHOST}:${process.env.NEXT_PUBLIC_APIPORT}/api/manage/changeIp`, { ip, gateway, netmask });

      if (response.data.success) {
        setStatus(response.data.message);
        setLoading(false);
        setRebuilding(false);
        setRestarting(true);
        setTimeout(async () => {
          setRestarting(false);
          await axios.post('/api/manage/restartSystem');
        }, 5000);
      } else {
        setError(response.data.message);
        setLoading(false);
        setRebuilding(false);
      }
    } catch (error) {
      console.error(error);
      setError("An error occurred while changing IP, Gateway, and Netmask.");
      setLoading(false);
      setRebuilding(false);
    }
  };

  return (
    <>
      <h5 className="text-center">Change IP Address</h5>
      <div className="container-fluid">
        <div className="row text-center">
          <div className="col-2"></div>
          <div className="col-7">
            <div className="alert alert-warning" role="alert">
              <strong>Warning:</strong> Changing the IP address will cause the current IP to be inaccessible. The system will restart after completing the process.
            </div>
            <div className="input-group mb-3">
              <span className="input-group-text">IP Address</span>
              <input
                type="text"
                className="form-control"
                id="ip"
                value={deviceData?.ip || ""}
                onChange={handleChange}
                pattern="^(\d{1,3}\.){3}\d{1,3}$"
                title="Please enter a valid IP address format (e.g., 192.168.60.103)"
                required
              />
            </div>
            <div className="input-group mb-3">
              <span className="input-group-text">Gateway</span>
              <input
                type="text"
                className="form-control"
                id="gateway"
                value={deviceData?.gateway || ""}
                onChange={handleChange}
                pattern="^\d{1,3}\.\d{1,3}\.\d{1,3}\.\d{1,3}$"
                title="Please enter a valid gateway format (e.g., 192.168.60.1)"
                required
              />
            </div>
            <div className="input-group mb-3">
              <span className="input-group-text">Netmask</span>
              <input
                type="text"
                className="form-control"
                id="netmask"
                value={deviceData?.netmask || ""}
                onChange={handleChange}
                pattern="^(?:255\.){3}(?:0|128|192|224|240|248|252|254)$"
                title="Please enter a valid netmask format (e.g., 255.255.255.0)"
                required
              />
            </div>

            {error && <Alert variant="danger">{error}</Alert>}
            {status && <Alert variant="success">{status}</Alert>}

            {loading && rebuilding && (
              <div className="text-center">
                <Spinner animation="border" role="status">
                  <span className="visually-hidden">Loading...</span>
                </Spinner>
                <p>Rebuilding frontend...</p>
              </div>
            )}

            {!loading && !rebuilding && restarting && (
              <div className="text-center">
                <Alert variant="warning">The system will restart in 5 seconds...</Alert>
              </div>
            )}

            {!loading && !rebuilding && !restarting && !status && (
              <button className="btn btn-primary" type="button" onClick={saveDevice}>
                Save
              </button>
            )}
          </div>
          <div className="col-2"></div>
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
  );
};

const mapStateToProps = (state) => {
  return {
    devices: state.main.devices,
  };
};

const mapDispatchToProps = {
  setDevices,
};

export default connect(mapStateToProps, mapDispatchToProps)(IpManage);
