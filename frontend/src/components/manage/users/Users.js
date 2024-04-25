import { useState } from "react"
import { useEffect } from "react"
import { connect } from "react-redux"
import axios from "axios"
import { alert, confirm } from "react-bootstrap-confirmation"

const Users = (props) => {
  const { currentUser } = props
  const [users, setUsers] = useState([])
  const [user, setUser] = useState({})
  const [editStatus, setEditStatus] = useState(false)

  const [password, setPassword] = useState("")
  const [passwordStrength, setPasswordStrength] = useState("Very Weak")

  async function getUsers() {
    const usrs = await axios.get(process.env.NEXT_PUBLIC_APIPROTO + "://" + process.env.NEXT_PUBLIC_APIHOST + ":" + process.env.NEXT_PUBLIC_APIPORT + "/api/manage/users").then((res) => {
      return res.data
    })

    return usrs
  }

  useEffect(() => {
    getUsers().then((usrs) => {
      setUsers(usrs)
    })
    document.getElementById("userForm").style.display = "none"
  }, [])

  useEffect(() => {}, [users])

  const handleSubmit = (e) => {
    e.preventDefault()
    let user = {
      name: e.target.name.value,
      email: e.target.email.value,
      password: e.target.password.value,
      rol: e.target.rol.value,
    }

    let obj = users.find((o) => o.email === e.target.email.value)

    if (obj && !editStatus) {
      alert("there is already another user with email:" + e.target.email.value)
      return false
    }

    axios.post(process.env.NEXT_PUBLIC_APIPROTO + "://" + process.env.NEXT_PUBLIC_APIHOST + ":" + process.env.NEXT_PUBLIC_APIPORT + "/api/manage/addUser", user).then(
      (result) => {
        alert("User has been Registered")
      },
      (error) => {
        console.log(error)
      }
    )

    getUsers().then((usrs) => {
      setUsers(usrs)
    })

    closeForm()
  }

  const handleDelete = async (e) => {
    const selectedUserId = e.target.id

    if (currentUser.data._id === selectedUserId) {
      alert("You can't delete yourself!")
      return false
    }

    const result = await confirm("Are you sure you want to delete?")
    let usersArray = users
    if (result) {
      usersArray = users.filter(function (el) {
        return el._id != selectedUserId
      })
      axios.post(process.env.NEXT_PUBLIC_APIPROTO + "://" + process.env.NEXT_PUBLIC_APIHOST + ":" + process.env.NEXT_PUBLIC_APIPORT + "/api/manage/deleteuser", { id: selectedUserId }).then(
        (result) => {
          alert("User has been deleted")
        },
        (error) => {
          console.log(error)
        }
      )
    }
    setUsers(usersArray)
  }

  const handleEdit = (e) => {
    e.preventDefault()
    setEditStatus(true)
    let obj = users.find((o) => o._id === e.target.id)
    document.getElementById("name").value = obj.name
    document.getElementById("email").value = obj.email
    document.getElementById("email").readOnly = true
    document.getElementById("password").required = false
    document.getElementById("rol").value = obj.rol
    showForm()
  }

  const handleAdd = () => {
    setEditStatus(false)
    clearForm()
    showForm()
  }

  const showForm = () => {
    document.getElementById("userForm").style.display = "block"
    document.getElementById("userForm").style.visibility = "visible"
  }
  const closeForm = () => {
    setEditStatus(false)
    clearForm()
    document.getElementById("userForm").style.display = "none"
    document.getElementById("userForm").style.visibility = "hidden"
  }

  const clearForm = () => {
    document.getElementById("name").value = ""
    document.getElementById("email").value = ""
    document.getElementById("email").readOnly = false
    document.getElementById("password").value = ""
    document.getElementById("rol").value = "user"
    setPasswordStrength("Very Weak")
    setPassword("")
  }

  useEffect(() => {
    let strongPassword = new RegExp("(?=.*[a-z])(?=.*[A-Z])(?=.*[0-9])(?=.*[^A-Za-z0-9])(?=.{8,})")
    let veryStrongPassword = new RegExp("((?=.*[a-z])(?=.*[A-Z])(?=.*[0-9])(?=.*[^A-Za-z0-9])(?=.{8,}))|((?=.*[a-z])(?=.*[A-Z])(?=.*[^A-Za-z0-9])(?=.{12,}))")

    if (password.length <= 4) {
      setPasswordStrength("Very Weak")
    } else if (password.length <= 6) {
      setPasswordStrength("Weak")
    } else if (password.length <= 8) {
      setPasswordStrength("Medium")
    } else if (strongPassword.test(password)) {
      setPasswordStrength("Strong")
    } else if (veryStrongPassword.test(password)) {
      setPasswordStrength("Very Strong")
    }
  }, [password])

  const strengthChecker = (event) => {
    const enteredValue = event.target.value.trim()
    setPassword(enteredValue)
  }

  return (
    <>
    <h5 className="text-center w-100 sigmaRed text-light">Users Admin</h5>
    <div className="container-fluid col-10">
      <div className="text-center mt-2 mb-2">
      
      </div>
      <div className="card h-100">
        <div className="card-body">
          <div className="table-responsive text-center">
            <table className="table table-striped w-50 mx-auto">
              <tbody>
                {users?.map((user) => (
                  <tr>
                    <td>
                      {user?.name} ({user?.email})
                    </td>
                    <td className="text-right">
                      <button className="btn btn-success btn-sm mx-2 border-0" type="button" onClick={handleEdit} id={user._id}>
                        Edit
                      </button>
                      <button className="btn btn-danger btn-sm mx-2 border-0" type="button" onClick={handleDelete} id={user._id}>
                        Delete
                      </button>
                    </td>
                  </tr>
                ))}
              </tbody>
            </table>
          </div>
          <div className="row">
            <div className="col text-center">
              <button className="btn btn-primary border-0" type="button" onClick={handleAdd}>
                Create New User
              </button>
            </div>
          </div>
        </div>
        <form onSubmit={handleSubmit} id="userForm">
          <div className="container-fluid w-50">
            <div className="row">
              <div className="col-md-12">
                <div className="form-floating input-group mb-3">
                  <input type="name" className="form-control" id="name" placeholder={0} required />
                  <label for="name">name</label>
                </div>
              </div>
              <div className="col-md-12">
                <div className="form-floating input-group mb-3">
                  <input type="email" className="form-control" id="email" placeholder={0} required />
                  <label for="email">email</label>
                </div>
              </div>
              <div className="col-md-12">
                <div className="form-floating input-group mb-3">
                  <input type="password" className="form-control" id="password" placeholder={0} required onChange={strengthChecker} />
                  <label for="password">password</label>
                  <div style={styles.statusBar}>
                    <div
                      id="strengthBar"
                      style={{
                        ...styles.strength,
                        width: `${(password.length / 16) * 100}%`,
                      }}
                    ></div>
                    <span style={styles.message}>{passwordStrength}</span>
                  </div>
                </div>

                {/* Password strength message */}
              </div>

              <div className="col-md-12">
                <select className="form-select" aria-label="rol" id="rol">
                  <option value="user" selected>
                    Normal User
                  </option>
                  <option value="admin">Network Administrator</option>
                  <option value="sadmin">Super Administrator</option>
                </select>
              </div>
            </div>
            <div className="container b-3">
              <div className="row">
                <div className="col text-center">
                  <button className="btn btn-secondary border-0 mb-5 mt-3" type="button" onClick={closeForm}>
                    Cancel
                  </button>
                </div>
                <div className="col text-center">
                  <button className="btn btn-primary border-0 mb-5 mt-3" type="submit">
                    Save
                  </button>
                </div>
              </div>
            </div>
          </div>
        </form>
      </div>
    </div>
    </>
  )
}

