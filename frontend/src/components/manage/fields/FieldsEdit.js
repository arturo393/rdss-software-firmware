import axios from "axios"
import { useState, useEffect } from "react"

const FieldsEdit = (props) => {
    const [status, setStatus] = useState()
    const [formData, setFormData] = useState()
    const [selectedGroup, setSelectedGroup] = useState()
    const [groups, setGroups] = useState([])
    const [fields, setFields] = useState([])

    const url = `${process.env.NEXT_PUBLIC_APIPROTO}://${process.env.NEXT_PUBLIC_APIHOST}:${process.env.NEXT_PUBLIC_APIPORT}`

    const getFieldsData  = async() => {
        const fields = await axios.get(`${url}/api/fields`)
        const fields_group = await axios.get(`${url}/api/fields_group`)
        setFields(fields.data)
        setGroups(fields_group.data)
      }

    useEffect(() => {
        getFieldsData()
        document.getElementById("status").style.display = "none"
      }, [])

    const handleChange = (e) => {
        setFormData({
            ...formData,
            [e.target.id]: e.target.value,
        })
        document.getElementById("status").style.display = "none"
      }
    
    const handleGroupSelected = (e) => {
        e.preventDefault()
        setSelectedGroup(e.target.value)
        document.getElementById("status").style.display = "none"
    }
    const addGroup = async (e) => {
        e.preventDefault()

        if (!formData?.new_group_name) {
            setStatus("Group name cannot be empty")
        } else {
            try {
                const res = await axios.post(`${url}/api/fields_group`, {name:formData.new_group_name})
                setStatus(res.data.message)
                getFieldsData()
                setSelectedGroup(res.data.group.insertedId)    
            } catch (e) {
                setStatus("ERROR")
            }
            
            document.getElementById("new_group_name").value=""
        }
        document.getElementById("status").style.display = "block"

        
    }

    const addField = async (e) => {
        e.preventDefault()

        try {
            const newField =  {
                name: formData.new_field_name,
                group_id: selectedGroup,
                required: true, type: "string"
            }
            const res = await axios.post(`${url}/api/fields`, newField)

            getFieldsData()
            setStatus(JSON.stringify(res.data))
        } catch (e) {
            setStatus("ERROR")
        }
        
        document.getElementById("new_field_name").value=""
        document.getElementById("status").style.display = "block"
    }

    const removeField = async (name) => {
        const field = fields.find(field => field.group_id == selectedGroup && field.name == name)
        try {
            const id  = field._id
            const res = await axios.delete(`${url}/api/fields?id=${id}`)
            getFieldsData()
            setStatus(res.data)
        } catch (e) {
            console.log(e)
            setStatus("ERROR")
        }
        document.getElementById("new_field_name").value=""
        document.getElementById("status").style.display = "block"
    }

    const removeGroup = async (e) => {
        e.preventDefault()

        const groupHasFields = fields.some(field => field.group_id === selectedGroup);
        if (!groupHasFields && selectedGroup) {
            const group = groups.find(group => group._id === selectedGroup)
            const id = group?._id

            const res = await axios.delete(`${url}/api/fields_group?id=${id}`)
            getFieldsData()
            setStatus(res.data)
        } else {
            setStatus("You cannot delete this group")
        }
        document.getElementById("status").style.display = "block"
        
    }

    const handleFieldDefaultValue = async (name) => {
        console.log("FORM DATA",formData,name,selectedGroup)

   
        const field = fields.find(field => field.group_id === selectedGroup  && field.name === name )
        const res = await axios.put(`${url}/api/fields?id=${field._id}&name=${field.name}`, {default_value: formData?.new_field_default_value || ""})
        getFieldsData()
        setStatus(res.data)
        
        document.getElementById("status").style.display = "block"
    }

    console.log("FIELDS", fields)
    return (
        <div className="container-fluid">
            <div className="text-center mt-2 mb-2">
                <h5>Groups and Fields Admin</h5>
            </div>
            <div class="row text-center">
                <div class="col-2"></div>
                <div class="col-7">
                    <div className="input-group mb-5">
                        {/* <span className="input-group-text">Fields Group</span> */}
                        <input type="text" required className="form-control" id="new_group_name" onChange={handleChange} />
                        <button className="btn btn-primary" type="button" onClick={addGroup}>
                            Add Group
                        </button>
                    </div>
                    <div className="input-group mb-3">
                        <span className="input-group-text text-light bg-dark">Group Name</span>
                        <select className="form-control" id="selectedGroup" onChange={handleGroupSelected}>
                        <option value={0}>=== Select a Group ===</option>
                        {groups.map((group) => {
                            return (
                            <option value={group._id} selected={group._id === selectedGroup}>
                                {group.name} 
                            </option>
                            )
                        })}
                        </select>
                        <button className="btn btn-primary" type="button" onClick={removeGroup}>
                                Del
                            </button>

                        
                    </div>
                    <div className="input-group mb-3">
                        <span className="input-group-text text-light bg-dark">Field Name</span>
                        <input type="text" className="form-control" id="new_field_name" onChange={handleChange} />
                            <button className="btn btn-primary" type="button" onClick={addField}>
                                Add
                            </button>
                    </div>
                    
                        {selectedGroup && fields && fields.filter((field) => field.group_id === selectedGroup).map(field => (
                            <>
                            <div className="input-group d-flex bd-highlight mb-3">
                                <span className="input-group-text text-wrap p-2 flex-grow-1 bd-highlight w-25">{field.name} (default: {field.default_value})</span>
                                <input type="text" className="form-control" id="new_field_default_value"  onChange={handleChange} />

                                <button className="btn btn-secondary p-2 bd-highlight" type="button" onClick={() => handleFieldDefaultValue(field.name)}>
                                    Set Default Value
                                </button> 
                                <button className="btn btn-warning p-2 bd-highlight" type="button" onClick={() => removeField(field.name)}>
                                    Remove Field
                                </button> 
                                </div>  
                            </>
                        ))}
                        
                    
                    
                
                    {/* <button className="btn btn-primary w-50 mt-10" type="button" onClick={saveFieldsAndGroup}>
                        Save Changes
                    </button>  */}
                    <div className="row">
                <div className="col-md-12 text-center">
                    <div className="alert alert-info" role="alert" id="status">
                        {status}
                    </div>
                </div>
            </div>
                </div>
                <div class="col-2"></div>
            </div>
            
            
        </div>
     
    )
}
  export default FieldsEdit
  