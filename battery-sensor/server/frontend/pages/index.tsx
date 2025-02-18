import React from "react";
import styles from "../styles/Home.module.css";
import LoginComponent from "../components/LogIn/LogInComponent";

function Home() {

  return (
    <main className={styles.main}>
      <LoginComponent />
    </main>
  );
}

export default Home;