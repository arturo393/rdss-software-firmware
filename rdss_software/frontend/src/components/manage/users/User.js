import React, { useState } from "react"
import { Button, Card } from "react-bootstrap"
import NextLink from "next/link"
import { alert, confirm } from "react-bootstrap-confirmation"
import axios from "axios"

const User = ({ user }) => {
  const [state, setState] = useState({
    userState: user,
  })
  // console.log(state.userState)
  const handleDelete = async () => {
    const result = await confirm("Are you sure you want to delete?")
    if (result) {
      const idUser = { id: state.userState._id }
      delete state.userState
      setState(state)

      // console.log(state.userState)
      axios.post(process.env.NEXT_PUBLIC_APIPROTO + "://" + process.env.NEXT_PUBLIC_APIHOST + ":" + process.env.NEXT_PUBLIC_APIPORT + "/api/manage/deleteuser", idUser).then(
        (result) => {
          alert("User has been deleted")
        },
        (error) => {
          console.log(error)
        }
      )
    }
  }

  return (
    <tr>
      <td>
        {user?.name} ({user?.email})
      </td>
      <td>
        <button className="btn btn-primary btn-sm" type="button">
          Edit
        </button>
        <button className="btn btn-warning btn-sm" type="button" onClick={handleDelete}>
          Delete
        </button>
      </td>
    </tr>
  )
}

export default User
