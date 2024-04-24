function NumberList(props) {
    const users = props.users;
    const listUsers = users?.map((user) =>
      <li>{user.name}</li>
    );
    return (
      <ul>{listUsers}</ul>
    );
}

export default NumberList