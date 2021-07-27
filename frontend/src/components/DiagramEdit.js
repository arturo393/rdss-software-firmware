import { useState, useEffect } from "react"
import { Modal } from "react-bootstrap"

import Diagram from "./Diagram"
const DiagramEdit = () => {
  const [show, setShow] = useState(true)

  const handleClose = () => setShow(false)
  const handleShow = () => setShow(true)

  useEffect(() => {}, [])

  const openModal = () => {
    const m = document.getElementById("myModal")
    console.log(m)
  }

  return (
    <>
      <div className="containers">
        <div className="text-center mt-2 mb-2">
          <button className="btn btn-primary" onClick={handleShow}>
            Open Diagram Editor
          </button>
        </div>
      </div>
      <Modal
        aria-labelledby="contained-modal-title-vcenter"
        centered
        show={show}
        onHide={handleClose}
        dialogClassName="sigma-modal"
      >
        <Modal.Header className="sigmaDarkBg">
          <Modal.Title>Diagram Edit</Modal.Title>
          <div>
            <button className="btn btn-secondary" onClick={handleClose}>
              Close
            </button>
            <button className="btn btn-primary" onClick={handleClose}>
              Save Changes
            </button>
          </div>
        </Modal.Header>
        <Modal.Body>
          <Diagram />
        </Modal.Body>
        <Modal.Footer></Modal.Footer>
      </Modal>
    </>
  )
}

export default DiagramEdit
