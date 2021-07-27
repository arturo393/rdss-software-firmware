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

  async function getUsers() {
    const usrs = await axios
      .get("http://localhost:3000/api/manage/users")
      .then((res) => {
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

    axios.post("http://localhost:3000/api/manage/addUser", user).then(
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
      axios
        .post("http://localhost:3000/api/manage/deleteuser", { id: selectedUserId })
        .then(
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

  const showForm = () => {
    document.getElementById("userForm").style.display = "block"
    document.getElementById("userForm").style.visibility = "visible"
  }
  const closeForm = () => {
    setEditStatus(false)
    document.getElementById("userForm").style.display = "none"
    document.getElementById("userForm").style.visibility = "hidden"
    document.getElementById("name").value = ""
    document.getElementById("email").value = ""
    document.getElementById("email").readOnly = false
    document.getElementById("password").value = ""
    document.getElementById("rol").value = "user"
  }

  return (
    <div className="container-fluid ">
      <div className="text-center mt-2 mb-2">
        <h5>Users Admin</h5>
      </div>
      <div class="card h-100">
        <div className="card-body">
          <div class="table-responsive">
            <table class="table table-striped">
              <tbody>
                {users?.map((user) => (
                  <tr>
                    <td>
                      {user?.name} ({user?.email})
                    </td>
                    <td>
                      <button
                        className="btn btn-primary btn-sm"
                        type="button"
                        onClick={handleEdit}
                        id={user._id}
                      >
                        Edit
                      </button>
                      <button
                        className="btn btn-danger btn-sm"
                        type="button"
                        onClick={handleDelete}
                        id={user._id}
                      >
                        Delete
                      </button>
                    </td>
                  </tr>
                ))}
              </tbody>
            </table>
          </div>
          <div class="row">
            <div class="col text-center">
              <button className="btn btn-primary" type="button" onClick={showForm}>
                Create New User
              </button>
            </div>
          </div>
        </div>
        <form onSubmit={handleSubmit} id="userForm">
          <div className="container-fluid">
            <div className="row">
              <div className="col-md-12">
                <div className="form-floating input-group mb-3">
                  <input
                    type="name"
                    className="form-control"
                    id="name"
                    placeholder={0}
                    required
                  />
                  <label for="name">name</label>
                </div>
              </div>
              <div className="col-md-12">
                <div className="form-floating input-group mb-3">
                  <input
                    type="email"
                    className="form-control"
                    id="email"
                    placeholder={0}
                    required
                  />
                  <label for="email">email</label>
                </div>
              </div>
              <div className="col-md-12">
                <div className="form-floating input-group mb-3">
                  <input
                    type="password"
                    className="form-control"
                    id="password"
                    placeholder={0}
                    required
                  />
                  <label for="passsword">password</label>
                </div>
              </div>

              <div className="col-md-12">
                <select class="form-select" aria-label="rol" id="rol">
                  <option value="user" selected>
                    Normal User
                  </option>
                  <option value="admin">Network Administrator</option>
                  <option value="sadmin">Super Administrator</option>
                </select>
              </div>
            </div>
            <div class="container">
              <div class="row">
                <div class="col text-center">
                  <button
                    className="btn btn-secondary"
                    type="button"
                    onClick={closeForm}
                  >
                    Cancel
                  </button>
                </div>
                <div class="col text-center">
                  <button className="btn btn-primary" type="submit">
                    Save
                  </button>
                </div>
              </div>
            </div>
          </div>
        </form>
      </div>
    </div>
  )
}

const mapStateToProps = (state) => {
  return {
    currentUser: state.user.user,
  }
}

export default connect(mapStateToProps)(Users)
