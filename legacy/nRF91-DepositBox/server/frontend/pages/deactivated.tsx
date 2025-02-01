import React from "react";
import DeactivatedUserComponent from "../components/DeactivatedUser/DeactivatedUserComponent";
import styles from "../styles/Home.module.css";

function DeactivatedUserPage() {

  return (
    <main className={styles.main}>
     <DeactivatedUserComponent/>
     
    </main>
  );
}

export default DeactivatedUserPage;