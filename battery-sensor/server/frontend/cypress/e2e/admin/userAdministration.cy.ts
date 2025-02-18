describe("Test user administration functionality", () => {
  it("Test user deactivation", () => {
    cy.adminLogin();
    // Open detailed view of user Test Testesen
    cy.contains("Brukerliste").click();
    cy.contains("Test Testesen")
      .parent()
      .within(() => {
        cy.get(".MuiButton-contained").click();
      });
    // Press deactivate user
    cy.contains("Deaktiver bruker").click();
    cy.contains("Statusen til brukeren ble endret!").should("exist");
  });

  it("Test user activation", () => {
    cy.adminLogin();
    // Open detailed view of user Test Testesen
    cy.contains("Brukerliste").click();
    cy.contains("Test Testesen")
      .parent()
      .within(() => {
        cy.get(".MuiButton-contained").click();
      });
    // Press activate user
    cy.contains("Aktiver bruker").click();
    cy.contains("Statusen til brukeren ble endret!").should("exist");
  });

  it("Test adding new user", () => {
    cy.adminLogin();
    cy.contains("Legg til ny bruker").click();

    // Check that incomplete form can't be submitted
    cy.contains("Legg til bruker").click();
    cy.get("input:invalid").should("exist");

    // Enter user details in new user form
    cy.get("#mui-5").type("Navn Navnesen");
    cy.contains("Legg til bruker").click();
    cy.get("input:invalid").should("exist");

    cy.get("#mui-6").type("47666538");
    cy.contains("Legg til bruker").click();
    cy.get("input:invalid").should("exist");

    cy.get("#mui-7").type("navn.navnesen@navnesen.com");
    cy.contains("Legg til bruker").click();
    cy.contains("Brukeren ble lagt til").should("exist");

    // Check user was added to user list
    cy.contains("Brukerliste").click();
    cy.contains("Navn Navnesen").should("exist");
  });

  it("Test editing user", () => {
    cy.adminLogin();
    // Open detailed view of user Navn Navnesen
    cy.contains("Brukerliste").click();
    cy.contains("Navn Navnesen")
      .parent()
      .within(() => {
        cy.get(".MuiButton-contained").click();
      });
    // Edit user
    cy.get(".MuiButton-outlinedInfo").click();
    cy.get("#mui-5").should("exist");
    cy.get("#mui-6").should("exist").clear().type("47669538");
    cy.get("#mui-7").should("exist");
    cy.get("#mui-5").should("exist");
    cy.get("#mui-component-select-Role").should("exist");

    // Submit form
    cy.contains("Lagre endring").click();
    cy.contains("Brukeren ble lagret").should("exist");

    // Check that new user info is correct
    cy.reload();
    cy.contains("Brukerliste").click();
    cy.contains("Navn Navnesen")
      .parent()
      .within(() => {
        cy.get(".MuiButton-contained").click();
      });
    cy.get("#mui-6").should("have.value", "47669538");
  });

  it("Test deleting user", () => {
    cy.adminLogin();
    cy.contains("Brukerliste").click();

    // Deletes Navn Navnesen
    cy.contains("Navn Navnesen")
      .parent()
      .within(() => {
        cy.get(".MuiSvgIcon-root").parent().click();
      });
    cy.contains("Ja, jeg er sikker").should("exist").click();

    // Checks that user is removed from user list
    cy.contains("Navn Navnesen").should("not.exist");
  });
});
export {};