const mapStateToProps = (state) => {
  return {
    currentUser: state.user.user,
  }
}

const styles = {
  // container: {
  //   width: 400,
  //   padding: "30px 90px",
  //   margin: "50px auto",
  //   backgroundColor: "#f4ff81",
  //   borderRadius: "10px",
  //   display: "flex",
  //   flexDirection: "column",
  //   alignItems: "center",
  // },
  password: {
    width: 300,
    padding: "8px 10px",
    border: "1px solid #444",
    borderRadius: "10px",
    outline: "none",
  },
  statusBar: {
    width: 100,
    height: 10,
    marginTop: 20,
    marginLeft: 5,
    background: "#fff",
    border: "1px solid #444",
    borderRadius: "5px",
  },
  strength: {
    height: "100%",
    maxWidth: "100%",
    backgroundColor: "#0000ff",
  },
  message: {
    padding: "20px 0",
    fontSize: 15,
    marginTop: 5,
  },
  button: {
    padding: "15px 30px",
    cursor: "pointer",
    background: "purple",
    color: "#fff",
    fontWeight: "bold",
    border: "none",
    borderRadius: "30px",
  },
  disabledButton: {
    cursor: "not-allowed",
    opacity: 0.3,
  },
}

export default connect(mapStateToProps)(Users)
