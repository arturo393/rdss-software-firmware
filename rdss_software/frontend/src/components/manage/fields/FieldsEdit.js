import axios from "axios";
import React, { useState, useEffect } from "react";

const FieldsEdit = (props) => {
  const [status, setStatus] = useState("");
  const [displayStatus, setDisplayStatus] = useState("none");
  const [formData, setFormData] = useState({});
  const [selectedGroup, setSelectedGroup] = useState("");
  const [groups, setGroups] = useState([]);
  const [fields, setFields] = useState([]);

  const url = `${process.env.NEXT_PUBLIC_APIPROTO}://${process.env.NEXT_PUBLIC_APIHOST}:${process.env.NEXT_PUBLIC_APIPORT}`;

  const fetchData = async () => {
    try {
      const fieldsResponse = await axios.get(`${url}/api/fields`);
      const groupsResponse = await axios.get(`${url}/api/fields_group`);
      
      setFields(fieldsResponse.data);
      setGroups(groupsResponse.data);

      // Assuming each field has a unique identifier like _id
      const initialFormData = {};
      fieldsResponse.data.forEach((field) => {
        initialFormData[field._id] = {
          default_value: field.default_value,
          query: field.query,
          set: field.set,
          plottable: field.plottable,
        };
      });
      setFormData(initialFormData);
    } catch (error) {
      console.error("Error fetching data:", error);
    }
  };

  useEffect(() => {
    fetchData();
    setDisplayStatus("none");
  }, []);

  const handleCheckboxChange = (fieldId, checkboxType, checked) => {
    setFormData((prevFormData) => ({
      ...prevFormData,
      [fieldId]: {
        ...prevFormData[fieldId],
        [checkboxType]: checked,
      },
    }));
  };

  const handleFieldChange = (fieldId, key, value) => {
    // console.log("fieldId", fieldId, "key", key, "value", value)
    setFormData((prevFormData) => ({
      ...prevFormData,
      [fieldId]: {
        ...prevFormData[fieldId],
        [key]: value,
      },
    }));
  };

  const handleGroupSelected = (e) => {
    e.preventDefault();
    setSelectedGroup(e.target.value);
    setDisplayStatus("none");
    fetchData();
  };

  const addGroup = async (e) => {
    e.preventDefault();

    if (!formData.new_group_name) {
      setStatus("Group name cannot be empty");
    } else {
      try {
        const res = await axios.post(`${url}/api/fields_group`, { name: formData.new_group_name });
        setStatus(res.data.message);
        fetchData();
        setSelectedGroup(res.data.group.insertedId);
      } catch (error) {
        console.error("Error adding group:", error);
        setStatus("ERROR");
      }

      setFormData((prevFormData) => ({ ...prevFormData, new_group_name: "" }));
    }
    setDisplayStatus("block");
  };

  const addField = async (e) => {
    e.preventDefault();

    try {
      const newField = {
        name: formData.new_field_name,
        group_id: selectedGroup,
        required: true,
        type: "string",
      };
      const res = await axios.post(`${url}/api/fields`, newField);

      fetchData();
      setStatus(JSON.stringify(res.data));
    } catch (error) {
      console.error("Error adding field:", error);
      setStatus("ERROR");
    }

    setDisplayStatus("block");
    // setFormData((prevFormData) => ({new_field_name, ...prevFormData }));
    setFormData((prevFormData) => ({
        ...prevFormData,
        [selectedGroup]: {
          ...prevFormData[selectedGroup],
          new_field_name: "",
        },
      }));
    
  };

  const deleteField = async (id) => {
    try {
      const res = await axios.delete(`${url}/api/fields?id=${id}`);
      fetchData();
      setStatus(res.data);
    } catch (error) {
      console.error("Error deleting field:", error);
      setStatus("ERROR");
    }
    setDisplayStatus("block");
  };

  const removeGroup = async (e) => {
    e.preventDefault();

    const groupHasFields = fields.some((field) => field.group_id === selectedGroup);
    if (!groupHasFields && selectedGroup) {
      try {
        const group = groups.find((group) => group._id === selectedGroup);
        const id = group?._id;

        const res = await axios.delete(`${url}/api/fields_group?id=${id}`);
        fetchData();
        setStatus(res.data);
        setSelectedGroup()
      } catch (error) {
        console.error("Error removing group:", error);
        setStatus("ERROR");
      }
    } else {
      setStatus("You cannot delete this group, is not empty");
    }
    setDisplayStatus("block");

  };

  const handleChange = (e) => {   
    setFormData({[e.target.id]: e.target.value});
    setDisplayStatus("none");
  };

  const saveField = async (id) => {
    try {
      const field = fields.find((field) => field._id === id);
      // console.log("formData[id]?.default_value",formData[id]?.default_value)
      // console.log("field?.default_value",field?.default_value)
      // const newValues = {
      //   default_value: formData[id]?.default_value || field?.default_value || "",
      //   query: formData[id]?.query || field?.query || false,
      //   set: formData[id]?.set || field?.set || false,
      //   plottable: formData[id]?.plottable || field?.plottable || false,
      //   conv_min: formData[id]?.conv_min || field?.conv_min || "",
      //   conv_max: formData[id]?.conv_max || field?.conv_max || "",
      //   alert_min: formData[id]?.alert_min || field?.alert_min || "",
      //   alert_max: formData[id]?.alert_max || field?.alert_max || "",
      // };
      const newValues = {
        default_value: formData[id]?.default_value,
        query: formData[id]?.query,
        set: formData[id]?.set,
        plottable: formData[id]?.plottable,
        conv_min: formData[id]?.conv_min,
        conv_max: formData[id]?.conv_max,
        alert_min: formData[id]?.alert_min,
        alert_max: formData[id]?.alert_max,
      };
  
      const res = await axios.put(`${url}/api/fields?id=${id}&name=${field.name}`, newValues);
      fetchData();
      setStatus(res.data);
      setFormData((prevFormData) => ({ ...prevFormData, [id]: {} }));
    } catch (error) {
      console.error("Error saving field:", error);
      setStatus("ERROR");
    }
  
    setDisplayStatus("block");
  };


  return (
    <>
    <h5 className="text-center w-100 sigmaRed text-light">Devices fields definitions</h5>
    <div className="container-fluid" style={{ minHeight: "100vh", marginBottom: "100px"}}>
      <div className="text-center mt-2 mb-2">
        
      </div>
      <div className="row">
        <div className="col-2"></div>

        <div className="col-7">
          {/* ADD GROUP */}
          <div className="input-group mb-5">
            <input
              type="text"
              required
              className="form-control"
              id="new_group_name"
              onChange={(e) => handleChange(e)}
              placeholder="New Fields Group Name"
              value={formData.new_group_name || ""}
            />
            <button className="btn btn-success" type="button" onClick={addGroup}>
              Add New Fields Group
            </button>
          </div>
          {/* END ADD GROUP */}

          {/* LIST OF GROUPS */}
          <div className="input-group mb-3">
            <span className="input-group-text text-light bg-dark w-25">Fields Group Name</span>
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

          

          {/* ADD FIELD */}
          {selectedGroup && (
            <div className="input-group mb-3">
              <span className="input-group-text text-light bg-dark w-25">Field Name</span>
              <input
                type="text"
                className="form-control"
                id="new_field_name"
                onChange={(e) => handleChange(e)}
                value={formData.new_field_name || ""}
                required
              />
              <button className="btn btn-success" type="button" onClick={addField}>
                Add new field to this group
              </button>
            </div>
          )}
          {/* END ADD FIELD */}

          {selectedGroup &&
            fields &&
            fields
              .filter((field) => field.group_id === selectedGroup)
              .map((field) => (
              
                <React.Fragment key={field._id}>
                  <div className="card mb-3">
                    <div className="card-header">
                      <h5>{field.name}</h5>
                    </div>
                    <div className="card-body bg-light">
                      <div className="input-group">
                        <span className="input-group-text text-dark w-25 text-wrap">Default Value</span>
                        <input
                            type="text"
                            className="form-control"
                            data-field-id={field._id}
                            id={`default-value`}
                            onChange={(e) => handleFieldChange(field._id, "default_value", e.target.value)}
                            value={formData[field._id]?.default_value !== undefined ? formData[field._id].default_value : field.default_value}
                            />
                      </div>
                      <div className="input-group">
                        <span className="input-group-text text-dark w-25 text-wrap">Conv. Min.</span>
                        <input
                            type="text"
                            className="form-control"
                            data-field-id={field._id}
                            id={`conv-min`}
                            onChange={(e) => handleFieldChange(field._id, "conv_min", e.target.value)}
                            value={formData[field._id]?.conv_min !== undefined? formData[field._id].conv_min : field.conv_min}
                            />
                        <span className="input-group-text text-dark w-25 text-wrap">Conv. Max.</span>
                        <input
                            type="text"
                            className="form-control"
                            data-field-id={field._id}
                            id={`conv-max`}
                            onChange={(e) => handleFieldChange(field._id, "conv_max", e.target.value)}
                            value={formData[field._id]?.conv_max !== undefined ? formData[field._id].conv_max : field.conv_max}
                            />
                      </div>
                      <div className="input-group">
                        <span className="input-group-text text-dark w-25 text-wrap">Alert Min.</span>
                        <input
                            type="text"
                            className="form-control"
                            data-field-id={field._id}
                            id={`alert-min`}
                            onChange={(e) => handleFieldChange(field._id, "alert_min", e.target.value)}
                            value={formData[field._id]?.alert_min !== undefined ? formData[field._id].alert_min : field.alert_min}
                            />
                        <span className="input-group-text text-dark w-25 text-wrap">Alert Max.</span>
                        <input
                            type="text"
                            className="form-control"
                            data-field-id={field._id}
                            id={`alert-max`}
                            onChange={(e) => handleFieldChange(field._id, "alert_max", e.target.value)}
                            value={formData[field._id]?.alert_max !== undefined ? formData[field._id].alert_max : field.alert_max}
                            />
                      </div>
                      
                      <div className="input-group d-flex justify-content-between">
                        <span className="input-group-text text-dark w-25 text-wrap">Field Options</span>
                        <label htmlFor={`query`} className="checkbox-inline mt-2">
                            <input
                                className="m-2"
                                type="checkbox"
                                data-field-id={field._id}
                                id={`query`}
                                checked={formData[field._id]?.query !== undefined ? formData[field._id].query : field.query}
                                onChange={(e) => handleCheckboxChange(field._id, "query", e.target.checked)}
                            />
                            Query
                        </label>
                        <label htmlFor={`set`} className="checkbox-inline mt-2">
                            <input
                                className="m-2"
                                type="checkbox"
                                data-field-id={field._id}
                                id={`set`}
                                checked={formData[field._id]?.set !== undefined ? formData[field._id].set : field.set}
                                onChange={(e) => handleCheckboxChange(field._id, "set", e.target.checked)}
                            />
                            Set
                        </label>
                        <label htmlFor={`plottable`} className="checkbox-inline mt-2 mr-2">
                            <input
                                className="m-2"
                                type="checkbox"
                                data-field-id={field._id}
                                id={`plottable`}
                                checked={formData[field._id]?.plottable !== undefined ? formData[field._id].plottable : field.plottable}
                                onChange={(e) => handleCheckboxChange(field._id, "plottable", e.target.checked)}
                            />
                            Plottable
                        </label>
                       
                      </div>
                    </div>
                    <div className="card-footer d-flex justify-content-end">
                      <button
                        className="btn btn-success m-2"
                        id={`save-${field._id}`}
                        onClick={() => saveField(field._id)}
                        
                      >
                        Save
                      </button>
                      <button className="btn btn-warning m-2" id={`del-${field._id}`} onClick={() => deleteField(field._id)}>
                        Delete
                      </button>
                    </div>
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
            <button type="button" className="btn-close" aria-label="Close" onClick={()=> setDisplayStatus("none")}></button>

          </div>
        </div>
      </div>
    </div>
    </>
  );
}
export default FieldsEdit
