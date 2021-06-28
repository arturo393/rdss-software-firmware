import { connect } from "react-redux"
import styles from "../styles/Home.module.css"
function Title(props) {
  const { name } = props
  return (
    <div>
      <h1 className={styles.title}>{name}</h1>
      <h2>Este es otro componente</h2>
    </div>
  )
}

const mapStateToProps = (state) => {
  return { name: state.main.name }
}

export default connect(mapStateToProps)(Title)
