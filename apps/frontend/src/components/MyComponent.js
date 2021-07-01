import { connect } from "react-redux"
import styles from "../styles/Home.module.css"
function MyComponent(props) {
  const { name } = props
  return (
    <div>
      Este es otro componente: Heredando estado:
      <h1 className={styles.title}>{name}</h1>
    </div>
  )
}

const mapStateToProps = (state) => {
  return { name: state.main.name }
}

export default connect(mapStateToProps)(MyComponent)
